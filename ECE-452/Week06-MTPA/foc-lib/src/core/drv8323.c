/**
 * @file drv8323.c
 * @brief DRV8323 register logic, config pack/unpack, init sequence, fault decode.
 *        Transport-agnostic — calls injected transport callbacks only.
 *
 * Layer: B  No HAL.
 *
 * SPI frame format (16-bit):
 *   bit[15]:     R/W# (0=write, 1=read)
 *   bits[14:11]: register address (0–6)
 *   bits[10:0]:  data
 *
 * Register field positions per DRV8323 datasheet:
 *   DRIVER_CTRL [0x02]: bits[10:0]
 *     bit[10]:OCP_ACT, bit[9]:DIS_GDF, bit[8]:DIS_GDRV_FAULT,
 *     bits[7:6]:PWM_MODE, bit[5]:1x_PWM_DIR, bit[4]:1x_PWM_COM,
 *     bit[3]:CLR_FLT, bit[2]:BRAKE, bit[1]:COAST, bit[0]:DIR
 *   GATE_HS [0x03]: bits[10:0]
 *     bits[10:8]:LOCK, bits[7:4]:IDRIVEP_HS, bits[3:0]:IDRIVEN_HS
 *   GATE_LS [0x04]:
 *     bits[10:8]:TDRIVE, bits[7:4]:IDRIVEP_LS, bits[3:0]:IDRIVEN_LS
 *   OCP_CTRL [0x05]:
 *     bits[10:9]:TRETRY, bits[8:6]:DEAD_TIME (not used here),
 *     bits[5:4]:OCP_MODE, bits[3:2]:OCP_DEG, bits[1:0]:VDS_LVL (4 bits = [3:0])
 *     Note: VDS_LVL is 4 bits [3:0]; adjust as needed.
 *   CSA_CTRL [0x06]:
 *     bit[10]:VREF_DIV, bit[9]:LS_REF, bits[8:7]:CSA_GAIN, bit[6]:DIS_SEN,
 *     bits[5:4]:CSA_CAL_A, bits[3:2]:CSA_CAL_B, bits[1:0]:CSA_CAL_C / SEN_LVL
 */

#include "foc/drv8323.h"
#include "foc/motor_math.h"
#include <string.h>

/* =========================================================================
 * SPI frame helpers
 * ====================================================================== */
uint16_t drv8323_make_write_frame(drv8323_reg_id_t reg, uint16_t data)
{
    uint16_t addr = (uint16_t)((uint32_t)reg << 11u);
    return (uint16_t)(addr | (data & 0x07FFu));
}

uint16_t drv8323_make_read_frame(drv8323_reg_id_t reg)
{
    uint16_t addr = (uint16_t)((uint32_t)reg << 11u);
    return (uint16_t)(0x8000u | addr);
}

/* =========================================================================
 * Pack: cfg → register words
 * ====================================================================== */
void drv8323_pack(drv8323_t *drv)
{
    const drv8323_cfg_t *c = &drv->cfg;
    uint16_t r;

    /* DRIVER_CTRL [0x02] */
    r = 0u;
    r |= (uint16_t)((uint16_t)c->pwm_mode << 6);
    drv->regs.driver_ctrl = r;

    /* GATE_HS [0x03] */
    r = 0u;
    r |= (uint16_t)(0x6u << 8); /* LOCK=110b to unlock */
    r |= (uint16_t)((uint16_t)c->idrive_p_hs << 4);
    r |= (uint16_t)((uint16_t)c->idrive_n_hs);
    drv->regs.gate_hs = r;

    /* GATE_LS [0x04] */
    r = 0u;
    r |= (uint16_t)((uint16_t)c->tdrive << 8);
    r |= (uint16_t)((uint16_t)c->idrive_p_ls << 4);
    r |= (uint16_t)((uint16_t)c->idrive_n_ls);
    drv->regs.gate_ls = r;

    /* OCP_CTRL [0x05] */
    r = 0u;
    r |= (uint16_t)((uint16_t)c->ocp_mode << 4);
    r |= (uint16_t)((uint16_t)c->ocp_deg  << 2);
    r |= (uint16_t)((uint16_t)c->vds_lvl & 0x3u); /* lower 2 bits of VDS_LVL */
    drv->regs.ocp_ctrl = r;

    /* CSA_CTRL [0x06] */
    r = 0u;
    if (c->csa_vref_div2) r |= (uint16_t)(1u << 10);
    r |= (uint16_t)((uint16_t)c->csa_gain << 7);
    if (c->dis_sen) r |= (uint16_t)(1u << 6);
    r |= (uint16_t)(c->sense_ocp_lvl & 0x3u);
    drv->regs.csa_ctrl = r;
}

/* =========================================================================
 * Unpack: raw SPI read word → regs
 * ====================================================================== */
void drv8323_unpack(drv8323_t *drv, drv8323_reg_id_t reg, uint16_t word)
{
    word &= 0x07FFu; /* strip R/W and address bits */
    switch (reg) {
    case DRV_REG_FAULT_STATUS1: drv->regs.fault_status1 = word; break;
    case DRV_REG_VGS_STATUS2:   drv->regs.vgs_status2   = word; break;
    case DRV_REG_DRIVER_CTRL:   drv->regs.driver_ctrl   = word; break;
    case DRV_REG_GATE_HS:       drv->regs.gate_hs        = word; break;
    case DRV_REG_GATE_LS:       drv->regs.gate_ls        = word; break;
    case DRV_REG_OCP_CTRL:      drv->regs.ocp_ctrl       = word; break;
    case DRV_REG_CSA_CTRL:      drv->regs.csa_ctrl       = word; break;
    default: break;
    }
}

/* =========================================================================
 * Write one register (with read-back verify, up to 3 retries)
 * ====================================================================== */
static foc_status_t write_reg_verify(drv8323_t *drv, drv8323_reg_id_t reg,
                                     uint16_t expected)
{
    const drv8323_transport_t *t = drv->transport;
    for (int attempt = 0; attempt < 3; attempt++) {
        uint16_t rx = 0u;
        if (t->xfer16(drv8323_make_write_frame(reg, expected), &rx) != 0) {
            continue;
        }
        t->delay_us(10u);
        /* Read back */
        if (t->xfer16(drv8323_make_read_frame(reg), &rx) != 0) continue;
        uint16_t got = (uint16_t)(rx & 0x07FFu);
        /* Mask off read-only bits that may differ. */
        uint16_t mask = 0x07FFu;
        if (reg == DRV_REG_GATE_HS) mask = 0x07FFu & ~0x0700u; /* ignore LOCK readback */
        if ((got & mask) == (expected & mask)) return FOC_OK;
    }
    return FOC_EFAULT;
}

/* =========================================================================
 * Init sequence
 * ====================================================================== */
foc_status_t drv8323_init(drv8323_t *drv,
                           const drv8323_transport_t *transport,
                           const drv8323_cfg_t *cfg)
{
    if (!drv || !transport || !cfg) return FOC_EINVAL;
    memset(drv, 0, sizeof(*drv));
    drv->transport = transport;
    drv->cfg = *cfg;

    /* Enable pin high → wait ≥1 ms. */
    transport->enable_pin(true);
    transport->delay_us(1000u);

    /* Pack register values. */
    drv8323_pack(drv);

    /* Write and verify each writable register. */
    foc_status_t st;
    st = write_reg_verify(drv, DRV_REG_DRIVER_CTRL, drv->regs.driver_ctrl);
    if (st != FOC_OK) { drv->last_error = FOC_EFAULT; return st; }

    st = write_reg_verify(drv, DRV_REG_GATE_HS, drv->regs.gate_hs);
    if (st != FOC_OK) { drv->last_error = FOC_EFAULT; return st; }

    st = write_reg_verify(drv, DRV_REG_GATE_LS, drv->regs.gate_ls);
    if (st != FOC_OK) { drv->last_error = FOC_EFAULT; return st; }

    st = write_reg_verify(drv, DRV_REG_OCP_CTRL, drv->regs.ocp_ctrl);
    if (st != FOC_OK) { drv->last_error = FOC_EFAULT; return st; }

    st = write_reg_verify(drv, DRV_REG_CSA_CTRL, drv->regs.csa_ctrl);
    if (st != FOC_OK) { drv->last_error = FOC_EFAULT; return st; }

    /* Clear faults: write CLR_FLT bit in DRIVER_CTRL. */
    uint16_t clr_word = (uint16_t)(drv->regs.driver_ctrl | (1u << 3));
    uint16_t rx_dummy = 0u;
    (void)transport->xfer16(drv8323_make_write_frame(DRV_REG_DRIVER_CTRL, clr_word), &rx_dummy);
    transport->delay_us(100u);

    /* Verify nFAULT deasserted. */
    if (!transport->nfault_pin()) {
        drv->last_error = FOC_EFAULT;
        return FOC_EFAULT;
    }

    return FOC_OK;
}

/* =========================================================================
 * Read faults
 * ====================================================================== */
foc_status_t drv8323_read_faults(drv8323_t *drv, drv8323_faults_t *out)
{
    if (!drv || !out) return FOC_EINVAL;
    const drv8323_transport_t *t = drv->transport;
    uint16_t rx = 0u;

    if (t->xfer16(drv8323_make_read_frame(DRV_REG_FAULT_STATUS1), &rx) != 0)
        return FOC_EFAULT;
    drv->regs.fault_status1 = (uint16_t)(rx & 0x07FFu);

    if (t->xfer16(drv8323_make_read_frame(DRV_REG_VGS_STATUS2), &rx) != 0)
        return FOC_EFAULT;
    drv->regs.vgs_status2 = (uint16_t)(rx & 0x07FFu);

    uint16_t f1 = drv->regs.fault_status1;
    uint16_t f2 = drv->regs.vgs_status2;

    /* Decode FAULT_STATUS1 (per datasheet bit positions). */
    out->fault   = (bool)((f1 >> 10) & 1u);
    out->vds_ocp = (bool)((f1 >>  9) & 1u);
    out->gdf     = (bool)((f1 >>  8) & 1u);
    out->uvlo    = (bool)((f1 >>  7) & 1u);
    out->otsd    = (bool)((f1 >>  6) & 1u);
    out->vds_ha  = (bool)((f1 >>  5) & 1u);
    out->vds_la  = (bool)((f1 >>  4) & 1u);
    out->vds_hb  = (bool)((f1 >>  3) & 1u);
    out->vds_lb  = (bool)((f1 >>  2) & 1u);
    out->vds_hc  = (bool)((f1 >>  1) & 1u);
    out->vds_lc  = (bool)((f1 >>  0) & 1u);
    out->cpuv    = (bool)((f1 >> 10) & 1u); /* same as fault for summary */

    /* Decode VGS_STATUS2. */
    out->vgs_ha  = (bool)((f2 >>  9) & 1u);
    out->vgs_la  = (bool)((f2 >>  8) & 1u);
    out->vgs_hb  = (bool)((f2 >>  7) & 1u);
    out->vgs_lb  = (bool)((f2 >>  6) & 1u);
    out->vgs_hc  = (bool)((f2 >>  5) & 1u);
    out->vgs_lc  = (bool)((f2 >>  4) & 1u);

    return FOC_OK;
}

/* =========================================================================
 * CSA gain / cal
 * ====================================================================== */
foc_status_t drv8323_set_csa_gain(drv8323_t *drv, drv_csa_gain_t gain)
{
    if (!drv) return FOC_EINVAL;
    drv->cfg.csa_gain = gain;
    drv8323_pack(drv);
    return write_reg_verify(drv, DRV_REG_CSA_CTRL, drv->regs.csa_ctrl);
}

foc_status_t drv8323_cal_csa(drv8323_t *drv, bool cal_on)
{
    if (!drv) return FOC_EINVAL;
    uint16_t csa = drv->regs.csa_ctrl;
    /* CSA_CAL_A/B/C: bits [5:4], [3:2], [1:0] of CSA_CTRL? Datasheet: CSA_CAL bits. */
    /* Set bits [5], [4], [3] = all three CSA_CAL bits as a group. */
    if (cal_on) {
        csa |= (uint16_t)((1u << 5) | (1u << 3) | (1u << 2));
    } else {
        csa &= (uint16_t)~((1u << 5) | (1u << 3) | (1u << 2));
    }
    drv->regs.csa_ctrl = csa;
    const drv8323_transport_t *t = drv->transport;
    uint16_t rx = 0u;
    if (t->xfer16(drv8323_make_write_frame(DRV_REG_CSA_CTRL, csa), &rx) != 0)
        return FOC_EFAULT;
    return FOC_OK;
}

/* =========================================================================
 * amps_per_lsb
 * ====================================================================== */
q16_t drv8323_amps_per_lsb(const drv8323_cfg_t *cfg,
                             q16_t shunt_mohm,
                             q16_t vref_v,
                             uint16_t adc_fs)
{
    /* amps_per_lsb = vref / (adc_fs × csa_gain × shunt_ohm)
     * shunt_ohm = shunt_mohm / 1000 [Q16.16]
     * csa_gain_num: 5, 10, 20, or 40. */
    static const uint16_t s_gain_lut[4] = {5u, 10u, 20u, 40u};
    uint16_t gain_num = s_gain_lut[(uint8_t)cfg->csa_gain & 0x3u];

    /* shunt_ohm = shunt_mohm / 1000 (Q16.16) */
    q16_t shunt_ohm = q16_div(shunt_mohm, Q16(1000.0));

    /* denominator = adc_fs × csa_gain × shunt_ohm
     * adc_fs can be up to 65535 — too large to represent as Q16.16.
     * Decompose as: denom = (gain × shunt_ohm) × adc_fs.
     * gain × shunt_ohm stays within Q16 range (gain ≤ 40, shunt ≤ 1 Ω → product ≤ 40).
     * Then multiply by adc_fs using 64-bit to avoid overflow.
     */
    q16_t gain_q       = (q16_t)((int64_t)gain_num * Q16_ONE); /* Q16(gain) */
    q16_t gain_x_shunt = q16_mul(gain_q, shunt_ohm);           /* Q16.16 */
    /* denom = gain_x_shunt × adc_fs (promote adc_fs as raw integer scale) */
    int64_t denom64 = (int64_t)gain_x_shunt * (int64_t)adc_fs; /* Q16.16 × integer */
    /* denom64 is now in Q16.16 units; clamp to Q16 range before dividing. */
    q16_t denom;
    if (denom64 > (int64_t)Q16_MAX) denom = Q16_MAX;
    else if (denom64 <= 0)          denom = 1; /* guard against zero */
    else                            denom = (q16_t)denom64;

    return q16_div(vref_v, denom);
}

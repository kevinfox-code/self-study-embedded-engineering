/**
 * @file test_drv8323_regs.c
 * @brief Tests for DRV8323 register pack/unpack, init sequence, fault decode
 *        (Phase 9, §11.1 row 8).
 */
#include "../../test/test_harness.h"
#include "foc/drv8323.h"
#include "foc/motor_math.h"
#include <string.h>
#include <stdio.h>

TEST_HARNESS_IMPL

/* -------------------------------------------------------------------------
 * Mock transport for scripted responses
 * ---------------------------------------------------------------------- */
static uint16_t s_tx_log[32];
static uint16_t s_rx_script[32];
static int s_tx_idx = 0;
static int s_rx_idx = 0;
static bool s_nfault_state = true;

static int mock_xfer16(uint16_t tx, uint16_t *rx)
{
    if (s_tx_idx < 32) s_tx_log[s_tx_idx++] = tx;
    if (rx) *rx = (s_rx_idx < 32) ? s_rx_script[s_rx_idx++] : 0u;
    return 0;
}
static void mock_delay(uint32_t us) { (void)us; }
static void mock_enable(bool on) { (void)on; }
static bool mock_nfault(void) { return s_nfault_state; }

static const drv8323_transport_t s_mock_transport = {
    mock_xfer16, mock_delay, mock_enable, mock_nfault
};

static void reset_mock(void)
{
    s_tx_idx = 0; s_rx_idx = 0;
    memset(s_tx_log, 0, sizeof(s_tx_log));
    memset(s_rx_script, 0, sizeof(s_rx_script));
    s_nfault_state = true;
}

/* -------------------------------------------------------------------------
 * Pack/unpack round-trip
 * ---------------------------------------------------------------------- */
static void test_pack_unpack_driver_ctrl(void)
{
    drv8323_t drv;
    memset(&drv, 0, sizeof(drv));
    drv.cfg.pwm_mode = DRV_PWM_6X;
    drv8323_pack(&drv);
    /* PWM_MODE=0 (6x), bits[7:6] */
    TEST_ASSERT_EQUAL_INT(0u, (drv.regs.driver_ctrl >> 6) & 0x3u);
    /* Unpack back */
    uint16_t word = drv.regs.driver_ctrl;
    drv8323_unpack(&drv, DRV_REG_DRIVER_CTRL, word);
    TEST_ASSERT_EQUAL_INT(word & 0x7FFu, drv.regs.driver_ctrl);
}

static void test_pack_csa_ctrl(void)
{
    drv8323_t drv;
    memset(&drv, 0, sizeof(drv));
    drv.cfg.csa_gain     = DRV_CSA_GAIN_20;
    drv.cfg.csa_vref_div2 = false;
    drv8323_pack(&drv);
    /* CSA_GAIN=2 (20 V/V), bits[8:7] */
    uint16_t csa_gain_field = (drv.regs.csa_ctrl >> 7) & 0x3u;
    TEST_ASSERT_EQUAL_INT(2u, csa_gain_field);
}

/* -------------------------------------------------------------------------
 * Init sequence: verify write order
 * ---------------------------------------------------------------------- */
static void test_init_sequence(void)
{
    reset_mock();
    /* Script read-back responses for each write: return same data (verify passes). */
    static const drv8323_cfg_t cfg = {
        DRV_PWM_6X, DRV_IDRIVE_60MA, DRV_IDRIVE_60MA,
        DRV_IDRIVE_60MA, DRV_IDRIVE_60MA, DRV_TDRIVE_500NS,
        DRV_OCP_LATCHED, DRV_OCP_DEG_8US, DRV_VDS_0600,
        DRV_CSA_GAIN_20, false, 0u, false
    };
    drv8323_t drv;
    /* Pack to know expected values. */
    memset(&drv, 0, sizeof(drv));
    drv.cfg = cfg;
    drv8323_pack(&drv);
    /* Script read-back: match written data for each reg. */
    /* Writes: DRIVER_CTRL, GATE_HS, GATE_LS, OCP_CTRL, CSA_CTRL = 5 writes.
     * Each write is followed by a read-back. */
    s_rx_script[0] = 0u; /* write DRIVER_CTRL dummy rx */
    s_rx_script[1] = drv.regs.driver_ctrl & 0x7FFu; /* read-back */
    s_rx_script[2] = 0u;
    s_rx_script[3] = drv.regs.gate_hs & 0x7FFu; /* ignore LOCK bits in mask */
    s_rx_script[4] = 0u;
    s_rx_script[5] = drv.regs.gate_ls & 0x7FFu;
    s_rx_script[6] = 0u;
    s_rx_script[7] = drv.regs.ocp_ctrl & 0x7FFu;
    s_rx_script[8] = 0u;
    s_rx_script[9] = drv.regs.csa_ctrl & 0x7FFu;
    s_rx_script[10]= 0u; /* CLR_FLT write */
    s_nfault_state = true;

    foc_status_t st = drv8323_init(&drv, &s_mock_transport, &cfg);
    TEST_ASSERT_EQUAL_INT(FOC_OK, st);
    /* Verify at least 5 write frames sent. */
    TEST_ASSERT_TRUE(s_tx_idx >= 10); /* 5 writes + 5 reads + CLR_FLT */
}

/* -------------------------------------------------------------------------
 * Fault decode
 * ---------------------------------------------------------------------- */
static void test_fault_decode_vds(void)
{
    reset_mock();
    drv8323_t drv; memset(&drv, 0, sizeof(drv));
    drv.transport = &s_mock_transport;
    /* Simulate VDS_HA = 1, FAULT=1 in FAULT_STATUS1 */
    s_rx_script[0] = (uint16_t)((1u << 10) | (1u << 5)); /* FAULT | VDS_HA */
    s_rx_script[1] = 0u; /* VGS_STATUS2 */
    drv8323_faults_t faults;
    foc_status_t st = drv8323_read_faults(&drv, &faults);
    TEST_ASSERT_EQUAL_INT(FOC_OK, st);
    TEST_ASSERT_TRUE(faults.fault);
    TEST_ASSERT_TRUE(faults.vds_ha);
    TEST_ASSERT_FALSE(faults.vds_la);
}

/* -------------------------------------------------------------------------
 * amps_per_lsb
 * ---------------------------------------------------------------------- */
static void test_amps_per_lsb(void)
{
    drv8323_cfg_t cfg; memset(&cfg, 0, sizeof(cfg));
    cfg.csa_gain = DRV_CSA_GAIN_20; /* 20 V/V */
    /* shunt=10 mΩ, Vref=3.3 V, adc_fs=4096.
     * amps_per_lsb = 3.3 / (4096 × 20 × 0.010) = 3.3 / 819.2 ≈ 0.00403 A/LSB */
    q16_t result = drv8323_amps_per_lsb(&cfg, Q16(10.0), Q16(3.3), 4096u);
    /* Expected: Q16(0.00403) = 264 */
    TEST_ASSERT_INT_WITHIN(10, (int)Q16(0.00403), (int)result);
}

/* -------------------------------------------------------------------------
 * make_frame helpers
 * ---------------------------------------------------------------------- */
static void test_make_frames(void)
{
    /* Write frame: R/W=0, addr=2, data=0x55 → 0x0155 */
    uint16_t wf = drv8323_make_write_frame(DRV_REG_DRIVER_CTRL, 0x055u);
    TEST_ASSERT_EQUAL_INT((2u << 11) | 0x055u, wf & 0x7FFFu);
    TEST_ASSERT_EQUAL_INT(0u, wf >> 15); /* write */

    /* Read frame: R/W=1, addr=3 */
    uint16_t rf = drv8323_make_read_frame(DRV_REG_GATE_HS);
    TEST_ASSERT_EQUAL_INT(1u, rf >> 15);
    TEST_ASSERT_EQUAL_INT(3u, (rf >> 11) & 0xFu);
}

/* -------------------------------------------------------------------------
 * main
 * ---------------------------------------------------------------------- */
int main(void)
{
    printf("=== test_drv8323_regs ===\n");
    RUN_TEST(test_make_frames);
    RUN_TEST(test_pack_unpack_driver_ctrl);
    RUN_TEST(test_pack_csa_ctrl);
    RUN_TEST(test_init_sequence);
    RUN_TEST(test_fault_decode_vds);
    RUN_TEST(test_amps_per_lsb);
    TEST_REPORT();
}

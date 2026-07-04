/**
 * @file drv8323.h
 * @brief DRV8323 gate-driver: register map, pack/unpack, config, fault decode.
 *        Transport-agnostic (Layer B).  Transport callbacks supplied by Layer C.
 *
 * Register map (Decision D-017):
 *   0x00 FAULT_STATUS1, 0x01 VGS_STATUS2, 0x02 DRIVER_CTRL,
 *   0x03 GATE_HS, 0x04 GATE_LS, 0x05 OCP_CTRL, 0x06 CSA_CTRL.
 * Read/write via 16-bit SPI frame: bit[15]=R/W#, bits[14:11]=addr, bits[10:0]=data.
 */
#ifndef FOC_DRV8323_H
#define FOC_DRV8323_H

#include "motor_types.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------
 * Register IDs
 * ---------------------------------------------------------------------- */
typedef enum {
    DRV_REG_FAULT_STATUS1 = 0x00,
    DRV_REG_VGS_STATUS2   = 0x01,
    DRV_REG_DRIVER_CTRL   = 0x02,
    DRV_REG_GATE_HS       = 0x03,
    DRV_REG_GATE_LS       = 0x04,
    DRV_REG_OCP_CTRL      = 0x05,
    DRV_REG_CSA_CTRL      = 0x06
} drv8323_reg_id_t;

/* -------------------------------------------------------------------------
 * Register value struct (bitfield-free; use shift/mask helpers)
 * ---------------------------------------------------------------------- */
typedef struct {
    uint16_t fault_status1; /**< 0x00: read-only fault bits */
    uint16_t vgs_status2;   /**< 0x01: read-only VGS fault bits */
    uint16_t driver_ctrl;   /**< 0x02: PWM mode, lock, etc.     */
    uint16_t gate_hs;       /**< 0x03: HS gate drive strength   */
    uint16_t gate_ls;       /**< 0x04: LS gate drive strength   */
    uint16_t ocp_ctrl;      /**< 0x05: OCP/VDS thresholds       */
    uint16_t csa_ctrl;      /**< 0x06: CSA gain/sense/cal       */
} drv8323_regs_t;

/* -------------------------------------------------------------------------
 * Configuration
 * ---------------------------------------------------------------------- */
typedef enum { DRV_PWM_6X = 0, DRV_PWM_3X = 1, DRV_PWM_1X = 2 } drv_pwm_mode_t;
typedef enum { DRV_IDRIVE_10MA=0, DRV_IDRIVE_30MA=1, DRV_IDRIVE_60MA=2, DRV_IDRIVE_120MA=3,
               DRV_IDRIVE_240MA=4, DRV_IDRIVE_360MA=5, DRV_IDRIVE_520MA=6, DRV_IDRIVE_680MA=7 } drv_idrive_t;
typedef enum { DRV_TDRIVE_250NS=0, DRV_TDRIVE_500NS=1, DRV_TDRIVE_1000NS=2, DRV_TDRIVE_2000NS=3 } drv_tdrive_t;
typedef enum { DRV_OCP_LATCHED=0, DRV_OCP_RETRY=1, DRV_OCP_REPORT=2, DRV_OCP_DISABLED=3 } drv_ocp_mode_t;
typedef enum { DRV_OCP_DEG_4US=0, DRV_OCP_DEG_8US=1, DRV_OCP_DEG_16US=2, DRV_OCP_DEG_24US=3 } drv_ocp_deg_t;
typedef enum { DRV_VDS_0060=0, DRV_VDS_0130=1, DRV_VDS_0200=2, DRV_VDS_0260=3,
               DRV_VDS_0310=4, DRV_VDS_0450=5, DRV_VDS_0530=6, DRV_VDS_0600=7,
               DRV_VDS_0680=8, DRV_VDS_0750=9, DRV_VDS_0940=10, DRV_VDS_1130=11,
               DRV_VDS_1300=12, DRV_VDS_1500=13, DRV_VDS_1700=14, DRV_VDS_1880=15 } drv_vds_lvl_t;
typedef enum { DRV_CSA_GAIN_5=0, DRV_CSA_GAIN_10=1, DRV_CSA_GAIN_20=2, DRV_CSA_GAIN_40=3 } drv_csa_gain_t;

typedef struct {
    drv_pwm_mode_t  pwm_mode;
    drv_idrive_t    idrive_p_hs;
    drv_idrive_t    idrive_n_hs;
    drv_idrive_t    idrive_p_ls;
    drv_idrive_t    idrive_n_ls;
    drv_tdrive_t    tdrive;
    drv_ocp_mode_t  ocp_mode;
    drv_ocp_deg_t   ocp_deg;
    drv_vds_lvl_t   vds_lvl;
    drv_csa_gain_t  csa_gain;
    bool            csa_vref_div2;
    uint8_t         sense_ocp_lvl; /**< 0–15 per datasheet CSA_CTRL[3:0] */
    bool            dis_sen;       /**< Disable overcurrent sense          */
} drv8323_cfg_t;

/* -------------------------------------------------------------------------
 * Decoded fault struct
 * ---------------------------------------------------------------------- */
typedef struct {
    bool fault;        /**< Any fault latched         */
    bool vgs_ha, vgs_la, vgs_hb, vgs_lb, vgs_hc, vgs_lc;
    bool gdf;          /**< Gate-drive fault          */
    bool uvlo;         /**< Charge-pump UVLO          */
    bool otsd;         /**< Over-temperature shutdown */
    bool vds_ocp;      /**< VDS OCP on any FET        */
    bool cpuv;         /**< CP undervoltage           */
    bool vds_ha, vds_la, vds_hb, vds_lb, vds_hc, vds_lc;
} drv8323_faults_t;

/* -------------------------------------------------------------------------
 * Transport callbacks (supplied by Layer C)
 * ---------------------------------------------------------------------- */
typedef struct {
    int  (*xfer16)(uint16_t tx, uint16_t *rx); /**< SPI 16-bit transfer; returns 0 on success */
    void (*delay_us)(uint32_t us);
    void (*enable_pin)(bool on);               /**< ENABLE GPIO                                */
    bool (*nfault_pin)(void);                  /**< nFAULT GPIO read (false = fault)           */
} drv8323_transport_t;

/* -------------------------------------------------------------------------
 * Driver context
 * ---------------------------------------------------------------------- */
typedef struct {
    drv8323_regs_t          regs;
    drv8323_cfg_t           cfg;
    const drv8323_transport_t *transport;
    foc_status_t            last_error;
} drv8323_t;

/* -------------------------------------------------------------------------
 * API
 * ---------------------------------------------------------------------- */

/** Pack register values from cfg into regs struct. */
void drv8323_pack(drv8323_t *drv);

/** Unpack raw register words into regs struct (after SPI read). */
void drv8323_unpack(drv8323_t *drv, drv8323_reg_id_t reg, uint16_t word);

/** SPI write helper: frame = (0 << 15) | (addr << 11) | data. */
uint16_t drv8323_make_write_frame(drv8323_reg_id_t reg, uint16_t data);

/** SPI read helper: frame = (1 << 15) | (addr << 11). */
uint16_t drv8323_make_read_frame(drv8323_reg_id_t reg);

/**
 * Full init sequence (§5.9):
 *   enable pin high → wait ≥1 ms → write ctrl regs → read-back verify (×3)
 *   → clear faults → verify nFAULT deasserted.
 */
foc_status_t drv8323_init(drv8323_t *drv,
                           const drv8323_transport_t *transport,
                           const drv8323_cfg_t *cfg);

/** Read and decode fault registers. */
foc_status_t drv8323_read_faults(drv8323_t *drv, drv8323_faults_t *out);

/** Set CSA gain register field. */
foc_status_t drv8323_set_csa_gain(drv8323_t *drv, drv_csa_gain_t gain);

/** Assert CSA_CAL bits for amplifier offset calibration. */
foc_status_t drv8323_cal_csa(drv8323_t *drv, bool cal_on);

/**
 * Compute amps-per-LSB for hw_cal_t.
 * @param shunt_mohm  Shunt resistance [mΩ] Q16.16.
 * @param vref_v      ADC reference voltage [V] Q16.16.
 * @param adc_fs      ADC full-scale counts (e.g. 4096 for 12-bit).
 * @return amps_per_lsb [A/count] Q16.16.
 */
q16_t drv8323_amps_per_lsb(const drv8323_cfg_t *cfg,
                             q16_t shunt_mohm,
                             q16_t vref_v,
                             uint16_t adc_fs);

#ifdef __cplusplus
}
#endif

#endif /* FOC_DRV8323_H */

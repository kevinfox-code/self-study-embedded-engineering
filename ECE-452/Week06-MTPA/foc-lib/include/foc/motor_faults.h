/**
 * @file motor_faults.h
 * @brief Central fault manager: bitmask latch, debounce, severity, retry.
 *
 * Layer: B  Dependencies: motor_types.h only.
 */
#ifndef FOC_MOTOR_FAULTS_H
#define FOC_MOTOR_FAULTS_H

#include "motor_types.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------
 * Fault bit positions (§5.7 normative names)
 * ---------------------------------------------------------------------- */
typedef enum {
    FOC_FAULT_OC_HW          = (1u << 0),  /**< DRV nFAULT (HW overcurrent) */
    FOC_FAULT_OC_SW          = (1u << 1),  /**< Software overcurrent trip    */
    FOC_FAULT_OV_BUS         = (1u << 2),  /**< Bus overvoltage              */
    FOC_FAULT_UV_BUS         = (1u << 3),  /**< Bus undervoltage             */
    FOC_FAULT_OT             = (1u << 4),  /**< Overtemperature (I²t)        */
    FOC_FAULT_OBS_LOST       = (1u << 5),  /**< Observer confidence lost     */
    FOC_FAULT_IDENT_FAIL     = (1u << 6),  /**< Identification failure       */
    FOC_FAULT_ADC_CAL_FAIL   = (1u << 7),  /**< ADC calibration failure      */
    FOC_FAULT_DRV_SPI_FAIL   = (1u << 8),  /**< DRV8323 SPI readback mismatch */
    FOC_FAULT_SM_TIMEOUT     = (1u << 9),  /**< State-machine timeout        */
    FOC_FAULT_WDG_MISSED_LOOP= (1u << 10)  /**< Watchdog: missed fast loop   */
} foc_fault_t;

#define FOC_FAULT_ALL_MASK  0x7FFu

/* -------------------------------------------------------------------------
 * Severity classes
 * ---------------------------------------------------------------------- */
typedef enum {
    FOC_SEV_WARNING     = 0,  /**< Log only; no state change       */
    FOC_SEV_AUTO_RETRY  = 1,  /**< Bounded auto-restart            */
    FOC_SEV_FATAL_LATCH = 2   /**< Requires explicit CMD_CLEAR     */
} foc_fault_sev_t;

/* -------------------------------------------------------------------------
 * Per-fault configuration entry
 * ---------------------------------------------------------------------- */
typedef struct {
    foc_fault_t    bit;         /**< Fault bitmask entry            */
    foc_fault_sev_t severity;  /**< Classification                  */
    uint8_t        max_retries; /**< For AUTO_RETRY: attempts before FATAL */
    uint16_t       cooldown_ms; /**< Cooldown between retries [ms]  */
    uint8_t        debounce_cnt;/**< Consecutive trips before latch  */
} foc_fault_cfg_t;

/* -------------------------------------------------------------------------
 * Fault manager context
 * ---------------------------------------------------------------------- */
#define FOC_FAULT_NUM_ENTRIES 11u

typedef struct {
    uint32_t latched;           /**< Latched fault bitmask (written atomically) */
    uint32_t active;            /**< Currently-active (debounced) fault mask    */
    uint32_t fatal_mask;        /**< Cached mask of FATAL_LATCH faults          */
    uint8_t  debounce_cnt[FOC_FAULT_NUM_ENTRIES]; /**< Per-fault debounce counters */
    uint8_t  retry_cnt[FOC_FAULT_NUM_ENTRIES];    /**< Per-fault retry counters   */
    uint32_t cooldown_ticks[FOC_FAULT_NUM_ENTRIES]; /**< Countdown per fault      */
    uint32_t healthy_ticks;     /**< Ticks of healthy closed-loop run           */
    const foc_fault_cfg_t *map; /**< Fault configuration table (static)         */
    uint8_t  map_count;         /**< Number of entries in map                   */
} foc_faults_t;

/* -------------------------------------------------------------------------
 * API
 * ---------------------------------------------------------------------- */

/** Initialise fault manager with a fault configuration map. */
foc_status_t foc_faults_init(foc_faults_t *ctx,
                              const foc_fault_cfg_t *map, uint8_t count);

/** Immediately latch a fault (ISR-safe via 32-bit OR — atomic on M33). [ISR] */
void foc_faults_raise(foc_faults_t *ctx, foc_fault_t fault);

/** Debounced raise: fault latches only after count_limit consecutive calls. [ISR] */
void foc_faults_raise_debounced(foc_faults_t *ctx, foc_fault_t fault,
                                 uint8_t count_limit);

/** Return active fault bitmask. [ISR] */
uint32_t foc_faults_active(const foc_faults_t *ctx);

/** Clear specific fault bits (after user CMD_CLEAR_FAULTS or auto-retry). */
void foc_faults_clear(foc_faults_t *ctx, uint32_t mask);

/** Return true if an auto-retry is allowed for any active fault. */
foc_bool_t foc_faults_retry_allowed(foc_faults_t *ctx);

/** Tick cooldown counters (call from supervisor at 100 Hz). */
void foc_faults_tick(foc_faults_t *ctx, uint32_t ms_elapsed);

/** Mark healthy closed-loop tick; resets retry counters after 30 s. */
void foc_faults_healthy_tick(foc_faults_t *ctx, uint32_t ms_elapsed);

/* -------------------------------------------------------------------------
 * Default fault map (§8 policy)
 * ---------------------------------------------------------------------- */
extern const foc_fault_cfg_t g_foc_fault_map_default[FOC_FAULT_NUM_ENTRIES];

#ifdef __cplusplus
}
#endif

#endif /* FOC_MOTOR_FAULTS_H */

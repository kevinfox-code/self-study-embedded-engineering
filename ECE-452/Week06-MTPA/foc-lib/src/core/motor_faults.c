/**
 * @file motor_faults.c
 * @brief Fault manager implementation.
 *
 * Layer: B  Dependencies: motor_faults.h → motor_types.h.
 *
 * Atomicity note:
 *   On target (M33), 32-bit aligned writes/reads are atomic.
 *   On host, plain 32-bit OR is used (single-threaded test context).
 *   For ISR → task sharing, we follow the motor_hw_if.h critical section
 *   pattern where needed; the fast-raise path is a single 32-bit OR.
 */

#include "foc/motor_faults.h"
#include <string.h>

/* Default fault map (§8 policy): */
const foc_fault_cfg_t g_foc_fault_map_default[FOC_FAULT_NUM_ENTRIES] = {
    /* bit,                       severity,         max_retries, cooldown_ms, debounce */
    {FOC_FAULT_OC_HW,          FOC_SEV_FATAL_LATCH,  1u, 500u, 1u},
    {FOC_FAULT_OC_SW,          FOC_SEV_AUTO_RETRY,   2u, 500u, 3u},
    {FOC_FAULT_OV_BUS,         FOC_SEV_AUTO_RETRY,   2u, 200u, 1u},
    {FOC_FAULT_UV_BUS,         FOC_SEV_AUTO_RETRY,   2u, 200u, 1u},
    {FOC_FAULT_OT,             FOC_SEV_AUTO_RETRY,   1u,1000u, 1u},
    {FOC_FAULT_OBS_LOST,       FOC_SEV_AUTO_RETRY,   2u, 500u, 1u},
    {FOC_FAULT_IDENT_FAIL,     FOC_SEV_AUTO_RETRY,   1u, 500u, 1u},
    {FOC_FAULT_ADC_CAL_FAIL,   FOC_SEV_FATAL_LATCH,  0u,   0u, 1u},
    {FOC_FAULT_DRV_SPI_FAIL,   FOC_SEV_FATAL_LATCH,  0u,   0u, 1u},
    {FOC_FAULT_SM_TIMEOUT,     FOC_SEV_AUTO_RETRY,   3u, 500u, 1u},
    {FOC_FAULT_WDG_MISSED_LOOP,FOC_SEV_FATAL_LATCH,  0u,   0u, 1u},
};

/* Map fault bit to table index; returns -1 if not found. */
static int fault_to_idx(const foc_faults_t *ctx, foc_fault_t fault)
{
    for (uint8_t i = 0; i < ctx->map_count; i++) {
        if (ctx->map[i].bit == fault) return (int)i;
    }
    return -1;
}

foc_status_t foc_faults_init(foc_faults_t *ctx,
                              const foc_fault_cfg_t *map, uint8_t count)
{
    if (!ctx || !map || count == 0u) return FOC_EINVAL;
    memset(ctx, 0, sizeof(*ctx));
    ctx->map       = map;
    ctx->map_count = count;
    /* Cache fatal mask. */
    ctx->fatal_mask = 0u;
    for (uint8_t i = 0; i < count; i++) {
        if (map[i].severity == FOC_SEV_FATAL_LATCH) {
            ctx->fatal_mask |= (uint32_t)map[i].bit;
        }
    }
    return FOC_OK;
}

void foc_faults_raise(foc_faults_t *ctx, foc_fault_t fault)
{
    /* Atomic 32-bit OR (single instruction on M33). */
    ctx->latched |= (uint32_t)fault;
    ctx->active  |= (uint32_t)fault;
}

void foc_faults_raise_debounced(foc_faults_t *ctx, foc_fault_t fault,
                                 uint8_t count_limit)
{
    int idx = fault_to_idx(ctx, fault);
    if (idx < 0) { foc_faults_raise(ctx, fault); return; }

    ctx->debounce_cnt[idx]++;
    if (ctx->debounce_cnt[idx] >= count_limit) {
        ctx->debounce_cnt[idx] = count_limit; /* saturate */
        foc_faults_raise(ctx, fault);
    }
}

uint32_t foc_faults_active(const foc_faults_t *ctx)
{
    return ctx->active;
}

void foc_faults_clear(foc_faults_t *ctx, uint32_t mask)
{
    ctx->latched &= ~mask;
    ctx->active  &= ~mask;
    /* Reset debounce for cleared faults. */
    for (uint8_t i = 0; i < ctx->map_count; i++) {
        if ((uint32_t)ctx->map[i].bit & mask) {
            ctx->debounce_cnt[i] = 0u;
        }
    }
}

foc_bool_t foc_faults_retry_allowed(foc_faults_t *ctx)
{
    /* Retry is possible if:
     *   - At least one active fault is AUTO_RETRY.
     *   - No FATAL_LATCH faults are active.
     *   - retry_cnt < max_retries.
     *   - cooldown_ticks[idx] == 0.
     */
    if (ctx->active & ctx->fatal_mask) return FOC_FALSE;
    if (ctx->active == 0u) return FOC_FALSE;

    for (uint8_t i = 0; i < ctx->map_count; i++) {
        if (!((uint32_t)ctx->map[i].bit & ctx->active)) continue;
        if (ctx->map[i].severity != FOC_SEV_AUTO_RETRY) continue;
        if (ctx->retry_cnt[i] >= ctx->map[i].max_retries) {
            /* Budget exhausted → escalate to fatal. */
            ctx->fatal_mask |= (uint32_t)ctx->map[i].bit;
            return FOC_FALSE;
        }
        if (ctx->cooldown_ticks[i] > 0u) return FOC_FALSE;
        /* Increment retry counter on each call (caller responsible for once-per-fault-event). */
        ctx->retry_cnt[i]++;
        /* Arm cooldown. */
        ctx->cooldown_ticks[i] = ctx->map[i].cooldown_ms;
        return FOC_TRUE;
    }
    return FOC_FALSE;
}

void foc_faults_tick(foc_faults_t *ctx, uint32_t ms_elapsed)
{
    for (uint8_t i = 0; i < ctx->map_count; i++) {
        if (ctx->cooldown_ticks[i] > 0u) {
            if (ctx->cooldown_ticks[i] > ms_elapsed) {
                ctx->cooldown_ticks[i] -= ms_elapsed;
            } else {
                ctx->cooldown_ticks[i] = 0u;
            }
        }
    }
}

void foc_faults_healthy_tick(foc_faults_t *ctx, uint32_t ms_elapsed)
{
    ctx->healthy_ticks += ms_elapsed;
    if (ctx->healthy_ticks >= 30000u) { /* 30 s */
        ctx->healthy_ticks = 0u;
        /* Reset retry counters for AUTO_RETRY faults. */
        for (uint8_t i = 0; i < ctx->map_count; i++) {
            if (ctx->map[i].severity == FOC_SEV_AUTO_RETRY) {
                ctx->retry_cnt[i] = 0u;
            }
        }
    }
}

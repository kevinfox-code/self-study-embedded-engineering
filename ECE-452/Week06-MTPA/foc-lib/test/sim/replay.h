/**
 * @file replay.h
 * @brief Deterministic vector replay driver.  HOST ONLY.
 *        Feeds pre-recorded or live-simulated vectors into module APIs.
 */
#ifndef FOC_REPLAY_H
#define FOC_REPLAY_H

#include "foc/motor_types.h"
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/** One replay frame: input voltages + output measurements. */
typedef struct {
    uint32_t cycle;       /**< Fast-loop cycle index */
    double   v_alpha;     /**< Applied Vα [V]        */
    double   v_beta;      /**< Applied Vβ [V]        */
    double   i_alpha;     /**< Measured iα [A]       */
    double   i_beta;      /**< Measured iβ [A]       */
    double   theta_e;     /**< True angle  [rad]     */
    double   omega_e;     /**< True speed  [rad/s]   */
} replay_frame_t;

typedef struct {
    replay_frame_t *frames;
    uint32_t        count;
    uint32_t        cursor;
} replay_t;

/** Initialise from caller-allocated frame buffer. */
void replay_init(replay_t *r, replay_frame_t *frames, uint32_t max_frames);

/** Append one frame. */
int replay_push(replay_t *r, const replay_frame_t *f);

/** Reset cursor to start. */
void replay_rewind(replay_t *r);

/** Get next frame (returns 0 on success, -1 at end). */
int replay_next(replay_t *r, replay_frame_t *out);

/** Write frames to CSV file. */
int replay_write_csv(const replay_t *r, const char *path);

/** Read frames from CSV file (up to max_frames). */
int replay_read_csv(replay_t *r, const char *path);

#ifdef __cplusplus
}
#endif

#endif /* FOC_REPLAY_H */

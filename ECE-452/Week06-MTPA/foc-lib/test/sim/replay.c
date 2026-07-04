/**
 * @file replay.c
 * @brief Deterministic replay driver implementation.
 */
#include "replay.h"
#include <string.h>
#include <stdio.h>

void replay_init(replay_t *r, replay_frame_t *frames, uint32_t max_frames)
{
    r->frames = frames;
    r->count  = 0u;
    r->cursor = 0u;
    (void)max_frames;
}

int replay_push(replay_t *r, const replay_frame_t *f)
{
    r->frames[r->count++] = *f;
    return 0;
}

void replay_rewind(replay_t *r)
{
    r->cursor = 0u;
}

int replay_next(replay_t *r, replay_frame_t *out)
{
    if (r->cursor >= r->count) return -1;
    *out = r->frames[r->cursor++];
    return 0;
}

int replay_write_csv(const replay_t *r, const char *path)
{
    FILE *f = fopen(path, "w");
    if (!f) return -1;
    fprintf(f, "cycle,v_alpha,v_beta,i_alpha,i_beta,theta_e,omega_e\n");
    for (uint32_t i = 0; i < r->count; i++) {
        const replay_frame_t *fr = &r->frames[i];
        fprintf(f, "%u,%.8f,%.8f,%.8f,%.8f,%.8f,%.8f\n",
                fr->cycle, fr->v_alpha, fr->v_beta,
                fr->i_alpha, fr->i_beta, fr->theta_e, fr->omega_e);
    }
    fclose(f);
    return 0;
}

int replay_read_csv(replay_t *r, const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f) return -1;
    char line[256];
    /* Skip header */
    if (!fgets(line, sizeof(line), f)) { fclose(f); return -1; }
    r->count = 0u;
    while (fgets(line, sizeof(line), f)) {
        replay_frame_t fr;
        int n = sscanf(line, "%u,%lf,%lf,%lf,%lf,%lf,%lf",
                       &fr.cycle, &fr.v_alpha, &fr.v_beta,
                       &fr.i_alpha, &fr.i_beta, &fr.theta_e, &fr.omega_e);
        if (n == 7) {
            r->frames[r->count++] = fr;
        }
    }
    fclose(f);
    r->cursor = 0u;
    return 0;
}

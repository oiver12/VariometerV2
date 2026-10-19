/* Host adapter for the existing firmware filters. See replay_flight.py.
 * Scheduling and sensor calibration are intentionally outside this adapter.
 */
#include "Kalman/kalman.h"
#include "QuaternionFilter.h"
#include <math.h>
#include <string.h>

static kalman_t filter;
static float rotation[4];

void replay_init(void)
{
    memset(&filter, 0, sizeof(filter));
    kalman_filter_init(&filter);
    kalman_filter_reset(&filter);
    matrix_set(&filter.H, 0, 0, 1);
    matrix_set(&filter.R, 0, 0, 0.1298f * 0.1298f);
    matrix_set(&filter.P, 0, 0, 500);
    matrix_set(&filter.P, 1, 1, 500);
    rotation[0] = 1;
    rotation[1] = rotation[2] = rotation[3] = 0;
    QuaternionFilter_init();
}

void replay_step(const float *record, float altitude, float dt, float *output)
{
    /* Axis mapping and vertical projection from main/MainTasks.c. The log
     * already contains the magnetometer values supplied to the orientation filter.
     */
    float an = record[1] / 9.81f, ae = record[0] / 9.81f, ad = record[2] / 9.81f;
    QuaternionFilter_Madgwick(an, ae, ad, -record[4], -record[3], -record[5],
                             -record[6], record[7], -record[8], dt, rotation);
    float vertical = 9.81f * (
        2 * (rotation[1] * rotation[3] - rotation[0] * rotation[2]) * an +
        2 * (rotation[2] * rotation[3] + rotation[0] * rotation[1]) * ae +
        (2 * (rotation[0] * rotation[0] + rotation[3] * rotation[3]) - 1) * ad);
    matrix_set(&filter.F, 0, 0, 1);
    matrix_set(&filter.F, 0, 1, dt);
    matrix_set(&filter.F, 1, 0, 0);
    matrix_set(&filter.F, 1, 1, 1);
    matrix_set(&filter.G, 0, 0, 0.5f * dt * dt);
    matrix_set(&filter.G, 1, 0, dt);
    float variance = 0.2f * 0.2f;
    matrix_set(&filter.Q, 0, 0, dt * dt * dt * dt * variance / 4);
    matrix_set(&filter.Q, 0, 1, dt * dt * dt * variance / 2);
    matrix_set(&filter.Q, 1, 0, dt * dt * dt * variance / 2);
    matrix_set(&filter.Q, 1, 1, dt * dt * variance);
    matrix_set(&filter.z, 0, 0, altitude);
    matrix_set(&filter.u, 0, 0, vertical - 9.81f);
    /* Match the measurement-update / prediction order in TaskKalmanFilter. */
    kalman_update(&filter);
    kalman_predict(&filter);
    output[0] = matrix_get(&filter.x, 0, 0);
    output[1] = matrix_get(&filter.x, 1, 0);
}

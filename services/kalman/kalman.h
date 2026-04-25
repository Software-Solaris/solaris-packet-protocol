#ifndef KALMAN_H
#define KALMAN_H

#include <math.h>

#define g                (float)9.81f
#define GYRO_X_NOISE_STD 0.00026f
#define GYRO_Y_NOISE_STD 0.00026f
#define GYRO_Z_NOISE_STD 0.00026f

#define GYRO_X_VAR (GYRO_X_NOISE_STD * GYRO_X_NOISE_STD)
#define GYRO_Y_VAR (GYRO_Y_NOISE_STD * GYRO_Y_NOISE_STD)
#define GYRO_Z_VAR (GYRO_Z_NOISE_STD * GYRO_Z_NOISE_STD)

#define M_PI       3.14159265358979323846f
#define DEG_TO_RAD (M_PI / 180.0f)

#define SENSOR_DATA_TOL 0.000001f
typedef struct
{
    float qw;
    float qx;
    float qy;
    float qz;
    // float bx;
    // float by;
    // float bz;

    float P[16];
    float Q[16];
    float R[3];

} kalman_state;

typedef struct
{
    float gyro_data[3];
    float acc_data[3];
    //float mag_data[3];

    float gyro_old_data[3];
    float acc_old_data[3];
    float mag_old_data[3];

    int gyro_new_data;
    int acc_new_data;
    //int mag_new_data;

} sensor_data;


void SPP_SERVICES_KALMAN_ekfInit(kalman_state *kal, sensor_data *data, float Pinit, float *Q,
                                 float *R);
void SPP_SERVICES_KALMAN_ekfPredict(kalman_state *kal, float *gyr_rps, float T);
void SPP_SERVICES_KALMAN_ekfUpdate(kalman_state *kal, float *acc_ms2);
void SPP_SERVICES_KALMAN_run(kalman_state *kal, sensor_data *data, float T);

void SPP_SERVICES_KALMAN_newDataCheck(sensor_data *data);
void SPP_SERVICES_KALMAN_mat4x4Add(const float *restrict A, const float *restrict B,
                                   float *restrict out);
void SPP_SERVICES_KALMAN_mat4x4Sub(const float *restrict A, const float *restrict B,
                                   float *restrict out);
void SPP_SERVICES_KALMAN_mat4x4Mul(const float *restrict A, const float *restrict B,
                                   float *restrict out);
void SPP_SERVICES_KALMAN_mat4x4Mul4x3(const float *restrict A, const float *restrict B,
                                      float *restrict out);
void SPP_SERVICES_KALMAN_mat4x3Mul3x4(const float *restrict A, const float *restrict B,
                                      float *restrict out);
void SPP_SERVICES_KALMAN_mat4x3Mul3x3(const float *restrict A, const float *restrict B,
                                      float *restrict out);
void SPP_SERVICES_KALMAN_mat4x3Mul3x3diag(const float *restrict A, const float *restrict B,
                                          float *restrict out);
void SPP_SERVICES_KALMAN_mat4x3Mul3x1(const float *restrict A, const float *restrict B,
                                      float *restrict out);
void SPP_SERVICES_KALMAN_mat3x4Mul4x4(const float *restrict A, const float *restrict B,
                                      float *restrict out);
void SPP_SERVICES_KALMAN_mat3x4Mul4x3(const float *restrict A, const float *restrict B,
                                      float *restrict out);
void SPP_SERVICES_KALMAN_mat4x4Transpose(const float *restrict A, float *restrict out);
void SPP_SERVICES_KALMAN_mat3x4Transpose(const float *restrict A, float *restrict out);
void SPP_SERVICES_KALMAN_mat4x3Transpose(const float *restrict A, float *restrict out);
void SPP_SERVICES_KALMAN_mat3x3Transpose(const float *restrict A, float *restrict out);
int SPP_SERVICES_KALMAN_mat3x3Inverse(const float *restrict in, float *restrict out);

#endif

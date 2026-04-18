#ifndef KALMAN_H
#define KALMAN_H

#include <math.h>

#define g (float)9.81f;


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
    float Q[4];
    float R[3];

} kalman_state;


void SPP_SERVICES_KALMAN_ekfInit(kalman_state *kal, float Pinit, float *Q, float *R);
void SPP_SERVICES_KALMAN_ekfPredict(kalman_state *kal, float *gyr_rps, float T);
void SPP_SERVICES_KALMAN_ekfUpdate(kalman_state *kal, float *acc_ms2);

void SPP_SERVICES_KALMAN_mat4x4Add(const float *restrict A, const float *restrict B, float *restrict out);
void SPP_SERVICES_KALMAN_mat4x4Sub(const float *restrict A, const float *restrict B, float *restrict out);
void SPP_SERVICES_KALMAN_mat4x4Mul(const float *restrict A, const float *restrict B, float *restrict out);
void SPP_SERVICES_KALMAN_mat4x4Mul4x3(const float *restrict A, const float *restrict B, float *restrict out);
void SPP_SERVICES_KALMAN_mat4x3Mul3x4(const float *restrict A, const float *restrict B, float *restrict out);
void SPP_SERVICES_KALMAN_mat4x3Mul3x3(const float *restrict A, const float *restrict B, float *restrict out);
void SPP_SERVICES_KALMAN_mat4x3Mul3x1(const float *restrict A, const float *restrict B, float *restrict out);
void SPP_SERVICES_KALMAN_mat3x4Mul4x4(const float *restrict A, const float *restrict B, float *restrict out);
void SPP_SERVICES_KALMAN_mat3x4Mul4x3(const float *restrict A, const float *restrict B, float *restrict out);
void SPP_SERVICES_KALMAN_mat4x4Transpose(const float *restrict A, float *restrict out);
void SPP_SERVICES_KALMAN_mat3x4Transpose(const float *restrict A, float *restrict out);
void SPP_SERVICES_KALMAN_mat3x3Transpose(const float *restrict A, float *restrict out);
int SPP_SERVICES_KALMAN_mat3x3Inverse(const float *restrict in, float *restrict out);

void SPP_SERVICES_KALMAN_mat4x4Mul(const float *restrict A, const float *restrict B, float *restrict out);
void SPP_SERVICES_KALMAN_mat4x4Add(const float *restrict A, const float *restrict B, float *restrict out);
void SPP_SERVICES_KALMAN_mat4x4Transpose(const float *restrict A, float *restrict out);


#endif

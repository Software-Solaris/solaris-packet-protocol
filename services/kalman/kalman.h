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


void ekf_init(kalman_state *kal, float Pinit, float *Q, float *R);
void ekf_predict(kalman_state *kal, float *gyr_rps, float T);
void ekf_update(kalman_state *kal, float *acc_ms2);

void Mat4x4_Add(const float *restrict A, const float *restrict B, float *restrict out);
void Mat4x4_Sub(const float *restrict A, const float *restrict B, float *restrict out);
void Mat4x4_Mul(const float *restrict A, const float *restrict B, float *restrict out);
void Mat4x4_Mul_4x3(const float *restrict A, const float *restrict B, float *restrict out);
void Mat4x3_Mul_3x4(const float *restrict A, const float *restrict B, float *restrict out);
void Mat4x3_Mul_3x3(const float *restrict A, const float *restrict B, float *restrict out);
void Mat4x3_Mul_3x1(const float *restrict A, const float *restrict B, float *restrict out);
void Mat3x4_Mul_4x4(const float *restrict A, const float *restrict B, float *restrict out);
void Mat3x4_Mul_4x3(const float *restrict A, const float *restrict B, float *restrict out);
void Mat4x4_Transpose(const float *restrict A, float *restrict out);
void Mat3x4_Transpose(const float *restrict A, float *restrict out);
void Mat3x3_Transpose(const float *restrict A, float *restrict out);
int Mat3x3_Inverse(const float *restrict in, float *restrict out);

void Mat4x4_Mul(const float *restrict A, const float *restrict B, float *restrict out);
void Mat4x4_Add(const float *restrict A, const float *restrict B, float *restrict out);
void Mat4x4_Transpose(const float *restrict A, float *restrict out);


#endif

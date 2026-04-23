// VERSIÓN SIMPLIFICADA SIN BIAS Y SIN MAGNETÓMETRO


#include <stdio.h>
#include <string.h>
#include "kalman.h"


void SPP_SERVICES_KALMAN_ekfInit(kalman_state *kal, float Pinit, float *Q, float *R)
{
    kal->qw = 1.0f;
    kal->qx = 0.0f;
    kal->qy = 0.0f;
    kal->qz = 0.0f;

    // kal->bx = 0.0f;
    // kal->by = 0.0f;
    // kal->bz = 0.0f;

    memset(kal->P, 0, sizeof(kal->P));
    kal->P[0] = Pinit;
    kal->P[5] = Pinit;
    kal->P[10] = Pinit;
    kal->P[15] = Pinit;

    memset(kal->Q, 0, sizeof(kal->Q));
    kal->Q[0] = Q[0];
    kal->Q[5] = Q[5];
    kal->Q[10] = Q[10];
    kal->Q[15] = Q[15];

    kal->R[0] = R[0];
    kal->R[1] = R[1];
    kal->R[2] = R[2];
}


void SPP_SERVICES_KALMAN_ekfPredict(kalman_state *kal, float *gyr_rps, float T)
{
    /* Extract measurements */
    float p = gyr_rps[0];
    float q = gyr_rps[1];
    float r = gyr_rps[2];

    /* Save old coefficients */
    float qw_old = kal->qw;
    float qx_old = kal->qx;
    float qy_old = kal->qy;
    float qz_old = kal->qz;

    /* Compute common terms */
    float half_T = T * 0.5f;
    float dp = half_T * p;
    float dq = half_T * q;
    float dr = half_T * r;

    /* Prediction: x = f(x, u) */
    kal->qw = qw_old - dp * qx_old - dq * qy_old - dr * qz_old;
    kal->qx = qx_old + dp * qw_old - dq * qz_old + dr * qy_old;
    kal->qy = qy_old + dp * qz_old + dq * qw_old - dr * qx_old;
    kal->qz = qz_old - dp * qy_old + dq * qx_old + dr * qw_old;

    /* Quaternion normalization */
    float norm =
        sqrtf(kal->qw * kal->qw + kal->qx * kal->qx + kal->qy * kal->qy + kal->qz * kal->qz);
    kal->qw /= norm;
    kal->qx /= norm;
    kal->qy /= norm;
    kal->qz /= norm;

    /* Jacobian of f(x, u) */
    float F[16] = {1.0f, -dp, -dq, -dr, dp, 1.0f, dr, -dq, dq, -dr, 1.0f, dp, dr, dq, -dp, 1.0f};

    /* Update covariance matrix: P = F * P * F' + Q */

    // Intermidiate calculations
    float result1[16];
    SPP_SERVICES_KALMAN_mat4x4Mul(F, kal->P, result1);

    float F_trans[16];
    SPP_SERVICES_KALMAN_mat4x4Transpose(F, F_trans);

    float result2[16];
    SPP_SERVICES_KALMAN_mat4x4Mul(result1, F_trans, result2);

    // Fill updated covariance matrix P
    SPP_SERVICES_KALMAN_mat4x4Add(result2, kal->Q, kal->P);

    /* Update process noise covariance matrix Q */

    //Jacobian W_t
    float W[12];

    //Precalculate common terms
    float half_Tqw = half_T * qw_old;
    float half_Tqx = half_T * qx_old;
    float half_Tqy = half_T * qy_old;
    float half_Tqz = half_T * qz_old;

    //Row 1
    W[0] = -half_Tqx;
    W[1] = -half_Tqy;
    W[2] = -half_Tqz;

    //Row 2
    W[3] = half_Tqw;
    W[4] = -half_Tqz;
    W[5] = half_Tqy;

    //Row 3
    W[6] = half_Tqz;
    W[7] = half_Tqw;
    W[8] = -half_Tqx;

    //Row 4
    W[9] = -half_Tqy;
    W[10] = half_Tqx;
    W[11] = half_Tqw;

    //Sigma_w
    float Sigma_w[3];
    Sigma_w[0] = GYRO_X_VAR;
    Sigma_w[1] = GYRO_Y_VAR;
    Sigma_w[2] = GYRO_Z_VAR;

    float result3[12];
    SPP_SERVICES_KALMAN_mat4x3Mul3x3diag(W, Sigma_w, result3);

    float W_trans[12];
    SPP_SERVICES_KALMAN_mat4x3Transpose(W, W_trans);


    SPP_SERVICES_KALMAN_mat4x3Mul3x4(result3, W_trans, kal->Q);
}


void SPP_SERVICES_KALMAN_ekfUpdate(kalman_state *kal, float *acc_ms2)
{
    /* Extract measurements (this will be vector z)*/
    float ax = acc_ms2[0];
    float ay = acc_ms2[1];
    float az = acc_ms2[2];

    /* Extract previous quaternion */
    float qw = kal->qw;
    float qx = kal->qx;
    float qy = kal->qy;
    float qz = kal->qz;

    /* Compute squared terms */
    float qw2 = qw * qw;
    float qx2 = qx * qx;
    float qy2 = qy * qy;
    float qz2 = qz * qz;

    /* Compute crossed terms */
    float qwqx2 = 2.0f * qw * qx;
    float qwqy2 = 2.0f * qw * qy;
    float qwqz2 = 2.0f * qw * qz;
    float qxqy2 = 2.0f * qx * qy;
    float qxqz2 = 2.0f * qx * qz;
    float qyqz2 = 2.0f * qy * qz;

    /* Calculate rotation matrix C */
    float C[9];

    C[0] = qw2 + qx2 - qy2 - qz2;
    C[1] = qxqy2 - qwqz2;
    C[2] = qxqz2 + qwqy2;

    C[3] = qxqy2 + qwqz2;
    C[4] = qw2 - qx2 + qy2 - qz2;
    C[5] = qyqz2 - qwqx2;

    C[6] = qxqz2 - qwqy2;
    C[7] = qyqz2 + qwqx2;
    C[8] = qw2 - qx2 - qy2 + qz2;

    /* Calculate function h(x) = C' * g */
    float C_trans[9];
    SPP_SERVICES_KALMAN_mat3x3Transpose(C, C_trans);

    float g_iner[3] = {0};
    g_iner[2] = g;

    float h[3];
    h[0] = C_trans[0] * g_iner[0] + C_trans[1] * g_iner[1] + C_trans[2] * g;
    h[1] = C_trans[3] * g_iner[0] + C_trans[4] * g_iner[1] + C_trans[5] * g;
    h[2] = C_trans[6] * g_iner[0] + C_trans[7] * g_iner[1] + C_trans[8] * g;

    /* Jacobian of h(x) */

    // Compute more common terms
    float a = g_iner[0] * qw + g_iner[1] * qz - g_iner[2] * qy;
    float b = g_iner[0] * qx + g_iner[1] * qy + g_iner[2] * qz;
    float c = -g_iner[0] * qy + g_iner[1] * qx - g_iner[2] * qw;
    float d = -g_iner[0] * qz + g_iner[1] * qw + g_iner[2] * qx;

    // Multiply by 2
    float _2a = 2.0f * a;
    float _2b = 2.0f * b;
    float _2c = 2.0f * c;
    float _2d = 2.0f * d;

    // Fill Jacobian H
    float H[12];

    H[0] = _2a;
    H[1] = _2b;
    H[2] = _2c;
    H[3] = _2d;

    H[4] = _2d;
    H[5] = -_2c;
    H[6] = _2b;
    H[7] = -_2a;

    H[8] = -_2c;
    H[9] = -_2d;
    H[10] = _2a;
    H[11] = _2b;


    /* Kalman Gain: K = P * H' / (H * P * H' + R) */

    // Calculate: P * H' (4x3)
    float H_trans[12];
    SPP_SERVICES_KALMAN_mat3x4Transpose(H, H_trans);

    float term1[12];
    SPP_SERVICES_KALMAN_mat4x4Mul4x3(kal->P, H_trans, term1);

    // Calculate: H * P * H' + R (3x3)

    // H * P (3x4)
    float term2_1[12];
    SPP_SERVICES_KALMAN_mat3x4Mul4x4(H, kal->P, term2_1);

    // H * P * H' (3x3)
    float term2_2[9];
    SPP_SERVICES_KALMAN_mat3x4Mul4x3(H, term1, term2_2);

    // H * P * H' + R (3x3)
    float term2[9];
    term2[0] = term2_2[0] + kal->R[0];
    term2[1] = term2_2[1];
    term2[2] = term2_2[2];

    term2[3] = term2_2[3];
    term2[4] = term2_2[4] + kal->R[1];
    term2[5] = term2_2[5];

    term2[6] = term2_2[6];
    term2[7] = term2_2[7];
    term2[8] = term2_2[8] + kal->R[2];

    // Finally: K = P * H' / (H * P * H' + R) (4x3)
    float term2_inv[9];
    SPP_SERVICES_KALMAN_mat3x3Inverse(term2, term2_inv);

    float K[12];
    SPP_SERVICES_KALMAN_mat4x3Mul3x3(term1, term2_inv, K);


    /* Update state estimate x = x + K * (z - h) */

    float v[3];
    v[0] = ax - h[0];
    v[1] = ay - h[1];
    v[2] = az - h[2];

    float K_v[4];
    SPP_SERVICES_KALMAN_mat4x3Mul3x1(K, v, K_v);

    kal->qw += K_v[0];
    kal->qx += K_v[1];
    kal->qy += K_v[2];
    kal->qz += K_v[3];

    /* Update covariance matrix P = (I - K * H) * P */

    float K_H[16];
    SPP_SERVICES_KALMAN_mat4x3Mul3x4(K, H, K_H);

    float I4[16] = {0};
    I4[0] = 1.0f;
    I4[5] = 1.0f;
    I4[10] = 1.0f;
    I4[15] = 1.0f;

    float part1[16];
    SPP_SERVICES_KALMAN_mat4x4Sub(I4, K_H, part1);

    float tmp_P[16];
    tmp_P[0] = kal->P[0];
    tmp_P[1] = kal->P[1];
    tmp_P[2] = kal->P[2];
    tmp_P[3] = kal->P[3];

    tmp_P[4] = kal->P[4];
    tmp_P[5] = kal->P[5];
    tmp_P[6] = kal->P[6];
    tmp_P[7] = kal->P[7];

    tmp_P[8] = kal->P[8];
    tmp_P[9] = kal->P[9];
    tmp_P[10] = kal->P[10];
    tmp_P[11] = kal->P[11];

    tmp_P[12] = kal->P[12];
    tmp_P[13] = kal->P[13];
    tmp_P[14] = kal->P[14];
    tmp_P[15] = kal->P[15];

    SPP_SERVICES_KALMAN_mat4x4Mul(part1, tmp_P, kal->P);
}


// borrar?
void SPP_SERVICES_KALMAN_mat4x4Add(const float *restrict A, const float *restrict B,
                                   float *restrict out)
{
    /* Row 1 */
    out[0] = A[0] + B[0];
    out[1] = A[1] + B[1];
    out[2] = A[2] + B[2];
    out[3] = A[3] + B[3];

    /* Row 2 */
    out[4] = A[4] + B[4];
    out[5] = A[5] + B[5];
    out[6] = A[6] + B[6];
    out[7] = A[7] + B[7];

    /* Row 3 */
    out[8] = A[8] + B[8];
    out[9] = A[9] + B[9];
    out[10] = A[10] + B[10];
    out[11] = A[11] + B[11];

    /* Row 4 */
    out[12] = A[12] + B[12];
    out[13] = A[13] + B[13];
    out[14] = A[14] + B[14];
    out[15] = A[15] + B[15];
}


void SPP_SERVICES_KALMAN_mat4x4Sub(const float *restrict A, const float *restrict B,
                                   float *restrict out)
{
    // Row 1
    out[0] = A[0] - B[0];
    out[1] = A[1] - B[1];
    out[2] = A[2] - B[2];
    out[3] = A[3] - B[3];

    // Row 2
    out[4] = A[4] - B[4];
    out[5] = A[5] - B[5];
    out[6] = A[6] - B[6];
    out[7] = A[7] - B[7];

    // Row 3
    out[8] = A[8] - B[8];
    out[9] = A[9] - B[9];
    out[10] = A[10] - B[10];
    out[11] = A[11] - B[11];

    // Row 4
    out[12] = A[12] - B[12];
    out[13] = A[13] - B[13];
    out[14] = A[14] - B[14];
    out[15] = A[15] - B[15];
}


void SPP_SERVICES_KALMAN_mat4x4Mul(const float *restrict A, const float *restrict B,
                                   float *restrict out)
{
    /* restrict -> promise to compiler that aliasing will not occur -> makes it faster */

    /* Row 1 */
    out[0] = A[0] * B[0] + A[1] * B[4] + A[2] * B[8] + A[3] * B[12];
    out[1] = A[0] * B[1] + A[1] * B[5] + A[2] * B[9] + A[3] * B[13];
    out[2] = A[0] * B[2] + A[1] * B[6] + A[2] * B[10] + A[3] * B[14];
    out[3] = A[0] * B[3] + A[1] * B[7] + A[2] * B[11] + A[3] * B[15];

    /* Row 2 */
    out[4] = A[4] * B[0] + A[5] * B[4] + A[6] * B[8] + A[7] * B[12];
    out[5] = A[4] * B[1] + A[5] * B[5] + A[6] * B[9] + A[7] * B[13];
    out[6] = A[4] * B[2] + A[5] * B[6] + A[6] * B[10] + A[7] * B[14];
    out[7] = A[4] * B[3] + A[5] * B[7] + A[6] * B[11] + A[7] * B[15];

    /* Row 3 */
    out[8] = A[8] * B[0] + A[9] * B[4] + A[10] * B[8] + A[11] * B[12];
    out[9] = A[8] * B[1] + A[9] * B[5] + A[10] * B[9] + A[11] * B[13];
    out[10] = A[8] * B[2] + A[9] * B[6] + A[10] * B[10] + A[11] * B[14];
    out[11] = A[8] * B[3] + A[9] * B[7] + A[10] * B[11] + A[11] * B[15];

    /* Row 4 */
    out[12] = A[12] * B[0] + A[13] * B[4] + A[14] * B[8] + A[15] * B[12];
    out[13] = A[12] * B[1] + A[13] * B[5] + A[14] * B[9] + A[15] * B[13];
    out[14] = A[12] * B[2] + A[13] * B[6] + A[14] * B[10] + A[15] * B[14];
    out[15] = A[12] * B[3] + A[13] * B[7] + A[14] * B[11] + A[15] * B[15];
}


void SPP_SERVICES_KALMAN_mat4x4Mul4x3(const float *restrict A, const float *restrict B,
                                      float *restrict out)
{
    // Row 1
    out[0] = A[0] * B[0] + A[1] * B[3] + A[2] * B[6] + A[3] * B[9];
    out[1] = A[0] * B[1] + A[1] * B[4] + A[2] * B[7] + A[3] * B[10];
    out[2] = A[0] * B[2] + A[1] * B[5] + A[2] * B[8] + A[3] * B[11];

    // Row 2
    out[3] = A[4] * B[0] + A[5] * B[3] + A[6] * B[6] + A[7] * B[9];
    out[4] = A[4] * B[1] + A[5] * B[4] + A[6] * B[7] + A[7] * B[10];
    out[5] = A[4] * B[2] + A[5] * B[5] + A[6] * B[8] + A[7] * B[11];

    // Row 3
    out[6] = A[8] * B[0] + A[9] * B[3] + A[10] * B[6] + A[11] * B[9];
    out[7] = A[8] * B[1] + A[9] * B[4] + A[10] * B[7] + A[11] * B[10];
    out[8] = A[8] * B[2] + A[9] * B[5] + A[10] * B[8] + A[11] * B[11];

    // Row 4
    out[9] = A[12] * B[0] + A[13] * B[3] + A[14] * B[6] + A[15] * B[9];
    out[10] = A[12] * B[1] + A[13] * B[4] + A[14] * B[7] + A[15] * B[10];
    out[11] = A[12] * B[2] + A[13] * B[5] + A[14] * B[8] + A[15] * B[11];
}


void SPP_SERVICES_KALMAN_mat4x3Mul3x4(const float *restrict A, const float *restrict B,
                                      float *restrict out)
{
    // Row 1
    out[0] = A[0] * B[0] + A[1] * B[4] + A[2] * B[8];
    out[1] = A[0] * B[1] + A[1] * B[5] + A[2] * B[9];
    out[2] = A[0] * B[2] + A[1] * B[6] + A[2] * B[10];
    out[3] = A[0] * B[3] + A[1] * B[7] + A[2] * B[11];

    // Row 2
    out[4] = A[3] * B[0] + A[4] * B[4] + A[5] * B[8];
    out[5] = A[3] * B[1] + A[4] * B[5] + A[5] * B[9];
    out[6] = A[3] * B[2] + A[4] * B[6] + A[5] * B[10];
    out[7] = A[3] * B[3] + A[4] * B[7] + A[5] * B[11];

    // Row 3
    out[8] = A[6] * B[0] + A[7] * B[4] + A[8] * B[8];
    out[9] = A[6] * B[1] + A[7] * B[5] + A[8] * B[9];
    out[10] = A[6] * B[2] + A[7] * B[6] + A[8] * B[10];
    out[11] = A[6] * B[3] + A[7] * B[7] + A[8] * B[11];

    // Row 4
    out[12] = A[9] * B[0] + A[10] * B[4] + A[11] * B[8];
    out[13] = A[9] * B[1] + A[10] * B[5] + A[11] * B[9];
    out[14] = A[9] * B[2] + A[10] * B[6] + A[11] * B[10];
    out[15] = A[9] * B[3] + A[10] * B[7] + A[11] * B[11];
}


void SPP_SERVICES_KALMAN_mat4x3Mul3x3(const float *restrict A, const float *restrict B,
                                      float *restrict out)
{
    // Row 1
    out[0] = A[0] * B[0] + A[1] * B[3] + A[2] * B[6];
    out[1] = A[0] * B[1] + A[1] * B[4] + A[2] * B[7];
    out[2] = A[0] * B[2] + A[1] * B[5] + A[2] * B[8];

    // Row 2
    out[3] = A[3] * B[0] + A[4] * B[3] + A[5] * B[6];
    out[4] = A[3] * B[1] + A[4] * B[4] + A[5] * B[7];
    out[5] = A[3] * B[2] + A[4] * B[5] + A[5] * B[8];

    // Row 3
    out[6] = A[6] * B[0] + A[7] * B[3] + A[8] * B[6];
    out[7] = A[6] * B[1] + A[7] * B[4] + A[8] * B[7];
    out[8] = A[6] * B[2] + A[7] * B[5] + A[8] * B[8];

    // Row 4
    out[9] = A[9] * B[0] + A[10] * B[3] + A[11] * B[6];
    out[10] = A[9] * B[1] + A[10] * B[4] + A[11] * B[7];
    out[11] = A[9] * B[2] + A[10] * B[5] + A[11] * B[8];
}

void SPP_SERVICES_KALMAN_mat4x3Mul3x3diag(const float *restrict A, const float *restrict B,
                                          float *restrict out)
{
    // Row 1
    out[0] = A[0] * B[0];
    out[1] = A[1] * B[1];
    out[2] = A[2] * B[2];

    // Row 2
    out[3] = A[3] * B[0];
    out[4] = A[4] * B[1];
    out[5] = A[5] * B[2];

    // Row 3
    out[6] = A[6] * B[0];
    out[7] = A[7] * B[1];
    out[8] = A[8] * B[2];

    // Row 4
    out[9] = A[9] * B[0];
    out[10] = A[10] * B[1];
    out[11] = A[11] * B[2];
}


void SPP_SERVICES_KALMAN_mat4x3Mul3x1(const float *restrict A, const float *restrict B,
                                      float *restrict out)
{
    // Row 1
    out[0] = A[0] * B[0] + A[1] * B[1] + A[2] * B[2];

    // Row 2
    out[1] = A[3] * B[0] + A[4] * B[1] + A[5] * B[2];

    // Row 3
    out[2] = A[6] * B[0] + A[7] * B[1] + A[8] * B[2];

    // Row 4
    out[3] = A[9] * B[0] + A[10] * B[1] + A[11] * B[2];
}


void SPP_SERVICES_KALMAN_mat3x4Mul4x4(const float *restrict A, const float *restrict B,
                                      float *restrict out)
{
    // Row 1
    out[0] = A[0] * B[0] + A[1] * B[4] + A[2] * B[8] + A[3] * B[12];
    out[1] = A[0] * B[1] + A[1] * B[5] + A[2] * B[9] + A[3] * B[13];
    out[2] = A[0] * B[2] + A[1] * B[6] + A[2] * B[10] + A[3] * B[14];
    out[3] = A[0] * B[3] + A[1] * B[7] + A[2] * B[11] + A[3] * B[15];

    // Row 2
    out[4] = A[4] * B[0] + A[5] * B[4] + A[6] * B[8] + A[7] * B[12];
    out[5] = A[4] * B[1] + A[5] * B[5] + A[6] * B[9] + A[7] * B[13];
    out[6] = A[4] * B[2] + A[5] * B[6] + A[6] * B[10] + A[7] * B[14];
    out[7] = A[4] * B[3] + A[5] * B[7] + A[6] * B[11] + A[7] * B[15];

    // Row 3
    out[8] = A[8] * B[0] + A[9] * B[4] + A[10] * B[8] + A[11] * B[12];
    out[9] = A[8] * B[1] + A[9] * B[5] + A[10] * B[9] + A[11] * B[13];
    out[10] = A[8] * B[2] + A[9] * B[6] + A[10] * B[10] + A[11] * B[14];
    out[11] = A[8] * B[3] + A[9] * B[7] + A[10] * B[11] + A[11] * B[15];
}


void SPP_SERVICES_KALMAN_mat3x4Mul4x3(const float *restrict A, const float *restrict B,
                                      float *restrict out)
{
    // Row 1
    out[0] = A[0] * B[0] + A[1] * B[3] + A[2] * B[6] + A[3] * B[9];
    out[1] = A[0] * B[1] + A[1] * B[4] + A[2] * B[7] + A[3] * B[10];
    out[2] = A[0] * B[2] + A[1] * B[5] + A[2] * B[8] + A[3] * B[11];

    // Row 2
    out[3] = A[4] * B[0] + A[5] * B[3] + A[6] * B[6] + A[7] * B[9];
    out[4] = A[4] * B[1] + A[5] * B[4] + A[6] * B[7] + A[7] * B[10];
    out[5] = A[4] * B[2] + A[5] * B[5] + A[6] * B[8] + A[7] * B[11];

    // Row 3
    out[6] = A[8] * B[0] + A[9] * B[3] + A[10] * B[6] + A[11] * B[9];
    out[7] = A[8] * B[1] + A[9] * B[4] + A[10] * B[7] + A[11] * B[10];
    out[8] = A[8] * B[2] + A[9] * B[5] + A[10] * B[8] + A[11] * B[11];
}


void SPP_SERVICES_KALMAN_mat4x4Transpose(const float *restrict A, float *restrict out)
{
    /* Row 1 */
    out[0] = A[0];
    out[1] = A[4];
    out[2] = A[8];
    out[3] = A[12];

    /* Row 2 */
    out[4] = A[1];
    out[5] = A[5];
    out[6] = A[9];
    out[7] = A[13];

    /* Row 3 */
    out[8] = A[2];
    out[9] = A[6];
    out[10] = A[10];
    out[11] = A[14];

    /* Row 4 */
    out[12] = A[3];
    out[13] = A[7];
    out[14] = A[11];
    out[15] = A[15];
}


void SPP_SERVICES_KALMAN_mat3x4Transpose(const float *restrict A, float *restrict out)
{
    // Row 1
    out[0] = A[0];
    out[1] = A[4];
    out[2] = A[8];

    // Row 2
    out[3] = A[1];
    out[4] = A[5];
    out[5] = A[9];

    // Row 3
    out[6] = A[2];
    out[7] = A[6];
    out[8] = A[10];

    // Row 4
    out[9] = A[3];
    out[10] = A[7];
    out[11] = A[11];
}

void SPP_SERVICES_KALMAN_mat4x3Transpose(const float *restrict A, float *restrict out)
{
    // Row 1
    out[0] = A[0];
    out[1] = A[3];
    out[2] = A[6];
    out[3] = A[9];

    // Row 2
    out[4] = A[1];
    out[5] = A[4];
    out[6] = A[7];
    out[7] = A[10];
    // Row 3

    out[8] = A[2];
    out[9] = A[5];
    out[10] = A[8];
    out[11] = A[11];
}

void SPP_SERVICES_KALMAN_mat3x3Transpose(const float *restrict A, float *restrict out)
{
    // Row 1
    out[0] = A[0];
    out[1] = A[3];
    out[2] = A[6];

    // Row 2
    out[3] = A[1];
    out[4] = A[4];
    out[5] = A[7];

    // Row 3
    out[6] = A[2];
    out[7] = A[5];
    out[8] = A[8];
}


int SPP_SERVICES_KALMAN_mat3x3Inverse(const float *restrict in, float *restrict out)
{
    // Devuelve 1 si se invirtió con éxito, 0 si la matriz es singular (determinante cero)

    // 1. Precalculamos los cofactores de la primera fila
    // (los necesitamos tanto para el determinante como para la salida)
    float c00 = in[4] * in[8] - in[5] * in[7];
    float c01 = in[5] * in[6] - in[3] * in[8];
    float c02 = in[3] * in[7] - in[4] * in[6];

    // 2. Calculamos el determinante
    float det = in[0] * c00 + in[1] * c01 + in[2] * c02;

    // 3. Protección contra singularidad (evitar división por cero o NaNs)
    // Usamos un epsilon pequeño típico para floats de 32 bits
    if (det > -1e-6f && det < 1e-6f)
    {
        return 0; // Fallo: Matriz singular
    }

    // 4. Calculamos la inversa del determinante (una sola división)
    float inv_det = 1.0f / det;

    // 5. Rellenamos la matriz de salida multiplicando la adjunta por inv_det
    // Fila 1
    out[0] = c00 * inv_det;
    out[1] = (in[2] * in[7] - in[1] * in[8]) * inv_det;
    out[2] = (in[1] * in[5] - in[2] * in[4]) * inv_det;

    // Fila 2
    out[3] = c01 * inv_det;
    out[4] = (in[0] * in[8] - in[2] * in[6]) * inv_det;
    out[5] = (in[2] * in[3] - in[0] * in[5]) * inv_det;

    // Fila 3
    out[6] = c02 * inv_det;
    out[7] = (in[1] * in[6] - in[0] * in[7]) * inv_det;
    out[8] = (in[0] * in[4] - in[1] * in[3]) * inv_det;

    return 1; // Éxito
}

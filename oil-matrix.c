#include "oil.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

/* Note: almost all of this code is based on (or taken entirely from) code in
   COGL, the awesome C OpenGL wrapper that's managed by the GNOME
   project. That code was, in turn, based on code from Mesa. Spread the GPL
   love! */

/* swaps two float * variables */
#define SWAP_ROWS(a, b) { float *_tmp = (a); (a) = (b); (b) = _tmp; }

static float identity[4][4] = {{1.0f, 0.0f, 0.0f, 0.0f},
                               {0.0f, 1.0f, 0.0f, 0.0f},
                               {0.0f, 0.0f, 1.0f, 0.0f},
                               {0.0f, 0.0f, 0.0f, 1.0f}};

void oil_matrix_set_identity(OILMatrix *matrix) {
    memcpy(matrix->data, identity, sizeof(float) * 16);
}

/* row-major order: if the first row is (1, 2, ...) the memory will
   start out as 1.0f 2.0f ... */
void oil_matrix_set_data(OILMatrix *matrix, const float *data) {
    memcpy(matrix->data, data, sizeof(float) * 16);
}

int oil_matrix_is_identity(const OILMatrix *matrix)
{
    int i;
    for (i = 0; i < 4; i++) {
        if (matrix->data[i][0] - identity[i][0] != 0.0f ||
            matrix->data[i][1] - identity[i][1] != 0.0f ||
            matrix->data[i][2] - identity[i][2] != 0.0f ||
            matrix->data[i][3] - identity[i][3] != 0.0f)
            
            return 0;
    }
    return 1;
}

int oil_matrix_is_zero(const OILMatrix *matrix)
{
    int i;
    for (i = 0; i < 4; i++) {
        if (matrix->data[i][0] != 0.0f ||
            matrix->data[i][1] != 0.0f ||
            matrix->data[i][2] != 0.0f ||
            matrix->data[i][3] != 0.0f)
            
            return 0;
    }
    return 1;
}

/* result == a is allowed, result == b is not */

void oil_matrix_add(OILMatrix *result, const OILMatrix *a, const OILMatrix *b) {
    int i;
    for (i = 0; i < 4; i++) {
        result->data[i][0] = a->data[i][0] + b->data[i][0];
        result->data[i][1] = a->data[i][1] + b->data[i][1];
        result->data[i][2] = a->data[i][2] + b->data[i][2];
        result->data[i][3] = a->data[i][3] + b->data[i][3];
    }
}

void oil_matrix_subtract(OILMatrix *result, const OILMatrix *a, const OILMatrix *b) {
    int i;
    for (i = 0; i < 4; i++) {
        result->data[i][0] = a->data[i][0] - b->data[i][0];
        result->data[i][1] = a->data[i][1] - b->data[i][1];
        result->data[i][2] = a->data[i][2] - b->data[i][2];
        result->data[i][3] = a->data[i][3] - b->data[i][3];
    }
}

void oil_matrix_multiply(OILMatrix *result, const OILMatrix *a, const OILMatrix *b) {
    int i;
    for (i = 0; i < 4; i++) {
        const float a0 = a->data[i][0], a1 = a->data[i][1];
        const float a2 = a->data[i][2], a3 = a->data[i][3];
        result->data[i][0] = a0 * b->data[0][0] + a1 * b->data[1][0] +
            a2 * b->data[2][0] + a3 * b->data[3][0];
        result->data[i][1] = a0 * b->data[0][1] + a1 * b->data[1][1] +
            a2 * b->data[2][1] + a3 * b->data[3][1];
        result->data[i][2] = a0 * b->data[0][2] + a1 * b->data[1][2] +
            a2 * b->data[2][2] + a3 * b->data[3][2];
        result->data[i][3] = a0 * b->data[0][3] + a1 * b->data[1][3] +
            a2 * b->data[2][3] + a3 * b->data[3][3];
    }
}

void oil_matrix_negate(OILMatrix *result, const OILMatrix *matrix)
{
    int i;
    for (i = 0; i < 4; i++) {
        result->data[i][0] = -(matrix->data[i][0]);
        result->data[i][1] = -(matrix->data[i][1]);
        result->data[i][2] = -(matrix->data[i][2]);
        result->data[i][3] = -(matrix->data[i][3]);
    }
}

/* matrix == inverse is allowed, returns 0 on failure */
int oil_matrix_get_inverse(const OILMatrix* matrix, OILMatrix* inverse) {
    float wtmp[4][8];
    float m0, m1, m2, m3, s;
    float *r0, *r1, *r2, *r3;
    
    r0 = wtmp[0], r1 = wtmp[1], r2 = wtmp[2], r3 = wtmp[3];
    
    memcpy(&(r0[0]), matrix->data[0], sizeof(float) * 4);
    memcpy(&(r0[4]), identity[0], sizeof(float) * 4);

    memcpy(&(r1[0]), matrix->data[1], sizeof(float) * 4);
    memcpy(&(r1[4]), identity[1], sizeof(float) * 4);

    memcpy(&(r2[0]), matrix->data[2], sizeof(float) * 4);
    memcpy(&(r2[4]), identity[2], sizeof(float) * 4);

    memcpy(&(r3[0]), matrix->data[3], sizeof(float) * 4);
    memcpy(&(r3[4]), identity[3], sizeof(float) * 4);
    
    /* choose pivot or die */
    if (fabsf(r3[0]) > fabsf(r2[0]))
        SWAP_ROWS(r3, r2);
    if (fabsf(r2[0]) > fabsf(r1[0]))
        SWAP_ROWS(r2, r1);
    if (fabsf(r1[0]) > fabsf(r0[0]))
        SWAP_ROWS(r1, r0);
    if (r0[0] == 0.0f)
        return 0;
    
    /* eliminate first variable */
    
    m1 = r1[0] / r0[0];
    m2 = r2[0] / r0[0];
    m3 = r3[0] / r0[0];
    
    s = r0[1];
    r1[1] -= m1 * s;
    r2[1] -= m2 * s;
    r3[1] -= m3 * s;
    s = r0[2];
    r1[2] -= m1 * s;
    r2[2] -= m2 * s;
    r3[2] -= m3 * s;
    s = r0[3];
    r1[3] -= m1 * s;
    r2[3] -= m2 * s;
    r3[3] -= m3 * s;
    
    s = r0[4];
    if (s != 0.0f) {
        r1[4] -= m1 * s;
        r2[4] -= m2 * s;
        r3[4] -= m3 * s;
    }
    s = r0[5];
    if (s != 0.0f) {
        r1[5] -= m1 * s;
        r2[5] -= m2 * s;
        r3[5] -= m3 * s;
    }
    s = r0[6];
    if (s != 0.0f) {
        r1[6] -= m1 * s;
        r2[6] -= m2 * s;
        r3[6] -= m3 * s;
    }
    s = r0[7];
    if (s != 0.0f) {
        r1[7] -= m1 * s;
        r2[7] -= m2 * s;
        r3[7] -= m3 * s;
    }
    
    /* choose pivot or die */
    if (fabsf(r3[1]) > fabsf(r2[1]))
        SWAP_ROWS(r3, r2);
    if (fabsf(r2[1]) > fabsf(r1[1]))
        SWAP_ROWS(r2, r1);
    if (r1[1] == 0.0f)
        return 0;
    
    /* eliminate second variable */
    
    m2 = r2[1] / r1[1];
    m3 = r3[1] / r1[1];
    
    s = r1[2];
    r2[2] -= m2 * s;
    r3[2] -= m3 * s;
    s = r1[3];
    r2[3] -= m2 * s;
    r3[3] -= m3 * s;
    
    s = r1[4];
    if (s != 0.0f) {
        r2[4] -= m2 * s;
        r3[4] -= m3 * s;
    }
    s = r1[5];
    if (s != 0.0f) {
        r2[5] -= m2 * s;
        r3[5] -= m3 * s;
    }
    s = r1[6];
    if (s != 0.0f) {
        r2[6] -= m2 * s;
        r3[6] -= m3 * s;
    }
    s = r1[7];
    if (s != 0.0f) {
        r2[7] -= m2 * s;
        r3[7] -= m3 * s;
    }
    
    /* choose pivot or die */
    if (fabsf(r3[2]) > fabsf(r2[2]))
        SWAP_ROWS(r3, r2);
    if (r2[2] == 0.0f)
        return 0;
    
    /* eliminate third variable */
    m3 = r3[2] / r2[2];
    r3[3] -= m3 * r2[3];
    r3[4] -= m3 * r2[4];
    r3[5] -= m3 * r2[5];
    r3[6] -= m3 * r2[6];
    r3[7] -= m3 * r2[7];
    
    /* last check */
    if (r3[3] == 0.0f)
        return 0;
    
    /* now back substitute row 3 */
    s = 1.0f / r3[3];
    r3[4] *= s;
    r3[5] *= s;
    r3[6] *= s;
    r3[7] *= s;
    
    /* back substitute row 2 */
    m2 = r2[3];
    s = 1.0f / r2[2];
    r2[4] = s * (r2[4] - r3[4] * m2);
    r2[5] = s * (r2[5] - r3[5] * m2);
    r2[6] = s * (r2[6] - r3[6] * m2);
    r2[7] = s * (r2[7] - r3[7] * m2);
    m1 = r1[3];
    r1[4] -= r3[4] * m1;
    r1[5] -= r3[5] * m1;
    r1[6] -= r3[6] * m1;
    r1[7] -= r3[7] * m1;
    m0 = r0[3];
    r0[4] -= r3[4] * m0;
    r0[5] -= r3[5] * m0;
    r0[6] -= r3[6] * m0;
    r0[7] -= r3[7] * m0;
    
    /* back substitute row 1 */
    m1 = r1[2];
    s = 1.0f / r1[1];
    r1[4] = s * (r1[4] - r2[4] * m1);
    r1[5] = s * (r1[5] - r2[5] * m1);
    r1[6] = s * (r1[6] - r2[6] * m1);
    r1[7] = s * (r1[7] - r2[7] * m1);
    m0 = r0[2];
    r0[4] -= r2[4] * m0;
    r0[5] -= r2[5] * m0;
    r0[6] -= r2[6] * m0;
    r0[7] -= r2[7] * m0;
    
    /* back substitute row 0 */
    m0 = r0[1];
    s = 1.0f / r0[0];
    r0[4] = s * (r0[4] - r1[4] * m0);
    r0[5] = s * (r0[5] - r1[5] * m0);
    r0[6] = s * (r0[6] - r1[6] * m0);
    r0[7] = s * (r0[7] - r1[7] * m0);
    
    /* copy to output */
    memcpy(inverse->data[0], &(r0[4]), sizeof(float) * 4);
    memcpy(inverse->data[1], &(r1[4]), sizeof(float) * 4);
    memcpy(inverse->data[2], &(r2[4]), sizeof(float) * 4);
    memcpy(inverse->data[3], &(r3[4]), sizeof(float) * 4);
    
    return 1;
}
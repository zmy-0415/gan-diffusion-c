#include "linalg.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

// 生成高斯随机数
static float random_normal(float mean, float std) {
    static int has_spare = 0;
    static float spare;

    if (has_spare) {
        has_spare = 0;
        return mean + std * spare;
    }

    has_spare = 1;
    float u = (float)rand() / RAND_MAX;
    float v = (float)rand() / RAND_MAX;
    float s = sqrt(-2.0f * log(u));
    spare = s * sin(2.0f * M_PI * v);
    return mean + std * (s * cos(2.0f * M_PI * v));
}

Matrix* matrix_create(size_t rows, size_t cols) {
    Matrix *m = (Matrix *)malloc(sizeof(Matrix));
    m->rows = rows;
    m->cols = cols;
    m->data = (float *)calloc(rows * cols, sizeof(float));
    return m;
}

void matrix_free(Matrix *m) {
    if (m) {
        free(m->data);
        free(m);
    }
}

void matrix_fill(Matrix *m, float value) {
    for (size_t i = 0; i < m->rows * m->cols; i++) {
        m->data[i] = value;
    }
}

void matrix_random_normal(Matrix *m, float mean, float std) {
    for (size_t i = 0; i < m->rows * m->cols; i++) {
        m->data[i] = random_normal(mean, std);
    }
}
...
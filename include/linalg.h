#ifndef LINALG_H
#define LINALG_H

#include <stddef.h>
#include <math.h>

typedef struct {
    float *data;
    size_t rows;
    size_t cols;
} Matrix;

typedef struct {
    float *data;
    size_t size;
} Vector;

// 矩阵操作
Matrix* matrix_create(size_t rows, size_t cols);
void matrix_free(Matrix *m);
Matrix* matrix_copy(Matrix *src);
void matrix_fill(Matrix *m, float value);
void matrix_random_normal(Matrix *m, float mean, float std);
void matrix_zeros(Matrix *m);

// 矩阵运算
void matrix_add(Matrix *a, Matrix *b, Matrix *result);
void matrix_sub(Matrix *a, Matrix *b, Matrix *result);
void matrix_matmul(Matrix *a, Matrix *b, Matrix *result);
void matrix_transpose(Matrix *m, Matrix *result);
void matrix_scale(Matrix *m, float scalar);
void matrix_element_wise_mul(Matrix *a, Matrix *b, Matrix *result);

// 激活函数
void relu(Matrix *m);
void sigmoid(Matrix *m);
void tanh_activation(Matrix *m);
void softmax_rows(Matrix *m);

// 损失函数
float binary_cross_entropy(Matrix *pred, Matrix *target);
float mse_loss(Matrix *pred, Matrix *target);

#endif
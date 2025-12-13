#ifndef NN_H
#define NN_H

#include "linalg.h"

typedef enum {
    RELU,
    SIGMOID,
    TANH,
    LINEAR
} ActivationType;

typedef struct {
    Matrix *weights;  // [input_dim, output_dim]
    Vector *bias;
    ActivationType activation;
} DenseLayer;

// 全连接层
DenseLayer* dense_create(size_t input_dim, size_t output_dim, ActivationType act);
void dense_forward(DenseLayer *layer, Matrix *input, Matrix *output);
void dense_backward(DenseLayer *layer, Matrix *grad_output, Matrix *grad_input, 
                   float learning_rate);
void dense_free(DenseLayer *layer);

#endif
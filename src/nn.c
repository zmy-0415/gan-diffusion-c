#include "nn.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

DenseLayer* dense_create(size_t input_dim, size_t output_dim, ActivationType act) {
    DenseLayer *layer = (DenseLayer *)malloc(sizeof(DenseLayer));
    layer->weights = matrix_create(input_dim, output_dim);
    layer->bias = vector_create(output_dim);
    layer->activation = act;
    
    // Xavier初始化权重
    float limit = sqrtf(6.0f / (input_dim + output_dim));
    for (size_t i = 0; i < input_dim * output_dim; i++) {
        layer->weights->data[i] = (float)rand() / RAND_MAX * 2 * limit - limit;
    }
    
    vector_fill(layer->bias, 0.0f);
    return layer;
}

void dense_forward(DenseLayer *layer, Matrix *input, Matrix *output) {
    // output = input @ weights + bias
    matrix_matmul(input, layer->weights, output);
    
    // 加上偏置
    for (size_t i = 0; i < output->rows; i++) {
        for (size_t j = 0; j < output->cols; j++) {
            output->data[i * output->cols + j] += layer->bias->data[j];
        }
    }
    
    // 应用激活函数
    switch (layer->activation) {
        case RELU:
            relu(output);
            break;
        case SIGMOID:
            sigmoid(output);
            break;
        case TANH:
            tanh_activation(output);
            break;
        case LINEAR:
            break;
    }
}

void dense_backward(DenseLayer *layer, Matrix *grad_output, Matrix *grad_input,
                   float learning_rate) {
    // 简化的反向传播（不完整，用于演示）
    // 在实际应用中需要更复杂的实现
    
    Matrix *weights_T = matrix_create(layer->weights->cols, layer->weights->rows);
    matrix_transpose(layer->weights, weights_T);
    matrix_matmul(grad_output, weights_T, grad_input);
    matrix_free(weights_T);
}

void dense_free(DenseLayer *layer) {
    if (layer) {
        matrix_free(layer->weights);
        vector_free(layer->bias);
        free(layer);
    }
}
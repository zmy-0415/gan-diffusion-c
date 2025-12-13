#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "linalg.h"

typedef enum {
    SGD,
    ADAM,
    RMSPROP
} OptimizerType;

typedef struct {
    OptimizerType type;
    float learning_rate;
    float momentum;      // for SGD and ADAM
    float beta1;         // ADAM
    float beta2;         // ADAM
    float epsilon;
    
    // Adam状态
    Matrix *m;          // 一阶矩估计
    Matrix *v;          // 二阶矩估计
    int t;              // 时间步
} Optimizer;

Optimizer* optimizer_create(OptimizerType type, float lr);
void optimizer_free(Optimizer *opt);

void optimizer_step(Optimizer *opt, Matrix *params, Matrix *gradients);

#endif
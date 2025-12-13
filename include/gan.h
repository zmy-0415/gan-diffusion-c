#ifndef GAN_H
#define GAN_H

#include "linalg. h"
#include "nn.h"

typedef struct {
    // 生成器
    DenseLayer *gen_layer1;
    DenseLayer *gen_layer2;
    DenseLayer *gen_layer3;
    
    // 判别器
    DenseLayer *disc_layer1;
    DenseLayer *disc_layer2;
    DenseLayer *disc_layer3;
    
    int latent_dim;
    int output_dim;
} GANModel;

typedef struct {
    float gen_loss;
    float disc_loss;
} TrainingMetrics;

GANModel* gan_create(int latent_dim, int output_dim);
void gan_free(GANModel *gan);

void generator_forward(GANModel *gan, Matrix *noise, Matrix *fake_data);
void discriminator_forward(GANModel *gan, Matrix *data, Matrix *logits);

TrainingMetrics gan_train_step(GANModel *gan, Matrix *real_data, float learning_rate);

void gan_generate_samples(GANModel *gan, int num_samples, int sample_dim, Matrix *samples);

#endif
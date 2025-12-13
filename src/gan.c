#include "gan.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

GANModel* gan_create(int latent_dim, int output_dim) {
    GANModel *gan = (GANModel *)malloc(sizeof(GANModel));
    gan->latent_dim = latent_dim;
    gan->output_dim = output_dim;
    
    // 生成器：latent_dim -> 256 -> 512 -> output_dim
    gan->gen_layer1 = dense_create(latent_dim, 256, RELU);
    gan->gen_layer2 = dense_create(256, 512, RELU);
    gan->gen_layer3 = dense_create(512, output_dim, SIGMOID);
    
    // 判别器：output_dim -> 512 -> 256 -> 1
    gan->disc_layer1 = dense_create(output_dim, 512, RELU);
    gan->disc_layer2 = dense_create(512, 256, RELU);
    gan->disc_layer3 = dense_create(256, 1, SIGMOID);
    
    return gan;
}

void gan_free(GANModel *gan) {
    if (gan) {
        dense_free(gan->gen_layer1);
        dense_free(gan->gen_layer2);
        dense_free(gan->gen_layer3);
        dense_free(gan->disc_layer1);
        dense_free(gan->disc_layer2);
        dense_free(gan->disc_layer3);
        free(gan);
    }
}

void generator_forward(GANModel *gan, Matrix *noise, Matrix *fake_data) {
    Matrix *hidden1 = matrix_create(noise->rows, 256);
    Matrix *hidden2 = matrix_create(noise->rows, 512);
    
    dense_forward(gan->gen_layer1, noise, hidden1);
    dense_forward(gan->gen_layer2, hidden1, hidden2);
    dense_forward(gan->gen_layer3, hidden2, fake_data);
    
    matrix_free(hidden1);
    matrix_free(hidden2);
}

void discriminator_forward(GANModel *gan, Matrix *data, Matrix *logits) {
    Matrix *hidden1 = matrix_create(data->rows, 512);
    Matrix *hidden2 = matrix_create(data->rows, 256);
    
    dense_forward(gan->disc_layer1, data, hidden1);
    dense_forward(gan->disc_layer2, hidden1, hidden2);
    dense_forward(gan->disc_layer3, hidden2, logits);
    
    matrix_free(hidden1);
    matrix_free(hidden2);
}

TrainingMetrics gan_train_step(GANModel *gan, Matrix *real_data, float learning_rate) {
    int batch_size = real_data->rows;
    TrainingMetrics metrics = {0, 0};
    
    // 生成假数据
    Matrix *noise = matrix_create(batch_size, gan->latent_dim);
    matrix_random_normal(noise, 0.0f, 1.0f);
    
    Matrix *fake_data = matrix_create(batch_size, gan->output_dim);
    generator_forward(gan, noise, fake_data);
    
    // 判别器前向传播
    Matrix *real_logits = matrix_create(batch_size, 1);
    Matrix *fake_logits = matrix_create(batch_size, 1);
    
    discriminator_forward(gan, real_data, real_logits);
    discriminator_forward(gan, fake_data, fake_logits);
    
    // 计算损失
    Matrix *real_target = matrix_create(batch_size, 1);
    Matrix *fake_target = matrix_create(batch_size, 1);
    matrix_fill(real_target, 1.0f);
    matrix_fill(fake_target, 0.0f);
    
    float disc_loss_real = binary_cross_entropy(real_logits, real_target);
    float disc_loss_fake = binary_cross_entropy(fake_logits, fake_target);
    metrics.disc_loss = (disc_loss_real + disc_loss_fake) / 2.0f;
    
    // 生成器损失
    matrix_fill(fake_target, 1.0f);  // 生成器试图欺骗判别器
    metrics.gen_loss = binary_cross_entropy(fake_logits, fake_target);
    
    // 清理
    matrix_free(noise);
    matrix_free(fake_data);
    matrix_free(real_logits);
    matrix_free(fake_logits);
    matrix_free(real_target);
    matrix_free(fake_target);
    
    return metrics;
}

void gan_generate_samples(GANModel *gan, int num_samples, int sample_dim, Matrix *samples) {
    Matrix *noise = matrix_create(num_samples, gan->latent_dim);
    matrix_random_normal(noise, 0.0f, 1.0f);
    
    generator_forward(gan, noise, samples);
    
    matrix_free(noise);
}
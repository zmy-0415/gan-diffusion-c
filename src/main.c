#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "linalg.h"
#include "nn.h"
#include "gan.h"

int main() {
    srand((unsigned int)time(NULL));
    
    printf("╔════════════════════════════════════════════╗\n");
    printf("║   GAN Diffusion Model in C - Executable   ║\n");
    printf("╚════════════════════════════════════════════╝\n\n");
    
    // 配置参数
    int latent_dim = 100;
    int output_dim = 784;      // 28x28图像
    int batch_size = 32;
    int num_epochs = 50;
    float learning_rate = 0.0002f;
    
    printf("📊 训练配置:\n");
    printf("  - 隐空间维度: %d\n", latent_dim);
    printf("  - 输出维度: %d\n", output_dim);
    printf("  - 批大小: %d\n", batch_size);
    printf("  - 训练轮数: %d\n", num_epochs);
    printf("  - 学习率: %.6f\n\n", learning_rate);
    
    printf("🔧 初始化GAN模型...\n");
    GANModel *gan = gan_create(latent_dim, output_dim);
    printf("✓ 模型初始化完成\n\n");
    
    printf("🚀 开始训练...\n");
    printf("─────────────────────────────────────────\n");
    
    float avg_gen_loss = 0, avg_disc_loss = 0;
    
    // 训练循环
    for (int epoch = 0; epoch < num_epochs; epoch++) {
        // 创建模拟的真实数据
        Matrix *real_data = matrix_create(batch_size, output_dim);
        matrix_random_normal(real_data, 0.5f, 0.2f);
        
        // 裁剪到[0, 1]范围
        for (size_t i = 0; i < batch_size * output_dim; i++) {
            if (real_data->data[i] < 0) real_data->data[i] = 0;
            if (real_data->data[i] > 1) real_data->data[i] = 1;
        }
        
        // 训练步骤
        TrainingMetrics metrics = gan_train_step(gan, real_data, learning_rate);
        
        avg_gen_loss = 0.9f * avg_gen_loss + 0.1f * metrics.gen_loss;
        avg_disc_loss = 0.9f * avg_disc_loss + 0.1f * metrics.disc_loss;
        
        // 定期输出进度
        if ((epoch + 1) % 10 == 0) {
            printf("轮次 [%3d/%d] | 生成器损失:  %.6f | 判别器损失: %.6f\n",
                   epoch + 1, num_epochs, avg_gen_loss, avg_disc_loss);
        }
        
        matrix_free(real_data);
    }
    
    printf("─────────────────────────────────────────\n");
    printf("✓ 训练完成!\n\n");
    
    printf("🎨 生成样本...\n");
    Matrix *generated_samples = matrix_create(16, output_dim);
    gan_generate_samples(gan, 16, output_dim, generated_samples);
    
    printf("生成的样本统计:\n");
    float min_val = generated_samples->data[0];
    float max_val = generated_samples->data[0];
    float mean_val = 0;
    
    for (size_t i = 0; i < 16 * output_dim; i++) {
        if (generated_samples->data[i] < min_val) min_val = generated_samples->data[i];
        if (generated_samples->data[i] > max_val) max_val = generated_samples->data[i];
        mean_val += generated_samples->data[i];
    }
    mean_val /= (16 * output_dim);
    
    printf("  - 最小值: %.6f\n", min_val);
    printf("  - 最大值: %.6f\n", max_val);
    printf("  - 平均值:  %.6f\n", mean_val);
    printf("  - 生成样本数: 16\n");
    printf("  - 每个样本维度: %d\n\n", output_dim);
    
    printf("💾 保存样本...\n");
    FILE *f = fopen("generated_samples. csv", "w");
    if (f) {
        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < output_dim; j++) {
                fprintf(f, "%.6f", generated_samples->data[i * output_dim + j]);
                if (j < output_dim - 1) fprintf(f, ",");
            }
            fprintf(f, "\n");
        }
        fclose(f);
        printf("✓ 样本已保存到 'generated_samples.csv'\n\n");
    }
    
    // 清理内存
    printf("🧹 清理资源...\n");
    matrix_free(generated_samples);
    gan_free(gan);
    printf("✓ 资源清理完成\n\n");
    
    printf("╔════════════════════════════════════════════╗\n");
    printf("║            训练成功完成!  🎉              ║\n");
    printf("╚════════════════════════════════════════════╝\n");
    
    return 0;
}
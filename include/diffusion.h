#ifndef DIFFUSION_H
#define DIFFUSION_H

#include "linalg. h"
#include "nn. h"

typedef enum {
    LINEAR,
    COSINE,
    SIGMOID_SCHEDULE
} NoiseSchedule;

typedef struct {
    // 扩散参数
    int num_timesteps;
    NoiseSchedule schedule;
    float beta_start;
    float beta_end;
    
    // 预计算的参数
    Vector *betas;           // 噪声方差
    Vector *alphas;          // 1 - beta
    Vector *alphas_cumprod;  // 累积乘积 alpha
    Vector *sqrt_alphas_cumprod;
    Vector *sqrt_1m_alphas_cumprod;
    Vector *sqrt_recip_alphas_cumprod;
    Vector *sqrt_recip_alphas_cumprod_m1;
    
    // 模型
    DenseLayer *time_emb;    // 时间步嵌入
    DenseLayer *denoiser1;   // 去噪网络层1
    DenseLayer *denoiser2;   // 去噪网络层2
} DiffusionModel;

// 创建和销毁
DiffusionModel* diffusion_create(int num_timesteps, int input_dim, NoiseSchedule schedule);
void diffusion_free(DiffusionModel *model);

// 前向扩散（添加噪声）
void forward_diffusion(DiffusionModel *model, Matrix *x0, int t, Matrix *xt, Matrix *noise);

// 反向扩散（去噪）
void reverse_diffusion(DiffusionModel *model, Matrix *xt, int t, Matrix *xt_minus_1);

// 采样
void sample(DiffusionModel *model, int num_samples, int sample_dim, Matrix *samples);

// 损失计算
float compute_diffusion_loss(DiffusionModel *model, Matrix *x0, Matrix *predicted_noise, int t);

#endif
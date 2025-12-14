#ifndef ANIMATION_MANAGER_H
#define ANIMATION_MANAGER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// 前置声明（兼容 ImageManager）
struct ImageManager;
typedef struct ImageManager ImageManager;

// 单个动画帧信息
typedef struct AnimationFrame {
    SDL_Rect rect;  // 帧在精灵图中的矩形区域
} AnimationFrame;

// 动画序列（绑定索引序列）
typedef struct AnimationClip {
    char* name;             // 动画名称（如 "idle" "walk"）
    int* frame_indices;     // 帧索引序列（支持间隔，如 [0,2,4,6]）
    int frame_count;        // 索引序列长度
    float frame_duration;   // 每帧持续时间（秒）
    bool loop;              // 是否循环
    bool reverse;           // 是否反向播放
} AnimationClip;

// 动画对象（对应一张精灵图）
typedef struct Animation {
    char* texture_key;      // 关联的 ImageManager 纹理key
    SDL_Texture* texture;   // 精灵图纹理
    int rows;               // 精灵图行数
    int cols;               // 精灵图列数
    int total_frames;       // 总帧数（rows*cols）
    AnimationFrame* frames; // 所有帧的矩形缓存
    AnimationClip** clips;  // 动画序列数组
    int clip_count;         // 动画序列数量
    // 播放状态
    char* current_clip;     // 当前播放的动画序列名称
    float elapsed_time;     // 已播放时间
    int current_index;      // 当前播放到序列的索引位置
    bool is_playing;        // 是否播放中
    float speed;            // 播放速度（1.0为正常）
} Animation;

// 动画管理器（管理多个动画对象）
typedef struct AnimationManager {
    Animation** animations; // 动画对象数组
    int animation_count;    // 动画对象数量
    ImageManager* img_manager; // 关联的图像管理器
    SDL_Renderer* renderer; // 渲染器
} AnimationManager;

// ========== 核心接口 ==========
// 1. 创建动画管理器（关联 ImageManager 和渲染器）
AnimationManager* AnimationManager_Create(ImageManager* img_manager, SDL_Renderer* renderer);

// 2. 加载精灵图并创建动画对象（按行列分割）
Animation* AnimationManager_LoadAnimation(
    AnimationManager* manager,
    const char* anim_key,       // 动画唯一标识
    const char* texture_key,    // ImageManager 中的纹理key
    int rows,                   // 精灵图行数
    int cols                    // 精灵图列数
);

// 3. 为动画添加序列（自定义索引序列）
bool AnimationManager_AddClip(
    Animation* anim,
    const char* clip_name,      // 序列名称
    int* frame_indices,         // 帧索引数组（如 {0,1,2,3} 或 {0,2,4,6}）
    int frame_count,            // 索引数组长度
    float frame_duration,       // 每帧持续时间（秒）
    bool loop,                  // 是否循环
    bool reverse                // 是否反向播放
);

// 4. 更新动画（需在主循环中调用，传入deltaTime）
void AnimationManager_Update(AnimationManager* manager, float dt);

// 5. 绘制当前动画帧
void AnimationManager_Draw(
    AnimationManager* manager,
    const char* anim_key,       // 动画标识
    int x, int y,               // 绘制位置
    int w, int h,               // 绘制尺寸（0为原始尺寸）
    float scale,                // 缩放比例（1.0为原始）
    float rotation,             // 旋转角度（弧度）
    SDL_RendererFlip flip       // 翻转模式
);

// 6. 控制动画播放
void AnimationManager_Play(AnimationManager* manager, const char* anim_key, const char* clip_name);
void AnimationManager_Pause(AnimationManager* manager, const char* anim_key);
void AnimationManager_Resume(AnimationManager* manager, const char* anim_key);
void AnimationManager_SetSpeed(AnimationManager* manager, const char* anim_key, float speed);

// 7. 销毁动画对象/管理器
void AnimationManager_DestroyAnimation(AnimationManager* manager, const char* anim_key);
void AnimationManager_Destroy(AnimationManager* manager);

#endif // ANIMATION_MANAGER_H

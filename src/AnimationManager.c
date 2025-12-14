#include "AnimationManager.h"
#include "ImageManager.h"

// ========== 内部辅助函数 ==========
// 查找动画对象
static Animation* find_animation(AnimationManager* manager, const char* anim_key) {
    if (!manager || !anim_key) return NULL;
    for (int i = 0; i < manager->animation_count; i++) {
        if (strcmp(manager->animations[i]->texture_key, anim_key) == 0) {
            return manager->animations[i];
        }
    }
    return NULL;
}

// 查找动画序列
static AnimationClip* find_clip(Animation* anim, const char* clip_name) {
    if (!anim || !clip_name) return NULL;
    for (int i = 0; i < anim->clip_count; i++) {
        if (strcmp(anim->clips[i]->name, clip_name) == 0) {
            return anim->clips[i];
        }
    }
    return NULL;
}

// 计算精灵图单帧矩形
static void calculate_frame_rects(Animation* anim) {
    if (!anim || !anim->texture) return;

    // 获取纹理尺寸
    int tex_w, tex_h;
    SDL_QueryTexture(anim->texture, NULL, NULL, &tex_w, &tex_h);

    // 计算单帧尺寸
    int frame_w = tex_w / anim->cols;
    int frame_h = tex_h / anim->rows;

    // 缓存所有帧的矩形
    anim->total_frames = anim->rows * anim->cols;
    anim->frames = (AnimationFrame*)malloc(sizeof(AnimationFrame) * anim->total_frames);
    if (!anim->frames) {
        fprintf(stderr, "AnimationManager: Failed to allocate frames\n");
        return;
    }

    for (int row = 0; row < anim->rows; row++) {
        for (int col = 0; col < anim->cols; col++) {
            int idx = row * anim->cols + col;
            anim->frames[idx].rect = (SDL_Rect){
                col * frame_w,
                row * frame_h,
                frame_w,
                frame_h
            };
        }
    }
}

// ========== 核心接口实现 ==========
AnimationManager* AnimationManager_Create(ImageManager* img_manager, SDL_Renderer* renderer) {
    if (!img_manager || !renderer) {
        fprintf(stderr, "AnimationManager: Invalid img_manager or renderer\n");
        return NULL;
    }

    AnimationManager* manager = (AnimationManager*)malloc(sizeof(AnimationManager));
    if (!manager) {
        fprintf(stderr, "AnimationManager: Failed to allocate manager\n");
        return NULL;
    }

    manager->animations = NULL;
    manager->animation_count = 0;
    manager->img_manager = img_manager;
    manager->renderer = renderer;

    return manager;
}

Animation* AnimationManager_LoadAnimation(
    AnimationManager* manager,
    const char* anim_key,
    const char* texture_key,
    int rows,
    int cols
) {
    if (!manager || !anim_key || !texture_key || rows <= 0 || cols <= 0) {
        fprintf(stderr, "AnimationManager: Invalid params for LoadAnimation\n");
        return NULL;
    }

    // 检查是否已加载
    if (find_animation(manager, anim_key)) {
        fprintf(stderr, "AnimationManager: Animation '%s' already loaded\n", anim_key);
        return find_animation(manager, anim_key);
    }

    // 从 ImageManager 获取纹理
    SDL_Texture* texture = ImageManager_GetTexture(manager->img_manager, texture_key);
    if (!texture) {
        fprintf(stderr, "AnimationManager: Texture '%s' not found in ImageManager\n", texture_key);
        return NULL;
    }

    // 创建动画对象
    Animation* anim = (Animation*)malloc(sizeof(Animation));
    if (!anim) {
        fprintf(stderr, "AnimationManager: Failed to allocate animation\n");
        return NULL;
    }

    anim->texture_key = (char*)malloc(strlen(anim_key) + 1);
    strcpy(anim->texture_key, anim_key);
    anim->texture = texture;
    anim->rows = rows;
    anim->cols = cols;
    anim->total_frames = 0;
    anim->frames = NULL;
    anim->clips = NULL;
    anim->clip_count = 0;
    anim->current_clip = NULL;
    anim->elapsed_time = 0.0f;
    anim->current_index = 0;
    anim->is_playing = false;
    anim->speed = 1.0f;

    // 计算帧矩形
    calculate_frame_rects(anim);

    // 添加到管理器
    manager->animations = (Animation**)realloc(
        manager->animations,
        sizeof(Animation*) * (manager->animation_count + 1)
    );
    manager->animations[manager->animation_count++] = anim;

    printf("AnimationManager: Animation '%s' loaded (rows: %d, cols: %d)\n", anim_key, rows, cols);
    return anim;
}

bool AnimationManager_AddClip(
    Animation* anim,
    const char* clip_name,
    int* frame_indices,
    int frame_count,
    float frame_duration,
    bool loop,
    bool reverse
) {
    if (!anim || !clip_name || !frame_indices || frame_count <= 0 || frame_duration <= 0) {
        fprintf(stderr, "AnimationManager: Invalid params for AddClip\n");
        return false;
    }

    // 检查序列是否已存在
    if (find_clip(anim, clip_name)) {
        fprintf(stderr, "AnimationManager: Clip '%s' already exists\n", clip_name);
        return false;
    }

    // 验证索引有效性
    for (int i = 0; i < frame_count; i++) {
        if (frame_indices[i] < 0 || frame_indices[i] >= anim->total_frames) {
            fprintf(stderr, "AnimationManager: Invalid frame index %d (total: %d)\n", frame_indices[i], anim->total_frames);
            return false;
        }
    }

    // 创建序列
    AnimationClip* clip = (AnimationClip*)malloc(sizeof(AnimationClip));
    if (!clip) {
        fprintf(stderr, "AnimationManager: Failed to allocate clip\n");
        return false;
    }

    clip->name = (char*)malloc(strlen(clip_name) + 1);
    strcpy(clip->name, clip_name);
    clip->frame_indices = (int*)malloc(sizeof(int) * frame_count);
    memcpy(clip->frame_indices, frame_indices, sizeof(int) * frame_count);
    clip->frame_count = frame_count;
    clip->frame_duration = frame_duration;
    clip->loop = loop;
    clip->reverse = reverse;

    // 添加到动画
    anim->clips = (AnimationClip**)realloc(
        anim->clips,
        sizeof(AnimationClip*) * (anim->clip_count + 1)
    );
    anim->clips[anim->clip_count++] = clip;

    printf("AnimationManager: Clip '%s' added to animation (frames: %d)\n", clip_name, frame_count);
    return true;
}

void AnimationManager_Update(AnimationManager* manager, float dt) {
    if (!manager || dt <= 0) return;

    for (int i = 0; i < manager->animation_count; i++) {
        Animation* anim = manager->animations[i];
        if (!anim || !anim->is_playing || !anim->current_clip) continue;

        AnimationClip* clip = find_clip(anim, anim->current_clip);
        if (!clip) continue;

        // 更新已播放时间
        anim->elapsed_time += dt * anim->speed;

        // 计算是否切换帧
        float frame_time = clip->frame_duration;
        if (anim->elapsed_time >= frame_time) {
            anim->elapsed_time -= frame_time;

            // 更新序列索引
            if (clip->reverse) {
                anim->current_index--;
                if (anim->current_index < 0) {
                    if (clip->loop) {
                        anim->current_index = clip->frame_count - 1;
                    } else {
                        anim->current_index = 0;
                        anim->is_playing = false;
                    }
                }
            } else {
                anim->current_index++;
                if (anim->current_index >= clip->frame_count) {
                    if (clip->loop) {
                        anim->current_index = 0;
                    } else {
                        anim->current_index = clip->frame_count - 1;
                        anim->is_playing = false;
                    }
                }
            }
        }
    }
}

void AnimationManager_Draw(
    AnimationManager* manager,
    const char* anim_key,
    int x, int y,
    int w, int h,
    float scale,
    float rotation,
    SDL_RendererFlip flip
) {
    if (!manager || !anim_key) return;

    Animation* anim = find_animation(manager, anim_key);
    if (!anim || !anim->current_clip || !anim->frames) return;

    AnimationClip* clip = find_clip(anim, anim->current_clip);
    if (!clip || anim->current_index >= clip->frame_count) return;

    // 获取当前帧索引和矩形
    int frame_idx = clip->frame_indices[anim->current_index];
    SDL_Rect* src_rect = &anim->frames[frame_idx].rect;

    // 计算绘制尺寸
    SDL_Rect dst_rect;
    if (w == 0 || h == 0) {
        dst_rect.w = src_rect->w * scale;
        dst_rect.h = src_rect->h * scale;
    } else {
        dst_rect.w = w;
        dst_rect.h = h;
    }
    dst_rect.x = x - dst_rect.w / 2; // 居中绘制
    dst_rect.y = y - dst_rect.h / 2;

    // 绘制
    SDL_RenderCopyEx(
        manager->renderer,
        anim->texture,
        src_rect,
        &dst_rect,
        rotation * 180 / M_PI, // 转角度
        NULL,                  // 旋转中心（居中）
        flip
    );
}

void AnimationManager_Play(AnimationManager* manager, const char* anim_key, const char* clip_name) {
    if (!manager || !anim_key || !clip_name) return;

    Animation* anim = find_animation(manager, anim_key);
    if (!anim) {
        fprintf(stderr, "AnimationManager: Animation '%s' not found\n", anim_key);
        return;
    }

    AnimationClip* clip = find_clip(anim, clip_name);
    if (!clip) {
        fprintf(stderr, "AnimationManager: Clip '%s' not found in animation '%s'\n", clip_name, anim_key);
        return;
    }

    // 重置播放状态
    anim->current_clip = clip->name;
    anim->elapsed_time = 0.0f;
    anim->current_index = clip->reverse ? clip->frame_count - 1 : 0;
    anim->is_playing = true;

    printf("AnimationManager: Playing '%s' -> '%s'\n", anim_key, clip_name);
}

void AnimationManager_Pause(AnimationManager* manager, const char* anim_key) {
    if (!manager || !anim_key) return;

    Animation* anim = find_animation(manager, anim_key);
    if (anim) anim->is_playing = false;
}

void AnimationManager_Resume(AnimationManager* manager, const char* anim_key) {
    if (!manager || !anim_key) return;

    Animation* anim = find_animation(manager, anim_key);
    if (anim) anim->is_playing = true;
}

void AnimationManager_SetSpeed(AnimationManager* manager, const char* anim_key, float speed) {
    if (!manager || !anim_key || speed <= 0) return;

    Animation* anim = find_animation(manager, anim_key);
    if (anim) anim->speed = speed;
}

void AnimationManager_DestroyAnimation(AnimationManager* manager, const char* anim_key) {
    if (!manager || !anim_key) return;

    // 查找并删除动画
    for (int i = 0; i < manager->animation_count; i++) {
        Animation* anim = manager->animations[i];
        if (strcmp(anim->texture_key, anim_key) == 0) {
            // 释放序列
            for (int j = 0; j < anim->clip_count; j++) {
                free(anim->clips[j]->name);
                free(anim->clips[j]->frame_indices);
                free(anim->clips[j]);
            }
            free(anim->clips);
            free(anim->frames);
            free(anim->texture_key);
            free(anim);

            // 移除数组
            for (int j = i; j < manager->animation_count - 1; j++) {
                manager->animations[j] = manager->animations[j + 1];
            }
            manager->animation_count--;
            manager->animations = (Animation**)realloc(
                manager->animations,
                sizeof(Animation*) * manager->animation_count
            );

            printf("AnimationManager: Animation '%s' destroyed\n", anim_key);
            return;
        }
    }
}

void AnimationManager_Destroy(AnimationManager* manager) {
    if (!manager) return;

    // 销毁所有动画
    for (int i = 0; i < manager->animation_count; i++) {
        Animation* anim = manager->animations[i];
        for (int j = 0; j < anim->clip_count; j++) {
            free(anim->clips[j]->name);
            free(anim->clips[j]->frame_indices);
            free(anim->clips[j]);
        }
        free(anim->clips);
        free(anim->frames);
        free(anim->texture_key);
        free(anim);
    }

    free(manager->animations);
    free(manager);

    printf("AnimationManager: Destroyed\n");
}
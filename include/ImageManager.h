#ifndef IMAGE_MANAGER_H
#define IMAGE_MANAGER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 缓存节点结构体：存储纹理+key+引用计数（可选）
typedef struct ImageCacheNode {
    char* key;                // 图片唯一标识（如路径）
    SDL_Texture* texture;     // 缓存的纹理
    int ref_count;            // 引用计数（可选，防止误释放）
    struct ImageCacheNode* next; // 链表下一个节点
} ImageCacheNode;

// 图像管理器结构体（单例）
typedef struct ImageManager {
    ImageCacheNode* cache_head; // 缓存链表头节点
    SDL_Renderer* renderer;     // 全局渲染器（关联绘制）
} ImageManager;

// ========== 核心接口 ==========
// 1. 获取ImageManager单例（初始化+全局唯一）
ImageManager* ImageManager_GetInstance(SDL_Renderer* renderer);

// 2. 加载纹理（自动缓存，重复加载返回已有纹理）
SDL_Texture* ImageManager_LoadTexture(ImageManager* manager, const char* key, const char* file_path);

// 3. 获取已缓存的纹理
SDL_Texture* ImageManager_GetTexture(ImageManager* manager, const char* key);

// 4. 释放单个纹理（引用计数为0时真正释放）
void ImageManager_ReleaseTexture(ImageManager* manager, const char* key);

// 5. 清空所有缓存纹理
void ImageManager_ClearCache(ImageManager* manager);

// 6. 销毁ImageManager单例（释放所有资源）
void ImageManager_DestroyInstance();

#endif // IMAGE_MANAGER_H
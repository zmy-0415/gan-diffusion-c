#include "ImageManager.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

// 静态单例（全局唯一）
static ImageManager* s_instance = NULL;

// ========== 内部辅助函数 ==========
// 查找缓存节点（按key）
static ImageCacheNode* find_cache_node(ImageManager* manager, const char* key) {
    if (!manager || !key) return NULL;
    ImageCacheNode* current = manager->cache_head;
    while (current) {
        if (strcmp(current->key, key) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// 创建新缓存节点
static ImageCacheNode* create_cache_node(const char* key, SDL_Texture* texture) {
    if (!key || !texture) return NULL;
    ImageCacheNode* node = (ImageCacheNode*)malloc(sizeof(ImageCacheNode));
    if (!node) {
        fprintf(stderr, "ImageManager: Failed to allocate cache node\n");
        return NULL;
    }
    node->key = (char*)malloc(strlen(key) + 1);
    strcpy(node->key, key);
    node->texture = texture;
    node->ref_count = 1;
    node->next = NULL;
    return node;
}

// 释放单个缓存节点
static void free_cache_node(ImageCacheNode* node) {
    if (!node) return;
    if (node->key) free(node->key);
    if (node->texture) SDL_DestroyTexture(node->texture);
    free(node);
}

// ========== 核心接口实现 ==========
ImageManager* ImageManager_GetInstance(SDL_Renderer* renderer) {
    // 单例初始化（首次调用传入renderer，后续调用忽略）
    if (!s_instance) {
        s_instance = (ImageManager*)malloc(sizeof(ImageManager));
        if (!s_instance) {
            fprintf(stderr, "ImageManager: Failed to create instance\n");
            return NULL;
        }
        s_instance->cache_head = NULL;
        s_instance->renderer = renderer;
        printf("ImageManager: Instance created\n");
    }
    // 后续调用可更新renderer（可选）
    if (renderer) {
        s_instance->renderer = renderer;
    }
    return s_instance;
}

SDL_Texture* ImageManager_LoadTexture(ImageManager* manager, const char* key, const char* file_path) {
    if (!manager || !key || !file_path || !manager->renderer) {
        fprintf(stderr, "ImageManager: Invalid params for LoadTexture\n");
        return NULL;
    }

    // 1. 检查缓存：已加载则返回并增加引用计数
    ImageCacheNode* node = find_cache_node(manager, key);
    if (node) {
        node->ref_count++;
        printf("ImageManager: Texture '%s' hit cache (ref: %d)\n", key, node->ref_count);
        return node->texture;
    }

    // 2. 未缓存则加载纹理
    SDL_Texture* texture = IMG_LoadTexture(manager->renderer, file_path);
    if (!texture) {
        fprintf(stderr, "ImageManager: Failed to load texture '%s': %s\n", file_path, IMG_GetError());
        return NULL;
    }

    // 3. 创建缓存节点并加入链表
    node = create_cache_node(key, texture);
    if (!node) {
        SDL_DestroyTexture(texture);
        return NULL;
    }

    // 头插法（简单高效）
    node->next = manager->cache_head;
    manager->cache_head = node;
    printf("ImageManager: Texture '%s' loaded and cached\n", key);
    return texture;
}

SDL_Texture* ImageManager_GetTexture(ImageManager* manager, const char* key) {
    if (!manager || !key) return NULL;
    ImageCacheNode* node = find_cache_node(manager, key);
    return node ? node->texture : NULL;
}

void ImageManager_ReleaseTexture(ImageManager* manager, const char* key) {
    if (!manager || !key) return;

    ImageCacheNode* prev = NULL;
    ImageCacheNode* current = manager->cache_head;

    // 查找节点并处理引用计数
    while (current) {
        if (strcmp(current->key, key) == 0) {
            current->ref_count--;
            printf("ImageManager: Texture '%s' ref decreased to %d\n", key, current->ref_count);

            // 引用计数为0时释放节点
            if (current->ref_count <= 0) {
                if (prev) {
                    prev->next = current->next;
                } else {
                    manager->cache_head = current->next;
                }
                free_cache_node(current);
                printf("ImageManager: Texture '%s' released from cache\n", key);
            }
            return;
        }
        prev = current;
        current = current->next;
    }

    fprintf(stderr, "ImageManager: Texture '%s' not found in cache (release failed)\n", key);
}

void ImageManager_ClearCache(ImageManager* manager) {
    if (!manager) return;

    ImageCacheNode* current = manager->cache_head;
    while (current) {
        ImageCacheNode* next = current->next;
        free_cache_node(current);
        current = next;
    }
    manager->cache_head = NULL;
    printf("ImageManager: Cache cleared\n");
}

void ImageManager_DestroyInstance() {
    if (!s_instance) return;

    // 清空缓存
    ImageManager_ClearCache(s_instance);
    // 释放单例
    free(s_instance);
    s_instance = NULL;
    printf("ImageManager: Instance destroyed\n");
}
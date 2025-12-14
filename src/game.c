#include "game.h"


void init()
{
    
    SDL_Texture* tex_player = ImageManager_LoadTexture(commons->imageManager, "player_sprites", "./assets/image/player/player1.png");
    // 加载动画（4行8列分割精灵图）
    Animation* player_anim = AnimationManager_LoadAnimation(
        commons->g_anim_manager,
        "player",          // 动画标识
        "player_sprites",  // 纹理key
        20,                 // 行数
        8                  // 列数
    );
    int idle_frames[] = {0, 1, 2, 3, 4, 5}; // 循环帧，含间隔
    AnimationManager_AddClip(
        player_anim,
        "idle",            // 序列名称
        idle_frames,       // 帧索引序列
        6,                 // 序列长度
        0.2f,              // 每帧0.2秒
        true,              // 循环
        false              // 不反向
    );
    int wail_right_frames[] = {6, 7, 8, 9, 10, 11}; // 循环帧，含间隔
    AnimationManager_AddClip(
        player_anim,
        "wail_right",            // 序列名称
        wail_right_frames,       // 帧索引序列
        6,                 // 序列长度
        0.1f,              // 每帧0.2秒
        true,              // 循环
        false              // 不反向
    );

    int attack1_frames[] = {40, 41, 42, 43, 44}; // 循环帧，含间隔
    AnimationManager_AddClip(
        player_anim,
        "attack1",            // 序列名称
        attack1_frames,       // 帧索引序列
        6,                 // 序列长度
        0.2f,              // 每帧0.2秒
        true,              // 循环
        false              // 不反向
    );

    // 播放 idle 动画
    AnimationManager_Play(commons->g_anim_manager, "player", "attack1");
    // 设置播放速度（1.5倍速）
    AnimationManager_SetSpeed(commons->g_anim_manager, "player", 1.5f);
}
void update(float dt)
{
    AnimationManager_Update(commons->g_anim_manager, dt);
}

void draw()
{

    // 绘制动画（屏幕中心，缩放2倍，无旋转/翻转）
    AnimationManager_Draw(
        commons->g_anim_manager,
        "player",
        400, 300,    // 绘制位置（中心）
        0, 0,        // 自动尺寸
        10.0f,        // 缩放2倍
        0.0f,        // 无旋转
        SDL_FLIP_NONE // 无翻转
    );

}

void destroyed()
{

}
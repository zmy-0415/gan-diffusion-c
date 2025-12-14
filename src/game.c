#include "game.h"

SDL_Texture* tex_player = NULL;


void init()
{
    
    tex_player = ImageManager_LoadTexture(commons->imageManager, "player", "./assets/image/player/4.png");

}
void update(float dt)
{

}

void draw()
{
    if (tex_player) {
        // 绘制玩家（居中）
        SDL_Rect player_rect = {commons->WIN_WIDTH/2 - 50, commons->WIN_HEIGHT/2 - 50, 100, 100};
        SDL_RenderCopy(commons->g_renderer, tex_player, NULL, &player_rect);
    }
}

void destroyed()
{

}
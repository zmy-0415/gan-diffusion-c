#ifndef COMMON_H
#define COMMON_H
#include<SDL2/SDL.h>
#include "AnimationManager.h"
#include "ImageManager.h"



typedef struct {
    SDL_Renderer* g_renderer;
    SDL_Window* g_window;
    int WIN_WIDTH;
    int WIN_HEIGHT;
    ImageManager* imageManager;
    AnimationManager* g_anim_manager;
}CommonS;

extern CommonS *commons;

#endif
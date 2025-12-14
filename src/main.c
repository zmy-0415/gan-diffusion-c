
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "game.h"

// Windows系统API
#if defined(_WIN32) || defined(WIN32)
    #include <windows.h>
    #include <tchar.h>
#endif

#define WINDOW_TITLE  "SDL Fullscreen Transparent Window (Global Renderer)"


// ========== 1. 全局变量声明（核心） ==========
SDL_Renderer* g_renderer = NULL; // 全局渲染器（初始化为NULL）
SDL_Window* g_window = NULL;     // 可选：窗口也声明为全局，方便全局操作
CommonS *commons;
int WINDOW_WIDTH = 0;
int WINDOW_HEIGHT = 0;

// 错误处理宏
#define SDL_CHECK_ERROR(func) \
    do { \
        if (func != 0) { \
            fprintf(stderr, "SDL Error: %s (in %s)\n", SDL_GetError(), #func); \
            SDL_Quit(); \
            return 1; \
        } \
    } while (0)

// Windows API：查找窗口句柄
#if defined(_WIN32) || defined(WIN32)
    BOOL CALLBACK FindWindowByTitleCallback(HWND hwnd, LPARAM lParam) {
        TCHAR window_text[256];
        GetWindowText(hwnd, window_text, sizeof(window_text)/sizeof(TCHAR));
        if (_tcscmp(window_text, _T(WINDOW_TITLE)) == 0) {
            *((HWND*)lParam) = hwnd;
            return FALSE;
        }
        return TRUE;
    }

    HWND GetSDLWindowHandle() {
        HWND target_hwnd = NULL;
        EnumWindows(FindWindowByTitleCallback, (LPARAM)&target_hwnd);
        return target_hwnd;
    }

    void SetWindowAlwaysOnTopWin32(HWND hwnd, bool top) {
        SetWindowPos(hwnd, top ? HWND_TOPMOST : HWND_NOTOPMOST, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE);
    }

    void RemoveWindowBorder(HWND hwnd) {
        LONG style = GetWindowLongPtr(hwnd, GWL_STYLE);
        style &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
        SetWindowLongPtr(hwnd, GWL_STYLE, style);
        SetWindowPos(hwnd, NULL, 0,0,0,0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
    }
#endif


int main(int argc, char* argv[]) {
    // 初始化SDL
    SDL_CHECK_ERROR(SDL_Init(SDL_INIT_VIDEO));

    // 获取桌面分辨率
    SDL_DisplayMode dm;
    if (SDL_GetCurrentDisplayMode(0, &dm) == 0) {
        WINDOW_WIDTH = dm.w;
        WINDOW_HEIGHT = dm.h;
    } else {
        WINDOW_WIDTH = 1920;
        WINDOW_HEIGHT = 1080;
    }

    // 创建全屏无边框窗口（赋值给全局窗口）
    g_window = SDL_CreateWindow(
        WINDOW_TITLE,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_BORDERLESS | SDL_WINDOW_SHOWN
    );
    if (!g_window) {
        fprintf(stderr, "SDL Error: %s (in SDL_CreateWindow)\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // ========== 3. 初始化全局渲染器（核心） ==========
    g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!g_renderer) g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_SOFTWARE);
    if (!g_renderer) {
        fprintf(stderr, "SDL Error: %s (in SDL_CreateRenderer)\n", SDL_GetError());
        SDL_DestroyWindow(g_window);
        SDL_Quit();
        return 1;
    }

    // Windows API设置置顶+透明
    #if defined(_WIN32) || defined(WIN32)
        SDL_Delay(100);
        HWND sdl_hwnd = GetSDLWindowHandle();
        if (sdl_hwnd) {
            SetWindowAlwaysOnTopWin32(sdl_hwnd, true);
            RemoveWindowBorder(sdl_hwnd);
            SetWindowLongPtr(sdl_hwnd, GWL_EXSTYLE, GetWindowLongPtr(sdl_hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
            SetLayeredWindowAttributes(sdl_hwnd, RGB(0,0,0), 255, LWA_COLORKEY | LWA_ALPHA);
        }
    #endif

    // 启用混合模式
    SDL_SetRenderDrawBlendMode(g_renderer, SDL_BLENDMODE_BLEND);
    SDL_SetWindowAlwaysOnTop(g_window, SDL_TRUE);

    // 计时变量
    bool isRunning = true;
    SDL_Event event;
    Uint32 last_ticks = SDL_GetTicks();
    float dt_float = 0.0f;
    commons = (CommonS*)malloc(sizeof(CommonS));
    commons->g_renderer = g_renderer;
    commons->g_window = g_window;
    commons->WIN_WIDTH = WINDOW_WIDTH;
    commons->WIN_HEIGHT = WINDOW_HEIGHT;
    commons->imageManager = ImageManager_GetInstance(g_renderer);
    // 初始化 AnimationManager
    commons->g_anim_manager = AnimationManager_Create(commons->imageManager, g_renderer);


    init();

    // 主循环
    while (isRunning) {
        // 事件处理
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_q))) {
                isRunning = false;
            }
        }

        // 计算dt
        Uint32 current_ticks = SDL_GetTicks();
        dt_float = (current_ticks - last_ticks) / 1000.0f;
        last_ticks = current_ticks;

        update(dt_float);
         // 清空整个渲染器（删除上一帧所有绘制内容）
        if (SDL_RenderClear(g_renderer) != 0) {
            fprintf(stderr, "SDL_RenderClear failed: %s\n", SDL_GetError());
        }

        draw(); // 自定义绘制（也可直接用g_renderer）

        // 更新屏幕
        SDL_RenderPresent(g_renderer);
    }

    destroyed();

    // ========== 5. 释放全局资源（核心） ==========
    if (g_renderer) SDL_DestroyRenderer(g_renderer); // 释放全局渲染器
    if (g_window) SDL_DestroyWindow(g_window);       // 释放全局窗口
    SDL_Quit();

    return 0;
}
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include "game.h"
// Windows系统API（必须）
#if defined(_WIN32) || defined(WIN32)
    #include <windows.h>
    #include <tchar.h>
#endif

#define WINDOW_TITLE  "SDL Fullscreen Transparent Window (No Border)"
// 全屏模式下使用桌面分辨率（动态获取）
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

// Windows API：通过标题查找窗口句柄（兼容所有SDL版本）
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

    // Windows API 设置窗口置顶
    void SetWindowAlwaysOnTopWin32(HWND hwnd, bool top) {
        SetWindowPos(
            hwnd,
            top ? HWND_TOPMOST : HWND_NOTOPMOST, // 置顶/取消置顶
            0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE // 不改变窗口位置和大小
        );
    }

    // Windows API 移除窗口边框（兜底）
    void RemoveWindowBorder(HWND hwnd) {
        LONG style = GetWindowLongPtr(hwnd, GWL_STYLE);
        style &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
        SetWindowLongPtr(hwnd, GWL_STYLE, style);
        SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
    }
#endif

int main(int argc, char* argv[]) {
    // 初始化SDL视频子系统
    SDL_CHECK_ERROR(SDL_Init(SDL_INIT_VIDEO));

    // 获取桌面分辨率（全屏适配）
    SDL_DisplayMode dm;
    if (SDL_GetCurrentDisplayMode(0, &dm) == 0) {
        WINDOW_WIDTH = dm.w;
        WINDOW_HEIGHT = dm.h;
    } else {
        // 兜底分辨率
        WINDOW_WIDTH = 1920;
        WINDOW_HEIGHT = 1080;
        fprintf(stderr, "Warning: Failed to get display mode, use default %dx%d\n", WINDOW_WIDTH, WINDOW_HEIGHT);
    }

    // 1. 创建全屏无边框窗口（核心）
    SDL_Window* window = SDL_CreateWindow(
        WINDOW_TITLE,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        // 关键参数：全屏桌面模式 + 无边框 + 显示
        SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_BORDERLESS | SDL_WINDOW_SHOWN
    );
    if (!window) {
        fprintf(stderr, "SDL Error: %s (in SDL_CreateWindow)\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // ========== 窗口置顶设置 ==========
    // SDL原生API（跨平台）
    SDL_SetWindowAlwaysOnTop(window, SDL_TRUE);

    // Windows API兜底（更稳定）
    #if defined(_WIN32) || defined(WIN32)
        SDL_Delay(50); // 等待窗口创建完成
        HWND sdl_hwnd = GetSDLWindowHandle();
        if (sdl_hwnd) {
            SetWindowAlwaysOnTopWin32(sdl_hwnd, true);
            RemoveWindowBorder(sdl_hwnd); // 确保无窗口边框
            printf("Windows API: 全屏无边框窗口已置顶\n");
        }
    #endif

    // 2. Windows API 设置全屏窗口背景完全透明（核心）
    #if defined(_WIN32) || defined(WIN32)
        SDL_Delay(100); // 等待窗口稳定
        if (!sdl_hwnd) sdl_hwnd = GetSDLWindowHandle(); // 确保句柄有效
        if (!sdl_hwnd) {
            fprintf(stderr, "Failed to get window handle!\n");
            SDL_DestroyWindow(window);
            SDL_Quit();
            return 1;
        }

        // 设置分层窗口 + 背景完全透明
        SetWindowLongPtr(sdl_hwnd, GWL_EXSTYLE, GetWindowLongPtr(sdl_hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
        // 黑色背景透明，内容不透明
        SetLayeredWindowAttributes(sdl_hwnd, RGB(0,0,0), 255, LWA_COLORKEY | LWA_ALPHA);
    #endif

    // 3. 创建渲染器（硬件加速优先）
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer) {
        fprintf(stderr, "SDL Error: %s (in SDL_CreateRenderer)\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // 启用混合模式（支持半透明绘制）
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    bool isRunning = true;
    SDL_Event event;

    // 计时变量（计算dt）
    Uint32 last_ticks = SDL_GetTicks();
    float dt_float = 0.0f;

    init();
    // 主循环
    while (isRunning) {
        // 事件处理
        while (SDL_PollEvent(&event)) {
            // ESC退出，Q退出，关闭窗口退出
            if (event.type == SDL_QUIT || 
                (event.type == SDL_KEYDOWN && (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_q))) {
                isRunning = false;
            }
            // T键切换置顶/取消置顶
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_t) {
                static bool isTop = true;
                isTop = !isTop;
                SDL_SetWindowAlwaysOnTop(window, isTop ? SDL_TRUE : SDL_FALSE);
                #if defined(_WIN32) || defined(WIN32)
                    SetWindowAlwaysOnTopWin32(sdl_hwnd, isTop);
                #endif
                printf("窗口%s置顶\n", isTop ? "已" : "取消");
            }
            // F键切换全屏/窗口模式（可选）
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_f) {
                static bool isFullscreen = true;
                isFullscreen = !isFullscreen;
                SDL_SetWindowFullscreen(window, isFullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
                // 切换后重新设置无边框和透明
                if (!isFullscreen) {
                    SDL_SetWindowBordered(window, SDL_FALSE);
                    #if defined(_WIN32) || defined(WIN32)
                        RemoveWindowBorder(sdl_hwnd);
                        SetLayeredWindowAttributes(sdl_hwnd, RGB(0,0,0), 255, LWA_COLORKEY | LWA_ALPHA);
                    #endif
                }
            }
        }

        // 计算dt（秒级）
        Uint32 current_ticks = SDL_GetTicks();
        dt_float = (current_ticks - last_ticks) / 1000.0f;
        last_ticks = current_ticks;

        update(dt_float);

        // 4. 绘制逻辑（全屏适配）
        // 清空背景（黑色，被Windows设为透明）
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);

        // 绘制居中红色矩形（全屏下自适应）
        SDL_Rect rect = {
            WINDOW_WIDTH/2 - 100,  // 水平居中
            WINDOW_HEIGHT/2 - 100, // 垂直居中
            200, 200               // 矩形大小
        };
        // SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // 不透明红色
        // SDL_RenderFillRect(renderer, &rect);

        // // 绘制全屏十字线（绿色，不透明）
        // SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        // // 水平中线
        // SDL_RenderDrawLine(renderer, 0, WINDOW_HEIGHT/2, WINDOW_WIDTH, WINDOW_HEIGHT/2);
        // // 垂直中线
        // SDL_RenderDrawLine(renderer, WINDOW_WIDTH/2, 0, WINDOW_WIDTH/2, WINDOW_HEIGHT);

        draw(); // 自定义绘制逻辑

        // 更新屏幕
        SDL_RenderPresent(renderer);
    }

    destroyed();
    // 资源清理
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
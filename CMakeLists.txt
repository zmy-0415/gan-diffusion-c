cmake_minimum_required(VERSION 3.10)
project(main C)

# ========== 基础配置 ==========
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 调试/发布模式优化
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -march=native -lm")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -lm")
endif()

# ========== 禁用 WebP 依赖（核心） ==========
set(SDL2IMAGE_BUILD_WEBP OFF CACHE BOOL "Disable WebP support")
set(SDL2IMAGE_USE_WEBP OFF CACHE BOOL "Disable WebP usage")
set(webp_FOUND ON CACHE BOOL "Fake WebP found to skip check")

# ========== 环境检测（UCRT64/MinGW64） ==========
set(MSYS2_UCRT64 OFF)
if(MINGW AND CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    if(DEFINED ENV{MSYSTEM} AND "$ENV{MSYSTEM}" STREQUAL "UCRT64")
        set(MSYS2_UCRT64 ON)
        set(SDL2_PREFIX "/ucrt64")
    else()
        set(SDL2_PREFIX "/mingw64")
    endif()
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${SDL2_PREFIX})
endif()

# ========== SDL2 + SDL2_image 查找 ==========
find_package(SDL2 REQUIRED)
if(NOT SDL2_FOUND)
    message(FATAL_ERROR "SDL2 未找到！请安装：pacman -S mingw-w64-ucrt-x86_64-SDL2")
endif()

# 手动指定 SDL2_image 路径
set(SDL2_IMAGE_INCLUDE_DIRS "${SDL2_PREFIX}/include/SDL2")
set(SDL2_IMAGE_LIBRARIES "${SDL2_PREFIX}/lib/libSDL2_image.dll.a")
if(NOT EXISTS ${SDL2_IMAGE_LIBRARIES})
    message(FATAL_ERROR "SDL2_image 未找到！请安装：pacman -S mingw-w64-ucrt-x86_64-SDL2_image")
endif()

# ========== 头文件 + 源文件 ==========
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
)

set(SOURCES
    src/game.c
    src/common.c
    src/ImageManager.c
)

add_executable(main src/main.c ${SOURCES})

# ========== 链接库 ==========
target_link_libraries(main
    PRIVATE
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
)

# Windows 专属配置
if(WIN32)
    target_link_libraries(main PRIVATE SDL2main user32)
    set_target_properties(main PROPERTIES WIN32_EXECUTABLE ON) # 隐藏控制台

    # 拷贝必要 DLL
    set(REQUIRED_DLLS
        "${SDL2_PREFIX}/bin/SDL2.dll"
        "${SDL2_PREFIX}/bin/SDL2_image.dll"
        "${SDL2_PREFIX}/bin/libpng16-16.dll"
        "${SDL2_PREFIX}/bin/libjpeg-8.dll"
        "${SDL2_PREFIX}/bin/zlib1.dll"
    )
    foreach(DLL ${REQUIRED_DLLS})
        if(EXISTS ${DLL})
            add_custom_command(TARGET main POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DLL} $<TARGET_FILE_DIR:main>
                COMMENT "拷贝 DLL: ${DLL}"
            )
        endif()
    endforeach()
endif()

# Linux/macOS 适配
if(UNIX AND NOT APPLE)
    target_link_libraries(main PRIVATE pthread m)
elseif(APPLE)
    include_directories(/usr/local/include/SDL2)
    link_directories(/usr/local/lib)
    target_link_libraries(main PRIVATE SDL2 SDL2_image "-framework Cocoa")
endif()

# ========== 修复：Asset 目录同步（无循环依赖） ==========
# 1. 定义 asset 源目录和目标目录（注意：你原配置是 assets，统一为 assets）
set(ASSET_SOURCE_DIR "${PROJECT_SOURCE_DIR}/assets")
set(ASSET_TARGET_DIR "$<TARGET_FILE_DIR:main>/assets")

# 2. 仅保留「编译后拷贝」逻辑（无循环依赖）
if(EXISTS ${ASSET_SOURCE_DIR})
    # 核心：编译后自动拷贝整个 assets 目录（无依赖循环）
    add_custom_command(
        TARGET main POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${ASSET_SOURCE_DIR} ${ASSET_TARGET_DIR}
        COMMENT "📂 同步 Assets 目录：${ASSET_SOURCE_DIR} → ${ASSET_TARGET_DIR}"
    )

    # 可选：监听 assets 文件变化（手动执行 make sync_assets 同步，无循环依赖）
    file(GLOB_RECURSE ASSET_FILES ${ASSET_SOURCE_DIR}/*)
    add_custom_target(
        sync_assets  # 移除 ALL 关键字，避免隐式依赖 main
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${ASSET_SOURCE_DIR} ${ASSET_TARGET_DIR}
        COMMENT "🔄 手动同步 Assets 目录"
    )

    # 可选：打印 asset 路径（调试用）
    message(STATUS "📌 Asset 源目录: ${ASSET_SOURCE_DIR}")
    message(STATUS "📌 Asset 目标目录: ${ASSET_TARGET_DIR}")
else()
    message(WARNING "⚠️ Assets 目录不存在: ${ASSET_SOURCE_DIR}")
endif()
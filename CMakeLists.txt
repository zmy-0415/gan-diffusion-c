cmake_minimum_required(VERSION 3.10)
project(main C)

# 设置C标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 编译优化选项（保留你的原有配置）
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -march=native -lm")

# ========== SDL 2 配置 ==========
# 1. 查找 SDL 2 库（跨平台兼容）
find_package(SDL2 REQUIRED)
if(NOT SDL2_FOUND)
    message(FATAL_ERROR "SDL2 library not found! Please install SDL2 development files.")
endif()

# 2. 包含 SDL 2 头文件目录
include_directories(
    ${PROJECT_SOURCE_DIR}/include  # 你的自定义头文件目录
    ${SDL2_INCLUDE_DIRS}           # SDL2 头文件目录
)

# ========== 源文件配置 ==========
# 你的源文件列表（根据实际情况调整）
set(SOURCES
    src/game.c
)

# 生成可执行文件（main.c 是主入口，包含 SDL 逻辑）
add_executable(main src/main.c ${SOURCES})

# ========== 链接库 ==========
# 链接 SDL2 库 + 数学库（跨平台处理）
target_link_libraries(main
    PRIVATE
    ${SDL2_LIBRARIES}  # SDL2 核心库
    m                  # 数学库（Linux/macOS）
)

# Windows 平台额外处理（SDL2 可能需要手动指定库路径，可选）
if(WIN32)
    # 若 Windows 下 SDL2 未自动找到，可手动指定库路径（示例）
    # set(SDL2_LIB_DIR "D:/SDL2-devel-2.30.0-mingw/SDL2-2.30.0/x86_64-w64-mingw32/lib")
    # link_directories(${SDL2_LIB_DIR})
    # target_link_libraries(main PRIVATE SDL2main SDL2)
endif()

# macOS 平台额外处理（可选，确保 SDL2 路径正确）
if(APPLE)
    # 若通过 brew 安装 SDL2，强制指定路径（防止找不到）
    # include_directories(/usr/local/include/SDL2)
    # link_directories(/usr/local/lib)
endif()
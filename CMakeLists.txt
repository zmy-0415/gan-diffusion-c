cmake_minimum_required(VERSION 3.10)
project(main C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -march=native -lm")

include_directories(${PROJECT_SOURCE_DIR}/include)

set(SOURCES
    src/a.c
)

add_executable(main src/main.c ${SOURCES})
target_link_libraries(main PRIVATE m)

# 在大多数系统上可能需要额外的数学库
if(UNIX AND NOT APPLE)
    target_link_libraries(main PRIVATE m)
endif()
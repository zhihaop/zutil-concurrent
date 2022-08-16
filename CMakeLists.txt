cmake_minimum_required(VERSION 3.22)
project(zutil-concurrent)

set(CMAKE_C_STANDARD 11)

add_executable(
        ${PROJECT_NAME}
        src/main.c
        src/ArrayBlockingQueue.c 
        src/FixedThreadPoolExecutor.c
        src/LinkedBlockingQueue.c)

target_include_directories(${PROJECT_NAME} PRIVATE include)
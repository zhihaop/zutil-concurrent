cmake_minimum_required(VERSION 3.22)
project(zutil-concurrent)

set(CMAKE_C_STANDARD 11)

add_executable(
        ${PROJECT_NAME}
        test/main.c
        src/ArrayBlockingQueue.c
        src/LinkedBlockingQueue.c
        src/FixedThreadPoolExecutor.c
        src/ReentrantLock.c
        src/Condition.c
        src/CountDownLatch.c
        src/ThreadLocal.c
        test/benchmarkQueue.c)

target_include_directories(${PROJECT_NAME} PRIVATE include)
target_link_libraries(${PROJECT_NAME} PRIVATE pthread)
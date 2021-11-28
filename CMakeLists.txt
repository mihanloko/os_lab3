cmake_minimum_required(VERSION 3.20)
project(lab3)

set(CMAKE_CXX_STANDARD 14)

add_executable(lab3 main.cpp)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(lab3 PRIVATE Threads::Threads)


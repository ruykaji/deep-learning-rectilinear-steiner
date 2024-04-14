cmake_minimum_required(VERSION 3.19.0)
project(generator)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(generator src/main.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE include)
cmake_minimum_required(VERSION 3.10)
project(MyProject)

# 设置全局 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置 GLFW 路径为缓存变量
set(GLFW_ROOT "${PROJECT_SOURCE_DIR}/External/glfw-3.4" CACHE PATH "GLFW root directory")

# 添加子项目
add_subdirectory(proj4)
add_subdirectory(proj5)
cmake_minimum_required(VERSION 3.10)
project(MyProject)

# 设置全局 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加子项目
add_subdirectory(proj4)
add_subdirectory(proj5)
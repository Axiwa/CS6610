add_executable(proj4 
    main.cpp
    glad.c
    lodepng.cpp
)

# 设置 GLFW 的路径（注意：用双斜杠或转义）
set(GLFW_ROOT "D:/tool/glfw-3.4.bin")

# 包含 GLFW 的头文件
target_include_directories(proj4 PRIVATE 
    ${GLFW_ROOT}/include
    ${CMAKE_SOURCE_DIR}/include
)


# 链接 GLFW 静态或动态库
target_link_directories(proj4 PRIVATE ${GLFW_ROOT}/lib-vc2022)

# 链接 OpenGL 和 GLFW
target_link_libraries(proj4 PRIVATE glfw3dll opengl32)

# 编译时定义 DLL 模式（告诉 GLFW 不要用静态路径）
target_compile_definitions(proj4 PRIVATE GLFW_DLL)

# 构建后自动复制 DLL（非常推荐）
add_custom_command(TARGET proj4 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${GLFW_ROOT}/lib-vc2022/glfw3.dll"
        $<TARGET_FILE_DIR:proj4>)
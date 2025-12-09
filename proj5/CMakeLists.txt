add_executable(proj5 
    main.cpp
    glad.c
    lodepng.cpp
)

target_include_directories(proj5 PRIVATE
    ${GLFW_ROOT}/include
    ${CMAKE_SOURCE_DIR}/include
)

# 链接 GLFW 静态或动态库
target_link_directories(proj5 PRIVATE ${GLFW_ROOT}/lib-vc2022)

# 链接 OpenGL 和 GLFW
target_link_libraries(proj5 PRIVATE glfw3dll opengl32)

# 编译时定义 DLL 模式（告诉 GLFW 不要用静态路径）
target_compile_definitions(proj5 PRIVATE GLFW_DLL)

# 构建后自动复制 DLL（非常推荐）
add_custom_command(TARGET proj5 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${GLFW_ROOT}/lib-vc2022/glfw3.dll"
        $<TARGET_FILE_DIR:proj5>)
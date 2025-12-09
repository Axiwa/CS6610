#pragma once

#include <glfw/glfw3.h>
#include "oribitcamera.h"

struct myWindow{
    GLFWwindow* window = nullptr;
    int width, height;
    double mouse_x = 0, mouse_y = 0, last_x = 0, last_y = 0;
    bool is_dragging_left = false, is_dragging_right = false;
    orbit_camera scene_camera;

    myWindow():
    width(1024),
    height(1024),
    window(nullptr),
    scene_camera(M_PI / 2, width*1.0/height, 0.1, 10000)
    {
        window = glfwCreateWindow(width, height, "Default Window", nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
        glfwSetCursorPosCallback(window, CursorPosCallback);
        glfwSetMouseButtonCallback(window, MouseButtonCallback);
        glfwSetScrollCallback(window, ScrollCallback);
    }

    myWindow(int width_, int height_, const char* title):
    width(width_),
    height(height_),
    window(nullptr),
    scene_camera(M_PI / 2, width*1.0/height, 0.1, 10000)
    {
        window = glfwCreateWindow(width, height, title, nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
        glfwSetCursorPosCallback(window, CursorPosCallback);
        glfwSetMouseButtonCallback(window, MouseButtonCallback);
        glfwSetScrollCallback(window, ScrollCallback);
    }

    void deltaMouse(double& deltax, double& deltay){
        if (is_dragging_left || is_dragging_right){
            deltax = mouse_x - last_x;
            deltay = mouse_y - last_y;
        }
        else
        {
            deltax = 0;
            deltay = 0;
        }
    }
    
private:
    static void FramebufferSizeCallback(GLFWwindow* window, int width, int height){
        myWindow* myself = reinterpret_cast<myWindow*>(glfwGetWindowUserPointer(window));
        myself->width = width;
        myself->height = height;
        glViewport(0, 0, width, height);
        myself->scene_camera.on_aspect_change(width * 1.0 / height);
    }

    static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos){
		myWindow* self = static_cast<myWindow*>(glfwGetWindowUserPointer(window));
		self->mouse_x = xpos;
		self->mouse_y = ypos;
    }

    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods){
        myWindow* self = static_cast<myWindow*>(glfwGetWindowUserPointer(window));
        if (button == GLFW_MOUSE_BUTTON_LEFT){
			if (action == GLFW_PRESS) {
				self->is_dragging_left = true;
				self->last_x = self->mouse_x;
				self->last_y = self->mouse_y;
			}
			else {
				self->is_dragging_left = false;
			}
        }
        if (button == GLFW_MOUSE_BUTTON_RIGHT){
            if (action == GLFW_PRESS){
                self->is_dragging_right = true;
				self->last_x = self->mouse_x;
				self->last_y = self->mouse_y;
            }
            else{
                self->is_dragging_right = false;
            }
        }
    }

    static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset){
        myWindow* self = static_cast<myWindow*>(glfwGetWindowUserPointer(window));
        if (yoffset < 0){
            self->scene_camera._dist *= 1.0f;
        }
        else{
            self->scene_camera._dist /= 1.0f;
        }
    }

};
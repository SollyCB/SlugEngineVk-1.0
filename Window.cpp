#include "Window.hpp"
#include "VulkanErrors.hpp"

#include <GLFW/glfw3.h>

namespace Sol {

Window *Window::instance() { return &sWindow; }

void Window::init_glfw(void *ptr) {
    if (!glfwInit())
        ABORT(false, "GLFW init failed");
    if (!glfwVulkanSupported())
        ABORT(false, "GLFW no Vulkan support");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(WIDTH, HEIGHT, "SlugVk", NULL, NULL);
    if (!window)
        ABORT(false, "GLFW window creation failed");

    camera->width = WIDTH;
    camera->height = HEIGHT;

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    glfwSetKeyCallback(window, key_callback);
    glfwSetWindowUserPointer(window, ptr);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);
}

void Window::poll() {
    glfwPollEvents();

    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
        input = false;
        first_mouse = true;
    }
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        input = true;
    }

    if (input) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera->move(FORWARD);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera->move(BACKWARD);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera->move(LEFT);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera->move(RIGHT);

        if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
            camera->sens -= 500;
        if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
            camera->sens += 500;
        if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
            camera->speed -= 500;
        if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
            camera->speed += 500;
    }
}

void Window::key_callback(GLFWwindow *window, int key, int scancode, int action,
                          int mods) {
    if (key == GLFW_KEY_Q && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}
void Window::framebufferResizeCallback(GLFWwindow *window, int width,
                                       int height) {
    auto ptr = (Window *)glfwGetWindowUserPointer(window);
    ptr->width = width;
    ptr->height = height;
    ptr->camera->height = height;
    ptr->camera->width = width;
}
void Window::cursor_position_callback(GLFWwindow *window, double xpos,
                                      double ypos) {
    auto ptr = (Window *)glfwGetWindowUserPointer(window);

    if (ptr->input) {
        float xoffset = xpos - ptr->xpos;
        float yoffset = ypos - ptr->ypos;
        ptr->xpos = xpos;
        ptr->ypos = ypos;

        if (ptr->first_mouse) {
            ptr->first_mouse = false;
            xoffset = 0;
            yoffset = 0;
        }

        ptr->camera->look(xoffset, yoffset);
    }
}
void Window::scroll_callback(GLFWwindow *window, double xoffset,
                             double yoffset) {
    auto ptr = (Window *)glfwGetWindowUserPointer(window);
    ptr->camera->zoom(yoffset);
}

} // namespace Sol

#include "Window.hpp"
#include "VulkanErrors.hpp"
#include <GLFW/glfw3.h>

namespace Sol {
Window* Window::instance() {
  return &sWindow;
}

void Window::init(void* ptr) {
  init_glfw(ptr);
}
void Window::init_glfw(void *ptr) {
  if (!glfwInit())
    ABORT(false, "GLFW init failed");
  if (!glfwVulkanSupported())
    ABORT(false, "GLFW no Vulkan support");

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  window = glfwCreateWindow(WIDTH, HEIGHT, "SlugVk", NULL, NULL);
  if (!window)
    ABORT(false, "GLFW window creation failed");

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
  glfwSetKeyCallback(window, key_callback);
  glfwSetWindowUserPointer(window, ptr);
  glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
  glfwSetCursorPosCallback(window, cursor_position_callback);
  glfwSetScrollCallback(window, scroll_callback);
}

void Window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  auto ptr = (Window*)glfwGetWindowUserPointer(window);
  Camera *cam = ptr->camera;
  if (key == GLFW_KEY_Q && action == GLFW_PRESS)
      glfwSetWindowShouldClose(window, GLFW_TRUE);
}
void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
  auto ptr = (Window*)glfwGetWindowUserPointer(window);
  //glfwGetFramebufferSize(window, &ptr->width, &ptr->height);
  ptr->width = width;
  ptr->height = height;
  ptr->camera->window_width = width;
  ptr->camera->window_height = height;
}
void Window::cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
  auto ptr = (Window*)glfwGetWindowUserPointer(window);
  ptr->camera->xpos = xpos - ptr->xpos;
  ptr->camera->ypos = ypos - ptr->ypos;
  ptr->camera->updates();
  ptr->camera->get_proj();
  ptr->camera->get_view();
  ptr->xpos = xpos;
  ptr->ypos = ypos;
  ptr->camera->xpos = 0;
  ptr->camera->ypos = 0;
}
void Window::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
  auto ptr = (Window*)glfwGetWindowUserPointer(window);
  ptr->scroll = yoffset;
  ptr->camera->zoom(yoffset);
  ptr->camera->updates();
  ptr->camera->get_proj();
  ptr->camera->get_view();
}

} // Sol

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

  glfwSetKeyCallback(window, key_callback);
  glfwSetWindowUserPointer(window, ptr);
  glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void Window::kill() {
  glfwDestroyWindow(window);
  glfwTerminate();
}

void Window::size() {
  glfwGetFramebufferSize(window, &width, &height);
}
void Window::wait() {
  glfwWaitEvents();
}

bool Window::close() {
  return glfwWindowShouldClose(window);
}
void Window::poll() {
  glfwPollEvents();
}
void Window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_Q && action == GLFW_PRESS)
      glfwSetWindowShouldClose(window, GLFW_TRUE);
}
void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
  auto ptr = (Window*)glfwGetWindowUserPointer(window);
  glfwGetFramebufferSize(window, &ptr->width, &ptr->height);
}
 
}

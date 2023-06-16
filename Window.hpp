#pragma once

#include <GLFW/glfw3.h>
#include "Camera.hpp"

namespace Sol {

#define WIDTH 640
#define HEIGHT 480

struct Window {
  static Window* instance();
  Camera* camera = Camera::instance();

  int width = WIDTH;
  int height = HEIGHT;

  double xpos = 0;
  double ypos = 0;

  double scroll = 0;

  GLFWwindow *window;

  void init(void* ptr);

  void init_glfw(void *ptr);
  inline void kill() {
    glfwDestroyWindow(window);
    glfwTerminate();
  }

  inline bool close() {
    return glfwWindowShouldClose(window);
  } 
  inline void poll() {
    glfwPollEvents();
  } 
  inline void size() {
    glfwGetFramebufferSize(window, &width, &height);
  }
  inline void wait() {
    glfwWaitEvents();
  }

  static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
  static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
  static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
  static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
};

static Window sWindow;

} // Sol

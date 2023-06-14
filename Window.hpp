#pragma once

#include <GLFW/glfw3.h>

namespace Sol {

#define WIDTH 640
#define HEIGHT 480

struct Window {
  static Window* instance();

  GLFWwindow *window;
  int width = WIDTH;
  int height = HEIGHT;
  void init(void* ptr);

  void init_glfw(void *ptr);
  void kill();

  bool close();
  void poll();
  void size();
  void wait();

  static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
  static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
};

static Window sWindow;

} // Sol

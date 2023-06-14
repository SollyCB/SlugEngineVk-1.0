#pragma once

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>

#include "Window.hpp"
#include "Vec.hpp"

namespace Sol {

#define V_LAYERS true

struct SwapchainSettings {
  VkSurfaceTransformFlagBitsKHR transform;
  VkExtent2D extent;
  VkSurfaceFormatKHR format;
  VkPresentModeKHR present_mode;
};

struct Engine {

public:
static Engine* instance();

void init();
void run();
void kill();

private:
// Window
  Window* window;

// Instance
  VkInstance vk_instance;
  void init_instance();
  void kill_instance();

// Surface 
  VkSurfaceKHR vk_surface;
  void init_surface();
  void kill_surface();

// Devices 
  VkPhysicalDevice vk_physical_device;
  VkDevice vk_device;
  uint32_t graphics_queue_index;
  uint32_t present_queue_index;
  VkQueue vk_graphics_queue;
  VkQueue vk_present_queue;
  void choose_device();
  void init_device();
  void kill_device();

// Swapchain
  VkSwapchainKHR vk_swapchain;
  SwapchainSettings swapchain_settings;
  Vec<VkImage> swapchain_images;
  Vec<VkImageView> swapchain_image_views;
  void init_swapchain();
  void kill_swapchain();
  void get_swapchain_settings();
  void get_swapchain_images();
  void get_swapchain_image_views();
  void kill_swapchain_image_views();
  void resize_swapchain();



// Debug //////////////////////
#if V_LAYERS
  VkDebugUtilsMessengerEXT debug_messenger;
  void init_debug();
  void kill_debug();
  static void populate_debug_create_info(VkDebugUtilsMessengerCreateInfoEXT* create_info);
  static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData);
  VkResult vkCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
  void vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
#endif
};

} // Sol

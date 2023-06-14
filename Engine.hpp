#pragma once

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>

#include "Window.hpp"
#include "Vec.hpp"

namespace Sol {

#define V_LAYERS true

#define MAX_FRAME_COUNT 2

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

// Viewport
  VkViewport get_viewport();
  VkRect2D get_scissor();

// Renderpass
  VkRenderPass vk_renderpass;
  void init_renderpass();
  void kill_renderpass();

// Pipeline
  VkPipeline vk_pipeline;
  VkPipelineLayout vk_layout;
  void init_pipeline();
  void kill_pipeline();
  VkShaderModule create_shader_module(const char* file_name);

// Framebuffer
  Vec<VkFramebuffer> vk_framebuffers;
  void init_framebuffers();
  void kill_framebuffers();
  void resize_framebuffers();

// Command
  Vec<VkCommandBuffer> vk_commandbuffers;
  Vec<VkCommandPool> vk_commandpools;
  void init_command();
  void kill_command();
  void allocate_commandbuffers(uint32_t command_pool_index, uint32_t buffer_count);
  void record_command_buffer(VkCommandBuffer cmd, uint32_t image_index);

// Sync
  Vec<VkSemaphore> vk_semaphores;
  Vec<VkFence> vk_fences;
  void init_sync();
  void kill_sync();
  uint32_t create_semaphores(uint32_t count, bool binary);
  uint32_t create_fences(uint32_t count, bool signalled);

// Loop
  void render_loop();
  void draw_frame(uint32_t *frame_index);






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

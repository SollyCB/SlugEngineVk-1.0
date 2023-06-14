#include <vulkan/vulkan.hpp>

#include "Engine.hpp"
#include "VulkanErrors.hpp"
#include "FeaturesExtensions.hpp"

#include <iostream>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

namespace Sol {

static Engine Slug;
Engine* Engine::instance() {
  return &Slug;
}

// *Public ////////////////
void Engine::init() {
  window = Window::instance();
  window->init(window);
  init_instance();
  init_surface();
#if V_LAYERS 
  init_debug();
#endif
  init_device();
  init_swapchain();
}
void Engine::run() {
  int height = window->height;
  int width = window->width;

  while(!window->close()) {
    // TODO:: These calls are blocking and slow, move to a different thread...
    window->poll();

    if (window->height != height || window->width != width) {
      vkDeviceWaitIdle(vk_device);

      while (window->height == 0 || window->width == 0) 
        std::cout << "wait";

      height = window->height;
      width = window->width;

      resize_swapchain();
    }

    // draw()
  }

}
void Engine::kill() {
  kill_swapchain();
  kill_device();
  kill_surface();
#if V_LAYERS
  kill_debug();
#endif
  kill_instance();
}

// *Instance ////////////////////
void Engine::init_instance() {
  VkApplicationInfo app_info{};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "VK Engine";
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "VK Engine";
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.apiVersion = VK_API_VERSION_1_3;

  VkInstanceCreateInfo instance_info;
  instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_info.pApplicationInfo = &app_info;
  instance_info.flags = 0x0;

#if V_LAYERS 
  VkDebugUtilsMessengerCreateInfoEXT debug_create_info;
  populate_debug_create_info(&debug_create_info);
  instance_info.pNext = &debug_create_info;
#else 
  instance_info.pNext = nullptr;
#endif

  uint32_t layer_count;
  InstanceExtensionsFeatures::get_layer_names(&layer_count, nullptr, V_LAYERS);
  const char* layer_names[layer_count];
  InstanceExtensionsFeatures::get_layer_names(&layer_count, layer_names, V_LAYERS);
  instance_info.enabledLayerCount = layer_count;
  instance_info.ppEnabledLayerNames = layer_names;

  uint32_t glfw_ext_count;
  const char** glfw_ext = glfwGetRequiredInstanceExtensions(&glfw_ext_count);

  uint32_t other_ext_count;
  InstanceExtensionsFeatures::get_ext_names(&other_ext_count, nullptr, V_LAYERS);
  const char* other_exts[other_ext_count];
  InstanceExtensionsFeatures::get_ext_names(&other_ext_count, other_exts, V_LAYERS);

  const char* exts[glfw_ext_count + other_ext_count];
  for(int i = 0; i < glfw_ext_count; ++i) 
    exts[i] = glfw_ext[i];
  for(int i = glfw_ext_count; i < other_ext_count + glfw_ext_count; ++i) 
    exts[i] = other_exts[i - glfw_ext_count];
    
  instance_info.enabledExtensionCount = glfw_ext_count + other_ext_count;
  instance_info.ppEnabledExtensionNames = exts;

  VkResult check = vkCreateInstance(&instance_info, nullptr, &vk_instance);
  DEBUG_OBJ_CREATION(vkCreateInstance, check);
}
void Engine::kill_instance() {
  vkDestroyInstance(vk_instance, nullptr);
}

// *Surface //////////////////
void Engine::init_surface() {
  auto check = glfwCreateWindowSurface(vk_instance, window->window, NULL, &vk_surface);
  DEBUG_OBJ_CREATION(glfwCreateWindowSurface, check);
}
void Engine::kill_surface() {
  vkDestroySurfaceKHR(vk_instance, vk_surface, nullptr);
}

// *Device ////////////////////
void Engine::init_device() {
  choose_device();

  uint32_t queue_count;
  if (graphics_queue_index == present_queue_index)
    queue_count = 1;
  else 
    queue_count = 2;

  float priorities[] = { 1.0f };
  VkDeviceQueueCreateInfo queue_infos[2] = {
    {
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = graphics_queue_index,
      .queueCount = 1,
      .pQueuePriorities = priorities,
    },
    {
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = present_queue_index,
      .queueCount = 1,
      .pQueuePriorities = priorities,
    },
  };

  uint32_t ext_count;
  DeviceExtensionsFeatures::get_ext_names(&ext_count, nullptr);
  const char* device_extensions[ext_count];
  DeviceExtensionsFeatures::get_ext_names(&ext_count, device_extensions);

  VkDeviceCreateInfo info = {
    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .queueCreateInfoCount = queue_count,
    .pQueueCreateInfos = queue_infos, 
    .enabledLayerCount = 0,
    .enabledExtensionCount = ext_count,
    .ppEnabledExtensionNames = device_extensions,
    .pEnabledFeatures = nullptr,
  };

  auto check = vkCreateDevice(vk_physical_device, &info, nullptr, &vk_device);
  DEBUG_OBJ_CREATION(vkCreateDevice, check);
  
  vkGetDeviceQueue(vk_device, graphics_queue_index, 0, &vk_graphics_queue);
  vkGetDeviceQueue(vk_device, present_queue_index, 0, &vk_present_queue);
}
void Engine::kill_device() {
  vkDestroyDevice(vk_device, nullptr);
}
void Engine::choose_device() {
  uint32_t count;
  vkEnumeratePhysicalDevices(vk_instance, &count, nullptr);
  VkPhysicalDevice devices[count];
  vkEnumeratePhysicalDevices(vk_instance, &count, devices);

  for(uint32_t i = 0; i < count; ++i) {
    VkPhysicalDeviceProperties device_props;
    vkGetPhysicalDeviceProperties(devices[i], &device_props);
    bool discrete = false;
    if (device_props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
      discrete = true;

    uint32_t queue_prop_count;
    vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &queue_prop_count, nullptr);
    VkQueueFamilyProperties queue_props[queue_prop_count];
    vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &queue_prop_count, queue_props);

    bool graphics = false;
    VkBool32 present = VK_FALSE;
    for(uint32_t j = 0; j < queue_prop_count; ++j) {
      if (queue_props[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        graphics = true;
        graphics_queue_index = j; 
      }

      if (!present) {
        vkGetPhysicalDeviceSurfaceSupportKHR(devices[i], j, vk_surface, &present);
        present_queue_index = j;
      }
    }

    if (graphics && present && discrete) {
      std::cout << "Chose device " << device_props.deviceName << '\n';
      vk_physical_device = devices[i];
      return;
    }
  }
}

// *Swapchain /////////////////////////
void Engine::init_swapchain() {
  get_swapchain_settings();

  VkSwapchainCreateInfoKHR info = {
    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .surface = vk_surface,
    .minImageCount = 2,
    .imageFormat = swapchain_settings.format.format,
    .imageColorSpace = swapchain_settings.format.colorSpace,
    .imageExtent = swapchain_settings.extent, 
    .imageArrayLayers = 1,
    .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .preTransform = swapchain_settings.transform,
    .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    .presentMode = swapchain_settings.present_mode,
    .clipped = VK_TRUE,
    .oldSwapchain = VK_NULL_HANDLE,
  };

  auto check = vkCreateSwapchainKHR(vk_device, &info, nullptr, &vk_swapchain);
  DEBUG_OBJ_CREATION(vkCreateSwapchainKHR, check);

  swapchain_images.init(2);
  swapchain_image_views.init(2);

  get_swapchain_images();
  get_swapchain_image_views();
}
void Engine::kill_swapchain() {
  kill_swapchain_image_views();
  vkDestroySwapchainKHR(vk_device, vk_swapchain, nullptr);
  swapchain_image_views.kill();
  swapchain_images.kill();
}
void Engine::get_swapchain_settings() {
  VkSurfaceCapabilitiesKHR capabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk_physical_device, vk_surface, &capabilities);
  swapchain_settings.transform = capabilities.currentTransform;
  swapchain_settings.extent = capabilities.currentExtent;

  uint32_t format_count;
  vkGetPhysicalDeviceSurfaceFormatsKHR(vk_physical_device, vk_surface, &format_count, nullptr);
  VkSurfaceFormatKHR formats[format_count];
  vkGetPhysicalDeviceSurfaceFormatsKHR(vk_physical_device, vk_surface, &format_count, formats);

  uint32_t mode_count;
  vkGetPhysicalDeviceSurfacePresentModesKHR(vk_physical_device, vk_surface, &mode_count, nullptr);
  VkPresentModeKHR modes[mode_count];
  vkGetPhysicalDeviceSurfacePresentModesKHR(vk_physical_device, vk_surface, &mode_count, modes);

  bool format_check = false;
  for(uint32_t i = 0; i < format_count; ++i) {
    if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB
        && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
    { swapchain_settings.format = formats[i]; format_check = true; break; }
  }
  if (!format_check)
    swapchain_settings.format = formats[0];

  bool mode = false;
  for(uint32_t i = 0; i < mode_count; ++i) {
    if (modes[i] == VK_PRESENT_MODE_FIFO_KHR) 
    { mode = true; swapchain_settings.present_mode = modes[i]; break; }
  }

  DEBUG_ABORT(mode, "Bad swapchain settings (present mode)");
  DEBUG_ABORT(format_check, "Bad swapchain settings (format)");
}
void Engine::get_swapchain_images() {
  uint32_t image_count;
  vkGetSwapchainImagesKHR(vk_device, vk_swapchain, &image_count, nullptr);
  swapchain_images.resize(image_count);
  vkGetSwapchainImagesKHR(vk_device, vk_swapchain, &image_count, swapchain_images.data);
  swapchain_images.length = image_count;
}
void Engine::get_swapchain_image_views() {
  swapchain_image_views.length = swapchain_images.length;
  for(uint32_t i = 0; i < swapchain_image_views.length; ++i) {
    VkImageViewCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = swapchain_images[i],
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = swapchain_settings.format.format,
      .components = {
        .r = VK_COMPONENT_SWIZZLE_IDENTITY,
        .g = VK_COMPONENT_SWIZZLE_IDENTITY,
        .b = VK_COMPONENT_SWIZZLE_IDENTITY,
        .a = VK_COMPONENT_SWIZZLE_IDENTITY,
      },
      .subresourceRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
      },
    };
    auto check = vkCreateImageView(vk_device, &info, nullptr, &swapchain_image_views[i]);
    DEBUG_OBJ_CREATION(vkCreateImageView, check);
  }
}
void Engine::kill_swapchain_image_views() {
  for(int i = 0; i < swapchain_image_views.length; ++i)
    vkDestroyImageView(vk_device, swapchain_image_views[i], nullptr);
}
void Engine::resize_swapchain() {
  VkSwapchainKHR old_swapchain = vk_swapchain;

  kill_swapchain_image_views();

  VkSurfaceCapabilitiesKHR capabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk_physical_device, vk_surface, &capabilities);
  swapchain_settings.transform = capabilities.currentTransform;
  swapchain_settings.extent = capabilities.currentExtent;

  VkSwapchainCreateInfoKHR info = {
    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .surface = vk_surface,
    .minImageCount = 2,
    .imageFormat = swapchain_settings.format.format,
    .imageColorSpace = swapchain_settings.format.colorSpace,
    .imageExtent = swapchain_settings.extent, 
    .imageArrayLayers = 1,
    .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .preTransform = swapchain_settings.transform,
    .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    .presentMode = swapchain_settings.present_mode,
    .clipped = VK_TRUE,
    .oldSwapchain = old_swapchain,
  };

  auto check = vkCreateSwapchainKHR(vk_device, &info, nullptr, &vk_swapchain);
  DEBUG_OBJ_CREATION(vkCreateSwapchainKHR, check);

  vkDestroySwapchainKHR(vk_device, old_swapchain, nullptr);

  get_swapchain_images();
  get_swapchain_image_views();
}



#if V_LAYERS
void Engine::init_debug() {
  VkDebugUtilsMessengerCreateInfoEXT debug_create_info;
  populate_debug_create_info(&debug_create_info);
  VkResult check = vkCreateDebugUtilsMessengerEXT(vk_instance, &debug_create_info, nullptr, &debug_messenger);

  DEBUG_ABORT(check == VK_SUCCESS,  "FAILED TO INIT DEBUG MESSENGER (Aborting)...\n");
}
void Engine::kill_debug() {
  vkDestroyDebugUtilsMessengerEXT(vk_instance, debug_messenger, nullptr);
}

void Engine::populate_debug_create_info(VkDebugUtilsMessengerCreateInfoEXT* create_info) {
  create_info->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  create_info->flags = 0x0;
  create_info->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  create_info->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  create_info->pfnUserCallback = debug_callback;
  create_info->pNext = nullptr;
  create_info->pUserData = nullptr;
}

VKAPI_ATTR VkBool32 VKAPI_CALL Engine::debug_callback(
  VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
  VkDebugUtilsMessageTypeFlagsEXT messageType,
  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
  void* pUserData) {

  if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
  }

  return VK_FALSE;
}

VkResult Engine::vkCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr) {
      return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
  } else {
      return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

void Engine::vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr) {
      func(instance, debugMessenger, pAllocator);
  }
}
#endif

}

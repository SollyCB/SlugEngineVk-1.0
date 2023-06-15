#include <cmath>
#include <cstdint>
#include <chrono>
#include <exception>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vector_relational.hpp>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>

#include "Engine.hpp"
#include "VulkanErrors.hpp"
#include "FeaturesExtensions.hpp"
#include "File.hpp"

#include <iostream>
#include <GLFW/glfw3.h>

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
  init_allocator();
  init_swapchain();
  init_renderpass();
  init_desc_pool();
  init_desc_set_layout();
  init_framebuffers();
  init_pipeline();
  init_command();
  init_sync();
}
void Engine::kill() {
  kill_sync();
  kill_command();
  kill_pipeline();
  kill_desc_set_layout();
  kill_desc_pool();
  kill_framebuffers();
  kill_renderpass();
  kill_swapchain();
  kill_allocator();
  kill_device();
  kill_surface();
#if V_LAYERS
  kill_debug();
#endif
  kill_instance();
}

void Engine::run() {
  render_loop();
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


// *Allocator /////////////////////////
void Engine::init_allocator() {
  VmaVulkanFunctions vulkanFunctions = {};
  vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
  vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;
   
  VmaAllocatorCreateInfo allocatorCreateInfo = {};
  allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_3;
  allocatorCreateInfo.physicalDevice = vk_physical_device;
  allocatorCreateInfo.device = vk_device;
  allocatorCreateInfo.instance = vk_instance;
  allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;
   
  auto check = vmaCreateAllocator(&allocatorCreateInfo, &vma_allocator);
  DEBUG_OBJ_CREATION(vmaCreateAllocator, check);
}
void Engine::kill_allocator() {
  free_buffer(vert_buf);
  vmaDestroyAllocator(vma_allocator);
}
VkResult Engine::alloc_buffer(
    size_t size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags pref_flags,
    VkMemoryPropertyFlags req_flags,
    VmaAllocationCreateFlags vma_flags,
    GpuBuffer *buf) 
{
  VkBufferCreateInfo bufCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
  bufCreateInfo.size = size;
  bufCreateInfo.usage = usage;
   
  VmaAllocationCreateInfo allocCreateInfo = {};
  allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
  allocCreateInfo.requiredFlags = req_flags;
  allocCreateInfo.preferredFlags = pref_flags;
  allocCreateInfo.flags = vma_flags;
   
  auto check = vmaCreateBuffer(
      vma_allocator, 
      &bufCreateInfo,
      &allocCreateInfo,
      &buf->buf,
      &buf->alloc,
      &buf->alloc_info);

  DEBUG_OBJ_CREATION(vmaCreateBuffer, check);

  return check;
}
void Engine::free_buffer(GpuBuffer buf) {
  vmaDestroyBuffer(vma_allocator, buf.buf, buf.alloc);
}

// *VertexBuffer ////////////////////
VertexBuffer::~VertexBuffer() {}
// *StagingBuffer ///////////////////
StagingBuffer::~StagingBuffer() {}
void Engine::alloc_staging_buf(size_t size, void* data) {
  VkBufferCreateInfo bufCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
  bufCreateInfo.size = size;
  bufCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
   
  VmaAllocationCreateInfo allocCreateInfo = {};
  allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
  allocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
      VMA_ALLOCATION_CREATE_MAPPED_BIT;
   
  auto check = vmaCreateBuffer(
      vma_allocator,
      &bufCreateInfo,
      &allocCreateInfo,
      &staging_buf.buf,
      &staging_buf.alloc,
      &staging_buf.alloc_info);
  DEBUG_OBJ_CREATION(vmaCreateBuffer, check);
   
  memcpy(staging_buf.alloc_info.pMappedData, data, size);
}
void Engine::alloc_vert_buf(size_t size) {
  VkBufferCreateInfo bufCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
  bufCreateInfo.size = size;
  bufCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | 
    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | 
    VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
   
  VmaAllocationCreateInfo allocCreateInfo = {};
  allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
  allocCreateInfo.flags = 0x0;
  allocCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
   
  auto check = vmaCreateBuffer(
      vma_allocator,
      &bufCreateInfo,
      &allocCreateInfo,
      &vert_buf.buf,
      &vert_buf.alloc,
      &vert_buf.alloc_info);
   
  DEBUG_OBJ_CREATION(vmaCreateBuffer, check);
}

// *UBOs /////////////////////
void Engine::alloc_ubos(size_t size) {
  ubos.length = MAX_FRAME_COUNT;
  for(int i = 0; i < MAX_FRAME_COUNT; ++i) {
    VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufferInfo.size = sizeof(UBO);
    bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
     
    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | 
      VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    allocInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
     
    auto check = 
      vmaCreateBuffer(
          vma_allocator,
          &bufferInfo,
          &allocInfo,
          &ubos[i].buf,
          &ubos[i].alloc,
          &ubos[i].alloc_info);
    DEBUG_OBJ_CREATION(vmaCreateBuffer, check);
  }
}
void Engine::kill_ubos() {
  for(int i = 0; i < ubos.length; ++i) 
    vmaDestroyBuffer(vma_allocator, ubos[i].buf, ubos[i].alloc);
  ubos.kill();
}
void Engine::update_ubo(uint32_t frame_index) {
  static auto startTime = std::chrono::steady_clock::now();

  auto currentTime = std::chrono::steady_clock::now();
  float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

  UBO ubo = {};
  ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

  ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

  ubo.projection = glm::perspective(glm::radians(45.0f), swapchain_settings.extent.width / (float) swapchain_settings.extent.height, 0.1f, 10.0f);
  ubo.projection[1][1] *= -1;

  memcpy(ubos[frame_index].alloc_info.pMappedData, &ubo, sizeof(ubo));
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
  vk_framebuffers.init(2);

  get_swapchain_images();
  get_swapchain_image_views();
}
void Engine::kill_swapchain() {
  kill_swapchain_image_views();
  vkDestroySwapchainKHR(vk_device, vk_swapchain, nullptr);
  swapchain_image_views.kill();
  vk_framebuffers.kill();
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


// *Viewport //////////////////
VkViewport Engine::get_viewport() {
  return {
    .x = 0.0f,
    .y = 0.0f, 
    .width = (float)swapchain_settings.extent.width,
    .height = (float)swapchain_settings.extent.height,
    .minDepth = 0.0f,
    .maxDepth = 1.0f,
  };
}
VkRect2D Engine::get_scissor() {
  return {
    .offset = { 0, 0 },
    .extent = swapchain_settings.extent,
  };
}


// *Renderpass ////////////////
void Engine::init_renderpass() {
  VkAttachmentDescription attachment_description = {
    .format = swapchain_settings.format.format,
    .samples = VK_SAMPLE_COUNT_1_BIT,
    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
  };
  VkAttachmentReference attachment_reference = {
    .attachment = 0,
    .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
  };
  VkSubpassDescription subpass_description = {
    .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
    .colorAttachmentCount = 1,
    .pColorAttachments = &attachment_reference,
  };
  VkSubpassDependency subpass_dependency = {
    .srcSubpass = VK_SUBPASS_EXTERNAL,
    .dstSubpass = 0,
    .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    .srcAccessMask = 0,
    .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
  };
  VkRenderPassCreateInfo renderpass_info = {
    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
    .attachmentCount = 1,
    .pAttachments = &attachment_description,
    .subpassCount = 1,
    .pSubpasses = &subpass_description,
    .dependencyCount = 1,
    .pDependencies = &subpass_dependency,
  };

  auto check = vkCreateRenderPass(vk_device, &renderpass_info, nullptr, &vk_renderpass);
  DEBUG_OBJ_CREATION(vkCreateRenderPass, check);
}
void Engine::kill_renderpass() {
  vkDestroyRenderPass(vk_device, vk_renderpass, nullptr);
}


// *Framebuffer
void Engine::init_framebuffers() {
  vk_framebuffers.length = swapchain_image_views.length;

  for(uint32_t i = 0; i < swapchain_image_views.length; ++i) {
    VkFramebufferCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
      .renderPass = vk_renderpass,
      .attachmentCount = 1,
      .pAttachments = &swapchain_image_views[i],
      .width = swapchain_settings.extent.width,
      .height = swapchain_settings.extent.height,
      .layers = 1,
    };
    auto check = vkCreateFramebuffer(vk_device, &info, nullptr, &vk_framebuffers[i]);
    DEBUG_OBJ_CREATION(vkCreateFramebuffer, check);
  }
}
void Engine::kill_framebuffers() {
  for(int i = 0; i < vk_framebuffers.length; ++i)
    vkDestroyFramebuffer(vk_device, vk_framebuffers[i], nullptr);
}
void Engine::resize_framebuffers() {
  kill_framebuffers();
  init_framebuffers();
}


// *Descriptors
void Engine::init_desc_pool() {
  VkDescriptorPoolSize pool_size = {};
  pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  pool_size.descriptorCount = MAX_FRAME_COUNT;

  VkDescriptorPoolCreateInfo pool_info = {};
  pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.poolSizeCount = 1;
  pool_info.pPoolSizes = &pool_size;
  pool_info.maxSets = MAX_FRAME_COUNT;

  auto check = vkCreateDescriptorPool(vk_device, &pool_info, nullptr, &desc_pool);
  DEBUG_OBJ_CREATION(vkCreateDescriptorPool, check);
}
void Engine::kill_desc_pool() {
  vkDestroyDescriptorPool(vk_device, desc_pool, nullptr);
}
void Engine::init_desc_set_layout() {
  ubos.init(MAX_FRAME_COUNT);
  ubos.length = MAX_FRAME_COUNT;
  size_t size = sizeof(UBO);
  alloc_ubos(size);

  VkDescriptorSetLayoutBinding ubo_layout_binding = {};
  ubo_layout_binding.binding = 0;
  ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  ubo_layout_binding.descriptorCount = 1;
  ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  ubo_layout_binding.pImmutableSamplers = nullptr; // Optional

  VkDescriptorSetLayoutCreateInfo layout_info = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
  layout_info.bindingCount = 1;
  layout_info.pBindings = &ubo_layout_binding;

  auto check_layouts = 
    vkCreateDescriptorSetLayout(vk_device, &layout_info, nullptr, &vk_desc_set_layout);
  DEBUG_OBJ_CREATION(vkCreateDescriptorSetLayout, check_layouts);

  VkDescriptorSetLayout layouts[] = { vk_desc_set_layout, vk_desc_set_layout };
  VkDescriptorSetAllocateInfo alloc_info = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
  alloc_info.descriptorPool = desc_pool;
  alloc_info.descriptorSetCount = MAX_FRAME_COUNT;
  alloc_info.pSetLayouts = layouts;

  auto check_sets = 
    vkAllocateDescriptorSets(vk_device, &alloc_info, desc_sets);
  DEBUG_OBJ_CREATION(vkAllocateDescriptorSets, check_sets);

  for(int i = 0; i < MAX_FRAME_COUNT; ++i) {
    VkDescriptorBufferInfo buf_info = {};
    buf_info.buffer = ubos[i].buf;
    buf_info.offset = 0;
    buf_info.range = sizeof(UBO);

    VkWriteDescriptorSet desc_write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
    desc_write.dstSet = desc_sets[i];
    desc_write.dstBinding = 0;
    desc_write.dstArrayElement = 0;
    desc_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    desc_write.descriptorCount = 1;
    desc_write.pBufferInfo = &buf_info;

    vkUpdateDescriptorSets(vk_device, 1, &desc_write, 0, nullptr);
  }
}
void Engine::kill_desc_set_layout() {
  vkDestroyDescriptorSetLayout(vk_device, vk_desc_set_layout, nullptr);
  kill_ubos();
}


// *Pipeline ///////////////////////
void Engine::init_pipeline() {
  VkShaderModule vertex_module = create_shader_module("shaders/triangle3.vert.spv");
  VkShaderModule fragment_module = create_shader_module("shaders/triangle3.frag.spv");
  VkPipelineShaderStageCreateInfo stages[] = {
    {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_VERTEX_BIT,
      .module = vertex_module,
      .pName = "main",
    }, {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
      .module = fragment_module,
      .pName = "main",
    },
  };

  VkVertexInputBindingDescription input_desc;
  Vertex::get_binding_description(&input_desc);
  VkVertexInputAttributeDescription attribute_descs[2];
  Vertex::get_attribute_description(attribute_descs);

  VkPipelineVertexInputStateCreateInfo vertex_input_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .vertexBindingDescriptionCount = 1,
    .pVertexBindingDescriptions = &input_desc,
    .vertexAttributeDescriptionCount = 2,
    .pVertexAttributeDescriptions = attribute_descs,
  };

  VkPipelineInputAssemblyStateCreateInfo vertex_assembly_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
  };

  VkViewport viewport = get_viewport();

  VkRect2D scissor = get_scissor();

  VkPipelineViewportStateCreateInfo viewport_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
    .viewportCount = 1,
    .pViewports = &viewport,
    .scissorCount = 1,
    .pScissors = &scissor,
  };

  VkPipelineRasterizationStateCreateInfo rasterization_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    .depthClampEnable = VK_FALSE,
    .rasterizerDiscardEnable = VK_FALSE,
    .polygonMode = VK_POLYGON_MODE_FILL,
    .cullMode = VK_CULL_MODE_BACK_BIT,
    .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
    .depthBiasEnable = VK_FALSE,
    .lineWidth = 1.0f,
  };

  VkPipelineMultisampleStateCreateInfo multisample_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
    .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
    .sampleShadingEnable = VK_FALSE,
  };

  VkPipelineColorBlendAttachmentState blend_attachment = {
    .blendEnable = VK_FALSE,
    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
  };

  VkPipelineColorBlendStateCreateInfo blend_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .logicOpEnable = VK_FALSE,
    .attachmentCount = 1,
    .pAttachments = &blend_attachment,
  };

  VkPipelineLayoutCreateInfo layout_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .setLayoutCount = 1,
    .pSetLayouts = &vk_desc_set_layout,
  };
  auto check_layout = vkCreatePipelineLayout(vk_device, &layout_info, nullptr, &vk_layout);

  VkDynamicState dyn_states[] = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR,
  };

  VkPipelineDynamicStateCreateInfo dyn_state_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
    .dynamicStateCount = 2,
    .pDynamicStates = dyn_states,
  };

  VkGraphicsPipelineCreateInfo pipeline_info = {
    .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    .stageCount = 2,
    .pStages = stages,
    .pVertexInputState = &vertex_input_state,
    .pInputAssemblyState = &vertex_assembly_state,
    .pViewportState = &viewport_state,
    .pRasterizationState = &rasterization_state,
    .pMultisampleState = &multisample_state,
    .pColorBlendState = &blend_state,
    .pDynamicState = &dyn_state_info,
    .layout = vk_layout,
    .renderPass = vk_renderpass,
    .subpass = 0,
  };

  auto check_pipeline = vkCreateGraphicsPipelines(vk_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &vk_pipeline);
  DEBUG_OBJ_CREATION(vkCreateGraphicsPipelines, check_pipeline);

  vkDestroyShaderModule(vk_device, vertex_module, nullptr);
  vkDestroyShaderModule(vk_device, fragment_module, nullptr);
}
void Engine::kill_pipeline() {
  vkDestroyPipeline(vk_device, vk_pipeline, nullptr);
  vkDestroyPipelineLayout(vk_device, vk_layout, nullptr);
}
VkShaderModule Engine::create_shader_module(const char* file_name) {
  size_t code_size;
  const uint32_t *p_code = (const uint32_t*)File::read_spirv(&code_size, file_name);
  VkShaderModuleCreateInfo create_info = {
    .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
    .codeSize = code_size, 
    .pCode = p_code,
  };
  VkShaderModule module;
  auto check = vkCreateShaderModule(vk_device, &create_info, nullptr, &module);
  DEBUG_OBJ_CREATION(vkCreateShaderModule, check);

  mem_free(p_code);
  return module;
}


// *Command ////////////////////
void Engine::init_command() {
  vk_commandpools.init(2);
  vk_commandbuffers.init(2);
  vk_commandpools.length = 2;

  // TODO: : This will break if present and graphics queues have different indices...
  ABORT(graphics_queue_index == present_queue_index, "Queue families (present and graphics) are not equal");
  for(uint32_t i = 0; i < 2; ++i) { 
    VkCommandPoolCreateInfo pool_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .queueFamilyIndex = graphics_queue_index, 
    };
    auto check_pool = vkCreateCommandPool(vk_device, &pool_info, nullptr, &vk_commandpools[i]);
    DEBUG_OBJ_CREATION(vkCreateCommandPool, check_pool);
  }

  for(uint32_t i = 0; i < 2; ++i) 
    allocate_commandbuffers(i, 1);
}
void Engine::kill_command() {
  for(int i = 0; i < vk_commandpools.length; ++i) 
    vkDestroyCommandPool(vk_device, vk_commandpools[i], nullptr);

  vk_commandbuffers.kill();
  vk_commandpools.kill();
}
uint32_t Engine::allocate_commandbuffers(uint32_t pool_index, uint32_t buffer_count) {
  uint32_t length = (uint32_t)vk_commandbuffers.length;
  if (vk_commandbuffers.capacity - length < buffer_count)
    vk_commandbuffers.grow(buffer_count);

  VkCommandBufferAllocateInfo info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .commandPool = vk_commandpools[pool_index],
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandBufferCount = buffer_count,
  };
  auto check = vkAllocateCommandBuffers(vk_device, &info, vk_commandbuffers.data + vk_commandbuffers.length);
  DEBUG_OBJ_CREATION(vkAllocateCommandBuffers, check);

  vk_commandbuffers.length += buffer_count;
  return length;
}
void Engine::record_command_buffer(VkCommandBuffer cmd, uint32_t image_index) {
  VkCommandBufferBeginInfo cmd_begin_info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };

  auto check_begin_buffer = vkBeginCommandBuffer(cmd, &cmd_begin_info);
  DEBUG_OBJ_CREATION(vkBeginCommandBuffer, check_begin_buffer);

  VkClearValue clear_color = {{{ 0.0f, 0.0f, 0.0f }}};
  VkRenderPassBeginInfo renderpass_info = {
    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
    .renderPass = vk_renderpass,
    .framebuffer = vk_framebuffers[image_index],
    .renderArea = { 
      .offset = { 0, 0 },
      .extent = swapchain_settings.extent,
    },
    .clearValueCount = 1,
    .pClearValues = &clear_color,
  };

  vkCmdBeginRenderPass(cmd, &renderpass_info, VK_SUBPASS_CONTENTS_INLINE);
  vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline);

  VkDeviceSize offset = 0;
  vkCmdBindVertexBuffers(cmd, 0, 1, &vert_buf.buf, &offset);
  vkCmdBindIndexBuffer(cmd, vert_buf.buf, sizeof(Vertex) * 4, VK_INDEX_TYPE_UINT32);

  VkViewport viewport = get_viewport();
  VkRect2D scissor = get_scissor();
  vkCmdSetViewport(cmd, 0, 1, &viewport);
  vkCmdSetScissor(cmd, 0, 1, &scissor);

  vkCmdBindDescriptorSets(
      cmd,
      VK_PIPELINE_BIND_POINT_GRAPHICS,
      vk_layout,
      0,
      1,
      &desc_sets[current_frame],
      0,
      nullptr);

  //vkCmdDraw(cmd, 3, 1, 0, 0);
  vkCmdDrawIndexed(cmd, index_count, 1, 0, 0, 0);

  vkCmdEndRenderPass(cmd);
  auto check_end_buffer = vkEndCommandBuffer(cmd);
  DEBUG_OBJ_CREATION(vkEndCommandBuffer, check_end_buffer);
}
void Engine::record_and_submit_cpy(size_t size, size_t index_offset) {
  uint32_t pool_index = 0;
  uint32_t index = allocate_commandbuffers(pool_index, 1); 

  VkCommandBuffer cmd = vk_commandbuffers[index];
  VkCommandBufferBeginInfo begin_info{};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(cmd, &begin_info);
    
    VkBufferCopy copy_region{};
    copy_region.size = sizeof(Vertex) * vertex_count + sizeof(Index) * index_count;
    vkCmdCopyBuffer(cmd, staging_buf.buf, vert_buf.buf, 1, &copy_region);

  vkEndCommandBuffer(cmd);

  VkSubmitInfo submit_info{};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &cmd;

  vkQueueSubmit(vk_graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
  vkQueueWaitIdle(vk_graphics_queue);
  vkResetCommandPool(vk_device, vk_commandpools[pool_index], 0x0);
  vkFreeCommandBuffers(vk_device, vk_commandpools[pool_index], 1, &cmd);
}

// *Sync /////////////////
void Engine::init_sync() {
  vk_semaphores.init(4); 
  vk_fences.init(2);

  create_semaphores(4, true);
  create_fences(2, true);
}
void Engine::kill_sync() {
  for(uint32_t i = 0; i < vk_semaphores.length; ++i)
    vkDestroySemaphore(vk_device, vk_semaphores[i], nullptr);
  for(uint32_t i = 0; i < vk_fences.length; ++i) 
    vkDestroyFence(vk_device, vk_fences[i], nullptr);

  vk_semaphores.kill();
  vk_fences.kill();
}
uint32_t Engine::create_semaphores(uint32_t count, bool binary) {
  uint32_t length = vk_semaphores.length;
  if (vk_semaphores.capacity - length < count)
    vk_semaphores.grow(count * 2);

  for(int i = 0; i < count; ++i) {
    VkSemaphoreTypeCreateInfo timeline = {};

    VkSemaphoreCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
      .pNext = binary ? nullptr : &timeline,
    };
    auto check = vkCreateSemaphore(vk_device, &info, nullptr, vk_semaphores.data + vk_semaphores.length);
    DEBUG_OBJ_CREATION(vkCreateSemaphore, check);
    ++vk_semaphores.length;
  }

  return length;
}
uint32_t Engine::create_fences(uint32_t count, bool signalled) {
  uint32_t length = vk_fences.length;
  if (vk_fences.capacity - length < count)
    vk_fences.grow(count * 2);

  for(int i = 0; i < count; ++i) {
    VkFenceCreateFlags flags = signalled ? VK_FENCE_CREATE_SIGNALED_BIT : 0x0;
    VkFenceCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .flags = flags,
    };
    auto check = vkCreateFence(vk_device, &info, nullptr, vk_fences.data + vk_fences.length);
    DEBUG_OBJ_CREATION(vkCreateFence, check);
    ++vk_fences.length;
  }

  return length;
}


// *Loop /////////////////
void Engine::render_loop() {
  int height = window->height;
  int width = window->width;

  current_frame = 0;

  size_t size = sizeof(IndexVertex);
  IndexVertex index_vertex;
  alloc_staging_buf(size, &index_vertex); 
  alloc_vert_buf(size);
  record_and_submit_cpy(size, sizeof(Vertex) * 4);
  vmaDestroyBuffer(vma_allocator, staging_buf.buf, staging_buf.alloc);

  while(!window->close()) {
    // TODO:: These calls are blocking and slow, move to a different thread...
    window->poll();

    if (window->height != height || window->width != width) {

      while (window->height == 0 || window->width == 0) 
        window->wait();

      height = window->height;
      width = window->width;

      vkDeviceWaitIdle(vk_device);

      resize_swapchain();
      resize_framebuffers();
    }

    draw_frame(&current_frame);
  }

  vkDeviceWaitIdle(vk_device);
}

void Engine::draw_frame(uint32_t *frame_index) {
  VkFence render_done_fence = vk_fences[*frame_index];
  VkSemaphore image_available = vk_semaphores[*frame_index];
  VkSemaphore render_done = vk_semaphores[*frame_index + 1];
  VkCommandBuffer cmd = vk_commandbuffers[*frame_index];

  vkWaitForFences(vk_device, 1, &render_done_fence, VK_TRUE, UINT64_MAX);

  uint32_t image_index;
  auto check_acquire = vkAcquireNextImageKHR(vk_device, vk_swapchain, UINT64_MAX, image_available, VK_NULL_HANDLE, &image_index);

  switch(check_acquire) {
    case VK_SUCCESS:
    case VK_ERROR_OUT_OF_DATE_KHR: 
      break;
    case VK_SUBOPTIMAL_KHR:
      return;

    default:
      ABORT(false, "Failed to acquire swapchain image");
  }


  vkResetFences(vk_device, 1, &render_done_fence);
  vkResetCommandPool(vk_device, vk_commandpools[*frame_index], 0x0);
  record_command_buffer(cmd, image_index);

  update_ubo(*frame_index);

  VkPipelineStageFlags output_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  VkSubmitInfo graphics_submit_info = {
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores = &image_available,
    .pWaitDstStageMask = &output_stage,
    .commandBufferCount = 1,
    .pCommandBuffers = &cmd,
    .signalSemaphoreCount = 1,
    .pSignalSemaphores = &render_done,
  };
  auto check_graphics_submit = 
    vkQueueSubmit(vk_graphics_queue, 1, &graphics_submit_info, render_done_fence);
  DEBUG_OBJ_CREATION(vkQueueSubmit, check_graphics_submit);

  VkPresentInfoKHR present_info = {
    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores = &render_done,
    .swapchainCount = 1,
    .pSwapchains = &vk_swapchain,
    .pImageIndices = &image_index,
  };
  auto check_present = vkQueuePresentKHR(vk_present_queue, &present_info);

  *frame_index = (*frame_index + 1) % MAX_FRAME_COUNT;

  switch(check_present) {
    case VK_SUCCESS:
      break;
    case VK_ERROR_OUT_OF_DATE_KHR:
    case VK_SUBOPTIMAL_KHR:
      return;
    default:
      ABORT(false, "Presentation check failed");
  }
}




// *Debug //////////////////////
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

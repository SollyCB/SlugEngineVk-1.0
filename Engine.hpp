#pragma once

#include <glm/glm.hpp>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>

#include "Camera.hpp"
#include "Clock.hpp"
#include "Vec.hpp"
#include "Window.hpp"

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include "vk_mem_alloc.h"

namespace Sol {

#define V_LAYERS true

#define MAX_FRAME_COUNT 2

struct SwapchainSettings {
    VkSurfaceTransformFlagBitsKHR transform;
    VkExtent2D extent;
    VkSurfaceFormatKHR format;
    VkPresentModeKHR present_mode;
};

// NOTE:: BEWARE ALIGNMENT REQUIREMENTS!!
struct UBO {
    /* alignas(16) */ glm::mat4 model;
    /* alignas(16) */ glm::mat4 view;
    /* alignas(16) */ glm::mat4 projection;
};

struct Vertex {
    glm::vec2 position;
    glm::vec3 color;

    static void get_binding_description(VkVertexInputBindingDescription *desc) {
        *desc = {};
        desc->binding = 0;
        desc->stride = sizeof(Vertex);
        desc->inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    }
    static void
    get_attribute_description(VkVertexInputAttributeDescription *desc) {
        desc[0] = {};
        desc[0].binding = 0;
        desc[0].location = 0;
        desc[0].format = VK_FORMAT_R32G32_SFLOAT;
        desc[0].offset = offsetof(Vertex, position);

        desc[1] = {};
        desc[1].binding = 0;
        desc[1].location = 1;
        desc[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        desc[1].offset = offsetof(Vertex, color);
    }
};

const uint32_t vertex_count = 4;
const Vertex vertices[] = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},
};

typedef uint32_t Index;
const uint32_t index_count = 6;
const uint32_t indices[] = {
    0, 1, 2, 2, 3, 0,
};

struct IndexVertex {
    const Vertex vertexes[4] = {
        {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},
    };
    const Index indexes[6] = {
        0, 1, 2, 2, 3, 0,
    };
};

struct GpuBuffer {
    VkBuffer buf;
    VmaAllocation alloc;
    VmaAllocationInfo alloc_info;

    virtual ~GpuBuffer() {}
};

struct VertexBuffer : public GpuBuffer {
    ~VertexBuffer() override;

    uint32_t stride = sizeof(Vertex);
};
struct StagingBuffer : public GpuBuffer {
    ~StagingBuffer() override;
};
struct IndexBuffer : public GpuBuffer {
    ~IndexBuffer() override;
};

struct Engine {

  public:
    static Engine *instance();

    void init();
    void run();
    void kill();

    uint32_t current_frame;

  private:
    // Window
    Clock *clock = Clock::instance();
    Camera *camera = Camera::instance();
    Window *window = Window::instance();

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

    // Allocators
    VmaAllocator vma_allocator;
    void init_allocator();
    void kill_allocator();
    VkResult alloc_buffer(size_t size, VkBufferUsageFlags usage,
                          VkMemoryPropertyFlags pref_flags,
                          VkMemoryPropertyFlags req_flags,
                          VmaAllocationCreateFlags vma_flags, GpuBuffer *buf);
    void free_buffer(GpuBuffer buf);
    StagingBuffer staging_buf;
    void alloc_staging_buf(size_t size, void *data);
    VertexBuffer vert_buf;
    void alloc_vert_buf(size_t size);
    Vec<GpuBuffer> ubos;
    void alloc_ubos(size_t size);
    void kill_ubos();
    void update_ubo(uint32_t frame_index);

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

    // Descriptors
    VkDescriptorPool desc_pool;
    void init_desc_pool();
    void kill_desc_pool();
    VkDescriptorSetLayout vk_desc_set_layout;
    VkDescriptorSet desc_sets[MAX_FRAME_COUNT];
    void init_desc_set_layout();
    void kill_desc_set_layout();

    // Pipeline
    VkPipeline vk_pipeline;
    VkPipelineLayout vk_layout;
    void init_pipeline();
    void kill_pipeline();
    VkShaderModule create_shader_module(const char *file_name);

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
    uint32_t allocate_commandbuffers(uint32_t command_pool_index,
                                     uint32_t buffer_count);
    void record_command_buffer(VkCommandBuffer cmd, uint32_t image_index);
    void record_and_submit_cpy(size_t size, size_t index_offset);

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
    static void
    populate_debug_create_info(VkDebugUtilsMessengerCreateInfoEXT *create_info);
    static VKAPI_ATTR VkBool32 VKAPI_CALL
    debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                   VkDebugUtilsMessageTypeFlagsEXT messageType,
                   const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                   void *pUserData);
    VkResult vkCreateDebugUtilsMessengerEXT(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
        const VkAllocationCallbacks *pAllocator,
        VkDebugUtilsMessengerEXT *pDebugMessenger);
    void
    vkDestroyDebugUtilsMessengerEXT(VkInstance instance,
                                    VkDebugUtilsMessengerEXT debugMessenger,
                                    const VkAllocationCallbacks *pAllocator);
#endif // V_LAYERS

}; // struct Engine

} // namespace Sol

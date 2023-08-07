#pragma once

#include <glm/glm.hpp>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>

#include "Array.hpp"
#include "Camera.hpp"
#include "Clock.hpp"
#include "Image.hpp"
#include "Vec.hpp"
#include "Window.hpp"

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include "vk_mem_alloc.h"

namespace Sol {

#define V_LAYERS true

#define MAX_FRAME_COUNT 2

namespace PFN {
void vkGetDescriptorSetLayoutBindingOffsetEXT(
    VkDevice device, VkDescriptorSetLayout layout, uint32_t binding, VkDeviceSize *pOffset);
void vkCmdBindDescriptorBuffersEXT(
    VkDevice device, VkCommandBuffer commandBuffer, uint32_t bufferCount,
    const VkDescriptorBufferBindingInfoEXT *pBindingInfos);
} // namespace PFN

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
    static void get_attribute_description(VkVertexInputAttributeDescription *desc) {
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

struct OldGpuBuffer {
    VkBuffer buf;
    VmaAllocation alloc;
    VmaAllocationInfo alloc_info;

    virtual ~OldGpuBuffer() {}
};

struct VertexBuffer : public OldGpuBuffer {
    ~VertexBuffer() override;

    uint32_t stride = sizeof(Vertex);
};
struct StagingBuffer : public OldGpuBuffer {
    ~StagingBuffer() override;
};
struct IndexBuffer : public OldGpuBuffer {
    ~IndexBuffer() override;
};

struct TexImage {
    Image image;
    VkImage vkimage;
    VkImageView vkview;
    VkSampler sampler;

    TexImage make(VkDevice device, VmaAllocator vma);
    void kill(VkDevice device, VmaAllocator vma);
};

struct Sync {
    Vec<VkFence> fences;
    Vec<VkSemaphore> semaphores;
    VkDevice device;

    VkFence *fence_alloc(uint32_t count, bool signalled);
    VkSemaphore *semaphore_alloc(uint32_t count, bool timeline, uint64_t initial_value);

    static const uint32_t DEFAULT_SEMAPHORES_SIZE = 16;
    static const uint32_t DEFAULT_FENCES_SIZE = 8;
    // -1 for default sizes
    static Sync get(VkDevice device, int semaphores_size, int fences_size);
    void kill();
};

struct Cmd {
    Vec<VkCommandBuffer> bufs;
    VkCommandPool pool;
    VkDevice device;

    VkCommandBuffer *buf_alloc(uint32_t count, bool primary);

    struct InheritanceInfo {};
    static VkResult begin(VkCommandBuffer cmd, bool one_time, InheritanceInfo *inheritance);
    static VkResult end(VkCommandBuffer cmd);

    struct SubmitInfo {
        struct SemaphoreInfo {
            VkSemaphore semaphore = VK_NULL_HANDLE;
            uint64_t value = 0;
            VkPipelineStageFlags2 stage_mask = 0x0;
            uint32_t device_index = 0;
        };
        struct CmdInfo {
            VkCommandBuffer cmd = VK_NULL_HANDLE;
            uint32_t device_mask = 0;
        };

        uint32_t wait_count = 0;
        uint32_t signal_count = 0;
        uint32_t cmd_count = 0;
        SemaphoreInfo *wait_infos = nullptr;
        SemaphoreInfo *signal_infos = nullptr;
        CmdInfo *cmd_infos = nullptr;
    };
    static VkResult submit(VkQueue queue, uint32_t count, SubmitInfo *args, VkFence fence);

    void reset_pool(bool free);

    struct CreateInfo {
        VkCommandPoolCreateFlags flags;
        uint32_t family_index;
    };
    static const uint32_t DEFAULT_BUFS_SIZE = 8;
    static Cmd get(VkDevice device, CreateInfo *args);
    void kill();
};

struct DescLayout {
    VkDescriptorSetLayout layout = VK_NULL_HANDLE;
    Array<size_t> binding_offsets;
    uint32_t binding_count = 0;

    static DescLayout
    get(VkDevice device, uint32_t binding_count, VkDescriptorSetLayoutBinding *bindings);
    void kill(VkDevice device);
};

struct GpuBuffer {
    struct CreateInfo {
        size_t size = 0;
        VkBufferUsageFlags usage = 0x0;
        VmaAllocationCreateFlags vma_flags = 0x0;
        VkMemoryPropertyFlags vma_preferred_flags = 0x0;
        VkMemoryPropertyFlags vma_required_flags = 0x0;
    };

    VmaAllocation alloc;
    VmaAllocationInfo info;
    VkBuffer buf;
    VmaAllocator vma;

    VkDeviceAddress get_address(VkDevice device);

    static GpuBuffer get(VmaAllocator vma, CreateInfo *args);
    void kill();
};

struct DescBuf {
    Vec<DescLayout> layouts;
    GpuBuffer buf;
    VkDevice device;

    static void bind(VkCommandBuffer cmd, uint32_t count, DescBuf *bufs, bool *resource_buf);

    static const size_t DEFAULT_SIZE = 1024;
    static DescBuf get(VkDevice device, VmaAllocator vma, size_t size);
    void kill();
};

// TODO:(Sol): Current task...
struct MonoPl {
    VkDevice device = VK_NULL_HANDLE;
    VkPipeline pl = VK_NULL_HANDLE;

    struct CreateInfo {
        struct ShaderInfo {
            uint32_t stage_count = 0;
            const char **shader_files = nullptr;
        };
        struct VertInputInfo {
            uint32_t bind_desc_count = 0;
            uint32_t attrib_desc_count = 0;
            VkVertexInputBindingDescription *bind_descs;
            VkVertexInputAttributeDescription *attrib_descs;
            uint32_t *bindings = nullptr;
            uint32_t *strides = nullptr;
            uint32_t *locations = nullptr;
            uint32_t *attrib_bindings = nullptr;
            VkFormat *formats = nullptr;
            uint32_t *offsets = nullptr;
        };
        struct AssemblyInfo {
            VkPrimitiveTopology topology;
            VkBool32 prim_restart;
        };
        struct RasterInfo {
            enum Bools { NONE = 0x0, DEPTH_CLAMP = 0x01, DISCARD = 0x02, DEPTH_BIAS = 0x04 };
            // TODO:(Sol): DepthBias support...
            VkPolygonMode polygon_mode;
            VkCullModeFlags cull_mode;
            VkFrontFace front_face;
            Bools bools;
        };
        struct MultiSampleInfo {
            enum Bools {
                SAMPLE_SHADING = 0x01,
                ALPHA_TO_COVERAGE = 0x02,
                ALPHA_TO_ONE = 0x04,
            };
            float min_sample_shading = 0.0f;
            Bools bools;
            VkSampleMask *sample_mask = nullptr;
        };
        struct DepthStencilInfo {
            enum Bools {
                DEPTH_TEST = 0x01,
                DEPTH_WRITE = 0x02,
                DEPTH_BOUNDS = 0x04,
                STENCIL_TEST = 0x08,
            };
            VkStencilOpState *front = nullptr;
            VkStencilOpState *back = nullptr;
            VkCompareOp compare_op;
            VkPipelineDepthStencilStateCreateFlags flags;
            float max_depth = 0;
            float min_depth = 0;
            Bools bools;
        };
        struct BlendInfo {
            struct AttachmentInfo {
                VkBool32 blend_enable = VK_FALSE;
                VkColorComponentFlags color_write_mask =
                    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                    VK_COLOR_COMPONENT_A_BIT;
            };
            VkBool32 logic_op_enable = VK_FALSE;
            VkLogicOp logic_op = VK_LOGIC_OP_CLEAR;
            uint32_t attachment_count = 0;
            AttachmentInfo *attachments = nullptr;
            VkPipelineColorBlendStateCreateFlags flags = 0x0;
        };
        struct DynInfo {
            uint32_t count = 0;
            VkDynamicState *states = nullptr;
        };

        ShaderInfo *shader_info;
        VertInputInfo *vert_input;
        AssemblyInfo *assembly_info;
        VkExtent2D *viewport_info;
        RasterInfo *raster_info;
        MultiSampleInfo *multisample_info;
        DepthStencilInfo *depth_stencil_info;
        BlendInfo *blend_info;
        DynInfo *dyn_info;
        VkRenderPass renderpass;
    };
    static MonoPl get(VkDevice device, CreateInfo *args);
    void kill();

  private:
    static void mono_pl_shader_modules(
        VkDevice device, uint32_t count, VkShaderModule *modules, size_t *code_sizes,
        const uint32_t **shader_code);
    static void mono_pl_shader_stages(
        VkDevice device, uint32_t stage_count, VkPipelineShaderStageCreateInfo *stage_infos,
        const char **shader_files);
    static VkPipelineVertexInputStateCreateInfo
    mono_pl_input_state(MonoPl::CreateInfo::VertInputInfo *info);
    static VkPipelineInputAssemblyStateCreateInfo
    mono_pl_assembly_state(MonoPl::CreateInfo::AssemblyInfo *info);
    static VkPipelineViewportStateCreateInfo
    mono_pl_viewport(VkExtent2D *extent, VkViewport *viewport, VkRect2D *scissor);
    static VkPipelineRasterizationStateCreateInfo
    mono_pl_rasterization_state(MonoPl::CreateInfo::RasterInfo *info);
    static VkPipelineMultisampleStateCreateInfo
    mono_pl_multisample_state(MonoPl::CreateInfo::MultiSampleInfo *info);
    static VkPipelineDepthStencilStateCreateInfo
    mono_pl_depth_stencil_state(MonoPl::CreateInfo::DepthStencilInfo *info);
    static VkPipelineColorBlendStateCreateInfo mono_pl_blend_state(
        MonoPl::CreateInfo::BlendInfo *info, VkPipelineColorBlendAttachmentState *attachments);
    static VkPipelineDynamicStateCreateInfo mono_pl_dyn_state(MonoPl::CreateInfo::DynInfo *info);
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
    Allocator *alloc_heap = &MemoryService::instance()->system_allocator;
    Allocator *alloc_scratch = &MemoryService::instance()->scratch_allocator;

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
    VkPhysicalDeviceDescriptorBufferPropertiesEXT desc_buf_props;
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
    VkResult alloc_buffer(
        size_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags pref_flags,
        VkMemoryPropertyFlags req_flags, VmaAllocationCreateFlags vma_flags, OldGpuBuffer *buf);
    void free_buffer(OldGpuBuffer buf);
    StagingBuffer staging_buf;
    void alloc_staging_buf(size_t size, void *data);
    VertexBuffer vert_buf;
    void alloc_vert_buf(size_t size);
    Vec<OldGpuBuffer> ubos;
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

    // OldDescriptors
    VkDescriptorPool desc_pool;
    void init_desc_pool();
    void kill_desc_pool();
    VkDescriptorSetLayout vk_desc_set_layout;
    VkDescriptorSet desc_sets[MAX_FRAME_COUNT];
    void init_desc_set_layout_old();
    void kill_desc_set_layout_old();

    // DescriptorBuffers
    DescBuf desc_buf;
    void init_bind_desc_bufs();
    void init_ubos();

    // NewSync
    Sync sync;
    void sync_init();
    void sync_kill();

    // NewCmd
    // TODO:(Sol): Init cmd
    Cmd command;
    void cmd_init();

    // NewPL
    void mono_pl_init();

    // INIT2
    void init2();
    void kill2();

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
    uint32_t allocate_commandbuffers(uint32_t command_pool_index, uint32_t buffer_count);
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

    // Textures
    // TODO:(Sol):

// Debug //////////////////////
#if V_LAYERS
    VkDebugUtilsMessengerEXT debug_messenger;
    void init_debug();
    void kill_debug();
    static void populate_debug_create_info(VkDebugUtilsMessengerCreateInfoEXT *create_info);
    static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData);
    VkResult vkCreateDebugUtilsMessengerEXT(
        VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
        const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger);
    void vkDestroyDebugUtilsMessengerEXT(
        VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks *pAllocator);
#endif // V_LAYERS

}; // struct Engine

} // namespace Sol

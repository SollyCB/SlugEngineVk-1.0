#include "FeaturesExtensions.hpp"
#include <vulkan/vulkan_core.h>

namespace Sol {
namespace InstanceExtensionsFeatures {

static const uint32_t layer_name_count = 1;
static const char *layer_names[] = {
    "VK_LAYER_KHRONOS_validation",
};

static const uint32_t ext_name_count = 2;
static const char *ext_names[] = {
    "VK_EXT_validation_features",
    "VK_EXT_debug_utils",
};

void get_layer_names(uint32_t *count, const char **names, bool validation)
{
    if (!names && !validation) {
        *count = layer_name_count - 1;
        return;
    }
    else if (!names) {
        *count = layer_name_count;
        return;
    }

    for (int i = 0; i < *count; ++i)
        names[i] = layer_names[i];
}
void get_ext_names(uint32_t *count, const char **names, bool validation)
{
    if (!names && !validation) {
        *count = ext_name_count - 2;
        return;
    }
    else if (!names) {
        *count = ext_name_count;
        return;
    }

    for (int i = 0; i < *count; ++i)
        names[i] = ext_names[i];
}
void features(uint32_t count, VkBaseOutStructure *features); 

} // namespace InstanceExtensionsFeatures

namespace DeviceExtensionsFeatures {

static const uint32_t name_count = 2;
/*
   CHECK THE COUNTS!!!!!!
*/
static const char *ext_names[] = {
    "VK_KHR_swapchain",
    "VK_EXT_descriptor_buffer",
};

void get_ext_names(uint32_t *count, const char **names)
{
    if (!names) {
        *count = name_count;
        return;
    }

    for (int i = 0; i < *count; ++i)
        names[i] = ext_names[i];
}

static const VkPhysicalDeviceFeatures vk1_features = {
    .samplerAnisotropy = VK_TRUE,
};
static const VkPhysicalDeviceVulkan12Features vk12_features = {
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
    .bufferDeviceAddress = VK_TRUE,
};
static const VkPhysicalDeviceVulkan13Features vk13_features = {
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
    .pNext = (void*)&vk12_features,
    .synchronization2 = VK_TRUE,
};
static VkPhysicalDeviceDescriptorBufferFeaturesEXT descriptor_buffer = {
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT,
    .pNext = (void*)&vk13_features,
    .descriptorBuffer = VK_TRUE,
    .descriptorBufferCaptureReplay = VK_TRUE,
    .descriptorBufferImageLayoutIgnored = VK_TRUE,
    .descriptorBufferPushDescriptors = VK_TRUE,
};

static VkPhysicalDeviceFeatures2 features_full = {
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
    .pNext = &descriptor_buffer,
    .features = vk1_features,
};

VkPhysicalDeviceFeatures2 get_features() {
    return features_full;
}

} // namespace DeviceExtensionsFeatures
} // namespace Sol

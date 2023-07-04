#include "FeaturesExtensions.hpp"

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
void features(uint32_t count, VkBaseOutStructure *features) {}

} // namespace InstanceExtensionsFeatures

namespace DeviceExtensionsFeatures {

static uint32_t name_count = 1;
static const char *ext_names[] = {
    "VK_KHR_swapchain",
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

} // namespace DeviceExtensionsFeatures
} // namespace Sol

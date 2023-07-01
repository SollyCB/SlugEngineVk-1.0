#pragma once

#include <vulkan/vulkan.hpp>

namespace Sol {

struct InstanceExtensionsFeatures {
    static void get_layer_names(uint32_t *count, const char **names,
                                bool validation) {
        uint32_t name_count = 1;
        const char *ext_names[] = {
            "VK_LAYER_KHRONOS_validation",
        };

        if (!names && !validation) {
            *count = name_count - 1;
            return;
        } else if (!names) {
            *count = name_count;
            return;
        }

        for (int i = 0; i < *count; ++i)
            names[i] = ext_names[i];
    }
    static void get_ext_names(uint32_t *count, const char **names,
                              bool validation) {
        uint32_t name_count = 2;
        const char *ext_names[] = {
            "VK_EXT_validation_features",
            "VK_EXT_debug_utils",
        };

        if (!names && !validation) {
            *count = name_count - 2;
            return;
        } else if (!names) {
            *count = name_count;
            return;
        }

        for (int i = 0; i < *count; ++i)
            names[i] = ext_names[i];
    }
    static void features(uint32_t count, VkBaseOutStructure *features) {}
};

struct DeviceExtensionsFeatures {
    static void get_ext_names(uint32_t *count, const char **names) {
        uint32_t name_count = 1;
        const char *ext_names[] = {
            "VK_KHR_swapchain",
        };

        if (!names) {
            *count = name_count;
            return;
        }

        for (int i = 0; i < *count; ++i)
            names[i] = ext_names[i];
    }
};


} // namespace Sol

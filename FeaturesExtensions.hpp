#pragma once

#include <vulkan/vulkan.hpp>

namespace Sol {

namespace InstanceExtensionsFeatures {
    void get_layer_names(uint32_t *count, const char **names,
                                bool validation); 
    void get_ext_names(uint32_t *count, const char **names,
                              bool validation); 
    void get_features(uint32_t count, VkBaseOutStructure *features);

} // namespace InstanceExtensionsFeatures

namespace DeviceExtensionsFeatures {

    void get_ext_names(uint32_t *count, const char **names); 
    VkPhysicalDeviceFeatures2 get_features();

} // namespace DeviceExtensionsFeatures 


} // namespace Sol

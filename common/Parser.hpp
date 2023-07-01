#pragma once

#include <cstddef>
#include <cstdint>
#include <vulkan/vulkan.hpp>

namespace Sol {

enum ShaderStageType {
    VERTEX,
    FRAGMENT,
    UNKNOWN,
};

struct Spirv {
    static VkShaderStageFlagBits parse(size_t code_size,
                                       const uint32_t *p_code);
};

} // namespace Sol

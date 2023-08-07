#include "Image.hpp"
#include "VulkanErrors.hpp"
#include "Assert.hpp"
#include "stb_image.h"

namespace Sol {

Image Image::load(const char* filename, int n_channels) {
    Image image;
    image.data = stbi_load(filename, &image.width, &image.height, &image.n_channels, n_channels);
    DEBUG_ASSERT(image.data, "Failed to load image");
    return image;
}
void Image::free() {
    stbi_image_free(data);
}

} // namespace Sol

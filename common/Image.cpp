#include "Image.hpp"
#include "VulkanErrors.hpp"
#include "stb_image.h"

namespace Sol {

Image Image::load(const char* filename, int n_channels) {
    Image image;
    image.mem = stbi_load(filename, &image.width, &image.height, &image.n_channels, n_channels);
    DEBUG_ABORT(image.data, "Failed to load image");
    return image;
}
void Image::free() {
    stbi_image_free(mem);
}

} // namespace Sol

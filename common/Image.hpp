#pragma once

namespace Sol {

struct Image {
    int width = 0;
    int height = 0;
    int n_channels = 0;
    unsigned char* data = nullptr;

    static Image load(const char* filename, int n_channels);
    void free();
};

} // namespace Sol

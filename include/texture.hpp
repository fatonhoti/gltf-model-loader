#pragma once

#include "glad.h"

struct Texture {

    struct TextureConfig {
        uint32_t target{GL_TEXTURE_2D};

        uint32_t min_filter{GL_NEAREST};
        uint32_t mag_filter{GL_NEAREST};
        uint32_t wrap_s{GL_CLAMP_TO_EDGE};
        uint32_t wrap_t{GL_CLAMP_TO_EDGE};

        uint32_t internalformat{0};
        uint32_t format{0};
        uint32_t type{0};
        uint32_t width{0}, height{0};
    };

    Texture() = default;
    ~Texture() { /*if (ID) glDeleteTextures(1, &ID);*/ }

    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ DON'T FORGET ABOUT THIS

    bool create_texture(const TextureConfig& conf, const unsigned char* data);

    uint32_t ID{0};
};
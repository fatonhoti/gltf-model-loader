#include "texture.hpp"

#include <cstdio>
#include <assert.h>

/*s
bool Texture::create_texture(const TextureConfig &conf, const unsigned char* data)
{
    if (ID != 0) {
        printf("This texture has already been loaded: ID=%u\n", ID);
        return false;
    }

    if (conf.target != GL_TEXTURE_2D) {
        printf("Error: Only target=GL_TEXTURE_2D supported.\n");
        return false;
    }
    
    if (conf.width == 0 || conf.height == 0) {
        printf("Error: You must specify valid dimensions for the texture.\n");
        return false;
    }

    int internalformat_ = 0;
    int format_ = 0;
    switch (conf.format) {
        case 8:
            switch (conf.internalformat) {
                case 1: internalformat_ = GL_RED; format_ = GL_R8; break;
                case 2: internalformat_ = GL_RG; format_ = GL_RG8; break;
                case 3: internalformat_ = GL_RGB; format_ = GL_RGB8; break;
                case 4: internalformat_ = GL_RGBA; format_ = GL_RGBA8; break;
                default: return false;
            }
            break;
        case 16:
            switch (conf.internalformat) {
                case 1: internalformat_ = GL_RED; format_ = GL_R16; break;
                case 2: internalformat_ = GL_RG; format_ = GL_RG16; break;
                case 3: internalformat_ = GL_RGB; format_ = GL_RGB16; break;
                case 4: internalformat_ = GL_RGBA; format_ = GL_RGBA16; break;
                default: return false;
            }
            break;
        case 32:
            switch (conf.internalformat) {
                case 1: internalformat_ = GL_RED; format_ = GL_R32F; break;
                case 2: internalformat_ = GL_RG; format_ = GL_RG32F; break;
                case 3: internalformat_ = GL_RGB; format_ = GL_RGB32F; break;
                case 4: internalformat_ = GL_RGBA; format_ = GL_RGBA32F; break;
                default: return false;
            }
            break;
        default:
            return false;
    }

    glGenTextures(1, &ID);
    glBindTexture(conf.target, ID);

    glTexParameteri(conf.target, GL_TEXTURE_WRAP_S, conf.wrap_s);
    glTexParameteri(conf.target, GL_TEXTURE_WRAP_T, conf.wrap_t);
    glTexParameteri(conf.target, GL_TEXTURE_MIN_FILTER, conf.min_filter);
    glTexParameteri(conf.target, GL_TEXTURE_MAG_FILTER, conf.mag_filter);

    glTexImage2D(conf.target, 0, internalformat_, conf.width, conf.height, GL_FALSE, format_, conf.type, data);
    glGenerateMipmap(conf.target);

    glBindTexture(conf.target, 0);

    return true;
}
*/

bool Texture::create_texture(const TextureConfig &conf, const unsigned char* data)
{
    if (ID != 0) {
        printf("This texture has already been loaded: ID=%u\n", ID);
        return false;
    }

    if (conf.target != GL_TEXTURE_2D) {
        printf("Error: Only target=GL_TEXTURE_2D supported.\n");
        return false;
    }
    
    if (conf.width == 0 || conf.height == 0) {
        printf("Error: You must specify valid dimensions for the texture.\n");
        return false;
    }

    GLenum internalformat = 0;
    GLenum format = 0;
    switch (conf.internalformat) {
        case 1:
            internalformat = GL_RED;
            format = GL_RED;
            break;
        case 2:
            internalformat = GL_RG;
            format = GL_RG;
            break;
        case 3:
            internalformat = GL_RGB;
            format = GL_RGB;
            break;
        case 4:
            internalformat = GL_RGBA;
            format = GL_RGBA;
            break;
        default:
            printf("Unsupported internal format: %d\n", conf.internalformat);
            return false;
    }

    // Handle different bit depths
    GLenum type = GL_UNSIGNED_BYTE; // default
    if (conf.type != 0) {
        type = conf.type;
    }

    glGenTextures(1, &ID);
    glBindTexture(conf.target, ID);

    glTexParameteri(conf.target, GL_TEXTURE_WRAP_S, conf.wrap_s);
    glTexParameteri(conf.target, GL_TEXTURE_WRAP_T, conf.wrap_t);
    glTexParameteri(conf.target, GL_TEXTURE_MIN_FILTER, conf.min_filter);
    glTexParameteri(conf.target, GL_TEXTURE_MAG_FILTER, conf.mag_filter);

    glTexImage2D(conf.target, 0, internalformat, conf.width, conf.height, 
                 0, format, type, data);
    
    if (conf.min_filter == GL_LINEAR_MIPMAP_LINEAR || 
        conf.min_filter == GL_LINEAR_MIPMAP_NEAREST ||
        conf.min_filter == GL_NEAREST_MIPMAP_LINEAR || 
        conf.min_filter == GL_NEAREST_MIPMAP_NEAREST) {
        glGenerateMipmap(conf.target);
    }

    glBindTexture(conf.target, 0);

    return true;
}

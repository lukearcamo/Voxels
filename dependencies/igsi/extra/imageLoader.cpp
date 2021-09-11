// Needs access to OpenGL definitions
// Replace this with whatever loader you are using
#include <glad/gl.h>

#include "imageLoader.h"

#include "../core/vec2.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_MAX_DIMENSIONS 4096
#include "stb_image.h"

namespace Igsi {
    GLenum defaultFormats[4] = { GL_RED, GL_RG, GL_RGB, GL_RGBA };
    vec2 loadImage(const char* imagePath, GLenum target, int forceNumChannels, GLenum internalFormat, GLenum format) {
        // stbi_set_flip_vertically_on_load(true);
        int w, h, nrChannels;
        void* data = stbi_load(imagePath, &w, &h, &nrChannels, forceNumChannels);
        
        if (forceNumChannels) nrChannels = forceNumChannels;
        if (format == GL_ZERO) format = defaultFormats[nrChannels - 1];
        if (internalFormat == GL_ZERO) internalFormat = format;

        glTexImage2D(target, 0, internalFormat, w, h, 0, format, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);

        return vec2(w, h);
    }
}
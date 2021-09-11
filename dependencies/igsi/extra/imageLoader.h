// Needs access to OpenGL definitions
// Replace this with whatever loader you are using
#include <glad/gl.h>

namespace Igsi {
    class vec2;

    // This will try and automatically guess which formats the texture should use,
    // but you still have the option to specify your own internalFormat and format values
    vec2 loadImage(const char* imagePath, GLenum target, int forceNumChannels=0, GLenum internalFormat=GL_ZERO, GLenum format=GL_ZERO);
}
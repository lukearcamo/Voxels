#ifndef IGSI_HELPERS_H
#define IGSI_HELPERS_H

// Needs access to OpenGL definitions
// Replace this with whatever loader you are using
#include <glad/gl.h>

#include <vector>
#include <string>

namespace Igsi {
    class vec2;
    class vec3;
    class vec4;
    class mat4;
    class Geometry;

    std::string readFile(std::string path);
    GLuint compileShader(GLenum shaderType, const char* source);
    GLuint createShaderProgram(std::string vsSource, std::string fsSource, std::string gsSource=std::string());

    GLuint createVBO(GLint location, Geometry* geometry, std::string key, GLsizei stride=0, GLsizeiptr offset=0, GLenum usage=GL_STATIC_DRAW);
    GLuint createVBO(GLint location, void* data, GLsizeiptr bytes, GLint itemSize, GLenum type, GLboolean normalized=GL_FALSE, GLsizei stride=0, GLsizeiptr offset=0, GLenum usage=GL_STATIC_DRAW);
    // GLuint createVBO(GLint location, std::vector<GLfloat> &buffer, GLint itemSize, GLsizei stride=0, int offset=0, GLenum usage=GL_STATIC_DRAW);

    GLuint createEBO(std::vector<GLuint> &buffer, GLenum usage=GL_STATIC_DRAW);
    GLuint createVAO();
    GLuint createTexture(GLenum target, unsigned int unit=0);
    GLuint createFBO(GLenum target);
    GLuint createRBO();

    void set1(int idx, float value);
    void set2(int idx, vec2 value);
    void set3(int idx, vec3 value);
    void set4(int idx, vec4 value);

    // extern GLint currentShaderProgram;
    // void useProgram(GLint shaderProgram);
    GLuint getCurrentShaderProgram();

    void setUniformInt(const GLchar* name, int value, GLuint program=0);
    void setUniform(const GLchar* name, float value, GLuint program=0);
    void setUniform(const GLchar* name, vec2 value, GLuint program=0);
    void setUniform(const GLchar* name, vec3 value, GLuint program=0);
    void setUniform(const GLchar* name, vec4 value, GLuint program=0);
    void setUniform(const GLchar* name, mat4 value, GLuint program=0, GLboolean transpose=GL_FALSE);

    // Must have a texture currently bound
    void setTexParams(GLenum target, GLenum minFilter=GL_LINEAR, GLenum magFilter=GL_LINEAR, GLenum wrapS=GL_CLAMP_TO_EDGE, GLenum wrapT=GL_CLAMP_TO_EDGE, GLenum wrapR=GL_ZERO);
}

#endif
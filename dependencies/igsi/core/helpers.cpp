// Needs access to OpenGL definitions
// Replace this with whatever loader you are using
#include <glad/gl.h>

#include "helpers.h"

#include "vec2.h"
#include "vec3.h"
#include "vec4.h"
#include "mat4.h"
#include "geometry.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

namespace Igsi {
    std::string readFile(std::string path) {
        std::ifstream file(path);
        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();
        return buffer.str();
    }
    GLuint compileShader(GLenum shaderType, const char* source) {
        GLuint shader = glCreateShader(shaderType);
        glShaderSource(shader, 1, &source, NULL);
        glCompileShader(shader);

        GLint compileStatus, logSize;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
        if (!compileStatus) {
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
            GLchar infoLog[logSize];
            glGetShaderInfoLog(shader, logSize, NULL, infoLog);
            if (shaderType == GL_VERTEX_SHADER) std::cerr << "Vertex";
            else if (shaderType == GL_FRAGMENT_SHADER) std::cerr << "Fragment";
            else if (shaderType == GL_GEOMETRY_SHADER) std::cerr << "Geometry";
            std::cerr << " shader compilation failed: " << infoLog << std::endl;
        }

        return shader;
    }
    GLuint createShaderProgram(std::string vsSource, std::string fsSource, std::string gsSource) {
        GLuint shaderProgram = glCreateProgram();

        GLuint vs = compileShader(GL_VERTEX_SHADER, vsSource.c_str());
        glAttachShader(shaderProgram, vs);

        GLuint fs = compileShader(GL_FRAGMENT_SHADER, fsSource.c_str());
        glAttachShader(shaderProgram, fs);

        GLuint gs = 0;
        if (!gsSource.empty()) {
            gs = compileShader(GL_GEOMETRY_SHADER, fsSource.c_str());
            glAttachShader(shaderProgram, gs);
        }

        glLinkProgram(shaderProgram);
        glDeleteShader(vs);
        glDeleteShader(fs);
        if (gs != 0) glDeleteShader(gs);

        GLint linkStatus, logSize;
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linkStatus);
        if (!linkStatus) {
            glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &logSize);
            GLchar infoLog[logSize];
            glGetProgramInfoLog(shaderProgram, logSize, NULL, infoLog);
            std::cerr << "Shader program linking failed: " << infoLog << std::endl;
        }

        glUseProgram(shaderProgram);
        return shaderProgram;
    }
    
    GLuint createVBO(GLint location, Geometry* geometry, std::string key, GLsizei stride, GLsizeiptr offset, GLenum usage) {
        std::vector<GLfloat> &buffer = geometry->getAttribute(key);
        GLuint VBO;
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(GLfloat), buffer.data(), usage);
            glVertexAttribPointer(location, geometry->itemSizes.at(key), GL_FLOAT, false, stride, (void*)offset); // Normalized irrelevant for floats
            glEnableVertexAttribArray(location);
        return VBO;
    }
    GLuint createVBO(GLint location, void* data, GLsizeiptr bytes, GLint itemSize, GLenum type, GLboolean normalized, GLsizei stride, GLsizeiptr offset, GLenum usage) {
        GLuint VBO;
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, bytes, data, usage);
            glVertexAttribPointer(location, itemSize, type, normalized, stride, (void*)offset);
            glEnableVertexAttribArray(location);
        return VBO;
    }
    // GLuint createVBO(GLint location, std::vector<GLfloat> buffer, GLint itemSize, GLenum type, GLsizei stride, int offset, GLenum usage) {
    //     GLuint VBO;
    //     glGenBuffers(1, &VBO);
    //     glBindBuffer(GL_ARRAY_BUFFER, VBO);
    //         glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(GLfloat), buffer.data(), usage);
    //         glVertexAttribPointer(location, itemSize, GL_FLOAT, GL_FALSE, stride, (void*)(offset * sizeof(GLfloat)));
    //         glEnableVertexAttribArray(location);
    //     return VBO;
    // }

    GLuint createEBO(std::vector<GLuint> &buffer, GLenum usage) {
        GLuint EBO;
        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, buffer.size() * sizeof(GLuint), buffer.data(), usage);
        return EBO;
    }
    GLuint createVAO() {
        GLuint VAO;
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
        return VAO;
    }
    GLuint createTexture(GLenum target, unsigned int unit) {
        GLuint tex;
        glGenTextures(1, &tex);
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(target, tex);
        return tex;
    }
    GLuint createFBO(GLenum target) {
        GLuint fbo;
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(target, fbo);
        return fbo;
    }
    GLuint createRBO() {
        GLuint rbo;
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        return rbo;
    }

    void set1(int idx, float value) {
        glBufferSubData(GL_ARRAY_BUFFER, idx * sizeof(float), sizeof(float), &value);  // (idx * itemsize + offset) * sizeof(float)
    }
    void set2(int idx, vec2 value) {
        float vec[2] = { value.x, value.y };
        glBufferSubData(GL_ARRAY_BUFFER, idx * 2 * sizeof(float), sizeof(float) * 2, vec);
    }
    void set3(int idx, vec3 value) {
        float vec[3] = { value.x, value.y, value.z };
        glBufferSubData(GL_ARRAY_BUFFER, idx * 3 * sizeof(float), sizeof(float) * 3, vec);
    }
    void set4(int idx, vec4 value) {
        float vec[4] = { value.x, value.y, value.z, value.w };
        glBufferSubData(GL_ARRAY_BUFFER, idx * 4 * sizeof(float), sizeof(float) * 4, vec);
    }

    // GLuint currentShaderProgram = 0;
    // void useProgram(GLint shaderProgram) {
    //     if (currentShaderProgram != shaderProgram) {
    //         glUseProgram(shaderProgram);
    //         currentShaderProgram = shaderProgram;
    //     }
    // }
    GLuint getCurrentShaderProgram() {
        GLint shaderProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &shaderProgram);
        return shaderProgram;
    }

    void setUniformInt(const GLchar* name, int value, GLuint program) {
        glUniform1i(glGetUniformLocation(program ? program : getCurrentShaderProgram(), name), value);
    }
    void setUniform(const GLchar* name, float value, GLuint program) {
        glUniform1f(glGetUniformLocation(program ? program : getCurrentShaderProgram(), name), value);
    }
    void setUniform(const GLchar* name, vec2 value, GLuint program) {
        glUniform2f(glGetUniformLocation(program ? program : getCurrentShaderProgram(), name), value.x, value.y);
    }
    void setUniform(const GLchar* name, vec3 value, GLuint program) {
        glUniform3f(glGetUniformLocation(program ? program : getCurrentShaderProgram(), name), value.x, value.y, value.z);
    }
    void setUniform(const GLchar* name, vec4 value, GLuint program) {
        glUniform4f(glGetUniformLocation(program ? program : getCurrentShaderProgram(), name), value.x, value.y, value.z, value.w);
    }
    void setUniform(const GLchar* name, mat4 value, GLuint program, GLboolean transpose) {
        glUniformMatrix4fv(glGetUniformLocation(program ? program : getCurrentShaderProgram(), name), 1, transpose, value.elements);
    }

    // Must have a texture currently bound
    void setTexParams(GLenum target, GLenum minFilter, GLenum magFilter, GLenum wrapS, GLenum wrapT, GLenum wrapR) {
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilter);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, magFilter);
        glTexParameteri(target, GL_TEXTURE_WRAP_S, wrapS);
        glTexParameteri(target, GL_TEXTURE_WRAP_T, wrapT);
        if (wrapR) glTexParameteri(target, GL_TEXTURE_WRAP_R, wrapR);
    }
}
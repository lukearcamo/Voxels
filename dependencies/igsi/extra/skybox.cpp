// Needs access to OpenGL definitions
// Replace this with whatever loader you are using
#include <glad/gl.h>

#include "skybox.h"

#include "../core/mat4.h"
#include "../core/helpers.h"
#include "../core/geometry.h"
#include "../core/transform.h"
#include "imageLoader.h"

#include <string>

namespace Igsi {
    const char* faceImgs[6] = { "px.png", "nx.png", "py.png", "ny.png", "pz.png", "nz.png" };
    
    Skybox::Skybox(unsigned int texUnit, std::string path) {
        cubemap = createTexture(GL_TEXTURE_CUBE_MAP, texUnit);
        setTexParams(GL_TEXTURE_CUBE_MAP, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

        for (int i = 0; i < 6; i++) {
            loadImage((path + faceImgs[i]).c_str(), GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 3);
        }

        glDepthFunc(GL_LEQUAL); // So skybox shader can put skybox always at the back of the scene

        shaderProgram = createShaderProgram(
            "#version 330 core\n"
            "layout (location = 0) in vec3 position;\n"
            "uniform mat4 projectionMatrix;\n"
            "uniform mat4 viewMatrix;\n"
            "out vec3 vPosition;\n"
            "void main() {\n"
            "   vPosition = position;\n"
            "   gl_Position = projectionMatrix * mat4(mat3(viewMatrix)) *  vec4(position, 1.0);\n"
            "   gl_Position.z = gl_Position.w;\n"
            "}",
            
            "#version 330 core\n"
            "in vec3 vPosition;\n"
            "uniform samplerCube map;\n"
            "out vec4 fragColor;\n"
            "void main() {\n"
            // "   fragColor = vec4(vPosition, 1.0);\n"
            "   fragColor = vec4(texture(map, vPosition).rgb, 1.0);\n"
            "}"
        );
        setUniformInt("map", texUnit);

        boxGeometry(&geometry);
        drawCount = geometry.getDrawCount();
        VAO = createVAO();
            createVBO(0, &geometry, "position");
            createEBO(geometry.indices);
    }
    void Skybox::draw(Transform* camera, mat4 projectionMatrix) {
        glCullFace(GL_FRONT);

        glUseProgram(shaderProgram);
        GLuint current = getCurrentShaderProgram();
        setUniform("projectionMatrix", projectionMatrix, current);
        setUniform("viewMatrix", camera->inverseWorldMatrix, current);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, drawCount, GL_UNSIGNED_INT, NULL);

        glCullFace(GL_BACK);
    }
}
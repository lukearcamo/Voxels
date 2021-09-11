#include "text.h"

// Needs access to OpenGL definitions
// Replace this with whatever loader you are using
#include <glad/gl.h>

#include "../core/vec2.h"
#include "../core/vec3.h"
#include "../core/helpers.h"
#include "../core/geometry.h"

#include <vector>
#include <string>

namespace Igsi {
    const float positionData[12] = { 0, 0, 0, -1, 1, 0, 0, -1, 1, -1, 1, 0 };
    const float uvData[12] = { 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0 }; // Flipped vertically to make right side up

    Text::Text(int maxNumChars, vec2 offset, vec2 scale, vec3 color, TextAlign textAlign) {
        glEnable(GL_BLEND);
        this->maxNumChars = maxNumChars;
        charAspectRatio = 1.0; // Defaults to square

        this->offset = offset;
        this->scale = scale;
        this->color = color;
        this->textAlign = textAlign;

        shaderProgram = createShaderProgram(
            "#version 330 core\n"
            "layout (location = 0) in vec2 position;\n"
            "layout (location = 1) in vec2 uv;\n"
            "out vec2 vUv;\n"
            "uniform vec2 scale;\n"
            "uniform vec2 offset;\n"
            "void main() {\n"
            "    vUv = uv;\n"
            "    gl_Position = vec4(position * scale + offset, 0.0, 1.0);\n"
            "}",
            
            "#version 330 core\n"
            "in vec2 vUv;\n"
            "uniform sampler2D map;\n"
            "uniform vec3 color;\n"
            "out vec4 fragColor;\n"
            "void main() {\n"
            "    float alpha = texture(map, vUv).r;\n"
            "    fragColor = vec4(color, alpha);\n"
            "}"
        );
        
        Geometry geometry;
        std::vector<float> &vertices = geometry.addAttribute("position", 2); // Note that positions are a vec2
        std::vector<float> &uvs = geometry.addAttribute("uv", 2);
        vertices.assign(maxNumChars * 12, 0);
        uvs.assign(maxNumChars * 12, 0);

        float shift = 0;
        if (textAlign == RIGHT) shift = -maxNumChars;
        else if (textAlign == CENTER) shift = 0.5 * -maxNumChars;

        for (int i = 0; i < maxNumChars; i++) {
            int idxOffset = i * 12;
            for (int j = 0; j < 6; j++) {
                int e = j * 2;
                int f = idxOffset + e;
                vertices.at(f) = (positionData[e] + i) + shift;
                vertices.at(f + 1) = positionData[e + 1];
            }
        }

        drawCount = geometry.getDrawCount();
        VAO = createVAO();
            createVBO(0, &geometry, "position");
            uvsVBO = createVBO(1, &geometry, "uv");
    }
    void Text::setFontTexture(unsigned int texUnit, vec2 fontTexDims) {
        charAspectRatio = (fontTexDims.x / 16.0) / (fontTexDims.y / 8.0);
        glUseProgram(shaderProgram);
        setUniformInt("map", texUnit);
    }
    void Text::updateText(std::string str) {
        if (str.size() < maxNumChars) {
            if (textAlign == LEFT) {
                // str.resize(maxNumChars, ' ');
                str = str + std::string(maxNumChars - str.size(), ' ');
            }
            else if (textAlign == RIGHT) {
                str = std::string(maxNumChars - str.size(), ' ') + str;
            }
            // If the text changes length a lot, especially between odd and even lengths,
            // this won't be exact center because this only pads text evenly, rather than translating geometry
            else if (textAlign == CENTER) {
                str = std::string((maxNumChars - str.size()) / 2, ' ') + str;
                str = str + std::string(maxNumChars - str.size(), ' ');
            }
        }

        glBindBuffer(GL_ARRAY_BUFFER, uvsVBO);
        float* bufferPtr = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        for (int i = 0; i < maxNumChars; i++) {
            int idxOffset = i * 12;
            char current = str.at(i);
            for (int j = 0; j < 6; j++) {
                int e = j * 2;
                int f = idxOffset + e;
                *(bufferPtr + f) = (uvData[e] + (current % 16)) / 16.0;
                f++; e++;
                *(bufferPtr + f) = (uvData[e] + (current / 16)) / 8.0;
            }
        }
        glUnmapBuffer(GL_ARRAY_BUFFER);
    }
    void Text::draw(float aspectRatio) {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);
        glUseProgram(shaderProgram);
        GLuint current = getCurrentShaderProgram();
        setUniform("color", color, current);
        setUniform("offset", offset, current);
        setUniform("scale", scale * vec2(charAspectRatio / aspectRatio, 1.0), current);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, drawCount);
    }
}
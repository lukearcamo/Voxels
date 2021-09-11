#ifndef IGSI_TEXT_H
#define IGSI_TEXT_H

#include "../core/vec2.h"
#include "../core/vec3.h"

#include <string>

namespace Igsi {
    class Text {
    public:
        unsigned int shaderProgram;
        
        float charAspectRatio;
        int maxNumChars;
        int drawCount;

        enum TextAlign { LEFT, RIGHT, CENTER };
        TextAlign textAlign;

        // In screen space
        vec2 offset;
        vec2 scale;
        vec3 color;

        unsigned int VAO;
        unsigned int uvsVBO;
        
        Text(int maxNumChars, vec2 offset=vec2(-1, 1), vec2 scale=vec2(0.2), vec3 color=vec3(1.0), TextAlign textAlign=LEFT);
        void setFontTexture(unsigned int texUnit, vec2 fontTexDims);
        void updateText(std::string str);
        void draw(float aspectRatio);
    };
}

#endif
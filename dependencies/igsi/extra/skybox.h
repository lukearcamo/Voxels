#ifndef IGSI_SKYBOX_H
#define IGSI_SKYBOX_H

#include "../core/geometry.h"

#include <string>

namespace Igsi {
    class mat4;
    class Transform;

    extern const char* faceImgs[6];

    class Skybox {
    private:
        unsigned int shaderProgram;
        Geometry geometry;
        int drawCount;
        unsigned int VAO;
    public:
        unsigned int cubemap; // Public in case you want to use this texture as an environment map too

        Skybox(unsigned int texUnit, std::string path); // path e.g. "./textures/sky/earth/"
        void draw(Transform* camera, mat4 projectionMatrix);
    };
}

#endif
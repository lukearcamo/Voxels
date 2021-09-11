#ifndef IGSI_TRANSFORM_H
#define IGSI_TRANSFORM_H

#include "vec3.h"
#include "mat4.h"

#include <vector>

namespace Igsi {
    class vec4;

    class Transform {
    public:
        mat4 matrix;
        mat4 worldMatrix;
        mat4 inverseWorldMatrix;

        vec3 position;
        vec3 rotation;
        const char* rotationOrder;
        vec3 scale;

        std::vector<Transform*> children;
        Transform* parent;
        bool dynamic;
        bool updateLocalMatrix;

        Transform();
        ~Transform();

        Transform& add(Transform* child);
        Transform& remove(Transform* child);
        void clear();

        void updateMatrices();
        void updateChildrenMatrices();

        vec4 localToWorld(vec4 a);
        vec4 worldToLocal(vec4 a);
        vec3 getWorldPosition();
        vec3 getWorldDirection();

        void setDefaultUniforms(Transform* camera, mat4 projectionMatrix);
    };
}

#endif
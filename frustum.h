#ifndef VOXELS_FRUSTUM_H
#define VOXELS_FRUSTUM_H

#include "dependencies/igsi/core/vec3.h"

namespace Voxels {
    struct Plane {
        Igsi::vec3 normal; // Assumes normal is normalized
        Igsi::vec3 origin; // Defaults to 0,0,0
    };

    class Frustum {
    public:
        Plane planes[6];
        float fovHalfVert;
        float aspect;

        Frustum(float fov, float aspect, float near, float far);
        void updateFOV(float fov);
        void updateAspect(float aspect);
        
        float planeToPointDistance(Plane plane, Igsi::vec3 p);
        bool intersectsPoint(Igsi::vec3 p); // p is relative to camera's local space
        bool intersectsSphere(Igsi::vec3 center, float radius); // center is relative to camera's local space
        // bool intersectsAABB(Igsi::vec3 min, Igsi::vec3 max);
    };
}

#endif
#include "frustum.h"

#include "dependencies/igsi/core/vec3.h"

#include <cmath>

using namespace Igsi;

namespace Voxels {
    Frustum::Frustum(float fov, float aspect, float near, float far) {
        // https://en.wikipedia.org/wiki/Field_of_view_in_video_games#Field_of_view_calculations
        
        // Why did I construct a frustum like this?
        // - More intuitive (for me at least) -- See the math rather than surgically extracting numbers from a projection matrix
        // - If I update one value like aspect, I don't need to update the whole frustum, only the planes that are affected by it

        // Near
        planes[0].normal = vec3(0.0, 0.0, -1.0);
        planes[0].origin.z = -near;

        // Far
        planes[1].normal = vec3(0.0, 0.0, 1.0);
        planes[1].origin.z = -far;

        this->fovHalfVert = fov / 2.0;
        float c = std::cos(fovHalfVert);
        float s = std::sin(fovHalfVert);
        
        planes[2].normal = vec3(0.0, -c, -s); // Top
        planes[3].normal = vec3(0.0, c, -s); // Bottom

        this->aspect = aspect;
        float fovHalfHoriz = std::atan(tan(fovHalfVert) * aspect);
        c = std::cos(fovHalfHoriz);
        s = std::sin(fovHalfHoriz);
        
        planes[4].normal = vec3(-c, 0.0, -s); // Left
        planes[5].normal = vec3(c, 0.0, -s); // Right
    }
    void Frustum::updateFOV(float fov) {
        this->fovHalfVert = fov / 2.0;
        float c = std::cos(fovHalfVert);
        float s = std::sin(fovHalfVert);
        
        planes[2].normal = vec3(0.0, -c, -s); // Top
        planes[3].normal = vec3(0.0, c, -s); // Bottom

        updateAspect(aspect);
    }
    void Frustum::updateAspect(float aspect) {
        this->aspect = aspect;
        float fovHalfHoriz = std::atan(tan(fovHalfVert) * aspect);
        float c = std::cos(fovHalfHoriz);
        float s = std::sin(fovHalfHoriz);
        
        planes[4].normal = vec3(-c, 0.0, -s); // Left
        planes[5].normal = vec3(c, 0.0, -s); // Right
    }
    
    float Frustum::planeToPointDistance(Plane plane, vec3 p) { return dot(p - plane.origin, plane.normal); } // Scalar projection
    bool Frustum::intersectsPoint(vec3 p) { // p is relative to camera's local space
        for (int i = 0; i < 6; i++) {
            if (planeToPointDistance(planes[i], p) < 0.0) return false;
        }
        return true;
    }
    bool Frustum::intersectsSphere(vec3 center, float radius) { // center is relative to camera's local space
        for (int i = 0; i < 6; i++) {
            // If the magnitude of the distance is further than the radius
            if (planeToPointDistance(planes[i], center) < -radius) return false;
        }
        return true;
    }
    // bool Frustum::intersectsAABB(vec3 min, vec3 max) {

    // }
}
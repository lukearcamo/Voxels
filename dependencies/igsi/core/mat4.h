#ifndef IGSI_MAT4_H
#define IGSI_MAT4_H

#include <ostream>

namespace Igsi {
    class vec3;
    class vec4;

    class mat4 {
    public:
        float elements[16] = {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        };

        // Note: this transposes the input because OpenGL uses column-major ordering
        mat4& set(float a0, float a1, float a2, float a3,
                  float a4, float a5, float a6, float a7,
                  float a8, float a9, float a10, float a11,
                  float a12, float a13, float a14, float a15);

        mat4& identity();
        mat4& transpose();

        mat4& setTranslation(float x, float y, float z);
        mat4& setTranslation(vec3 a);
        mat4& translation(float x, float y, float z);
        mat4& translation(vec3 a);
        
        mat4& rotationX(float radians);
        mat4& rotationY(float radians);
        mat4& rotationZ(float radians);

        mat4& setScale(float x, float y, float z);
        mat4& setScale(vec3 a);
        mat4& scale(float x, float y, float z);
        mat4& scale(vec3 a);

        mat4& lookAt(vec3 origin, vec3 target, vec3 up);

        mat4& rotationAxisAngle(vec3 axis, float angle);
        mat4& rotationEuler(vec3 radians, const char* order);

        mat4& perspective(float fov, float aspect, float near, float far); // fov is vertical fov in radians
        mat4& orthographic(float left, float right, float bottom, float top, float near, float far);
        
        mat4& invert();

        bool operator == (mat4 a);
        // bool operator != (mat4 a);

        mat4& operator = (mat4 a);

        mat4& operator += (float a);
        mat4& operator -= (float a);
        mat4& operator *= (float a);
        mat4& operator /= (float a);

        mat4& operator += (mat4 a);
        mat4& operator -= (mat4 a);
        mat4& operator &= (mat4 a);
        mat4& operator /= (mat4 a);

        mat4 operator + (float a);
        mat4 operator - (float a);
        mat4 operator * (float a);
        mat4 operator / (float a);

        mat4 operator + (mat4 a);
        mat4 operator - (mat4 a);
        mat4 operator & (mat4 a);
        mat4 operator / (mat4 a);

        vec4 operator * (vec4 a);
        mat4 operator * (mat4 a);
        mat4& operator *= (mat4 a);
    };

    std::ostream &operator << (std::ostream &os, const mat4 &a);
}

#endif
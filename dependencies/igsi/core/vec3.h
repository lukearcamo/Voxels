#ifndef IGSI_VEC3_H
#define IGSI_VEC3_H

#include <ostream>

namespace Igsi {
    class vec3 {
    public:
        float x, y, z;

        vec3(float a = 0.0f);
        vec3(float ax, float ay, float az);

        vec3& operator = (float a);
        vec3& operator = (vec3 a);

        vec3 operator + (float a);
        vec3 operator + (vec3 a);
        vec3& operator += (float a);
        vec3& operator += (vec3 a);

        vec3 operator - (float a);
        vec3 operator - (vec3 a);
        vec3& operator -= (float a);
        vec3& operator -= (vec3 a);

        vec3 operator * (float a);
        vec3 operator * (vec3 a);
        vec3& operator *= (float a);
        vec3& operator *= (vec3 a);

        vec3 operator / (float a);
        vec3 operator / (vec3 a);
        vec3& operator /= (float a);
        vec3& operator /= (vec3 a);

        bool operator == (vec3 a);
        bool operator != (vec3 a);

        vec3 operator - ();

        float &operator [] (int i);
    };
    
    std::ostream &operator << (std::ostream &os, const vec3 &a);
    
    float dot(vec3 a, vec3 b);
    vec3 floor(vec3 a);
    vec3 abs(vec3 a);

    float length(vec3 a);
    vec3 normalize(vec3 a);
    vec3 cross(vec3 a, vec3 b);
}

#endif
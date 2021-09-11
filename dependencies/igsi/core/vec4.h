#ifndef IGSI_VEC4_H
#define IGSI_VEC4_H

#include <ostream>

namespace Igsi {
    class vec4 {
    public:
        float x, y, z, w;

        vec4(float a = 0.0f);
        vec4(float ax, float ay, float az, float aw);

        vec4& operator = (float a);
        vec4& operator = (vec4 a);

        vec4 operator + (float a);
        vec4 operator + (vec4 a);
        vec4& operator += (float a);
        vec4& operator += (vec4 a);

        vec4 operator - (float a);
        vec4 operator - (vec4 a);
        vec4& operator -= (float a);
        vec4& operator -= (vec4 a);

        vec4 operator * (float a);
        vec4 operator * (vec4 a);
        vec4& operator *= (float a);
        vec4& operator *= (vec4 a);

        vec4 operator / (float a);
        vec4 operator / (vec4 a);
        vec4& operator /= (float a);
        vec4& operator /= (vec4 a);

        bool operator == (vec4 a);
        bool operator != (vec4 a);

        vec4 operator - ();

        float &operator [] (int i);
    };
    
    std::ostream &operator << (std::ostream &os, const vec4 &a);

    float dot(vec4 a, vec4 b);
    vec4 floor(vec4 a);
    vec4 abs(vec4 a);

    float length(vec4 a);
    vec4 normalize(vec4 a);
}

#endif
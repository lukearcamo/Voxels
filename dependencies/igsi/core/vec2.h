#ifndef IGSI_VEC2_H
#define IGSI_VEC2_H

#include <ostream>

namespace Igsi {
    class vec2 {
    public:
        float x, y;

        vec2(float a = 0.0f);
        vec2(float ax, float ay);

        vec2& operator = (float a);
        vec2& operator = (vec2 a);

        vec2 operator + (float a);
        vec2 operator + (vec2 a);
        vec2& operator += (float a);
        vec2& operator += (vec2 a);

        vec2 operator - (float a);
        vec2 operator - (vec2 a);
        vec2& operator -= (float a);
        vec2& operator -= (vec2 a);

        vec2 operator * (float a);
        vec2 operator * (vec2 a);
        vec2& operator *= (float a);
        vec2& operator *= (vec2 a);

        vec2 operator / (float a);
        vec2 operator / (vec2 a);
        vec2& operator /= (float a);
        vec2& operator /= (vec2 a);

        bool operator == (vec2 a);
        bool operator != (vec2 a);

        vec2 operator - ();

        float &operator [] (int i);
    };
    
    std::ostream &operator << (std::ostream &os, const vec2 &a);

    float dot(vec2 a, vec2 b);
    vec2 floor(vec2 a);
    vec2 abs(vec2 a);

    float length(vec2 a);
    vec2 normalize(vec2 a);
}

#endif
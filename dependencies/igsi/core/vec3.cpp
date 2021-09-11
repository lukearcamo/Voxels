#include "vec3.h"

#include <ostream>
#include <cmath>

namespace Igsi {
    vec3::vec3(float a) { x = a; y = a; z = a; }
    vec3::vec3(float ax, float ay, float az) { x = ax; y = ay; z = az; }

    vec3 & vec3::operator = (float a) { x = a; y = a; z = a; return *this; }
    vec3 & vec3::operator = (vec3 a) { x = a.x; y = a.y; z = a.z; return *this; }

    vec3 vec3::operator + (float a) { return vec3(x + a, y + a, z + a); }
    vec3 vec3::operator + (vec3 a) { return vec3(x + a.x, y + a.y, z + a.z); }
    vec3& vec3::operator += (float a) { x += a; y += a; z += a; return *this; }
    vec3& vec3::operator += (vec3 a) { x += a.x; y += a.y; z += a.z; return *this; }

    vec3 vec3::operator - (float a) { return vec3(x - a, y - a, z - a); }
    vec3 vec3::operator - (vec3 a) { return vec3(x - a.x, y - a.y, z - a.z); }
    vec3& vec3::operator -= (float a) { x -= a; y -= a; z -= a; return *this; }
    vec3& vec3::operator -= (vec3 a) { x -= a.x; y -= a.y; z -= a.z; return *this; }

    vec3 vec3::operator * (float a) { return vec3(x * a, y * a, z * a); }
    vec3 vec3::operator * (vec3 a) { return vec3(x * a.x, y * a.y, z * a.z); }
    vec3& vec3::operator *= (float a) { x *= a; y *= a; z *= a; return *this; }
    vec3& vec3::operator *= (vec3 a) { x *= a.x; y *= a.y; z *= a.z; return *this; }

    vec3 vec3::operator / (float a) { return vec3(x / a, y / a, z / a); }
    vec3 vec3::operator / (vec3 a) { return vec3(x / a.x, y / a.y, z / a.z); }
    vec3& vec3::operator /= (float a) { x /= a; y /= a; z /= a; return *this; }
    vec3& vec3::operator /= (vec3 a) { x /= a.x; y /= a.y; z /= a.z; return *this; }

    bool vec3::operator == (vec3 a) { return x == a.x && y == a.y && z == a.z; }
    bool vec3::operator != (vec3 a) { return x != a.x || y != a.y || z != a.z; }

    vec3 vec3::operator - () { return vec3(-x, -y, -z); }

    float& vec3::operator [] (int i) {
        if (i == 0) return x;
        else if (i == 1) return y;
        else return z;
    }
    
    std::ostream &operator << (std::ostream &os, const vec3 &a) {
        return os << a.x << ", " << a.y << ", " << a.z;
    }
    
    float dot(vec3 a, vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
    vec3 floor(vec3 a) { return vec3(std::floor(a.x), std::floor(a.y), std::floor(a.z)); }
    vec3 abs(vec3 a) { return vec3(std::fabs(a.x), std::fabs(a.y), std::fabs(a.z)); }
    float length(vec3 a) { return std::sqrt(dot(a, a)); }
    vec3 normalize(vec3 a) { // To prevent divide by zero, if len = 0 return vec of 0
        float len = length(a);
        return len != 0.0 ? a / len : a;
    }
    vec3 cross(vec3 a, vec3 b) {
        return vec3(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        );
    }
}
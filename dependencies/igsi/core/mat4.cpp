#include "mat4.h"

#include "vec3.h"
#include "vec4.h"

#include <ostream>
#include <cmath>

namespace Igsi {
    mat4& mat4::set(float a0, float a1, float a2, float a3,
                    float a4, float a5, float a6, float a7,
                    float a8, float a9, float a10, float a11,
                    float a12, float a13, float a14, float a15) {
        elements[0] = a0; elements[4] = a1; elements[8] = a2; elements[12] = a3;
        elements[1] = a4; elements[5] = a5; elements[9] = a6; elements[13] = a7;
        elements[2] = a8; elements[6] = a9; elements[10] = a10; elements[14] = a11;
        elements[3] = a12; elements[7] = a13; elements[11] = a14; elements[15] = a15;
        return *this;
    }
    mat4& mat4::identity() {
        return set(
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        );
    }
    mat4& mat4::transpose() {
        return set(
            elements[0], elements[1], elements[2], elements[3],
            elements[4], elements[5], elements[6], elements[7],
            elements[8], elements[9], elements[10], elements[11],
            elements[12], elements[13], elements[14], elements[15]
        );
    }

    mat4& mat4::setTranslation(float x, float y, float z) { elements[12] = x; elements[13] = y; elements[14] = z; return *this; }
    mat4& mat4::setTranslation(vec3 a) { return setTranslation(a.x, a.y, a.z); }
    mat4& mat4::translation(float x, float y, float z) { return identity().setTranslation(x, y, z); }
    mat4& mat4::translation(vec3 a) { return identity().setTranslation(a); }
    
    mat4& mat4::rotationX(float radians) {
        float c = std::cos(radians);
        float s = std::sin(radians);
        return set(
            1, 0, 0, 0,
            0, c,-s, 0,
            0, s, c, 0,
            0, 0, 0, 1
        );
    }
    mat4& mat4::rotationY(float radians) {
        float c = std::cos(radians);
        float s = std::sin(radians);
        return set(
            c, 0, s, 0,
            0, 1, 0, 0,
            -s, 0, c, 0,
            0, 0, 0, 1
        );
    }
    mat4& mat4::rotationZ(float radians) {
        float c = std::cos(radians);
        float s = std::sin(radians);
        return set(
            c, s, 0, 0,
            -s, c, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        );
    }

    mat4& mat4::setScale(float x, float y, float z) { elements[0] = x; elements[5] = y; elements[10] = z; return *this; }
    mat4& mat4::setScale(vec3 a) { elements[0] = a.x; elements[5] = a.y; elements[10] = a.z; return *this; }
    mat4& mat4::scale(float x, float y, float z) { return identity().setScale(x, y, z); }
    mat4& mat4::scale(vec3 a) { return identity().setScale(a); }

    mat4& mat4::lookAt(vec3 origin, vec3 target, vec3 up) {
        // vec3 ww = normalize(target - origin);
        vec3 ww = normalize(origin - target);
        vec3 uu = normalize(cross(ww, up));
        vec3 vv = normalize(cross(uu, ww));
        return set(
            uu.x, uu.y, uu.z, 0,
            vv.x, vv.y, vv.z, 0,
            ww.x, ww.y, ww.z, 0,
            0,    0,    0,    1
        );
    }

    // Adapted from https://en.wikipedia.org/wiki/Rotation_matrix#Rotation_matrix_from_axis_and_angle
    mat4& mat4::rotationAxisAngle(vec3 axis, float angle) {
        vec3 s = axis * std::sin(angle);
        float c = std::cos(angle);
        vec3 t = axis * (1.0 - c);
        return set(
            axis.x * t.x + c,   axis.y * t.x - s.z, axis.z * t.x + s.y, 0,
            axis.x * t.y + s.z, axis.y * t.y + c,   axis.z * t.y - s.x, 0,
            axis.x * t.z - s.y, axis.y * t.z + s.x, axis.z * t.z + c,   0,
            0, 0, 0, 1
        );
    }
    mat4& mat4::rotationEuler(vec3 radians, const char* order) {
        if (order == "XZY") return rotationX(radians.x) *= mat4().rotationZ(radians.z) *= mat4().rotationY(radians.y);
        if (order == "YXZ") return rotationY(radians.y) *= mat4().rotationX(radians.x) *= mat4().rotationZ(radians.z);
        if (order == "YZX") return rotationY(radians.y) *= mat4().rotationZ(radians.z) *= mat4().rotationX(radians.x);
        if (order == "ZXY") return rotationZ(radians.z) *= mat4().rotationX(radians.x) *= mat4().rotationY(radians.y);
        if (order == "ZYX") return rotationZ(radians.z) *= mat4().rotationY(radians.y) *= mat4().rotationX(radians.x);
        return rotationX(radians.x) *= mat4().rotationY(radians.y) *= mat4().rotationZ(radians.z); // Default
    }
    
    // Adapted from https://webglfundamentals.org/webgl/lessons/webgl-3d-perspective.html
    mat4& mat4::perspective(float fov, float aspect, float near, float far) { // fov is vertical fov in radians
        float d = 1.0 / std::tan(fov / 2.0);
        float r = 1.0 / (near - far);
        return set(
            d / aspect, 0, 0, 0,
            0, d, 0, 0,
            0, 0, (near + far) * r, 2 * near * far * r,
            0, 0, -1, 0
        );
    }
    mat4& mat4::orthographic(float left, float right, float bottom, float top, float near, float far) {
        float w = 1 / (right - left);
        float h = 1 / (top - bottom);
        float p = 1 / (far - near);
        return set(
            2 * w, 0, 0, (right + left) * -w,
            0, 2 * h, 0, (top + bottom) * -h,
            0, 0, -2 * p, (far + near) * -p,
            0, 0, 0, 1
        );
    }

    // Adapted from Matrix4 class in THREE.js library
    // https://github.com/mrdoob/three.js/blob/master/src/math/Matrix4.js
    mat4& mat4::invert() {
        float
        n11 = elements[0], n21 = elements[1], n31 = elements[2], n41 = elements[3],
        n12 = elements[4], n22 = elements[5], n32 = elements[6], n42 = elements[7],
        n13 = elements[8], n23 = elements[9], n33 = elements[10], n43 = elements[11],
        n14 = elements[12], n24 = elements[13], n34 = elements[14], n44 = elements[15],
        t11 = n23 * n34 * n42 - n24 * n33 * n42 + n24 * n32 * n43 - n22 * n34 * n43 - n23 * n32 * n44 + n22 * n33 * n44,
        t12 = n14 * n33 * n42 - n13 * n34 * n42 - n14 * n32 * n43 + n12 * n34 * n43 + n13 * n32 * n44 - n12 * n33 * n44,
        t13 = n13 * n24 * n42 - n14 * n23 * n42 + n14 * n22 * n43 - n12 * n24 * n43 - n13 * n22 * n44 + n12 * n23 * n44,
        t14 = n14 * n23 * n32 - n13 * n24 * n32 - n14 * n22 * n33 + n12 * n24 * n33 + n13 * n22 * n34 - n12 * n23 * n34;

        float det = n11 * t11 + n21 * t12 + n31 * t13 + n41 * t14;
        if (det == 0) return set(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        float detInv = 1 / det;

        elements[0] = t11 * detInv;
        elements[1] = (n24 * n33 * n41 - n23 * n34 * n41 - n24 * n31 * n43 + n21 * n34 * n43 + n23 * n31 * n44 - n21 * n33 * n44) * detInv;
        elements[2] = (n22 * n34 * n41 - n24 * n32 * n41 + n24 * n31 * n42 - n21 * n34 * n42 - n22 * n31 * n44 + n21 * n32 * n44) * detInv;
        elements[3] = (n23 * n32 * n41 - n22 * n33 * n41 - n23 * n31 * n42 + n21 * n33 * n42 + n22 * n31 * n43 - n21 * n32 * n43) * detInv;
        elements[4] = t12 * detInv;
        elements[5] = (n13 * n34 * n41 - n14 * n33 * n41 + n14 * n31 * n43 - n11 * n34 * n43 - n13 * n31 * n44 + n11 * n33 * n44) * detInv;
        elements[6] = (n14 * n32 * n41 - n12 * n34 * n41 - n14 * n31 * n42 + n11 * n34 * n42 + n12 * n31 * n44 - n11 * n32 * n44) * detInv;
        elements[7] = (n12 * n33 * n41 - n13 * n32 * n41 + n13 * n31 * n42 - n11 * n33 * n42 - n12 * n31 * n43 + n11 * n32 * n43) * detInv;
        elements[8] = t13 * detInv;
        elements[9] = (n14 * n23 * n41 - n13 * n24 * n41 - n14 * n21 * n43 + n11 * n24 * n43 + n13 * n21 * n44 - n11 * n23 * n44) * detInv;
        elements[10] = (n12 * n24 * n41 - n14 * n22 * n41 + n14 * n21 * n42 - n11 * n24 * n42 - n12 * n21 * n44 + n11 * n22 * n44) * detInv;
        elements[11] = (n13 * n22 * n41 - n12 * n23 * n41 - n13 * n21 * n42 + n11 * n23 * n42 + n12 * n21 * n43 - n11 * n22 * n43) * detInv;
        elements[12] = t14 * detInv;
        elements[13] = (n13 * n24 * n31 - n14 * n23 * n31 + n14 * n21 * n33 - n11 * n24 * n33 - n13 * n21 * n34 + n11 * n23 * n34) * detInv;
        elements[14] = (n14 * n22 * n31 - n12 * n24 * n31 - n14 * n21 * n32 + n11 * n24 * n32 + n12 * n21 * n34 - n11 * n22 * n34) * detInv;
        elements[15] = (n12 * n23 * n31 - n13 * n22 * n31 + n13 * n21 * n32 - n11 * n23 * n32 - n12 * n21 * n33 + n11 * n22 * n33) * detInv;

        return *this;
    }

    bool mat4::operator == (mat4 a) { for (int i = 0; i < 16; i++) { if (elements[i] != a.elements[i]) return false; } return true; }
    // bool operator != (mat4 a) { for (int i = 0; i < 16; i++) { if (elements[i] == a.elements[i]) return false; } return true; }

    mat4& mat4::operator = (mat4 a) { for (int i = 0; i < 16; i++) { elements[i] = a.elements[i]; } return *this; }

    mat4& mat4::operator += (float a) { for (int i = 0; i < 16; i++) { elements[i] += a; } return *this; }
    mat4& mat4::operator -= (float a) { for (int i = 0; i < 16; i++) { elements[i] -= a; } return *this; }
    mat4& mat4::operator *= (float a) { for (int i = 0; i < 16; i++) { elements[i] *= a; } return *this; }
    mat4& mat4::operator /= (float a) { for (int i = 0; i < 16; i++) { elements[i] /= a; } return *this; }

    mat4& mat4::operator += (mat4 a) { for (int i = 0; i < 16; i++) { elements[i] += a.elements[i]; } return *this; }
    mat4& mat4::operator -= (mat4 a) { for (int i = 0; i < 16; i++) { elements[i] -= a.elements[i]; } return *this; }
    mat4& mat4::operator &= (mat4 a) { for (int i = 0; i < 16; i++) { elements[i] *= a.elements[i]; } return *this; }
    mat4& mat4::operator /= (mat4 a) { for (int i = 0; i < 16; i++) { elements[i] /= a.elements[i]; } return *this; }

    mat4 mat4::operator + (float a) { mat4 c; for (int i = 0; i < 16; i++) { c.elements[i] = elements[i] + a; } return c; }
    mat4 mat4::operator - (float a) { mat4 c; for (int i = 0; i < 16; i++) { c.elements[i] = elements[i] - a; } return c; }
    mat4 mat4::operator * (float a) { mat4 c; for (int i = 0; i < 16; i++) { c.elements[i] = elements[i] * a; } return c; }
    mat4 mat4::operator / (float a) { mat4 c; for (int i = 0; i < 16; i++) { c.elements[i] = elements[i] / a; } return c; }

    mat4 mat4::operator + (mat4 a) { mat4 c; for (int i = 0; i < 16; i++) { c.elements[i] = elements[i] + a.elements[i]; } return c; }
    mat4 mat4::operator - (mat4 a) { mat4 c; for (int i = 0; i < 16; i++) { c.elements[i] = elements[i] - a.elements[i]; } return c; }
    mat4 mat4::operator & (mat4 a) { mat4 c; for (int i = 0; i < 16; i++) { c.elements[i] = elements[i] * a.elements[i]; } return c; }
    mat4 mat4::operator / (mat4 a) { mat4 c; for (int i = 0; i < 16; i++) { c.elements[i] = elements[i] / a.elements[i]; } return c; }

    vec4 mat4::operator * (vec4 a) {
        vec4 c;
        c.x = elements[0] * a.x + elements[4] * a.y + elements[8] * a.z + elements[12] * a.w;
        c.y = elements[1] * a.x + elements[5] * a.y + elements[9] * a.z + elements[13] * a.w;
        c.z = elements[2] * a.x + elements[6] * a.y + elements[10] * a.z + elements[14] * a.w;
        c.w = elements[3] * a.x + elements[7] * a.y + elements[11] * a.z + elements[15] * a.w;
        return c;
    }
    mat4 mat4::operator * (mat4 a) {
        mat4 c;
        int e = 0;
        float sum;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                sum = 0;
                for (int k = 0; k < 4; k++) {
                    sum += elements[i * 4 + k] * a.elements[k * 4 + j];
                }
                c.elements[e] = sum;
                e++;
            }
        }
        return c;
    }
    mat4& mat4::operator *= (mat4 a) {
        *this = *this * a;
        return *this;
    }

    std::ostream &operator << (std::ostream &os, const mat4 &a) {
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                os << a.elements[i * 4 + j] << ", ";
            }
            os << '\n';
        }
        return os;
    }
}
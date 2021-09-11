#include "transform.h"

#include "vec4.h"
#include "helpers.h"

#include <iostream>
#include <vector>
#include <algorithm>

namespace Igsi {
    Transform::Transform() {
        scale = vec3(1.0);
        rotationOrder = "XYZ";
        parent = nullptr;
        dynamic = true;
        updateLocalMatrix = true;
    }
    Transform::~Transform() {
        clear();
        if (parent) parent->remove(this);
    }

    Transform& Transform::add(Transform* child) {
        if (child == this) {
            std::cerr << "Transform::add: Transform cannot be child of itself" << std::endl;
        }
        else if (child->parent == this) {
            std::cerr << "Transform::add: Already child of Transform" << std::endl;
        }
        else {
            children.push_back(child);
            if (child->parent != nullptr) child->parent->remove(child);
            child->parent = this;
        }
        return *this;
    }
    Transform& Transform::remove(Transform* child) {
        std::vector<Transform*>::iterator idx = std::find(children.begin(), children.end(), child);
        if (idx != children.end()) {
            child->parent = nullptr;
            children.erase(idx);
        }
        return *this;
    }
    void Transform::clear() {
        std::vector<Transform*>::iterator ptr;
        for (ptr = children.begin(); ptr < children.end(); ptr++) {
            (*ptr)->parent = nullptr;
        }
        children.clear();
    }

    void Transform::updateMatrices() {
        if (updateLocalMatrix) {
            matrix.scale(scale);
            if (rotation != vec3(0.0)) matrix *= mat4().rotationEuler(rotation, rotationOrder);
            matrix.setTranslation(position);
        }

        if (parent) worldMatrix = matrix * parent->worldMatrix;
        else worldMatrix = matrix;

        inverseWorldMatrix = worldMatrix;
        inverseWorldMatrix.invert();
    }
    void Transform::updateChildrenMatrices() {
        if (dynamic) updateMatrices();
        std::vector<Transform*>::iterator ptr;
        for (ptr = children.begin(); ptr < children.end(); ptr++) {
            (*ptr)->updateChildrenMatrices();
        }
    }

    vec4 Transform::localToWorld(vec4 a) {
        if (dynamic) updateMatrices();
        return worldMatrix * a;
    }
    vec4 Transform::worldToLocal(vec4 a) {
        if (dynamic) updateMatrices();
        return inverseWorldMatrix * a;
    }
    vec3 Transform::getWorldPosition() {
        if (dynamic) updateMatrices();
        return vec3(worldMatrix.elements[12], worldMatrix.elements[13], worldMatrix.elements[14]);
    }
    vec3 Transform::getWorldDirection() { // https://community.khronos.org/t/get-direction-from-transformation-matrix-or-quat/65502/2
        if (dynamic) updateMatrices();
        return vec3(
            worldMatrix.elements[8],
            worldMatrix.elements[9],
            worldMatrix.elements[10]
        );
    }

    void Transform::setDefaultUniforms(Transform* camera, mat4 projectionMatrix) {
        unsigned int current = getCurrentShaderProgram();
        setUniform("projectionMatrix", projectionMatrix, current);
        setUniform("worldMatrix", worldMatrix, current);
        setUniform("normalMatrix", inverseWorldMatrix, current, true);
        setUniform("viewMatrix", camera->inverseWorldMatrix, current);
        setUniform("cameraPosition", camera->position, current);
    }
}
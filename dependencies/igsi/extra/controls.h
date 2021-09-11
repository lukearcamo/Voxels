#ifndef CONTROLS_H
#define CONTROLS_H

// #define GLFW_DLL
#define GLFW_INCLUDE_NONE // Just to be sure
#include <GLFW/glfw3.h>

#include <iostream>

namespace Igsi {
    class vec2;
    class Transform;

    namespace Controls {
        vec2 getMousePos(GLFWwindow* window);
        
        // Pointer lock stuff
        // Either set these directly as the GLFW callbacks or put them inside your own callbacks
        void mouseEvent(GLFWwindow* window, int button, int action, int mods);
        void keyEvent(GLFWwindow* window, int key, int scancode, int action, int mods);
        void focusEvent(GLFWwindow* window, int focused);

        void update(GLFWwindow* window, Transform* camera, float deltaTime, float movementSpeed=3.0, float sprintSpeed=6.0, float mouseSensitivity=0.001);
    }
}

#endif
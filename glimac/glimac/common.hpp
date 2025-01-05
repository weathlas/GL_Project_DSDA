#pragma once

#include <GLFW/glfw3.h>
#include "glm.hpp"

namespace glimac {

    struct ShapeVertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoords;
    };

    struct Transform {
        glm::vec3 m_Position;
        glm::vec3 m_Angles;
        glm::vec3 m_Scales;
        Transform() : m_Position(glm::vec3(0)), m_Angles(glm::vec3(0)), m_Scales(glm::vec3(1))  {}
        Transform(glm::vec3 pos) : m_Position(pos), m_Angles(glm::vec3(0)), m_Scales(glm::vec3(1))  {}
        Transform(glm::vec3 pos, glm::vec3 angle) : m_Position(pos), m_Angles(angle), m_Scales(glm::vec3(1))  {}
        Transform(glm::vec3 pos, glm::vec3 angle, glm::vec3 scale) : m_Position(pos), m_Angles(angle), m_Scales(scale)  {}
    };

    const float degToRad = glm::pi<float>()/180;

    const float kmToMs = 1/3.6;

    const glm::vec3 X_Vector = glm::vec3(1, 0, 0);
    const glm::vec3 Y_Vector = glm::vec3(0, 1, 0);
    const glm::vec3 Z_Vector = glm::vec3(0, 0, 1);

}

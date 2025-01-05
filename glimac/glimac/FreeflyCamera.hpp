#pragma once

#include <vector>

#include "common.hpp"
#include "glm.hpp"

#define MIN_ZOOM 1

using namespace glm;

namespace glimac {

    class FreeflyCamera {
        public:
            FreeflyCamera(){
                m_Position = vec3(.0f, .0f, .0f);
                m_fPhy     = glm::pi<float>();
                m_fTheta   = 0;
                computeDirectionVectors();
            };
            void moveLeft(float t) {
                m_Position += m_LeftVector * t;
            }
            void moveFront(float t) {
                m_Position += m_FrontVector * t;
            }
            void rotateLeft(float degrees) {
                m_fPhy += degrees*(glm::pi<float>()/180);
                computeDirectionVectors();
            }
            void rotateUp(float degrees) {
                m_fTheta += degrees*(glm::pi<float>()/180);
                computeDirectionVectors();
            }

            mat4 getViewMatrix() const {
                return glm::lookAt(m_Position, m_Position + m_FrontVector, m_UpVector);
            }

            void printPos() {
                std::cout << m_Position << std::endl;
            }
            void printAngles() {
                std::cout << m_fTheta << m_fPhy << std::endl;
            }

        private:
            vec3 m_Position;
            float m_fPhy;
            float m_fTheta;

            vec3 m_FrontVector;
            vec3 m_LeftVector;
            vec3 m_UpVector;

            void computeDirectionVectors() {
                m_FrontVector = vec3(cos(m_fTheta)*sin(m_fPhy), sin(m_fTheta), cos(m_fTheta)*cos(m_fPhy));
                m_LeftVector = vec3(sin(m_fPhy+ glm::pi<float>()/2), 0, cos(m_fPhy+ glm::pi<float>()/2));
                m_UpVector = glm::cross(m_FrontVector, m_LeftVector);
            }
    };

}
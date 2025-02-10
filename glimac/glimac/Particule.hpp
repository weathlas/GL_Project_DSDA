#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <vector>


#include "common.hpp"
#include "glm.hpp"

using namespace glm;

namespace glimac {

    enum ParticuleComputeType {
        fixed = 0u,
        euler = 1u,
        leapfrog = 2u
    };

    class Particule {

        public:

            float m_mass;
            vec3 m_pos;
            vec3 m_speed;
            // vec3 m_accel
            vec3 m_forces_acc;

            Particule(float mass, vec3 speed) {
                m_mass = mass;
                m_speed = speed;
                m_forces_acc = vec3(0);
                m_type = fixed;
            }

            void update(float h) {
                switch (m_type)
                {
                case ParticuleComputeType::euler:
                    update_euler(h);
                    break;
                case ParticuleComputeType::leapfrog:
                    update_leapfrog(h);
                    break;
                default:
                    update_fixed(h);
                    break;
                }
            }

            
        private:
            ParticuleComputeType m_type;

            void update_leapfrog(float h) {
                m_speed += h * m_forces_acc / m_mass;
                m_pos += h * m_speed;
                m_forces_acc = vec3(0);
            }
                
            void update_euler(float h) {
                m_pos += h * m_speed;
                m_speed += h * m_forces_acc / m_mass;
                m_forces_acc = vec3(0);
            }
                
            void update_fixed(float h) {
                m_forces_acc = vec3(0);
            }

    };

}
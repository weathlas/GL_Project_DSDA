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

            Particule(float mass, vec3 pos, ParticuleComputeType type) {
                m_mass = mass;
                m_pos = pos;
                m_speed = vec3(0);
                m_forces_acc = vec3(0);
                m_type = type;

                m_initial_pos = m_pos;
                m_initial_speed = m_speed;
            }

            Particule(float mass, vec3 pos) : Particule(mass, pos, fixed) {
            }

            ~Particule(){}

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

            void reset() {
                m_pos = m_initial_pos;
                m_speed = m_initial_speed;
            }

            
        private:
            ParticuleComputeType m_type;

            vec3 m_initial_pos;
            vec3 m_initial_speed;

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
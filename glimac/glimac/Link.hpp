#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <vector>

#include <glimac/Particule.hpp>

#include "common.hpp"
#include "glm.hpp"

using namespace glm;

namespace glimac {

    enum LinkType {
        hook_spring,
        damper,
        damped_hook,
        cond_damped_hook
    };

    class Link {

        public:
            float m_k, m_length, m_z, m_s;
            vec3 m_world_pos, m_world_direction;
            Particule *m_M1, *m_M2;

            Link(Particule *M1, Particule *M2) {
                connect(M1, M2);
            }
            
            Link(Particule *M1, Particule *M2, LinkType type, float length, float k, float z, float s) {
                connect(M1, M2);
                m_type = type;
                m_length = length;
                m_k = k;
                m_z = z;
                m_s = s;
            }

            ~Link(){}

            void connect(Particule *M1, Particule *M2) {
                m_M1 = M1;
                m_M2 = M2;
                m_length = length(M1->m_pos - M2->m_pos);
            }

            void make_hook_spring(float k) {
                m_type = LinkType::hook_spring;
                m_k = k;
            }

            void make_damper(float z) {
                m_type = LinkType::damper;
                m_z = z;
            }

            void make_damped_hook(float k, float z) {
                m_type = LinkType::damped_hook;
                m_k = k;
                m_z = z;
            }

            void make_cond_damped_hook(float k, float z, float s) {
                m_type = LinkType::cond_damped_hook;
                m_k = k;
                m_z = z;
                m_s = s;
            }

            void update() {
                switch (m_type)
                {
                case LinkType::hook_spring:
                    update_hook_spring();
                    break;
                case LinkType::damper:
                    update_damper();
                    break;
                case LinkType::damped_hook:
                    update_damped_hook();
                    break;
                case LinkType::cond_damped_hook:
                    update_cond_damped_hook();
                    break;
                default:
                    break;
                }
            }
            
        private:
            LinkType m_type;

            void update_hook_spring() {
                vec3 v_diff = m_M1->m_pos - m_M2->m_pos;
                float d = length(v_diff);
                if (d == 0)  {
                    return;
                }
                vec3 u = normalize(v_diff);
                vec3 F = - m_k * (d - m_length) * u;
                m_M1->m_forces_acc += F;
                m_M2->m_forces_acc -= F;
            }

            void update_damper() {
                vec3 F = + m_z * (m_M2->m_speed - m_M1->m_speed);
                m_M1->m_forces_acc += F;
                m_M2->m_forces_acc -= F;
            }

            void update_damped_hook() {
                vec3 v_diff = m_M1->m_pos - m_M2->m_pos;
                float d = length(v_diff);
                if (d == 0)  {
                    return;
                }
                vec3 u = normalize(v_diff);
                vec3 F = - m_k * (d - m_length) * u + m_z * (m_M2->m_speed - m_M1->m_speed);
                m_M1->m_forces_acc += F;
                m_M2->m_forces_acc -= F;
            }

            void update_cond_damped_hook() {
                vec3 v_diff = m_M1->m_pos - m_M2->m_pos;
                float d = length(v_diff);
                if (d == 0)  {
                    return;
                }
                if(d > m_s * m_length) {
                    return;
                }
                vec3 u = normalize(v_diff);
                vec3 F = - m_k * (d - m_length) * u + m_z * (m_M2->m_speed - m_M1->m_speed);
                m_M1->m_forces_acc += F;
                m_M2->m_forces_acc -= F;
            }
    };
}
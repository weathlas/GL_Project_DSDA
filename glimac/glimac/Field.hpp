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

    enum FieldType {
        field_directional,
        field_point,
        field_fluid,
        field_magnet // might be too complex
    };

    const float gravityConstant = 6.67f;

    class Field {

        public:
            float m_k, m_z, m_s;
            vec3 m_world_pos, m_world_direction;
            std::vector<Particule*> m_particules;

            Field() {
            }
            
            Field(FieldType type, float k, float z, float s) {
                m_type = type;
                m_k = k;
                m_z = z;
                m_s = s;
            }

            ~Field(){}

            void connect(Particule *M1) {
                m_particules.push_back(M1);
            }

            void make_directional(vec3 direction, float k) {
                m_type = FieldType::field_directional;
                m_world_direction = direction;
                m_k = k;
            }

            void make_point(vec3 position, float k) {
                m_type = FieldType::field_point;
                m_world_pos = position;
                m_k = k;
            }

            void make_fluid(float k) {
                m_type = FieldType::field_fluid;
                m_k = k;
            }

            void update() {
                switch (m_type)
                {

                case FieldType::field_directional:
                    update_field_directional();
                    break;
                case FieldType::field_point:
                    update_field_point();
                    break;
                case FieldType::field_fluid:
                    update_field_fluid();
                    break;
                default:
                    break;
                }
            }
            
        private:
            FieldType m_type;

            void update_field_directional() {
                for (uint i = 0; i < m_particules.size(); i++) {
                    m_particules.at(i)->m_forces_acc += m_k * m_world_direction;
                }
                
            }

            void update_field_point() {
                for (uint i = 0; i < m_particules.size(); i++) {
                    vec3 v_diff = m_particules.at(i)->m_pos - m_world_pos;
                    float d = length(v_diff);
                    if(d < 0.25f) {
                        d = 0.25f;
                        // continue;
                    }
                    // if(v_diff.x == 0.0f && v_diff.y == 0.0f && v_diff.z == 0.0f) {
                    //     continue;
                    // }
                    // float epsilon = 0.005;
                    // if (d < epsilon)  {
                    //     return;
                    // }
                    vec3 u = normalize(v_diff);

                    m_particules.at(i)->m_forces_acc -= gravityConstant * ((m_particules.at(i)->m_mass * m_k) / (d*d)) * u;

                    // m_particules.at(i)->m_forces_acc += m_k * u * (gravityConstant/d);
                    // m_particules.at(i)->m_forces_acc -= 0.0001f * vec3(m_k/v_diff.x, m_k/v_diff.y, m_k/v_diff.z);
                    // m_particules.at(i)->m_forces_acc += v_diff;
                    // m_particules.at(i)->m_forces_acc += vec3(0, -0.1, 0);
                }

            }

            void update_field_fluid() {
                for (uint i = 0; i < m_particules.size(); i++) {
                    float speed = length(m_particules.at(i)->m_speed);
                    if (speed == 0.0f) {
                        continue;
                    }
                    vec3 speed_unit_vector = m_particules.at(i)->m_speed / speed;
                    m_particules.at(i)->m_forces_acc -= (speed * speed * m_k) * speed_unit_vector;
                }
            }
    };
}
#pragma once

#include <vector>

#include "common.hpp"
#include "glm.hpp"

#define MIN_ZOOM 1

using namespace glm;

namespace glimac {

    enum LightType {
        pointLight = 0u,
        directionalLight = 1u
    };

    struct LightStruct {
        vec3 pos;
        vec3 color;
        vec3 caracteristics;

        LightStruct() : pos(vec3(0)), color(vec3(1)), caracteristics(vec3(0)) {}
        LightStruct(vec3 init_pos, vec3 init_color, vec3 init_dir) : pos(init_pos), color(init_color), caracteristics(init_dir) {}
    };

    class Light {
        public:

            Light() {

            };

            void add(LightStruct newLight) {
                m_lights.push_back(newLight);
                
                m_lightValues.push_back(newLight.pos.x);
                m_lightValues.push_back(newLight.pos.y);
                m_lightValues.push_back(newLight.pos.z);

                m_lightValues.push_back(newLight.color.x);
                m_lightValues.push_back(newLight.color.y);
                m_lightValues.push_back(newLight.color.z);

                m_lightValues.push_back(newLight.caracteristics.x);
                m_lightValues.push_back(newLight.caracteristics.y);
                m_lightValues.push_back(newLight.caracteristics.z);
            }

            void updateAt(unsigned int index, LightStruct light) {
                m_lights.at(index) = light;
                computeAt(index);
            }

            void updatePosition(unsigned int index, vec3 newPos) {
                m_lights.at(index).pos = newPos;
                computeAt(index);
            }
            void updateColor(unsigned int index, vec3 newColor) {
                m_lights.at(index).color = newColor;
                computeAt(index);
            }
            void updateDirection(unsigned int index, vec3 newDirection) {
                m_lights.at(index).caracteristics = newDirection;
                computeAt(index);
            }

            vec3 getPositionAt(unsigned int index) {
                return m_lights.at(index).pos;
            }
            
            vec3 getColorAt(unsigned int index) {
                return m_lights.at(index).color;
            }

            vec3 getDirectionAt(unsigned int index) {
                return m_lights.at(index).caracteristics;
            }

            size_t size() {
                return m_lights.size();
            }

            GLfloat* data() {
                return m_lightValues.data();
            }


        private:
            std::vector<LightStruct> m_lights;
            LightStruct m_directionalLight;
            std::vector<GLfloat> m_lightValues;

            void computeAt(unsigned int index) {
                m_lightValues.at(index*9)   = m_lights.at(index).pos.x;
                m_lightValues.at(index*9+1) = m_lights.at(index).pos.y;
                m_lightValues.at(index*9+2) = m_lights.at(index).pos.z;

                m_lightValues.at(index*9+3) = m_lights.at(index).color.x;
                m_lightValues.at(index*9+4) = m_lights.at(index).color.y;
                m_lightValues.at(index*9+5) = m_lights.at(index).color.z;

                m_lightValues.at(index*9+6) = m_lights.at(index).caracteristics.x;
                m_lightValues.at(index*9+7) = m_lights.at(index).caracteristics.y;
                m_lightValues.at(index*9+8) = m_lights.at(index).caracteristics.z;
            }
    };

}
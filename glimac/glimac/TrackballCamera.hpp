#pragma once

#include <vector>

#include "common.hpp"
#include "glm.hpp"

#define MIN_ZOOM 1

using namespace glm;

namespace glimac {

    class TrackballCamera {
        public:
            TrackballCamera(){
                m_fDistance = 5.0f;
                m_fAngleX   = 0.0f;
                m_fAngleY   = 0.0f;
            };
            void moveFront(float delta) {
                m_fDistance = m_fDistance - delta;
                if (m_fDistance < MIN_ZOOM) {
                    m_fDistance = MIN_ZOOM;
                }
            }
            void rotateLeft(float degrees) {
                m_fAngleY = m_fAngleY + degrees;
            }
            void rotateUp(float degrees) {
                m_fAngleX = m_fAngleX + degrees;
                if (m_fAngleX < -90.0) {
                    m_fAngleX = -90.0;
                }
                if (m_fAngleX > 90.0) {
                    m_fAngleX = 90.0;
                }
            }

            mat4 getViewMatrix() const {
                auto transform = translate(glm::mat4(vec4(1, 0, 0, 0), vec4(0, 1, 0, 0), vec4(0, 0, 1, 0), vec4(0, 0, 0, 1)), vec3(0.f, 0.f, -m_fDistance));
                transform = rotate(transform, m_fAngleX*3.1415926f/180.0f, vec3(1, 0, 0));
                transform = rotate(transform, m_fAngleY*3.1415926f/180.0f, vec3(0, 1, 0));
                return transform;
            }

        private:
            float m_fDistance;
            float m_fAngleX;
            float m_fAngleY;
    };

}
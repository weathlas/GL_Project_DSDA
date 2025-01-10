#pragma once

#include <vector>

#include <glimac/FPSCamera.hpp>
// #include <glimac/KeyCodes.hpp>

#include "common.hpp"
#include "glm.hpp"

using namespace glm;

namespace glimac {

    class Button {

        private:
            vec3 m_pos;
            float m_size;
            bool m_clicked;
            bool m_activable;

        public:

            Button(vec3 pos, float size) {
                m_pos = pos;
                m_size = size;
            }

            bool isClickable(FPSCamera &camera) {
                if(m_clicked) {
                    return false;
                }
                auto distance = m_pos - camera.getPos();
                if(length2(distance) > powf(camera.getReach(), 2.0f)) {
                    return false;
                }
                auto dotResult = dot(distance, camera.getFrontVector());
                auto pPoint = m_pos - dotResult * camera.getFrontVector();

                if(length2(pPoint - camera.getPos()) > powf(m_size, 2.0f)) {
                    return false;
                }

                return true;
            }

            bool click(FPSCamera &camera, bool clicked) {
                if(!clicked) {
                    m_clicked = false;
                    m_activable = true;
                    return m_clicked;
                }
                // if (m_clicked) {
                //     m_clicked = clicked;
                // }
                if(m_clicked) {
                    return false;
                }
                auto distance = m_pos - camera.getPos();
                if(length2(distance) > powf(camera.getReach(), 2.0f)) {
                    m_activable = false;
                    return false;
                }
                auto dotResult = dot(distance, camera.getFrontVector());
                auto pPoint = m_pos - dotResult * camera.getFrontVector();

                if(length2(pPoint - camera.getPos()) > powf(m_size, 2.0f)) {
                    m_activable = false;
                    return false;
                }

                if(m_activable) {
                    m_clicked = clicked;
                }
                return m_clicked;
            }

            const vec3 pos() {
                return m_pos;
            }

            float size() {
                return m_size;
            }

            // vec3(1, 0, 0)
            // vec3(0, 0, 0)
            // vec3(1, 0, 0)

            // bool isClickable(vec3 pos, vec3 front, float reach) {
            //     auto distance = m_pos -pos;
            //     if(length2(distance) > powf(reach, 2.0f)) {
            //         return false;
            //     }
            //     auto dotResult = dot(distance, front);
            //     auto pPoint = m_pos - dotResult * front;

            //     std::cout << m_pos << std::endl;
            //     std::cout << pos << std::endl;
            //     std::cout << front << std::endl;
            //     std::cout << distance << std::endl;

            //     std::cout << dotResult << std::endl;
            //     std::cout << pPoint << std::endl;

            //     return false;
            // }
    };

}
#pragma once

#include <vector>

#include <glimac/BasicProgram.hpp>
#include <glimac/VertexData.hpp>
#include <glimac/Light.hpp>
#include <glimac/Instance.hpp>
#include <glimac/FPSCamera.hpp>
// #include <glimac/ShadowMap.hpp>

#include "common.hpp"
#include "glm.hpp"

#define MIN_ZOOM 1

using namespace glm;

namespace glimac {

    class Scene {
        public:

            Scene() {
            };

            ~Scene() {
                for (auto instance : m_instances) {
                    instance.get()->~Instance();
                }
            };

            void addInstance(const std::shared_ptr<Instance> &instance) {
                m_instances.push_back(instance);
            }

            size_t count() {
                return m_instances.size();
            }

            void drawScene(WindowManager& window, BasicProgram &program, FPSCamera &camera, mat4 &shadowMatrix, Light &lights, GLuint shadowTex, vec3 &sunPos)  {
                program.activate(window, camera, shadowMatrix, lights, sunPos);

                for (size_t i = 0; i < m_instances.size(); i++) {
                    // std::cout << "Drawing " << i << std::endl;
                    m_instances.at(i)->drawAll(program, camera.getViewMatrix(), camera.getProjMatrix(), shadowTex);
                }
            }

            // void drawScene(WindowManager& window, BasicProgram &program, const mat4 &matMV, const mat4 &matProj, const mat4 &matNormal, vec3 camPos, const mat4 &shadowMatrix, Light &lights, GLuint shadowTex, vec3 &sunPos) {


            //     // check if have shadow map

            //     // activate shadow program

            //     // set all opengl shadow map variables

            //     // render to the shadow map

            //     // deactivate the shadow program and variables

            //     program.activate(window, camPos, matNormal, shadowMatrix, lights, sunPos);

            //     // program.m_Program.use();
            //     // glUniformMatrix4fv(program.uNormalMatrixLoc, 1, GL_FALSE, glm::value_ptr((matNormal)));
            //     // glUniform1i(program.uBaseTextureLoc, 0);
            //     // glUniform1i(program.uAlternateTextureLoc, 1);
            //     // glUniform3fv(program.uCameraPositionLoc, 1, glm::value_ptr(camPos));
            //     // glUniformMatrix3fv(program.ulightsArrayLoc, 1, GL_FALSE, lights.data());

            //     for (size_t i = 0; i < m_instances.size(); i++) {
            //         // std::cout << "Drawing " << i << std::endl;
            //         m_instances.at(i)->drawAll(program, matMV, matProj, shadowTex);
            //     }
            // }

        private:
            std::vector<std::shared_ptr<Instance>> m_instances;
    };

}
#pragma once

#include <vector>

#include <glimac/BasicProgram.hpp>
#include <glimac/VertexData.hpp>
#include <glimac/Light.hpp>
#include <glimac/Instance.hpp>

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

            void drawScene(BasicProgram &program, mat4 &matMV, mat4 &matProj, mat4 &matNormal, vec3 camPos, mat4 &shadowMatrix, Light &lights, GLuint shadowTex) {


                // check if have shadow map

                // activate shadow program

                // set all opengl shadow map variables

                // render to the shadow map

                // deactivate the shadow program and variables

                program.activate(camPos, matNormal, shadowMatrix, lights);

                // program.m_Program.use();
                // glUniformMatrix4fv(program.uNormalMatrixLoc, 1, GL_FALSE, glm::value_ptr((matNormal)));
                // glUniform1i(program.uBaseTextureLoc, 0);
                // glUniform1i(program.uAlternateTextureLoc, 1);
                // glUniform3fv(program.uCameraPositionLoc, 1, glm::value_ptr(camPos));
                // glUniformMatrix3fv(program.ulightsArrayLoc, 1, GL_FALSE, lights.data());

                for (size_t i = 0; i < m_instances.size(); i++) {
                    // std::cout << "Drawing " << i << std::endl;
                    m_instances.at(i)->drawAll(program, matMV, matProj, shadowTex);
                }
            }

        private:
            std::vector<std::shared_ptr<Instance>> m_instances;
    };

}
#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <vector>

#include <glimac/KeyCodes.hpp>
#include <glimac/Scene.hpp>
#include <glimac/BasicProgram.hpp>

#include "common.hpp"
#include "glm.hpp"

using namespace glm;

namespace glimac {

    class Texture {

        const unsigned int TEXTURE_VERTEX_COUNT = 6;
        const std::vector<float> TEXTURE_DATAPOINTER = {
            -1.0,
            -1.0,
            0.0,
            0.0,
            0.0,
            //
            1.0,
            -1.0,
            0.0,
            1.0,
            0.0,
            //
            -1.0,
            1.0,
            0.0,
            0.0,
            1.0,
            //
            -1.0,
            1.0,
            0.0,
            0.0,
            1.0,
            //
            1.0,
            -1.0,
            0.0,
            1.0,
            0.0,
            //
            1.0,
            1.0,
            0.0,
            1.0,
            1.0,
        };

        public:
            Texture(const FilePath &applicationPath) : m_program(applicationPath, "src/shaders/utils/screenQuad.vs.glsl", "src/shaders/utils/screenQuad.fs.glsl", ProgramType::TEXTURE) {
                glGenBuffers(1, &m_vbo);
                glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
                glBufferData(GL_ARRAY_BUFFER, TEXTURE_VERTEX_COUNT * 5 * sizeof(GL_FLOAT),
                TEXTURE_DATAPOINTER.data(), GL_STATIC_DRAW);
                glBindBuffer(GL_ARRAY_BUFFER, 0);

                glGenVertexArrays(1, &m_vao);
                glBindVertexArray(m_vao);

                glEnableVertexAttribArray(0);
                glEnableVertexAttribArray(2);

                glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), 0);
                glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT),
                    (const GLvoid *)(3 * sizeof(GL_FLOAT)));
                glBindBuffer(GL_ARRAY_BUFFER, 0);

                glBindVertexArray(0);
            }
            
            void render(GLuint texture) {
                m_program.activateBarebone();
                m_program.activateTextureUniforms();

                m_program.bindTextures(texture, 0, 0, 0);
                
                // glViewport(0, 0, m_nWindowWidth, m_nWindowHeight);
                
                glClearColor(.0f, .0f, .0f, 1.f);
                // glClear(GL_COLOR_BUFFER_BIT);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                
                glBindVertexArray(m_vao);
                glDrawArrays(GL_TRIANGLES, 0, GLsizei(6));
                m_program.unbindTextures();
            }

        private:
            GLuint m_vbo, m_vao;
            BasicProgram m_program;

    };

}
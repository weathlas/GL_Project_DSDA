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

    class ShadowMap {

        const float widthShadowMap = 4096, heightShadowMap = 4096;

        const GLchar *pathVSDepthName = "src/shaders/utils/shadow.vs.glsl";
        const GLchar *pathFSDepthName = "src/shaders/utils/shadow.fs.glsl";

        const float near_plane = 1.0f, far_plane = 100.0f;

        const float sizeSideShadowMap = 25.0f;

        public:
            ShadowMap(const FilePath &applicationPath, const glimac::FilePath &vsFile, const glimac::FilePath &fsFile)
                        : m_program(applicationPath, vsFile, fsFile, ProgramType::DEPTH_COMPUTE) {
                float bias = 0.0005;
                m_lightProjToTexture = translate(mat4(1), vec3(0.5, 0.5, 0.5 - bias)) * scale(mat4(1), vec3(0.5));
                m_lightProjection = glm::ortho(-sizeSideShadowMap, sizeSideShadowMap, -sizeSideShadowMap, sizeSideShadowMap, near_plane, far_plane);
            }
            
            ShadowMap(const FilePath &applicationPath) : ShadowMap(applicationPath, "src/shaders/utils/shadow.vs.glsl", "src/shaders/utils/shadow.fs.glsl") {}

            ~ShadowMap(){
                glDeleteTextures(1, &m_depthMap);
                glDeleteFramebuffers(1, &m_depthMapFBO);
            }

            bool init(unsigned int width, unsigned int height) {
                m_width = width;
                m_height = height;

                glGenTextures(1, &m_depthMap);
                glBindTexture(GL_TEXTURE_2D, m_depthMap);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

                // white outside of the shadow map (no shadows)
                float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
                glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

                // Framebuffer setup for shadow mapping
                glGenFramebuffers(1, &m_depthMapFBO);
                glBindFramebuffer(GL_FRAMEBUFFER, m_depthMapFBO);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthMap, 0);
                glDrawBuffer(GL_NONE);
                glReadBuffer(GL_NONE);

                if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                    std::cout << "Framebuffer is not complete!" << std::endl;
                }

                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glBindTexture(GL_TEXTURE_2D, 0);

                m_initialized = true;

                m_lights.add(LightStruct());

                return true;
            }

            bool init() {
                return init(widthShadowMap, heightShadowMap);
            }

            void computeTransforms(LightStruct light) {
                m_lights.updateAt(0, light);

                // set static for now for now
                // m_lights.updatePosition(0, vec3(40, 40, -40));

                // light.pos is considered to be the direction of the light
                // m_lightModelToView = glm::lookAt(m_lights.getPositionAt(0), vec3(0, 0, 0), vec3( 0.0f, 1.0f,  0.0f));
                m_lightModelToView = glm::lookAt(m_lights.getPositionAt(0), vec3(0, 0, 0), vec3( 0.0f, 1.0f,  0.0f));
                float bias = 0.005;
                m_lightProjToTexture = translate(mat4(1), vec3(0.5, 0.5, 0.5 - bias)) * scale(mat4(1), vec3(0.5));
                m_lightProjection = glm::ortho(-sizeSideShadowMap, sizeSideShadowMap, -sizeSideShadowMap, sizeSideShadowMap, near_plane, far_plane);

                m_lightSpaceMatrix = m_lightProjection * m_lightModelToView;
                m_normalMatrix = glm::transpose(glm::inverse(m_lightModelToView));
                m_shadowMatrix = m_lightProjToTexture * m_lightSpaceMatrix;
            }



            void renderTexture(Scene &scene) {
                if (!m_initialized) {
                    std::cout << "uninitialized shadow map" << std::endl;
                    return;
                }

                vec3 lightPos = m_lights.getPositionAt(0);

                GLint m_viewport[4];
                GLint origFB;

                // /// slow !!!!!!
                glGetIntegerv( GL_VIEWPORT, m_viewport);
                glGetIntegerv( GL_DRAW_FRAMEBUFFER_BINDING, &origFB);

                // 1. Render scene to depth map
                glViewport(0, 0, m_width, m_height);
                glBindFramebuffer(GL_FRAMEBUFFER, m_depthMapFBO);
                glClear(GL_DEPTH_BUFFER_BIT);

                // programm.activate(camPos, m_normalMatrix, m_shadowMatrix, m_lights);
                m_program.activate(lightPos, m_normalMatrix, m_shadowMatrix, m_lights);
                // Set uniforms and render scene from light's perspective

                scene.drawScene(m_program, m_lightModelToView, m_lightProjection, m_normalMatrix, lightPos, m_shadowMatrix, m_lights, 0);
                // scene.drawScene(programm, m_lightSpaceMatrix, m_lightProjection, m_normalMatrix, camPos, m_shadowMatrix, m_lights, m_depthMap);

                glReadBuffer(GL_NONE);

                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
                glBindFramebuffer(GL_READ_FRAMEBUFFER, m_depthMapFBO);

                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, m_depthMap);

                glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 0, 0, m_width, m_height, 0);

                glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_viewport[2] - m_viewport[0], m_viewport[3] - m_viewport[1], GL_DEPTH_BUFFER_BIT, GL_NEAREST);

                glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

                glBindFramebuffer(GL_FRAMEBUFFER, 0);

                // 2. Render scene with shadows
                glViewport(m_viewport[0], m_viewport[1], m_viewport[2], m_viewport[3]);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            }

            GLuint getDepthMap() {
                return m_depthMap;
            }

            mat4 getLightTransform() {
                return mat4(m_lightSpaceMatrix);
            }

            mat4 getShadowMatrix() {
                return mat4(m_shadowMatrix);
            }

            mat4 getModelToLight() {
                return mat4(m_lightModelToView);
            }

            mat4 getLightProj() {
                return mat4(m_lightProjection);
            }

            mat4 getLightNormal() {
                return mat4(m_normalMatrix);
            }

            
        private:
            GLuint m_width = 0;
            GLuint m_height = 0;
            GLuint m_depthMapFBO;
            GLuint m_depthMap;
            BasicProgram m_program;
            bool m_initialized;

            mat4 m_lightProjToTexture;
            mat4 m_shadowMatrix;

            mat4 m_lightSpaceMatrix;
            mat4 m_lightModelToView;
            mat4 m_lightProjection;
            mat4 m_normalMatrix;
            Light m_lights;
    };

}
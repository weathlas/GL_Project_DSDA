#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <vector>

#include <glimac/KeyCodes.hpp>
#include <glimac/Scene.hpp>
#include <glimac/BasicProgram.hpp>
#include <glimac/FPSCamera.hpp>

#include "common.hpp"
#include "glm.hpp"

using namespace glm;

namespace glimac {

    GLuint my_bind_texture(glimac::FilePath path) {
        std::unique_ptr<Image> image = loadImage(path);

        GLuint out;
        glGenTextures(1, &out);
        glBindTexture(GL_TEXTURE_2D, out);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.get()->getWidth(), image.get()->getHeight(), 0, GL_RGBA, GL_FLOAT, image.get()->getPixels());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        return out;
    }

    class Mirror {

        const float widthShadowMap = 4096, heightShadowMap = 4096;

        const GLchar *pathVSMirrorName = "src/shaders/utils/mirror_cutout.vs.glsl";
        const GLchar *pathFSMirrorName = "src/shaders/utils/mirror_cutout.fs.glsl";

        const float near_plane = 1.0f, far_plane = 100.0f;

        const float sizeSideShadowMap = 25.0f;

                    
        private:
            vec3 m_position;
            vec3 m_normal;

            vec3 m_reflectedPos;
            vec3 m_reflectedUp;
            vec3 m_reflectedForward;

            mat4 m_reflectedViewMatrix;
            mat4 m_reflectedNormalMatrix;
            mat4 m_reflectedProjMatrix;

            BasicProgram m_program;

            GLuint m_imageWhiteInt;

            std::shared_ptr<glimac::Instance> m_mirrorInstances;

            GLuint m_width = 0;
            GLuint m_height = 0;

            GLuint m_mirrorMapFBO;
            GLuint m_mirrorMap;

            // Light m_lights;
            bool m_initialized;

            GLint m_viewport[4];
            GLint m_origFB;

            mat4 m_mirrorTexjMatrix;

            mat4 m_mirrorProjToTexture;
            mat4 m_mirrorSpaceMatrix;

        public:
            Mirror(const vec3& position, const vec3& direction, const FilePath &applicationPath) : m_program(applicationPath, pathVSMirrorName, pathFSMirrorName, ProgramType::NONE){
                m_position = position;
                m_normal = direction;
                m_imageWhiteInt = my_bind_texture(applicationPath.dirPath() + "/assets/textures/white.png");
                m_mirrorInstances = std::make_shared<Instance>(applicationPath.dirPath(), "mirrorGeometry", m_imageWhiteInt, 0);
                m_mirrorInstances.get()->add(Transform(vec3(10.5, 1.5, 9), vec3(-degToRad*90, 0, 0), vec3(6)));
                m_mirrorInstances.get()->add(Transform(vec3(10.5, 1.5, 9), vec3(degToRad*90, 0, 0), vec3(6)));

                m_mirrorProjToTexture = translate(mat4(1), vec3(0.5, 0.5, 0.5)) * scale(mat4(1), vec3(0.5));
            }

            ~Mirror(){
                m_mirrorInstances.get()->~Instance();
                glDeleteTextures(1, &m_imageWhiteInt);
            }

            bool init(unsigned int width, unsigned int height) {
                m_width = width;
                m_height = height;

                glGenTextures(1, &m_mirrorMap);
                glBindTexture(GL_TEXTURE_2D, m_mirrorMap);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
                // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

                // // white outside of the shadow map (no shadows)
                // float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
                // glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

                // Framebuffer setup for shadow mapping
                glGenFramebuffers(1, &m_mirrorMapFBO);
                glBindFramebuffer(GL_FRAMEBUFFER, m_mirrorMapFBO);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_mirrorMap, 0);
                glDrawBuffer(GL_COLOR_ATTACHMENT0);
                glReadBuffer(GL_COLOR_ATTACHMENT0);

                if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                    std::cout << "Framebuffer is not complete!" << std::endl;
                }

                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glBindTexture(GL_TEXTURE_2D, 0);

                m_initialized = true;

                return true;
            }

            vec3 reflectVector(const vec3& v) {
                return v - 2.0f * dot(v, m_normal) * m_normal;
            }

            void computeCamera(FPSCamera& camera) {

                // Compute reflected position
                auto camPos = vec3(camera.getPos());
                float dist = dot(camPos - m_position, m_normal);
                m_reflectedPos = camPos - 2.0f * dist * m_normal;

                // Reflect Forward and Up vectors
                m_reflectedForward = reflectVector(-camera.getFrontVector());
                m_reflectedUp = reflectVector(camera.getUpVector());

                // m_reflectedViewMatrix = glm::lookAt(m_reflectedPos, m_reflectedForward, m_reflectedUp);
                m_reflectedViewMatrix = glm::lookAt(m_reflectedPos, m_reflectedPos-m_reflectedForward, m_reflectedUp);
                m_reflectedNormalMatrix = glm::transpose(glm::inverse(m_reflectedViewMatrix));
                m_reflectedProjMatrix = camera.getProjMatrix();


                m_mirrorSpaceMatrix = m_reflectedProjMatrix * m_reflectedViewMatrix;
                m_mirrorTexjMatrix = m_mirrorProjToTexture * m_mirrorSpaceMatrix;
            }

            mat4 getViewMatrix() {
                return glm::lookAt(m_reflectedPos, m_reflectedPos-m_reflectedForward, m_reflectedUp);
            }

            mat4 getNormalMatrix() {
                return glm::transpose(glm::inverse(m_reflectedViewMatrix));
            }

            mat4 getMirrorMatrix() {
                return mat4(m_mirrorTexjMatrix);
                // return mat4(m_mirrorSpaceMatrix);
            }

            vec3 getMirrorCamPos() {
                return m_reflectedPos;
            }

            mat4 getMirrorProj() {
                return m_reflectedProjMatrix;
            }

            GLuint getMirrorMap() {
                return m_mirrorMap;
            }

            GLuint getMirrorFBO() {
                return m_mirrorMapFBO;
            }

            void startRenderMirrorTexture() {
                if (!m_initialized) {
                    std::cout << "uninitialized shadow map" << std::endl;
                    return;
                }

                GLint m_viewport[4];
                GLint origFB;

                // /// slow !!!!!!
                glGetIntegerv( GL_VIEWPORT, m_viewport);
                glGetIntegerv( GL_DRAW_FRAMEBUFFER_BINDING, &origFB);

                // 1. Render scene to mirror map
                glViewport(0, 0, m_width, m_height);
                glBindFramebuffer(GL_FRAMEBUFFER, m_mirrorMapFBO);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            }

            void stopRenderMirrorTexture() {
                glReadBuffer(GL_NONE);

                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
                glBindFramebuffer(GL_READ_FRAMEBUFFER, m_mirrorMapFBO);

                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, m_mirrorMap);

                glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 0, 0, m_width, m_height, 0);

                glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_viewport[2] - m_viewport[0], m_viewport[3] - m_viewport[1], GL_COLOR_BUFFER_BIT, GL_NEAREST);

                glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

                glBindFramebuffer(GL_FRAMEBUFFER, 0);

                glViewport(m_viewport[0], m_viewport[1], m_viewport[2], m_viewport[3]);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            }

            // void renderTexture(Scene &scene) {
            //     if (!m_initialized) {
            //         std::cout << "uninitialized shadow map" << std::endl;
            //         return;
            //     }

            //     // /// slow !!!!!!
            //     glGetIntegerv( GL_VIEWPORT, m_viewport);
            //     glGetIntegerv( GL_DRAW_FRAMEBUFFER_BINDING, &m_origFB);

            //     m_mirrorMapFBO;
            //     m_mirrorMap;

            //     // 1. Render scene to mirror map
            //     glViewport(0, 0, m_width, m_height);
            //     glBindFramebuffer(GL_FRAMEBUFFER, m_mirrorMapFBO);
            //     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            //     // programm.activate(camPos, m_normalMatrix, m_shadowMatrix, m_lights);
            //     m_program.activate(lightPos, m_normalMatrix, m_shadowMatrix, m_lights);
            //     // Set uniforms and render scene from light's perspective

            //     scene.drawScene(m_program, m_lightModelToView, m_lightProjection, m_normalMatrix, lightPos, m_shadowMatrix, m_lights, 0);
            //     // scene.drawScene(programm, m_lightSpaceMatrix, m_lightProjection, m_normalMatrix, camPos, m_shadowMatrix, m_lights, m_mirrorMap);

            //     glReadBuffer(GL_NONE);

            //     glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            //     glBindFramebuffer(GL_READ_FRAMEBUFFER, m_mirrorMapFBO);

            //     glActiveTexture(GL_TEXTURE2);
            //     glBindTexture(GL_TEXTURE_2D, m_mirrorMap);

            //     glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 0, 0, m_width, m_height, 0);

            //     glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_viewport[2] - m_viewport[0], m_viewport[3] - m_viewport[1], GL_DEPTH_BUFFER_BIT, GL_NEAREST);

            //     glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

            //     glBindFramebuffer(GL_FRAMEBUFFER, 0);

            //     // 2. Render scene with shadows
            //     glViewport(m_viewport[0], m_viewport[1], m_viewport[2], m_viewport[3]);
            //     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            // }

            void drawMirrorGeometry(Instance &Instance) {

                // internal program as simple as possible
                // Instance.drawAll(programSky, ModelToViewVMatrix, projMatrix, 0);
            }

            void drawStencil(FPSCamera& camera) {
                // stencil buffer activation
                glEnable(GL_STENCIL_TEST);
                glClear(GL_STENCIL_BUFFER_BIT);
                glStencilFunc(GL_ALWAYS, 1, 0xFF);
                glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

                // glDisable(GL_CULL_FACE);


                auto ModelToViewVMatrix = camera.getViewMatrix();
                auto NormalMatrix = glm::transpose(glm::inverse(ModelToViewVMatrix));
                auto currentCamPos = camera.getPos();
                auto projMatrix = camera.getProjMatrix();

                // m_program.activateSimple(currentCamPos, NormalMatrix);
                // m_program.activateSimple(m_reflectedPos, m_reflectedNormalMatrix);

                // // Render the mirror surface into the stencil buffer
                // m_mirrorInstances.get()->drawAll(m_program, ModelToViewVMatrix, projMatrix, 0);
                // m_mirrorInstances.get()->drawAll(m_program, m_mirrorSpaceMatrix, m_reflectedProjMatrix, 0);

                // glEnable(GL_CULL_FACE);

                // // Step 2: Render the reflected scene
                glStencilFunc(GL_EQUAL, 1, 0xFF); // Render only where stencil is 1
                glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

                // // Reflect the camera
                // reflectCamera(mirrorPlane);

                // // Render the scene
                // drawScene();

                // // Step 3: Restore the original view
                // restoreCamera();
                // glDisable(GL_STENCIL_TEST);
            }

            void deactivate() {
                glDisable(GL_STENCIL_TEST);
            }



            void renderTexture(Scene &scene) {
                
            }

    };

}
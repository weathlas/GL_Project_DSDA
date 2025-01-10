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

            vec4 m_clipPlane;


            GLuint m_imageWhiteInt;

            std::shared_ptr<glimac::Instance> m_mirrorInstances;

            GLuint m_width = 0;
            GLuint m_height = 0;

            GLuint m_mirrorMapFBO;
            GLuint m_mirrorMap;
            GLuint m_renderBuffer;
            GLuint m_depthMap;

            bool m_initialized;

            GLint m_viewport[4];
            GLint m_origFB;

            mat4 m_mirrorTexjMatrix;

            mat4 m_mirrorProjToTexture;
            mat4 m_mirrorSpaceMatrix;

        public:
            Mirror(const vec3& position, const vec3& direction, const FilePath &applicationPath) : m_program(applicationPath, pathVSMirrorName, pathFSMirrorName, ProgramType::NONE), m_camera(1.0, 1.0){
                m_position = position;
                m_normal = direction;
                m_imageWhiteInt = my_bind_texture(applicationPath.dirPath() + "/assets/textures/white.png");
                m_mirrorInstances = std::make_shared<Instance>(applicationPath.dirPath(), "mirrorGeometry", m_imageWhiteInt, 0);
                m_mirrorInstances.get()->add(Transform(vec3(11, 1.51, 11.99), vec3(-degToRad*90, 0, 0), vec3(3.0f, 1.0f, 6.0f)));
                m_mirrorInstances.get()->add(Transform(vec3(11, 1.51, 11.99), vec3(+degToRad*90, 0, 0), vec3(3.0f, 1.0f, 6.0f)));

                m_mirrorProjToTexture = translate(mat4(1), vec3(0.5, 0.5, 0.5)) * scale(mat4(1), vec3(0.5));

                m_clipPlane = computePlaneEquation(m_normal, m_position);
            }

            ~Mirror(){
                m_mirrorInstances.get()->~Instance();
                glDeleteTextures(1, &m_imageWhiteInt);
                glDeleteTextures(1, &m_mirrorMap);
                glDeleteFramebuffers(1, &m_mirrorMapFBO);
                glDeleteRenderbuffers(1, &m_renderBuffer);
                glDeleteRenderbuffers(1, &m_depthMap);
            }

            bool init(unsigned int width, unsigned int height) {
                m_width = width;
                m_height = height;

                glEnable(GL_DEPTH_TEST);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                glGenTextures(1, &m_mirrorMap);
                glBindTexture(GL_TEXTURE_2D, m_mirrorMap);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
                // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

                // // white outside of the shadow map (no shadows)
                // float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };

                glGenTextures(1, &m_depthMap);
                glBindTexture(GL_TEXTURE_2D, m_depthMap);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                
                // glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

                // Framebuffer setup for shadow mapping
                glGenFramebuffers(1, &m_mirrorMapFBO);
                glBindFramebuffer(GL_FRAMEBUFFER, m_mirrorMapFBO);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_mirrorMap, 0);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthMap, 0);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BITS);
                // glDrawBuffer(GL_COLOR_ATTACHMENT0);
                // glReadBuffer(GL_COLOR_ATTACHMENT0);

                // glClear(GL_DEPTH_BUFFER_BIT);

                if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                    std::cout << "Framebuffer is not complete!" << std::endl;
                }

                // glGenFramebuffers(1, &m_renderBuffer);
                // glBindFramebuffer(GL_FRAMEBUFFER, m_renderBuffer);
                // glDrawBuffer(GL_NONE);
                // glReadBuffer(GL_NONE);

                // // Generate an id for the buffer object
                glGenRenderbuffers(1, &m_renderBuffer);
                // // Bind the object to the OpenGL context
                glBindRenderbuffer(GL_RENDERBUFFER, m_renderBuffer);
                // // Allocate storage
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL, GL_RENDERBUFFER, m_renderBuffer);



                // // glClear(GL_DEPTH_BUFFER_BIT);
                // // glBindRenderbuffer(GL_RENDERBUFFER, 0);
                // // Attach the depth buffer to the frame buffer
                // glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_renderBuffer);
                // // Enable OpenGL depth test
                // glEnable(GL_DEPTH_TEST);
                // // glClear(GL_DEPTH_BUFFER_BIT);

                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glBindTexture(GL_TEXTURE_2D, 0);

                m_initialized = true;

                return true;
            }

            void computeCamera(FPSCamera& camera) {
                camera.setMirrorCamera(&m_camera, m_position, m_normal);
                m_mirrorSpaceMatrix = m_camera.getProjMatrix() * m_camera.getViewMatrix();
                m_mirrorTexjMatrix = m_mirrorProjToTexture * m_mirrorSpaceMatrix;
            }

            const FPSCamera getCamera() {
                return m_camera;
            }

            const mat4 getViewMatrix() {
                return m_camera.getViewMatrix();
            }

            mat4 getNormalMatrix() {
                return m_camera.getNormalMatrix();
            }

            mat4 getMirrorMatrix() {
                return mat4(m_mirrorTexjMatrix);
            }

            const vec3 getMirrorCamPos() {
                return m_camera.getPos();
            }

            const vec4 getPlaneEquation() {
                return m_clipPlane;
            }

            mat4 getMirrorProj() {
                return m_camera.getProjMatrix();
            }

            GLuint getMirrorMap() {
                return m_mirrorMap;
            }

            GLuint getMirrorFBO() {
                return m_mirrorMapFBO;
            }

            GLuint getMirrorDepthFBO() {
                return m_renderBuffer;
            }

            void startRenderMirrorTexture() {
                if (!m_initialized) {
                    std::cout << "uninitialized shadow map" << std::endl;
                    return;
                }
                // /// slow !!!!!!
                glGetIntegerv( GL_VIEWPORT, m_viewport);
                glGetIntegerv( GL_DRAW_FRAMEBUFFER_BINDING, &m_origFB);

                // 1. Render scene to mirror map
                glViewport(0, 0, m_width, m_height);
                glBindFramebuffer(GL_FRAMEBUFFER, m_mirrorMapFBO);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            }

            void drawStencil(WindowManager& window, FPSCamera& camera) {
                // stencil buffer activation
                glClearColor(0, 0, 0, 1);
                glClearStencil(0);
                glStencilMask(0xFF);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

                glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
                glEnable(GL_STENCIL_TEST);
                // glDepthMask(GL_FALSE);
                // glClear(GL_STENCIL_BUFFER_BIT);
                // glStencilFunc(GL_ALWAYS, 1, 0xFF);
                glStencilFunc(GL_ALWAYS, 1, 0xFF);
                glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
                glStencilMask(0xFF);
                // glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);


                // glClear(GL_STENCIL_BITS | GL_DEPTH_BUFFER_BIT);

                // glDisable(GL_CULL_FACE);

                // auto ModelToViewVMatrix = camera.getViewMatrix();
                // auto NormalMatrix = glm::transpose(glm::inverse(ModelToViewVMatrix));
                // auto currentCamPos = camera.getPos();
                // auto projMatrix = camera.getProjMatrix();

                // vec4 values(0.0f, 0.0f, 0.0f, 0.0f);

                // glClipPlane(GL_CLIP_PLANE0, &values);

                // m_program.activateSimple(currentCamPos, NormalMatrix);
                m_program.activateSimple(window, m_camera);

                // // Render the mirror surface into the stencil buffer
                m_mirrorInstances.get()->drawAll(m_program, m_camera.getViewMatrix(), m_camera.getProjMatrix(), 0);
                // m_mirrorInstances.get()->drawAll(m_program, m_mirrorSpaceMatrix, m_reflectedProjMatrix, 0);

                // glEnable(GL_CULL_FACE);

                glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                glDepthMask(GL_TRUE); 

                // // Step 2: Render the reflected scene
                // glStencilFunc(GL_EQUAL, 1, 0xFF); // Render only where stencil is 1
                glStencilFunc(GL_EQUAL, 1, 0x01); // Render only where stencil is 1
                glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);


                glClear(GL_DEPTH_BUFFER_BIT);
                glEnable(GL_STENCIL_TEST);

                // // Reflect the camera
                // reflectCamera(mirrorPlane);

                // // Render the scene
                // drawScene();

                // // Step 3: Restore the original view
                // restoreCamera();
                // glDisable(GL_STENCIL_TEST);
            }

            void stopRenderMirrorTexture() {
                glReadBuffer(GL_NONE);

                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
                glBindFramebuffer(GL_READ_FRAMEBUFFER, m_mirrorMapFBO);

                // glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, m_mirrorMap);

                glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, m_width, m_height, 0);
                // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_mirrorMap, 0);

                // glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_viewport[2] - m_viewport[0], m_viewport[3] - m_viewport[1], GL_COLOR_BUFFER_BIT, GL_NEAREST);

                glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

                glBindFramebuffer(GL_FRAMEBUFFER, 0);

                glViewport(m_viewport[0], m_viewport[1], m_viewport[2], m_viewport[3]);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            }

            
            void deactivate() {
                glDisable(GL_STENCIL_TEST);
            }

        private:
            FPSCamera m_camera;
            BasicProgram m_program;

            vec4 computePlaneEquation(const vec3& normal, const vec3& point) {
                float A = normal.x;
                float B = normal.y;
                float C = normal.z;
                float D = -(A * point.x + B * point.y + C * point.z);
                return vec4(A, B, C, D);
            }

    };

}
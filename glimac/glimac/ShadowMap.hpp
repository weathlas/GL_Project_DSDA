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
                        : m_program(applicationPath, vsFile, fsFile, (ProgramType::DEPTH_COMPUTE)) {}
            
            ShadowMap(const FilePath &applicationPath) : ShadowMap(applicationPath, "src/shaders/utils/shadow.vs.glsl", "src/shaders/utils/shadow.fs.glsl") {}

            ~ShadowMap(){}

            // bool init(unsigned int width, unsigned int height) {
            //     m_width = width;
            //     m_height = height;

            //     glGenTextures(1, &m_depthMap);
            //     glBindTexture(GL_TEXTURE_2D, m_depthMap);

            //     // glActiveTexture(GL_TEXTURE2);

            //     // Allocate memory to the GPU for our depth map
            //     glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
            //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                
            //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE); 
            //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
            //     // float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
            //     // glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

            //     // glActiveTexture(GL_TEXTURE2);
            //     // glBindTexture(GL_TEXTURE_2D, 0);

            //     // not talked about


            //     // depth output
            //     glGenRenderbuffers(1, &m_depthBuffer);
            //     glBindRenderbuffer( GL_RENDERBUFFER, m_depthBuffer);
            //     glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 4096, 4096);


            //     glGenFramebuffers(1, &m_depthMapFBO);


            //     glBindFramebuffer(GL_FRAMEBUFFER, m_depthMapFBO);
            //     glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthBuffer);

            //     // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthMap, 0);
            //     // glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depthMap, 0);
            //     glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR, m_depthMap, 0);

            //     GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0};
            //     glDrawBuffers(1, drawBuffers);

            //     // glDrawBuffer(GL_NONE);
            //     // glReadBuffer(GL_NONE);
                
            //     if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            //     {
            //         std::cout << "Could not validate framebuffer" << std::endl;
            //     }
            //     else {
            //         std::cout << "framebuffer is valid" << std::endl;
            //     }


            //     glBindFramebuffer(GL_FRAMEBUFFER, 0);

            //     m_program.linkDepthMap(m_depthMapFBO, m_depthMap);

            //     m_initialized = true;

            //     m_lights.add(LightStruct());

            //     return true;
            // }

            bool init(unsigned int width, unsigned int height) {
                m_width = width;
                m_height = height;

                glGenFramebuffers(1, &m_depthMapFBO);

                GLuint m_depthMap;
                glGenTextures(1, &m_depthMap);
                glBindTexture(GL_TEXTURE_2D, m_depthMap);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 4096, 4096, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
                float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
                glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

                glBindFramebuffer(GL_FRAMEBUFFER, m_depthMapFBO);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthMap, 0);
                glDrawBuffer(GL_NONE);
                glReadBuffer(GL_NONE);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);







                // glGenTextures(1, &m_depthMap);
                // glBindTexture(GL_TEXTURE_2D, m_depthMap);

                // // glActiveTexture(GL_TEXTURE2);

                // // Allocate memory to the GPU for our depth map
                // glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
                // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                
                // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE); 
                // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
                // // float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
                // // glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

                // // glActiveTexture(GL_TEXTURE2);
                // // glBindTexture(GL_TEXTURE_2D, 0);

                // // not talked about


                // // depth output
                // glGenRenderbuffers(1, &m_depthBuffer);
                // glBindRenderbuffer( GL_RENDERBUFFER, m_depthBuffer);
                // glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 4096, 4096);



                // glBindFramebuffer(GL_FRAMEBUFFER, m_depthMapFBO);
                // glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthBuffer);

                // // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthMap, 0);
                // // glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depthMap, 0);
                // glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR, m_depthMap, 0);

                // GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0};
                // glDrawBuffers(1, drawBuffers);

                // // glDrawBuffer(GL_NONE);
                // // glReadBuffer(GL_NONE);
                
                // if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                // {
                //     std::cout << "Could not validate framebuffer" << std::endl;
                // }
                // else {
                //     std::cout << "framebuffer is valid" << std::endl;
                // }


                // glBindFramebuffer(GL_FRAMEBUFFER, 0);

                m_program.linkDepthMap(m_depthMapFBO, m_depthMap);

                m_initialized = true;

                m_lights.add(LightStruct());

                return true;
            }

            bool init() {
                return init(widthShadowMap, heightShadowMap);
            }

            void computeTransforms(LightStruct light, mat4 camProjTemp, mat4 camViewTemp, vec3 camPosTemp) {
                m_lights.updateAt(0, light);

                // m_lights.updatePosition(0, camPosTemp);
                m_lights.updatePosition(0, vec3(0, 1.78, 0));

                // m_lightModelToView = glm::lookAt(m_lights.getPositionAt(0), vec3(0.0f), vec3( 0.0f, 1.0f,  0.0f)); // light.pos is considered the direction of the light
                m_lightProjection = glm::ortho(-sizeSideShadowMap, sizeSideShadowMap, -sizeSideShadowMap, sizeSideShadowMap, near_plane, far_plane);
                
                // m_lightProjection = perspective(glm::radians(90.0f), 1.0f, 0.01f, 1000.f);
                m_lightModelToView = glm::lookAt(vec3(0, 1.78, 0), vec3(1, 1.78, -1), vec3( 0.0f, 1.0f,  0.0f)); // light.pos is considered the direction of the light
                m_lightModelToView = glm::lookAt(vec3(40, 40, -40), vec3(0, 0, 0), vec3( 0.0f, 1.0f,  0.0f)); // light.pos is considered the direction of the light
                

                // m_lightProjection = camProjTemp;
                // m_lightModelToView = camViewTemp; // light.pos is considered the direction of the light
                float bias = 0.005;
                m_lightProjToTexture = translate(mat4(1), vec3(0.5, 0.5, 0.5 - bias)) * scale(mat4(1), vec3(0.5));

                // m_lightProjToTexture = scale(translate(mat4(1), vec3(0.5)), vec3(0.5));
                // m_lightProjToTexture = translate(scale(mat4(1), vec3(0.5)), vec3(0.5)); // could be static

                m_lightSpaceMatrix = m_lightProjection * m_lightModelToView;
                m_normalMatrix = glm::transpose(glm::inverse(m_lightModelToView));

                m_shadowMatrix = m_lightProjToTexture * m_lightSpaceMatrix;
            }

            void renderTexture(Scene &scene) {
                if (!m_initialized) {
                    return;
                }

                // glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_depthMapFBO);
                // glClear(GL_DEPTH_BUFFER_BIT);
                // std::cout << "Binding draw buffer to " << m_depthMapFBO << std::endl;

                // float near_plane = 1.0f, far_plane = 75.0f;

                //  // ModelToViewVMatrix
                // // glm::mat4 lightModelToView = glm::lookAt(pos+light.pos, light.pos, vec3( 0.0f, 1.0f,  0.0f)); // light.pos is considered the direction of the light
                // glm::mat4 lightModelToView = glm::lookAt(light.pos, vec3(0), vec3( 0.0f, 1.0f,  0.0f)); // light.pos is considered the direction of the light
                // glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
                // m_lightSpaceMatrix = lightProjection * lightModelToView;
                // glm::mat4 NormalMatrix = glm::transpose(glm::inverse(lightModelToView));

                // m_program.activate(pos, NormalMatrix, );

                // save the viewport dimension
                GLint m_viewport[4];
                GLint origFB;
                if (true) {
                    glGetIntegerv( GL_VIEWPORT, m_viewport);
                    glGetIntegerv( GL_DRAW_FRAMEBUFFER_BINDING, &origFB);

                    // set to the texture dimensions
                    glViewport(0, 0, m_width, m_height);

                    glClear(GL_DEPTH_BUFFER_BIT);
                    glDepthFunc(GL_LESS); // default mode
                }

                glBindFramebuffer(GL_FRAMEBUFFER, m_depthMapFBO);
                glClear(GL_DEPTH_BUFFER_BIT);

                m_program.activate(m_lights.getPositionAt(0), m_normalMatrix, m_shadowMatrix, m_lights);
                
                // draw into the shadow map hopefully
                // scene.drawScene(m_program, m_lightModelToView, m_lightProjection, m_normalMatrix, m_lights.getPositionAt(0), m_shadowMatrix, m_lights);

                // glClearColor(0.1,0.1,0.1,1);

                if (true && false) {

                    // tutorial
                    glGenerateMipmap(m_depthMap);

                    // after drawing shadow
                    // glBindFramebuffer(GL_DRAW_FRAMEBUFFER, origFB); // origFB is probably 0 so it should be ok
                    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // origFB is probably 0 so it should be ok

                    // restore the viewport
                    glViewport(m_viewport[0], m_viewport[1], m_viewport[2], m_viewport[3]);

                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                    // activate the program which use the texture

                    // send the MVP matrix of the program camera

                    // send the light shadow matrix of the program camera which is different from the MLP matrix
                    // its a model to depthMap texture coordinate

                    // draw the scene

                    // glClearColor(0.1f, 0.0f, 0.0f, 0.0f);

                    // glBindFramebuffer(GL_READ_FRAMEBUFFER, m_depthMapFBO);
                    // glBindTexture(GL_TEXTURE_2D, m_depthMap);

                    // copy the texture ???
                    // glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
                    // glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_depthMapFBO);
                    // glBlitFramebuffer(0, 0, 4096, 4096, 0, 0, 1600, 900, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
                    // glBindFramebuffer(GL_FRAMEBUFFER, 0);

                    // glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, m_width, m_height);
                    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                    // glPopAttrib();
                    // ConfigureShaderAndMatrices();

                    // ATTENTION
                    // glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
                    // glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
                    // glBindFramebuffer(GL_FRAMEBUFFER, 0);

                    // glDrawBuffer(GL_BACK);
                    // glReadBuffer(GL_BACK);

                }

            }

            void use() {
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, m_depthMap);
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
            GLuint m_depthBuffer;
            BasicProgram m_program;
            FilePath m_filepath;
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
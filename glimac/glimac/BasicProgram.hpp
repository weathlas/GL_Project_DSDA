#pragma once

#include <vector>

#include <glimac/Program.hpp>
#include <glimac/Light.hpp>
#include <glimac/WindowManager.hpp>
#include <glimac/FPSCamera.hpp>

#include "common.hpp"
#include "glm.hpp"

#define MIN_ZOOM 1

using namespace glm;

namespace glimac
{

    enum Location {
        enumDiffuseTextureLoc = 0u,
        enumRoughnessTextureLoc = 1u,
        enumShadowTextureLoc = 2u
    };

    const GLchar *uMVPMatrixName = "uMVPMatrix";
    const GLchar *uMVMatrixName = "uMVMatrix";
    const GLchar *uModelMatrixName = "uModelMatrix";
    const GLchar *uNormalMatrixName = "uNormalMatrix";
    const GLchar *uCameraPositionName = "uCameraPosition";
    const GLchar *uCameraFrontName = "uCameraFront";
    const GLchar *uBaseTextureName = "uBaseTexture";
    const GLchar *uAlternateTextureName = "uAlternateTexture";
    const GLchar *ulightsArrayName = "uLights";
    const GLchar *ulightsCountName = "uLightsCount";
    const GLchar *uLightDepthMapName = "uLightDepthMap";
    const GLchar *uShadowMatrixName = "uShadowMatrix";
    const GLchar *uSunLightPosName = "uSunLightPos";
    const GLchar *uWindowDimensionsName = "uWindowDimensions";
    const GLchar *uClippingPlaneName = "uClippingPlane";
    const GLchar *uClippingPlaneActiveName = "uClippingPlaneActive";

    enum ProgramType {
        NONE              = 0u,
        LIGHTS            = (1u << 0),
        SHADOWS           = (1u << 1),
        TEXTURE           = (1u << 2),
        ALTERNATE_TEXTURE = (1u << 2),
        DEPTH_COMPUTE     = (1u << 3),
        // CLIPPING_PLANE    = (1u << 4),
        DEFAULT           = (LIGHTS | SHADOWS | TEXTURE | ALTERNATE_TEXTURE)
    };

    struct BasicProgram
    {
        Program m_Program;

        ProgramType m_programType;

        GLint uMVPMatrixLoc = 0;
        GLint uMVMatrixLoc = 0;
        GLint uModelMatrixLoc = 0;
        GLint uNormalMatrixLoc = 0;
        GLint uCameraPositionLoc = 0;
        GLint uCameraFrontLoc = 0;
        GLint uBaseTextureLoc = 0;
        GLint uAlternateTextureLoc = 0;
        GLint uLightsArrayLoc = 0;
        GLint uLightsCountLoc = 0;
        GLint uLightDepthMapLoc = 0;

        GLint uShadowMatrixLoc = 0;
        GLint uSunLightPosLoc = 0;

        GLint uWindowDimensionsLoc = 0;

        GLint uClippingPlaneLoc = 0;
        GLint uClippingPlaneActiveLoc = 0;
        // GLint baseTextureid;
        // GLint alternateTextureid;

        GLuint m_depthFBO;
        GLuint m_depthMapInt;

        BasicProgram(const FilePath &applicationPath, const glimac::FilePath &vsFile, const glimac::FilePath &fsFile, ProgramType programType) //, GLuint baseTex, GLuint alternateTex
                    : m_Program(loadProgram(applicationPath.dirPath() + vsFile, applicationPath.dirPath() + fsFile))
        {
            uMVPMatrixLoc           = glGetUniformLocation(m_Program.getGLId(), uMVPMatrixName);
            uMVMatrixLoc            = glGetUniformLocation(m_Program.getGLId(), uMVMatrixName);
            uModelMatrixLoc         = glGetUniformLocation(m_Program.getGLId(), uModelMatrixName);
            uNormalMatrixLoc        = glGetUniformLocation(m_Program.getGLId(), uNormalMatrixName);
            uCameraPositionLoc      = glGetUniformLocation(m_Program.getGLId(), uCameraPositionName);
            uCameraFrontLoc         = glGetUniformLocation(m_Program.getGLId(), uCameraFrontName);
            uWindowDimensionsLoc    = glGetUniformLocation(m_Program.getGLId(), uWindowDimensionsName);
            uClippingPlaneActiveLoc = glGetUniformLocation(m_Program.getGLId(), uClippingPlaneActiveName);
            uClippingPlaneLoc       = glGetUniformLocation(m_Program.getGLId(), uClippingPlaneName);

            m_programType = programType;

            uBaseTextureLoc      = (m_programType & TEXTURE)           ? glGetUniformLocation(m_Program.getGLId(), uBaseTextureName)      : 0;
            uAlternateTextureLoc = (m_programType & ALTERNATE_TEXTURE) ? glGetUniformLocation(m_Program.getGLId(), uAlternateTextureName) : 0;
            uLightsArrayLoc      = (m_programType & LIGHTS)            ? glGetUniformLocation(m_Program.getGLId(), ulightsArrayName)      : 0;
            uLightsCountLoc      = (m_programType & LIGHTS)            ? glGetUniformLocation(m_Program.getGLId(), ulightsCountName)      : 0;
            uLightDepthMapLoc    = (m_programType & SHADOWS)           ? glGetUniformLocation(m_Program.getGLId(), uLightDepthMapName)    : 0;
            uShadowMatrixLoc     = (m_programType & SHADOWS)           ? glGetUniformLocation(m_Program.getGLId(), uShadowMatrixName)     : 0;
            uSunLightPosLoc      = (m_programType & SHADOWS)           ? glGetUniformLocation(m_Program.getGLId(), uSunLightPosName)      : 0;
            // uClippingPlaneLoc    = (m_programType & CLIPPING_PLANE)    ? glGetUniformLocation(m_Program.getGLId(), uClippingPlaneName)    : 0;

            // baseTextureid = baseTex;
            // alternateTextureid = alternateTex;

            // std::cout << "Id of textures: " << baseTextureid << " " << alternateTextureid << std::endl;
        }

        BasicProgram(const FilePath &applicationPath, const glimac::FilePath &vsFile, const glimac::FilePath &fsFile)
                    : BasicProgram(applicationPath, vsFile, fsFile, ProgramType::DEFAULT) {}

        ~BasicProgram() {
            m_Program.~Program();
        }

        void linkDepthMap(GLuint depthFBO, GLuint depthMapInt) {
            if ( ! (m_programType & ProgramType::DEPTH_COMPUTE)) {
                // Error
                std::cout << "This program is not a depth compute program" << std::endl;
                return;
            }

            std::cout << "Binded the depthFBO and texture id " << depthFBO << " | " << depthMapInt << std::endl;
            m_depthFBO = depthFBO;
            m_depthMapInt = depthMapInt;
        }

        void bindTextures(GLuint tex, GLuint alternateTex, GLuint shadowTex) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, tex);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, alternateTex);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, shadowTex);
        }

        void unbindTextures() {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, 0);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, 0);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        void activateSimple(WindowManager &window, FPSCamera& camera) {
            m_Program.use();
            glUniformMatrix4fv(uNormalMatrixLoc, 1, GL_FALSE, glm::value_ptr(camera.getNormalMatrix()));
            glUniform3fv(uCameraPositionLoc, 1, glm::value_ptr(camera.getPos()));
            glUniform2fv(uWindowDimensionsLoc, 1, glm::value_ptr(window.getDimensions()));
            glUniform3fv(uCameraFrontLoc, 1, glm::value_ptr(camera.getFrontVector()));
        }

        void sendClippingPlane(const vec4& plane) {
            // if (m_programType & CLIPPING_PLANE)    glUniform3fv(uClippingPlaneLoc, 1, glm::value_ptr(plane));
            // if (m_programType & CLIPPING_PLANE)    glUniform1f(uClippingPlaneActiveLoc, 1.0f);
            glUniform4fv(uClippingPlaneLoc, 1, glm::value_ptr(plane));
            glUniform1f(uClippingPlaneActiveLoc, 1.0f);
        }

        void removeClippingPlane() {
            // if (m_programType & CLIPPING_PLANE)    glUniform1f(uClippingPlaneActiveLoc, 0.0f);
            glUniform1f(uClippingPlaneActiveLoc, 0.0f);
        }

        void activate(WindowManager &window, FPSCamera& camera, const mat4 &shadowMatrix, Light &lights, vec3 &sunPos) {
            activateSimple(window, camera);

            if (m_programType & TEXTURE)           glUniform1i(uBaseTextureLoc, Location::enumDiffuseTextureLoc);
            if (m_programType & ALTERNATE_TEXTURE) glUniform1i(uAlternateTextureLoc, Location::enumRoughnessTextureLoc);
            if (m_programType & LIGHTS)            glUniformMatrix3fv(uLightsArrayLoc, lights.size(), GL_FALSE, lights.data());
            if (m_programType & LIGHTS)            glUniform1i(uLightsCountLoc, lights.size());
            if (m_programType & SHADOWS) {
                // std::cout << "Active and have shadows " << uLightDepthMapLoc << " | " << Location::enumShadowTextureLoc << std::endl;
                // glUniform1i(uLightDepthMapLoc, 2);
                glUniform1i(uLightDepthMapLoc, Location::enumShadowTextureLoc); // check with the diffuse texture
                glUniformMatrix4fv(uShadowMatrixLoc, 1, GL_FALSE, glm::value_ptr((shadowMatrix))); // check with the diffuse texture
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, m_depthMapInt);
                glUniform3fv(uSunLightPosLoc, 1, glm::value_ptr(sunPos));
            }
            // if (m_programType & DEPTH_COMPUTE) {
            //     std::cout << "This programm compute the shadow map, hopefully: " << m_depthFBO << std::endl;
            //     glBindFramebuffer(GL_FRAMEBUFFER, m_depthFBO);
            //     glClear(GL_DEPTH_BUFFER_BIT);

            // }
        }
    };
}
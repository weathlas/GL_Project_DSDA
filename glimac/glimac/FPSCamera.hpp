#pragma once

#include <vector>

#include <glimac/BBox.hpp>
#include <glimac/KeyCodes.hpp>
#include <glimac/WindowManager.hpp>

#include "common.hpp"
#include "glm.hpp"

using namespace glm;

namespace glimac {

    const float radiusPlayer = 0.45f;

    const float playerMaxHeadPos = 1.78f;
    const float playerMinHeadPos = 0.85f;

    const float playerCrouchSpeed = 5.2f;
    const float playerZoomSpeed = 360.0f;

    const float playerMinFov = 30.0f;
    const float playerMaxFov = 75.0f;
    const float playerDefaultFov = 75.0f;

    const float defaultNearPlane = 0.08f;
    const float defaultFarPlane = 10000.0f;
    
    const float playerRunningFovOffset = 5.0f;
    const float playerRunningZoomSpeedUp = 45.0f;
    const float playerRunningZoomSpeedDown = 60.0f;

    const float playerFlyingFovOffset = 10.0f;
    const float playerFlyingZoomSpeedUp = 90.0f;
    const float playerFlyingZoomSpeedDown = 120.0f;

    const float playerGravity = 9.81f*0.500;

    const float playerMaxFallSpeed = playerGravity*9.0;
    const float playerJumpSpeed = playerGravity*1.0;

    const float walkSpeed = 2.54f;
    const float runMultiplier = 2.0f;
    const float crouchSpeed = 1.28f;

    const float mouseRotationMultiplier = 1.0f;
    const float keysRotationMultiplier = 1.0f;
    const float keyRotationSpeed = 90.f;

    // const float mouseSensitivity = 0.2f;
    const float mouseSensitivity = 4.56f;

    const float cameraDefaultReach = 1.5f;

    const float playerSmoothAccel = 5.0f;

    vec3 reflectVector(const vec3& v, const vec3& normal) {
        return v - 2.0f * dot(v, normal) * normal;
    }

    class FPSCamera {

        private:

            BBox3f m_bbox;
            mat4 m_projMatrix;
            mat4 m_normalMatrix;
            mat4 m_viewMatrix;
            vec3 m_Position;
            vec3 m_FootPosition;
            vec3 m_HeadDisplacement;
            vec3 m_ShakeDisplacement;
            float m_fPhy;
            float m_fTheta;
            float m_fFov;
            float m_fFovRunOffset;
            float m_fFovFlyOffset;

            float m_nearPlane;
            float m_farPlane;

            float m_verticalSpeed;

            float m_fWinWidth;
            float m_fWinHeight;

            vec3 m_FrontVector;
            vec3 m_GroundFrontVector;
            vec3 m_LeftVector;
            vec3 m_UpVector;

            vec2 m_oldMousePos;

            float m_buttonReach;

            bool m_isCrouched;
            bool m_isGrounded;
            bool m_isRunning;
            bool m_isFlying;
            bool m_inJump;
            bool m_canJump;
            bool m_jumped;

            bool m_allowFlyMode=true;

            bool m_canFly;

            vec3 m_speed;

            bool m_cheats;

            bool m_ortho;

        public:
            FPSCamera(float fov, float win_width, float win_height, float playerHeight, bool isOrtho, float nearPlane, float farPlane) : m_bbox(vec3(-radiusPlayer, 0.0f, -radiusPlayer), vec3(radiusPlayer, 1.78f, radiusPlayer)){
                m_HeadDisplacement = vec3(0, playerHeight, 0);
                m_ShakeDisplacement = vec3(0);
                m_FootPosition = vec3(.0f, 0.05f, .0f);
                m_fPhy     = glm::pi<float>();
                m_fTheta   = 0;
                m_fFov = fov;
                m_fFovRunOffset = 0.0f;
                m_fWinWidth = win_width;
                m_fWinHeight = win_height;
                m_Position = m_FootPosition + m_HeadDisplacement = m_ShakeDisplacement;
                m_verticalSpeed = 0.0f;
                computeDirectionVectors();
                m_oldMousePos = vec2(0);
                m_buttonReach = cameraDefaultReach;

                m_nearPlane = nearPlane;
                m_farPlane = farPlane;

                m_ortho = isOrtho;

                if(m_ortho) {
                    m_projMatrix = glm::ortho(-25.0f, 25.0f, -25.0f, 25.0f, m_nearPlane, m_farPlane);
                }
                else {
                    m_projMatrix = perspective(glm::radians(m_fFov+m_fFovRunOffset+m_fFovFlyOffset), 1.0f*m_fWinWidth/m_fWinHeight, 0.08f, 10000.f);
                }

                m_viewMatrix = glm::lookAt(m_FootPosition+m_HeadDisplacement + m_ShakeDisplacement, m_FootPosition+m_HeadDisplacement + m_ShakeDisplacement + m_FrontVector, m_UpVector);
                m_normalMatrix = glm::transpose(glm::inverse(m_viewMatrix));

                m_isCrouched = false;
                m_isGrounded = false;
                m_isRunning = false;
                m_isFlying = false;
                m_inJump = false;
                m_jumped = false;

                m_canJump = false;

                m_canFly = false;
                m_speed = vec3(0);

            };

            FPSCamera(float win_width, float win_height) : FPSCamera(playerDefaultFov, win_width, win_height, playerMaxHeadPos, false, defaultNearPlane, defaultFarPlane) {
            }

            bool resetMouse(vec2 mousePos) {
                m_oldMousePos = mousePos;
                return true;
            }

            void updateKeys(unsigned int keys, vec2 mousePos, float deltaT) {

                auto diffMouse = m_oldMousePos - mousePos;
                m_oldMousePos = mousePos;
                vec2 direction = vec2(((keys & keyLeft) > 0u) - ((keys & keyRight) > 0u), ((keys & keyUp) > 0u) - ((keys & keyDown) > 0u));

                m_isCrouched = keys & ctrlKey;
                // m_isRunning = (keys & shiftKey) && (keys & (keyLeft | keyRight | keyUp | keyDown));
                m_isRunning = (keys & shiftKey) && (keys & (keyUp));

                if(m_isFlying) {
                    //  * m_isFlying?playerJumpSpeed:1.0f
                    float finalSpeed = (((keys & shiftKey) ? runMultiplier : 1.0) * walkSpeed * playerJumpSpeed);
                    move(direction, vec2(finalSpeed, finalSpeed), deltaT);
                }
                else {
                    float finalSpeed = ((keys & ctrlKey) ? crouchSpeed : walkSpeed);
                    // direction.y > 0 ? moveFront(direction.y * finalSpeed * ((keys & shiftKey) ? runMultiplier : 1.0) * deltaT) : moveFront(direction.y * finalSpeed * deltaT);
                    
                    move(direction, vec2(finalSpeed, finalSpeed * ((keys & shiftKey) ? runMultiplier : 1.0)), deltaT);
                }
                
                
                // std::cout << m_canFly << " " << m_isFlying << " " << m_isGrounded << " " <<  m_isFlying << " " << m_verticalSpeed <<" " << ((keys & spacebar) > 0u) << " " << m_FootPosition.y << std::endl;
                if(m_allowFlyMode) {
                    if((m_canFly || m_isFlying) && (m_verticalSpeed >= -playerGravity && (keys & spacebar))) {

                        // if the player jumped in the air and falling while pressing the spacebar
                        if (!m_isFlying && !m_isGrounded) {
                            std::cout <<"activated"<<std::endl;
                            m_isFlying = true;
                            m_verticalSpeed = 0.0f;
                            // m_inJump = false;
                            m_canFly = false;
                        }
                        else {

                            // // the player is flying and 
                            if((m_canFly && m_isFlying && m_verticalSpeed > -playerGravity) || (m_isGrounded && (!(keys & spacebar)))) {
                                std::cout <<"normal"<<std::endl;
                                m_isFlying = false;
                                m_verticalSpeed = 0.0f;
                                m_canFly = false;
                            }
                        }

                    }

                    if (m_isFlying) {
                        if(m_isGrounded && (keys & ctrlKey)) {
                            std::cout <<"gfdgdfgfgdf"<<std::endl;
                            m_verticalSpeed = 0.0f;
                            m_isFlying = false;
                            m_canFly = false;
                        }
                        else {
                            fly((((keys & spacebar) > 0u) - ((keys & ctrlKey) > 0u)) * playerJumpSpeed * ((keys & shiftKey) ? runMultiplier : 1.0), deltaT);
                        }
                    }
                }

                // will just adjust the vertical speed if the player is falling
                jump((keys & spacebar));
                m_canJump = m_isGrounded && !(keys & spacebar);

                m_canFly = m_verticalSpeed > 0.0 && (!(keys & spacebar));
                crouch(!m_isFlying && (keys & ctrlKey), deltaT);
                zoom(keys & rightClick, deltaT);

                rotate(diffMouse*mouseSensitivity * m_fFov/playerMaxFov, deltaT);
                // rotate(horizontalRotation, verticalRotation, deltaT);
            }

            void shake(float amount) {
                if (amount <= 0) {
                    m_ShakeDisplacement = vec3(0);
                    return;
                }
                m_ShakeDisplacement = sphericalRand(linearRand(0.0f, amount));
            }

            bool update(WindowManager& window, std::vector<BBox3f> walls, float deltaT) {
                if(m_fWinWidth != window.width() || m_fWinHeight != window.height()) {
                    std::cout << "From" << m_fWinWidth<< " & " << m_fWinHeight << " to " << window.getDimensions() << std::endl;
                    m_fWinWidth = window.width();
                    m_fWinHeight = window.height();
                    m_projMatrix = perspective(glm::radians(m_fFov+m_fFovRunOffset+m_fFovFlyOffset), 1.0f*m_fWinWidth/m_fWinHeight, 0.08f, 10000.f);
                }
                m_isGrounded = false;
                auto before = m_verticalSpeed;
                m_verticalSpeed = clamp(m_verticalSpeed - (playerGravity*playerGravity * deltaT), -playerMaxFallSpeed, playerJumpSpeed);
                m_FootPosition.y += m_isFlying?0.0:m_verticalSpeed * deltaT;
                computeBBox();
                bool collide = updateCollision(walls);
                // bool collide = false;
                if(m_FootPosition.y < 0) {
                    m_FootPosition.y = 0;
                    m_verticalSpeed = 0;
                }
                if(m_verticalSpeed <= 0) {
                    m_inJump = false;
                }
                m_Position = m_FootPosition + m_HeadDisplacement + m_ShakeDisplacement;
                updateMatrix();
                // std::cout << before << " | " << m_verticalSpeed << std::endl;
                return collide;
            }

            void makeLookAt(vec3& newPos, vec3& point) {

                m_FootPosition = newPos;

                // trouver le vecteur direction entre cam et point
                auto dir = normalize(m_FootPosition - point);

                // trouver l'angle phy sur l'axe y
                float phy = atan2(dot(glm::cross(vec3(0, 0, -1), dir), vec3(0, 1, 0)), glm::dot(vec3(0, 0, -1), dir));
                
                auto otherNorm = glm::cross(dir, vec3(0, -1, 0));
                
                // trouver l'angle theta sur l'axe x
                float theta = atan2(dot(glm::cross(normalize(vec3(dir.x, 0, dir.z)), dir), otherNorm), glm::dot(normalize(vec3(dir.x, 0, dir.z)), dir));
                
                m_fTheta = theta;// * radToDeg;
                m_fPhy = phy;// * radToDeg;

                rotate(vec2(0.0f), 0);

                updateMatrix();

                std::cout << phy << " / " << theta << " : " << m_FootPosition << std::endl;

                // cos-1 [ (a * b) / (|a| * |b|) ]

                // atan2((Va x Vb) . Vn, Va . Vb)
            } 

            void setMirrorCamera(FPSCamera *other, const vec3 &mirrorPos, vec3 &mirrorNormal) {
                // Compute reflected position
                // auto camPos = vec3(getPos());
                float dist = dot(m_Position - mirrorPos, mirrorNormal);
                other->m_Position = m_Position - 2.0f * dist * mirrorNormal;

                // Reflect Forward and Up vectors
                other->m_FrontVector = reflectVector(-getFrontVector(), mirrorNormal);
                other->m_UpVector = reflectVector(getUpVector(), mirrorNormal);


                other->m_viewMatrix =glm::lookAt(other->m_Position, other->m_Position-other->m_FrontVector, other->m_UpVector);
                other->m_normalMatrix = glm::transpose(glm::inverse(other->m_viewMatrix));
                other->m_projMatrix = getProjMatrix();

            }

            const mat4 getViewMatrix() const {
                return m_viewMatrix;
            }

            const vec3 getPos() {
                return m_Position;
            }

            const mat4 getProjMatrix() const {
                // return glm::ortho(-10, 10, -10, 10, 1, 75);
                return m_projMatrix;
            }

            const mat4 getNormalMatrix() {
                return m_normalMatrix;
            }

            float getFov() {
                return m_fFov;
            }

            const vec3 getFrontVector() {
                return m_FrontVector;
            }

            const vec3 getUpVector() {
                return m_UpVector;
            }

            float getReach() {
                return m_buttonReach;
            }

        private:

            void computeBBox() {
                m_bbox.lower = m_FootPosition + vec3(-radiusPlayer, 0.0f, -radiusPlayer);
                m_bbox.upper = m_FootPosition + vec3(radiusPlayer, m_HeadDisplacement.y, radiusPlayer);
            }

            bool updateCollision(std::vector<BBox3f> walls) {
                bool collide = false;
                vec3 finalOffset(0);
                for (auto wall : walls) {
                    if(conjoint(m_bbox, wall)) {
                        finalOffset += getCollisionOffset(wall, true);
                        collide = true;
                    }
                }
                if(collide) {
                    m_FootPosition += finalOffset;
                    if(finalOffset.x != 0.0) {
                        m_speed.x = 0.0;
                    }
                    if(finalOffset.y != 0.0) {
                        m_speed.y = 0.0;
                    }
                    if(finalOffset.z != 0.0) {
                        m_speed.z = 0.0;
                    }
                }
                return collide;
            }

            vec3 getCollisionOffset(BBox3f &box, bool computeSpeedReset) {
                float xOverlap = std::max(0.0f, std::min(m_bbox.upper.x, box.upper.x) - std::max(m_bbox.lower.x, box.lower.x));
                float yOverlap = std::max(0.0f, std::min(m_bbox.upper.y, box.upper.y) - std::max(m_bbox.lower.y, box.lower.y));
                float zOverlap = std::max(0.0f, std::min(m_bbox.upper.z, box.upper.z) - std::max(m_bbox.lower.z, box.lower.z));
                float xOffset = 0.0f;
                float yOffset = 0.0f;
                float zOffset = 0.0f;

                if (xOverlap > 0) {
                    auto left = m_bbox.upper.x - box.lower.x;
                    auto right = box.upper.x - m_bbox.lower.x;
                    xOffset = left < right ? -xOverlap : xOverlap;
                }

                if (yOverlap > 0) {
                    auto bottom = m_bbox.upper.y - box.lower.y;
                    auto top = box.upper.y - m_bbox.lower.y;
                    yOffset = bottom < top ? -yOverlap : yOverlap;
                }

                if (zOverlap > 0) {
                    auto back = m_bbox.upper.z - box.lower.z;
                    auto front = box.upper.z - m_bbox.lower.z;
                    zOffset = back < front ? -zOverlap : zOverlap;
                }

                // std::cout << xOverlap << " | " << zOverlap << " -> " << xOffset << " | " << zOffset << std::endl;

                if(xOverlap < zOverlap) {
                    if (xOverlap < yOverlap) {
                        return vec3(xOffset, 0.0f, 0.0f);
                    }
                    if(computeSpeedReset && m_verticalSpeed < 0) {
                        m_verticalSpeed = 0.0f;
                    }
                    if(yOffset > 0 || m_verticalSpeed < 0) {
                        m_isGrounded = true;
                        return vec3(0.0f, yOffset, 0.0f);
                    }
                }
                else if (yOverlap < zOverlap) {
                    if(computeSpeedReset && m_verticalSpeed < 0) {
                        m_verticalSpeed = 0.0f;
                    }
                    if(yOffset > 0 || m_verticalSpeed < 0) {
                        m_isGrounded = true;
                        return vec3(0.0f, yOffset, 0.0f);
                    }
                }
                return vec3(0.0f, 0.0f, zOffset);

            }

            void computeDirectionVectors() {
                m_FrontVector = vec3(cos(m_fTheta)*sin(m_fPhy), sin(m_fTheta), cos(m_fTheta)*cos(m_fPhy));
                m_GroundFrontVector = vec3(sin(m_fPhy), 0, cos(m_fPhy));
                m_LeftVector = vec3(sin(m_fPhy+ glm::pi<float>()/2), 0, cos(m_fPhy+ glm::pi<float>()/2));
                m_UpVector = glm::cross(m_FrontVector, m_LeftVector);
            }
    
            void fly(float value, float deltaT) {
                m_FootPosition.y += value * deltaT;
            }

            // void moveLeft(float t) {
            //     m_FootPosition += m_LeftVector * t;
            //     computeBBox();
            // }

            // void moveFront(float t) {
            //     m_FootPosition += m_GroundFrontVector * t;
            //     computeBBox();
            // }

            void move(vec2 direction, vec2 speed, float deltaT) {
                if(direction.x != 0 || direction.y != 0) {
                    direction = normalize(direction);
                }

                auto localAccel = playerSmoothAccel;

                if(!m_isGrounded && !m_isFlying) {
                    localAccel /= 3.0;
                    // m_FootPosition += m_speed * deltaT;
                    // std::cout << length(m_speed) << std::endl;
                    // return;
                }
                if(m_isFlying) {
                    localAccel *= 3.0;
                }

                if(direction.x == 0 && direction.y == 0) {
                    auto lengthSpeed = length(m_speed);
                    if(lengthSpeed <= 0.0){
                        computeBBox();
                        return;
                    } 

                    auto speedNormal = m_speed / lengthSpeed;

                    auto combined = m_speed - speedNormal*(walkSpeed*localAccel) * deltaT;

                    if (dot(m_speed, combined) <= 0.0) {
                        m_speed = vec3(0.0);
                        computeBBox();
                        return;
                    }

                    m_speed = combined;
                    m_FootPosition += m_speed * deltaT;
                    computeBBox();
                    return;
                }


                auto speedAdded = (m_LeftVector * direction.x * speed.x + m_GroundFrontVector * direction.y * speed.y) * playerSmoothAccel * deltaT;

                auto combined = m_speed + speedAdded;

                auto distCombined = length(combined);

                auto localMaxSpeed = std::max(abs(speed.x), abs(speed.y));

                if(distCombined > localMaxSpeed) {
                    auto transitionSpeed = m_speed - localAccel * (combined / distCombined) * deltaT;
                    if(dot(m_speed, transitionSpeed) <= 0.0) {
                        m_speed = vec3(0);
                    }
                    else {
                        m_speed = transitionSpeed;
                    }
                }
                else {
                    m_speed = combined;
                }

                m_FootPosition += m_speed * deltaT;
                computeBBox();
            }

            void rotate(float degreesLeft, float degreeUp, float deltaT) {
                // m_fPhy += degreesLeft*degToRad*deltaT;
                // m_fTheta += degreeUp*degToRad*deltaT;
                m_fPhy += degreesLeft*degToRad*0.04;
                m_fTheta += degreeUp*degToRad*0.04;
                m_fTheta = clamp<float>(m_fTheta, -glm::pi<float>()/2, glm::pi<float>()/2);
                computeDirectionVectors();
            }

            void rotate(vec2 angles, float deltaT){rotate(angles.x, angles.y, deltaT);}

            void crouch(bool direction, float deltaT) {
                float height = m_HeadDisplacement.y + (deltaT * playerCrouchSpeed * (1 - 2*direction));
                height = clamp(height, playerMinHeadPos, playerMaxHeadPos);
                m_HeadDisplacement.y = height;
            }

            void zoom(bool direction, float deltaT) {
                float fov = m_fFov + deltaT * playerZoomSpeed * (1 - 2*direction);
                fov = clamp(fov, playerMinFov, playerMaxFov);

                float fovRunOffset = m_fFovRunOffset + deltaT * (playerRunningZoomSpeedUp * m_isRunning - playerRunningZoomSpeedDown * !m_isRunning);
                fovRunOffset = clamp(fovRunOffset, 0.0f, playerRunningFovOffset);

                float fovFlyOffset = m_fFovFlyOffset + deltaT * (playerFlyingZoomSpeedUp * m_isFlying - playerFlyingZoomSpeedDown * !m_isFlying);
                fovFlyOffset = clamp(fovFlyOffset, 0.0f, playerFlyingFovOffset);
                if (fov != m_fFov || fovRunOffset != m_fFovRunOffset || fovFlyOffset != m_fFovFlyOffset) {
                    m_fFovRunOffset = fovRunOffset;
                    m_fFovFlyOffset = fovFlyOffset;
                    m_fFov = fov;
                    // m_projMatrix = perspective(glm::radians(m_fFov+m_fFovRunOffset+m_fFovFlyOffset), 1.0f*m_fWinWidth/m_fWinHeight, 0.08f, 10000.f);
                }
                // m_fFov = clamp(m_fFov + deltaT * playerZoomSpeed * (1 - 2*direction), playerMinFov, playerMaxFov)
            }

            void jump(bool state) {
                if ((!state || m_verticalSpeed > 0) && !m_isFlying) {
                    return;
                }
                // std::cout << m_FootPosition.y << std::endl;
                if((m_FootPosition.y <= 0.05 && m_canJump) || m_isGrounded || (m_isFlying && state)) {//
                    m_canJump = false;
                    m_verticalSpeed = playerJumpSpeed*playerJumpSpeed;
                    // std::cout << "Jumped" << std::endl;
                }
            }
            
            void updateMatrix() {

                if(m_ortho) {
                    m_projMatrix = glm::ortho(-25.0f, 25.0f, -25.0f, 25.0f, m_nearPlane, m_farPlane);
                }
                else {
                    m_projMatrix = perspective(glm::radians(m_fFov+m_fFovRunOffset+m_fFovFlyOffset), 1.0f*m_fWinWidth/m_fWinHeight, 0.08f, 10000.f);
                }

                m_viewMatrix = glm::lookAt(m_FootPosition+m_HeadDisplacement + m_ShakeDisplacement, m_FootPosition+m_HeadDisplacement + m_ShakeDisplacement + m_FrontVector, m_UpVector);
                m_normalMatrix = glm::transpose(glm::inverse(m_viewMatrix));
            }

    };

}
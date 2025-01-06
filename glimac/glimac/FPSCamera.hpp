#pragma once

#include <vector>

#include <glimac/BBox.hpp>
#include <glimac/KeyCodes.hpp>

#include "common.hpp"
#include "glm.hpp"

using namespace glm;

namespace glimac {

    const float radiusPlayer = 0.5f;

    const float playerMaxHeadPos = 1.78f;
    const float playerMinHeadPos = 0.85f;

    const float playerCrouchSpeed = 5.2f;
    const float playerZoomSpeed = 360.0f;

    const float playerMinFov = 30.0f;
    const float playerMaxFov = 90.0f;
    const float playerDefaultFov = 90.0f;

    const float playerGravity = 9.81f*0.500;

    const float playerMaxFallSpeed = playerGravity*3.0;
    const float playerJumpSpeed = playerGravity*1.0;

    const float walkSpeed = 1.6f;
    const float runMultiplier = 3.0f;
    const float crouchSpeed = 0.63f;

    const float mouseRotationMultiplier = 1.0f;
    const float keysRotationMultiplier = 1.0f;
    const float keyRotationSpeed = 90.f;

    // const float mouseSensitivity = 0.2f;
    const float mouseSensitivity = 4.56f;

    class FPSCamera {

        private:
            mat4 m_projMatrix;
            vec3 m_Position;
            vec3 m_FootPosition;
            vec3 m_HeadDisplacement;
            vec3 m_ShakeDisplacement;
            float m_fPhy;
            float m_fTheta;
            float m_fFov;

            float m_verticalSpeed;
            bool m_onGround;

            float m_fWinWidth;
            float m_fWinHeight;

            vec3 m_FrontVector;
            vec3 m_GroundFrontVector;
            vec3 m_LeftVector;
            vec3 m_UpVector;

            BBox3f m_bbox;

            vec2 m_oldMousePos;

        public:
            FPSCamera(float fov, float win_width, float win_height) : m_bbox(vec3(-radiusPlayer, 0.0f, -radiusPlayer), vec3(radiusPlayer, 1.78f, radiusPlayer)){
                m_projMatrix = perspective(glm::radians(fov), 1.0f*win_width/win_height, 0.03f, 1000.f);
                m_FootPosition = vec3(.0f, 0.05f, .0f);
                m_fPhy     = glm::pi<float>();
                m_fTheta   = 0;
                m_fFov = fov;
                m_fWinWidth = win_width;
                m_fWinHeight = win_height;
                m_HeadDisplacement = vec3(0, playerMaxHeadPos, 0);
                m_ShakeDisplacement = vec3(0);
                m_Position = m_FootPosition + m_HeadDisplacement = m_ShakeDisplacement;
                m_verticalSpeed = 0.0f;
                computeDirectionVectors();
                m_oldMousePos = vec2(0);
            };

            FPSCamera(float win_width, float win_height) : FPSCamera(playerDefaultFov, win_width, win_height) {
            }

            void moveLeft(float t) {
                m_FootPosition += m_LeftVector * t;
                computeBBox();
            }

            void moveFront(float t) {
                m_FootPosition += m_GroundFrontVector * t;
                computeBBox();
            }

            bool resetMouse(vec2 mousePos) {
                m_oldMousePos = mousePos;
                return true;
            }

            void updateKeys(unsigned int keys, vec2 mousePos, float deltaT) {

                auto diffMouse = m_oldMousePos - mousePos;
                m_oldMousePos = mousePos;
                vec2 direction = vec2(((keys & keyLeft) > 0u) - ((keys & keyRight) > 0u), ((keys & keyUp) > 0u) - ((keys & keyDown) > 0u));


                crouch(keys & ctrlKey, deltaT);
                zoom(keys & rightClick, deltaT);

                float finalSpeed = ((keys & ctrlKey) ? crouchSpeed : walkSpeed) * ((keys & shiftKey) ? runMultiplier : 1.0);
                move(direction, finalSpeed, deltaT); 

                jump((keys & spacebar), deltaT);

                auto horizontalRotation = keyRotationSpeed * (((keys & KeyCode::rotateLeft)?1.0f:0.0f+(keys & rotateRight)?-1.0f:0.0f)*((keys & shiftKey) ? keysRotationMultiplier : 1.0f));
                auto verticalRotation = keyRotationSpeed * (((keys & KeyCode::rotateUp)?1.0f:0.0f+(keys & rotateDown )?-1.0f:0.0f)*((keys & shiftKey) ? keysRotationMultiplier : 1.0f));


                rotate(diffMouse*mouseSensitivity * m_fFov/playerMaxFov, deltaT);
                rotate(horizontalRotation, verticalRotation, deltaT);

                // if (keys & rotateLeft) {
                //     rotateLeft(keyRotationSpeed*(1+1.0f*((keys & shiftKey) > 0u)));
                // }
                // if (keys & rotateRight) {
                //     rotateLeft(-keyRotationSpeed*(1+1.0f*((keys & shiftKey) > 0u)));
                // }
                // if (keys & rotateUp) {
                //     rotateUp(keyRotationSpeed*(1+1.0f*((keys & shiftKey) > 0u)));
                // }
                // if (keys & rotateDown) {
                //     rotateUp(-keyRotationSpeed*(1+1.0f*((keys & shiftKey) > 0u)));
                // }
                // if (keys & rightClick) {
                //     rotateLeft(diffMouse.x*mouseSensitivity * m_fFov/playerMaxFov);
                //     rotateUp(diffMouse.y*mouseSensitivity * m_fFov/playerMaxFov);
                // }
            }

            void move(vec2 direction, float speed, float deltaT) {
                if(direction.x != 0 || direction.y != 0) {
                    direction = normalize(direction);
                    m_FootPosition += (m_LeftVector * direction.x + m_GroundFrontVector * direction.y) * deltaT * speed;
                    computeBBox();
                }
            }

            void rotateLeft(float degrees) {
                m_fPhy += degrees*degToRad;
                computeDirectionVectors();
            }

            void rotateUp(float degrees) {
                m_fTheta += degrees*degToRad;
                m_fTheta = clamp<float>(m_fTheta, -glm::pi<float>()/2, glm::pi<float>()/2);
                computeDirectionVectors();
            }

            void rotate(float degreesLeft, float degreeUp, float deltaT) {
                m_fPhy += degreesLeft*degToRad*deltaT;
                m_fTheta += degreeUp*degToRad*deltaT;
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
                if (fov != m_fFov) {
                    m_fFov = fov;
                    m_projMatrix = perspective(glm::radians(m_fFov), 1.0f*m_fWinWidth/m_fWinHeight, 0.08f, 1000.f);
                }
                // m_fFov = clamp(m_fFov + deltaT * playerZoomSpeed * (1 - 2*direction), playerMinFov, playerMaxFov)
            }

            void jump(bool state, float deltaT) {
                if (!state || m_verticalSpeed > 0) {
                    return;
                }
                // std::cout << m_FootPosition.y << std::endl;
                if(m_FootPosition.y <= 0.05 || m_onGround) {
                    m_verticalSpeed = playerJumpSpeed*playerJumpSpeed;
                }
            }

            void shake(float amount) {
                if (amount <= 0) {
                    m_ShakeDisplacement = vec3(0);
                    return;
                }
                m_ShakeDisplacement = sphericalRand(linearRand(0.0f, amount));
            }

            bool updatePhysic(std::vector<BBox3f> walls, float deltaT) {
                m_onGround = false;
                m_verticalSpeed = clamp(m_verticalSpeed - (playerGravity*playerGravity * deltaT), -playerMaxFallSpeed, playerJumpSpeed);
                m_FootPosition.y += m_verticalSpeed * deltaT;
                m_Position = m_FootPosition + m_HeadDisplacement + m_ShakeDisplacement;
                computeBBox();
                bool collide = updateCollision(walls);
                // bool collide = false;
                if(m_FootPosition.y < 0) {
                    m_FootPosition.y = 0;
                    m_verticalSpeed = 0;
                }
                return collide;
            }

            mat4 getViewMatrix() const {
                return glm::lookAt(m_FootPosition+m_HeadDisplacement + m_ShakeDisplacement, m_FootPosition+m_HeadDisplacement + m_ShakeDisplacement + m_FrontVector, m_UpVector);
            }

            const vec3 getPos() {
                return m_Position;
            }

            const mat4 getProjMatrix() {
                // return glm::ortho(-10, 10, -10, 10, 1, 75);
                return m_projMatrix;
            }

            const float getFov() {
                return m_fFov;
            }

            const vec3 getFrontVector() {
                return m_FrontVector;
            }

            const vec3 getUpVector() {
                return m_UpVector;
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
                        m_onGround = true;
                        return vec3(0.0f, yOffset, 0.0f);
                    }
                }
                else if (yOverlap < zOverlap) {
                    if(computeSpeedReset && m_verticalSpeed < 0) {
                        m_verticalSpeed = 0.0f;
                    }
                    if(yOffset > 0 || m_verticalSpeed < 0) {
                        m_onGround = true;
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
    };

}
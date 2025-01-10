#pragma once

#include <vector>

#include <glimac/BasicProgram.hpp>
#include <glimac/VertexData.hpp>
#include <glimac/common.hpp>
#include <glimac/BBox.hpp>

#include "common.hpp"
#include "glm.hpp"

#define MIN_ZOOM 1

using namespace glm;

namespace glimac {

    class Instance {
        private:
            Geometry m_geometry;
            std::shared_ptr<VertexData> m_vertexData;
            std::vector<Transform> m_transforms;
            std::vector<glm::mat4> m_matrices;

            GLenum m_sfactor = GL_ONE;
            GLenum m_dfactor = GL_ZERO;
            GLenum m_depthFac = GL_TRUE;

            GLuint m_baseTex;
            GLuint m_alternateTex;

            std::vector<vec3> getCornersOfBBox(BBox3f &box) {
                return {
                    vec3(box.lower.x, box.lower.y, box.lower.z), // lll
                    vec3(box.lower.x, box.lower.y, box.upper.z), // llu
                    vec3(box.lower.x, box.upper.y, box.lower.z), // lul
                    vec3(box.lower.x, box.upper.y, box.upper.z), // luu
                    vec3(box.upper.x, box.lower.y, box.lower.z), // ull
                    vec3(box.upper.x, box.lower.y, box.upper.z), // ulu
                    vec3(box.upper.x, box.upper.y, box.lower.z), // uul
                    vec3(box.upper.x, box.upper.y, box.upper.z)  // uuu
                };
            }

        public:

            ~Instance() {
                m_vertexData.get()->~VertexData();
            }

            Instance(glimac::FilePath root, std::string name, GLuint baseTex, GLuint alternateTex) {
                m_geometry.loadOBJ(root + ("assets/models/" + name + ".obj"), root + ("assets/models/" + name + ".mtl"), false);
                m_vertexData = std::make_shared<VertexData>(m_geometry);
                m_baseTex = baseTex;
                m_alternateTex = alternateTex;
            }
            
            Instance(size_t vertexCount, const glimac::ShapeVertex * dataPointer, GLuint baseTex, GLuint alternateTex) {
                m_vertexData = std::make_shared<VertexData>(vertexCount, dataPointer);
                m_baseTex = baseTex;
                m_alternateTex = alternateTex;
            }

            Instance(glimac::FilePath root, std::string name, GLuint baseTex, GLuint alternateTex, Transform t) : Instance(root, name, baseTex, alternateTex) {
                add(t);
            };

            Instance(size_t vertexCount, const glimac::ShapeVertex * dataPointer, GLuint baseTex, GLuint alternateTex, Transform t) : Instance(vertexCount, dataPointer, baseTex, alternateTex) {
                add(t);
            };

            size_t size() {
                return m_transforms.size();
            }

            void add() {
                add(Transform());
            }

            void add(Transform t) {
                m_transforms.push_back(t);
                m_matrices.push_back(mat4(1));
                compute(m_transforms.size() - 1);
            }

            void addPosition(unsigned int index, vec3 pos) {
                m_transforms.at(index).m_Position += pos;
            }

            void addScales(unsigned int index, vec3 scale) {
                m_transforms.at(index).m_Scales += scale;
            }

            void addAngles(unsigned int index, vec3 angles) {
                m_transforms.at(index).m_Angles += angles;
            }

            void updatePosition(unsigned int index, vec3 pos) {
                m_transforms.at(index).m_Position = pos;
            }

            void updateScales(unsigned int index, vec3 scale) {
                m_transforms.at(index).m_Scales = scale;
            }

            void updateAngles(unsigned int index, vec3 angles) {
                m_transforms.at(index).m_Angles = angles;
            }

            void update(Transform transform, unsigned int index) {
                m_transforms.at(index) = transform;
            }

            void computeLast() {
                compute(m_transforms.size()-1);
            }

            Transform get(unsigned int index) {
                return m_transforms.at(index);
            }

            BBox3f getDefautBBox() {
                return BBox3f(m_geometry.getBoundingBox());
            }

            void computeAll() {
                for (size_t i = 0; i < m_transforms.size(); i++) {
                    compute(i);
                }
            }

            void compute(unsigned int index) {
                Transform t = m_transforms.at(index);
                m_matrices.at(index) = scale(rotate(rotate(rotate(translate(mat4(1), t.m_Position), t.m_Angles.y, Y_Vector), t.m_Angles.x, X_Vector), t.m_Angles.z, Z_Vector), t.m_Scales);
            }

            std::vector<BBox3f> getBBox() {
                auto box = m_geometry.getBoundingBox();
                std::vector<BBox3f> allBBox;
                for (auto mat: m_matrices) {

                    std::vector<vec3> transformedCorners;
                    for (auto corner : getCornersOfBBox(box)) {
                        transformedCorners.push_back(mat * vec4(corner, 1));
                    }

                    BBox3f tBBox(transformedCorners.at(0), transformedCorners.at(0));

                    for (auto transformedCorner : transformedCorners) {
                        tBBox.grow(transformedCorner);
                    }

                    vec3 dimensions = tBBox.size();

                    float minWidth = 0.1f;

                    if (dimensions.x < minWidth) {
                        tBBox.lower.x -= (minWidth-dimensions.x)/2;
                        tBBox.upper.x += (minWidth-dimensions.x)/2;
                    }
                    if (dimensions.z < minWidth) {
                        tBBox.lower.z -= (minWidth-dimensions.z)/2;
                        tBBox.upper.z += (minWidth-dimensions.z)/2;
                    }

                    allBBox.push_back(tBBox);
                }
                return allBBox;
            }

            void drawAll(BasicProgram &program, const mat4 &matMV, const mat4 &matProj, GLuint shadowTex) {

                glBindVertexArray(m_vertexData->getVao());

                program.bindTextures(m_baseTex, m_alternateTex, shadowTex);

                if (true) {
                    glEnable(GL_BLEND);
                    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    glBlendFunc(m_sfactor, m_dfactor);

                    glDepthMask(m_depthFac);

                    if(m_depthFac == GL_FALSE) {
                        // glCullFace(GL_NONE);
                        glDisable(GL_CULL_FACE);
                    }
                }

                if(m_vertexData->type == VertexDataType::GEOMETRY) {
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vertexData->getelementBuffer());

                    for (size_t i = 0; i < m_matrices.size(); i++) {
                        auto finalMatMV = matMV * m_matrices.at(i);
                        glUniformMatrix4fv(program.uMVPMatrixLoc, 1, GL_FALSE, glm::value_ptr((matProj) * finalMatMV));
                        glUniformMatrix4fv(program.uMVMatrixLoc, 1, GL_FALSE, glm::value_ptr(finalMatMV));
                        glUniformMatrix4fv(program.uModelMatrixLoc, 1, GL_FALSE, glm::value_ptr(m_matrices.at(i)));
                        glDrawElements(
                            GL_TRIANGLES,      // mode
                            m_vertexData->getVertexCount(),    // count
                            GL_UNSIGNED_INT,   // type
                            (void*)0           // element array buffer offset
                        );
                    }
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
                }
                else {
                    for (size_t i = 0; i < m_matrices.size(); i++) {
                        auto finalMatMV = matMV * m_matrices.at(i);
                        glUniformMatrix4fv(program.uMVPMatrixLoc, 1, GL_FALSE, glm::value_ptr((matProj) * finalMatMV));
                        glUniformMatrix4fv(program.uMVMatrixLoc, 1, GL_FALSE, glm::value_ptr(finalMatMV));
                        glUniformMatrix4fv(program.uModelMatrixLoc, 1, GL_FALSE, glm::value_ptr(m_matrices.at(i)));
                        glDrawArrays(GL_TRIANGLES, 0, m_vertexData->getVertexCount()-1);
                    }
                }

                if(m_depthFac == GL_FALSE) {
                    // glCullFace(GL_BACK);
                    glEnable(GL_CULL_FACE);
                }

                glDepthMask(GL_TRUE);
                glDisable(GL_BLEND);

                program.unbindTextures();
                glBindVertexArray(0);
            }

            void drawAll(BasicProgram &program, const FPSCamera &camera, GLuint shadowTex) {
                drawAll(program, camera.getViewMatrix(), camera.getProjMatrix(), shadowTex);
            }

            void setBlendToOpaque() {
                setBlendFlags(GL_ONE, GL_ZERO, GL_TRUE);
            }

            void setBlendToTransparent() {
                setBlendFlags(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_FALSE);
            }

            void setBlendFlags(GLenum sfactor, GLenum dfactor, GLenum depthFac) {
                m_sfactor = sfactor;
                m_dfactor = dfactor;
                m_depthFac = depthFac;
            }
    };

}
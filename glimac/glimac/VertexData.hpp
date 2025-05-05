#pragma once

#include <vector>

#include <glimac/BasicProgram.hpp>
#include <glimac/Geometry.hpp>

#include "common.hpp"
#include "glm.hpp"

#include <cmath>

using namespace glm;

namespace glimac {

    const GLuint VERTEX_ATTR_POSITION = 0;
    const GLuint VERTEX_ATTR_NORMAL   = 1;
    const GLuint VERTEX_ATTR_COORDS   = 2;
    const GLuint VERTEX_ATTR_TANGEANT = 3;

    enum VertexDataType {CANONICAL, GEOMETRY, DYNAMIC};

    enum DynamicType {dynamic_none, dynamic_grid, dynamic_cube};

    class VertexData {
        public:
            const VertexDataType type;
            const DynamicType m_dynamic_type;

            VertexData(const glimac::Geometry *geometry) : type(GEOMETRY), m_dynamic_type(dynamic_none) {
                m_vertexCount = geometry->getVertexCount();
                m_vbo = vboInit(geometry);
                std::cout << "vboInit ok " << glGetError() << std::endl;
                m_vao = vaoInit(m_vbo);
                std::cout << "vaoInit ok " << glGetError() << std::endl;
                m_elementBuffer = elementBufferInit(geometry);
                std::cout << "elementBufferInit ok " << glGetError() << std::endl;
            };
            VertexData(size_t vertexCount, const glimac::ShapeVertex * dataPointer) : type(CANONICAL), m_dynamic_type(dynamic_none) {
                m_vertexCount = vertexCount;
                std::cout << "vertexCount:" << vertexCount << " %3?: " << (vertexCount%3==0) << std::endl;
                m_vbo = vboInit(vertexCount, dataPointer);
                m_vao = vaoInit(m_vbo);
                m_elementBuffer=0;
            };
            VertexData(size_t vertexCountDimension, DynamicType dynamicType) : type(DYNAMIC), m_dynamic_type(dynamicType) {
                assert(dynamicType!=dynamic_none);
                assert(vertexCountDimension>1);
                m_nb_point_dim = vertexCountDimension;
                switch (m_dynamic_type)
                {
                    case DynamicType::dynamic_grid:
                        m_vertexCount = m_nb_point_dim*m_nb_point_dim;
                        for (size_t y = 0; y < m_nb_point_dim - 1; ++y) {
                            for (size_t x = 0; x < m_nb_point_dim - 1; ++x) {
                                int i0 = y * m_nb_point_dim + x;
                                int i1 = y * m_nb_point_dim + x + 1;
                                int i2 = (y + 1) * m_nb_point_dim + x;
                                int i3 = (y + 1) * m_nb_point_dim + x + 1;

                                
                                // First triangle
                                m_index.push_back(i0);
                                m_index.push_back(i2);
                                m_index.push_back(i1);
                                
                                // Second triangle
                                m_index.push_back(i1);
                                m_index.push_back(i2);
                                m_index.push_back(i3);
                            }
                        }
                    break;
                    case DynamicType::dynamic_cube:
                        // m_vertexCount = vertexCountDimension*vertexCountDimension*vertexCountDimension - (vertexCountDimension-2)*(vertexCountDimension-2)*(vertexCountDimension-2);
                        m_vertexCount = m_nb_point_dim*m_nb_point_dim*6;
                        // m_vertexCount = vertexCountDimension*vertexCountDimension*vertexCountDimension;
            
                        addCubeIndexOffsetAll(m_nb_point_dim, m_index);
                    break;
                    
                    default:
                    break;
                }
                
                glGenVertexArrays(1, &m_vao);
                glGenBuffers(1, &m_vbo); // double buffering
                glGenBuffers(1, &m_vbo2); // double buffering
                glGenBuffers(1, &m_elementBuffer);
                
                glBindVertexArray(m_vao);
                
                // Set up both VBOs
                glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
                glBufferData(GL_ARRAY_BUFFER, sizeof(glimac::Geometry::Vertex) * m_vertexCount, nullptr, GL_DYNAMIC_DRAW);
                glBindBuffer(GL_ARRAY_BUFFER, m_vbo2);
                glBufferData(GL_ARRAY_BUFFER, sizeof(glimac::Geometry::Vertex) * m_vertexCount, nullptr, GL_DYNAMIC_DRAW);



                glEnableVertexAttribArray(VERTEX_ATTR_POSITION);
                glEnableVertexAttribArray(VERTEX_ATTR_NORMAL);
                glEnableVertexAttribArray(VERTEX_ATTR_COORDS);
                glEnableVertexAttribArray(VERTEX_ATTR_TANGEANT);

                glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
                glVertexAttribPointer(VERTEX_ATTR_POSITION, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GL_FLOAT), 0);
                glVertexAttribPointer(VERTEX_ATTR_NORMAL  , 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GL_FLOAT), (const GLvoid*)(3 * sizeof(GL_FLOAT)));
                glVertexAttribPointer(VERTEX_ATTR_COORDS  , 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GL_FLOAT), (const GLvoid*)(6 * sizeof(GL_FLOAT)));
                glVertexAttribPointer(VERTEX_ATTR_TANGEANT, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GL_FLOAT), (const GLvoid*)(8 * sizeof(GL_FLOAT)));
                glBindBuffer(GL_ARRAY_BUFFER, 0);

                glBindBuffer(GL_ARRAY_BUFFER, m_vbo2);
                glVertexAttribPointer(VERTEX_ATTR_POSITION, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GL_FLOAT), 0);
                glVertexAttribPointer(VERTEX_ATTR_NORMAL  , 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GL_FLOAT), (const GLvoid*)(3 * sizeof(GL_FLOAT)));
                glVertexAttribPointer(VERTEX_ATTR_COORDS  , 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GL_FLOAT), (const GLvoid*)(6 * sizeof(GL_FLOAT)));
                glVertexAttribPointer(VERTEX_ATTR_TANGEANT, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GL_FLOAT), (const GLvoid*)(8 * sizeof(GL_FLOAT)));
                glBindBuffer(GL_ARRAY_BUFFER, 0);

                glBindVertexArray(0);
                
                // Index buffer (static)
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_elementBuffer);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * m_index.size(), m_index.data(), GL_STATIC_DRAW);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

                
                // Attribute pointers
                glEnableVertexAttribArray(0); // Position
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glimac::Geometry::Vertex), (void*)0);
                
                glBindVertexArray(0);
            };

            ~VertexData() {
                glDeleteBuffers(1, &m_vbo);
                glDeleteBuffers(1, &m_vbo2);
                glDeleteBuffers(1, &m_elementBuffer);
                glDeleteVertexArrays(1, &m_vao);
            }

            GLuint getVao() {
                return m_vao;
            }

            GLuint getelementBuffer() {
                return m_elementBuffer;
            }

            size_t getVertexCount() {
                return m_vertexCount;
            }

            size_t getDynamicTriangleCount() {
                return m_index.size();
            }

            void updateDynamicMesh(std::vector<vec3> &positions) {
                if(m_dynamic_type == DynamicType::dynamic_none) {
                    return;
                }
                // Fill vertex buffer with updated positions
                // auto borderCount = 0;
                // switch (m_dynamic_type)
                // {
                // case DynamicType::dynamic_grid:
                //     borderCount = sqrt(float(m_vertexCount));
                //     break;
                // case DynamicType::dynamic_cube:
                //     borderCount = cbrt(float(m_vertexCount));
                //     break;
                
                // default:
                //     break;
                // }
                std::vector<glimac::Geometry::Vertex> frameVertices(m_vertexCount);

                std::vector<vec3> usefullPositions;

                switch (m_dynamic_type)
                {
                case DynamicType::dynamic_grid:
                    for (size_t i = 0; i < positions.size() && i < m_vertexCount; ++i) {
                        
                        float u, v;
                        u = (i / m_nb_point_dim) / float(m_nb_point_dim-1);
                        v = (i % m_nb_point_dim) / float(m_nb_point_dim-1);
                        frameVertices[i].m_Position = positions[i];
                        frameVertices[i].m_Normal = vec3(0, 0, 0);
                        frameVertices[i].m_Tangeant = vec3(1, 0, 0);
                        frameVertices[i].m_TexCoords = vec2(u, v);
                    }
                    break;
                case DynamicType::dynamic_cube:
                    getCubePositions(m_nb_point_dim, usefullPositions, positions);
                    // std::cout<<"Cube position "<< usefullPositions.size()<< " | " <<m_nb_point_dim<< " | " << positions.size() <<std::endl;
                    for (size_t i = 0; i < usefullPositions.size(); ++i) {
                        float u, v;
                        u = (((i % (m_nb_point_dim*m_nb_point_dim)) / m_nb_point_dim)) / float(m_nb_point_dim-1);
                        v = (((i % (m_nb_point_dim*m_nb_point_dim)) % m_nb_point_dim)) / float(m_nb_point_dim-1);
                        // uvCubeIndex(i%m_nb_point_dim, (i/m_nb_point_dim)%m_nb_point_dim, i/(m_nb_point_dim*m_nb_point_dim), i/(2*m_nb_point_dim*m_nb_point_dim), m_nb_point_dim, (i/(m_nb_point_dim*m_nb_point_dim))%2, &u, &v);
                        frameVertices[i].m_Position = usefullPositions[i];
                        frameVertices[i].m_Normal = vec3(0, 0, 0);
                        frameVertices[i].m_Tangeant = vec3(1, 0, 0);
                        frameVertices[i].m_TexCoords = vec2(u, v);
                    }
                    break;
                
                default:
                    break;
                }

                // Normals
                for (size_t i = 0; i < m_index.size(); i += 3) {
                    int i0 = m_index[i];
                    int i1 = m_index[i + 1];
                    int i2 = m_index[i + 2];

                    glm::vec3 v0 = frameVertices[i0].m_Position;
                    glm::vec3 v1 = frameVertices[i1].m_Position;
                    glm::vec3 v2 = frameVertices[i2].m_Position;

                    glm::vec3 edge1 = v1 - v0;
                    glm::vec3 edge2 = v2 - v0;

                    glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));

                    // Add the face normal to each vertex
                    frameVertices[i0].m_Normal += faceNormal;
                    frameVertices[i1].m_Normal += faceNormal;
                    frameVertices[i2].m_Normal += faceNormal;
                }

                for(auto &v: frameVertices) {
                    v.m_Normal = normalize(v.m_Normal);
                }

                // Bind VAO
                glBindVertexArray(m_vao);

                // Swap VBOs
                m_current_buffer = (m_current_buffer + 1) % 2;
                switch (m_current_buffer)
                {
                case 0:
                    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
                break;
                case 1:
                    glBindBuffer(GL_ARRAY_BUFFER, m_vbo2);
                    break;
                    default:
                    break;
                }

                // std::cout<<"Copying "<< frameVertices.size()<< " | " <<m_nb_point_dim<< " | " << usefullPositions.size() <<std::endl;
                
                // Update buffer
                glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glimac::Geometry::Vertex) * frameVertices.size(), frameVertices.data());
                
                // // Draw
                // glDrawElements(GL_TRIANGLES, m_index.size(), GL_UNSIGNED_INT, 0);
                glBindBuffer(GL_ARRAY_BUFFER, 0);

                glBindVertexArray(0);
            }

        private:
            GLuint m_vao;
            GLuint m_vbo;

            GLuint m_vbo2;
            std::vector<GLuint> m_index;
            uint m_current_buffer;

            GLuint m_elementBuffer;
            size_t m_vertexCount;
            size_t m_nb_point_dim;
            glimac::ShapeVertex * dataPointer;
            glimac::Geometry * geometry;

            GLuint elementBufferInit(const glimac::Geometry *geometry) {
                GLuint elementBuffer;
                glGenBuffers(1, &elementBuffer);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, geometry->getIndexCount() * sizeof(unsigned int), geometry->getIndexBuffer(), GL_STATIC_DRAW);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

                return elementBuffer;
            }

            GLuint vboInit(const glimac::Geometry *geometry) {
                GLuint vbo; // Vertex Buffer Object
                glGenBuffers(1, &vbo);
                glBindBuffer(GL_ARRAY_BUFFER, vbo);
                glBufferData(GL_ARRAY_BUFFER, geometry->getVertexCount() * sizeof(glimac::Geometry::Vertex), geometry->getVertexBuffer(), GL_STATIC_DRAW);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                return vbo;
            }

            GLuint vboInit(size_t vertexCount, const glimac::ShapeVertex * dataPointer) {
                GLuint vbo; // Vertex Buffer Object
                glGenBuffers(1, &vbo);
                glBindBuffer(GL_ARRAY_BUFFER, vbo);
                glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(glimac::ShapeVertex), dataPointer, GL_STATIC_DRAW);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                return vbo;
            }

            GLuint vboInit(size_t vertexCount) {
                GLuint vbo; // Vertex Buffer Object
                glGenBuffers(1, &vbo);
                glBindBuffer(GL_ARRAY_BUFFER, vbo);
                glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(glimac::ShapeVertex), dataPointer, GL_STATIC_DRAW);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                return vbo;
            }

            GLuint vaoInit(GLuint vbo) {
                GLuint vao; // Vertex Array Object
                glGenVertexArrays(1, &vao);
                glBindVertexArray(vao);

                glEnableVertexAttribArray(VERTEX_ATTR_POSITION);
                glEnableVertexAttribArray(VERTEX_ATTR_NORMAL);
                glEnableVertexAttribArray(VERTEX_ATTR_COORDS);
                glEnableVertexAttribArray(VERTEX_ATTR_TANGEANT);

                glBindBuffer(GL_ARRAY_BUFFER, vbo);
                glVertexAttribPointer(VERTEX_ATTR_POSITION, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GL_FLOAT), 0);
                glVertexAttribPointer(VERTEX_ATTR_NORMAL  , 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GL_FLOAT), (const GLvoid*)(3 * sizeof(GL_FLOAT)));
                glVertexAttribPointer(VERTEX_ATTR_COORDS  , 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GL_FLOAT), (const GLvoid*)(6 * sizeof(GL_FLOAT)));
                glVertexAttribPointer(VERTEX_ATTR_TANGEANT, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GL_FLOAT), (const GLvoid*)(8 * sizeof(GL_FLOAT)));
                glBindBuffer(GL_ARRAY_BUFFER, 0);

                glBindVertexArray(0);

                return vao;
            }

            bool uvCubeIndex(int x, int y, int z, int axis, int size, bool flip, float *u, float *v) {
                switch (axis)
                {
                case 0:
                    *u = (z / size) / float(size-1);
                    *v = (y % size) / float(size-1) - flip * 2.0f;
                    break;
                    case 1:
                    *u = (x / size) / float(size-1);
                    *v = (z % size) / float(size-1) - flip * 2.0f;
                    break;
                case 2:
                    *u = (x / size) / float(size-1);
                    *v = (y % size) / float(size-1) - flip * 2.0f;
                    break;
                default:
                    break;
                }
                return true;
            }

            int cubeIndex(int x, int y, int z, int n) {
                return x + y * n + z * n * n;
            }

            void getCubePositions(int size, std::vector<vec3> &output, std::vector<vec3> &input) {
                // for (size_t offset = 0; offset < 6; offset++)
                // {
                    for (int z = 0; z < size; z++) {
                        for (int y = 0; y < size; y++) {
                            output.push_back(input.at(cubeIndex(0, y, z, size)));
                        }
                    }
                    for (int z = 0; z < size; z++) {
                        for (int y = 0; y < size; y++) {
                            output.push_back(input.at(cubeIndex(size-1, y, z, size)));
                        }
                    }
                    for (int z = 0; z < size; z++) {
                        for (int x = 0; x < size; x++) {
                            output.push_back(input.at(cubeIndex(x, 0, z, size)));
                        }
                    }
                    for (int z = 0; z < size; z++) {
                        for (int x = 0; x < size; x++) {
                            output.push_back(input.at(cubeIndex(x, size-1, z, size)));
                        }
                    }
                    for (int y = 0; y < size; y++) {
                        for (int x = 0; x < size; x++) {
                            output.push_back(input.at(cubeIndex(x, y, 0, size)));
                        }
                    }
                    for (int y = 0; y < size; y++) {
                        for (int x = 0; x < size; x++) {
                            output.push_back(input.at(cubeIndex(x, y, size-1, size)));
                        }
                    }
                // }
            }

            void addCubeIndexOffsetAll(int size, std::vector<GLuint> &output) {
                for (int offset = 0; offset < 6; offset++)
                {
                    auto offsetFace = offset*size*size;
                    for (int y = 0; y < size - 1; ++y) {
                        for (int x = 0; x < size - 1; ++x) {
                            int i0 = offsetFace + y * size + x;
                            int i1 = offsetFace + y * size + x + 1;
                            int i2 = offsetFace + (y + 1) * size + x;
                            int i3 = offsetFace + (y + 1) * size + x + 1;
                            
                            if(offset==1||offset==2||offset==5) {
                                // First triangle
                                m_index.push_back(i0);
                                m_index.push_back(i1);
                                m_index.push_back(i2);
                    
                                // Second triangle
                                m_index.push_back(i1);
                                m_index.push_back(i3);
                                m_index.push_back(i2);
                            }
                            else {
                                // First triangle
                                m_index.push_back(i0);
                                m_index.push_back(i2);
                                m_index.push_back(i1);
                    
                                // Second triangle
                                m_index.push_back(i1);
                                m_index.push_back(i2);
                                m_index.push_back(i3);
    
                            }
                        }
                    }
                }
            }

    };

}
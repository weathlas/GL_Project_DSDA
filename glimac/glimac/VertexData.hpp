#pragma once

#include <vector>

#include <glimac/BasicProgram.hpp>
#include <glimac/Geometry.hpp>

#include "common.hpp"
#include "glm.hpp"

using namespace glm;

namespace glimac {

    const GLuint VERTEX_ATTR_POSITION = 0;
    const GLuint VERTEX_ATTR_NORMAL = 1;
    const GLuint VERTEX_ATTR_COORDS = 2;

    enum VertexDataType {CANONICAL, GEOMETRY};

    class VertexData {
        public:
            const VertexDataType type;

            VertexData(const glimac::Geometry &geometry) : type(GEOMETRY) {
                m_vertexCount = geometry.getVertexCount();
                m_vbo = vboInit(geometry);
                std::cout << "vboInit ok " << glGetError() << std::endl;
                m_vao = vaoInit(m_vbo);
                std::cout << "vaoInit ok " << glGetError() << std::endl;
                m_elementBuffer = elementBufferInit(geometry);
                std::cout << "elementBufferInit ok " << glGetError() << std::endl;
            };
            VertexData(size_t vertexCount, const glimac::ShapeVertex * dataPointer) : type(CANONICAL) {
                m_vertexCount = vertexCount;
                m_vbo = vboInit(vertexCount, dataPointer);
                m_vao = vaoInit(m_vbo);
                m_elementBuffer=0;
            };

            ~VertexData() {
                glDeleteBuffers(1, &m_vbo);
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

        private:
            GLuint m_vao;
            GLuint m_vbo;
            GLuint m_elementBuffer;
            size_t m_vertexCount;

            GLuint elementBufferInit(const glimac::Geometry &geometry) {
                GLuint elementBuffer;
                glGenBuffers(1, &elementBuffer);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, geometry.getIndexCount() * sizeof(unsigned int), geometry.getIndexBuffer(), GL_STATIC_DRAW);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

                return elementBuffer;
            }

            GLuint vboInit(const glimac::Geometry &geometry) {
                GLuint vbo; // Vertex Buffer Object
                glGenBuffers(1, &vbo);
                glBindBuffer(GL_ARRAY_BUFFER, vbo);
                glBufferData(GL_ARRAY_BUFFER, geometry.getVertexCount() * sizeof(glimac::Geometry::Vertex), geometry.getVertexBuffer(), GL_STATIC_DRAW);
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

            GLuint vaoInit(GLuint vbo) {
                GLuint vao; // Vertex Array Object
                glGenVertexArrays(1, &vao);
                glBindVertexArray(vao);

                glEnableVertexAttribArray(VERTEX_ATTR_POSITION);
                glEnableVertexAttribArray(VERTEX_ATTR_NORMAL);
                glEnableVertexAttribArray(VERTEX_ATTR_COORDS);

                glBindBuffer(GL_ARRAY_BUFFER, vbo);
                glVertexAttribPointer(VERTEX_ATTR_POSITION, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), 0);
                glVertexAttribPointer(VERTEX_ATTR_NORMAL, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (const GLvoid*)(3 * sizeof(GL_FLOAT)));
                glVertexAttribPointer(VERTEX_ATTR_COORDS, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (const GLvoid*)(6 * sizeof(GL_FLOAT)));
                glBindBuffer(GL_ARRAY_BUFFER, 0);

                glBindVertexArray(0);

                return vao;
            }
    };

}
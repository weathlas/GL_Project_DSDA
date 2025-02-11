#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <vector>

#include <glimac/Particule.hpp>
#include <glimac/Link.hpp>
#include <glimac/Instance.hpp>

#include "common.hpp"
#include "glm.hpp"

using namespace glm;

namespace glimac {

    enum AnimType {
        none,
        rope,
        grid
    };

    class Animation {

        const float particuleSize = 0.05;

        public:

            Animation(glimac::FilePath root, std::string name, GLuint baseTex, GLuint alternateTex, GLuint normalTex) {
                m_instance = std::make_shared<Instance>(root, name, baseTex, alternateTex, normalTex);
                m_type = AnimType::none;
            }

            Animation(size_t vertexCount, const glimac::ShapeVertex * dataPointer, GLuint baseTex, GLuint alternateTex, GLuint normalTex) {
                m_instance = std::make_shared<Instance>(vertexCount, dataPointer, baseTex, alternateTex, normalTex);
                m_type = AnimType::none;
            }

            ~Animation() {}

            std::shared_ptr<Instance> getInstance() {
                return m_instance;
            }

            void make_rope(vec3 p1, vec3 p2, uint count, float mass, float k, float z) {
                if(m_type != AnimType::none) {
                    std::cout << "Anim is already set (" << m_type << ")" << std::endl;
                    return;
                }
                m_type = AnimType::rope;
                // m_instance.get()->add(Transform(p1, vec3(), vec3(1)));
                // m_instance.get()->add(Transform(p2, vec3(), vec3(1)));
                auto diff = p2 - p1;
                auto length_segment = length(diff * (1.0f/(count-1)));
                m_instance.get()->add(Transform(p1, vec3(), vec3(particuleSize)));
                m_particules.push_back(Particule(mass, p1, ParticuleComputeType::fixed));
                std::cout << "particule at " << p1 << std::endl;
                for (uint i = 1; i < count-1; i++) {
                    auto pos = p1 + diff * (1.0f*i/(count-1));
                    m_instance.get()->add(Transform(pos, vec3(), vec3(particuleSize)));
                    m_particules.push_back(Particule(mass, pos, ParticuleComputeType::leapfrog));
                    std::cout << "particule at " << pos << std::endl;
                }
                m_instance.get()->add(Transform(p2, vec3(), vec3(particuleSize)));
                m_particules.push_back(Particule(mass, p2, ParticuleComputeType::fixed));
                std::cout << "particule at " << p2 << std::endl;

                for (uint i = 1; i < count; i++) {
                    m_links.push_back(Link(&m_particules.at(i-1), &m_particules.at(i), LinkType::damped_hook, length_segment, k, z, 0));
                }
                
            }

            void make_grid(vec3 p1, vec3 p2, vec3 p3, vec3 p4, uint count, float mass, float k, float z) {
                if(m_type != AnimType::none) {
                    std::cout << "Anim is already set (" << m_type << ")" << std::endl;
                    return;
                }
                m_type = AnimType::grid;
                auto diff12 = p2 - p1;
                auto diff13 = p3 - p1;
                auto diff34 = p4 - p3;
                auto diff24 = p4 - p2;
                // auto offset12 = diff12 * (1.0f/count);
                // auto offset13 = diff13 * (1.0f/count);
                // auto offset34 = diff34 * (1.0f/count);
                // auto offset24 = diff24 * (1.0f/count);

                for (float Y = 0; Y < count; Y++) {
                    for (float X = 0; X < count; X++) {
                        auto axisX = diff12 * (1.0f/count) * ((count-Y)/count) + diff34 * (1.0f/count) * (Y/count);
                        auto axisY = diff13 * (1.0f/count) * ((count-X)/count) + diff24 * (1.0f/count) * (X/count);
                        // auto pos = p1 + diff12 * X * (1.0f/count) + diff34 * (count - X) * (1.0f/count) + diff13 * Y * (1.0f/count) + diff24 * (count - Y) * (1.0f/count);
                        auto pos = p1 + axisX * X + axisY * Y;

                        auto type = ParticuleComputeType::leapfrog;
                        if((X == 0 && Y == 0) || (X == count-1 && Y == 0) || (X == 0 && Y == count-1) || (X == count-1 && Y == count-1)) {
                            type = ParticuleComputeType::fixed;
                        }

                        m_instance.get()->add(Transform(pos, vec3(), vec3(particuleSize)));
                        m_particules.push_back(Particule(mass, pos, type));
                    }
                }

                for (uint Y = 0; Y < count; Y++) {
                    for (uint X = 0; X < count; X++) {
                        if(X > 0) {
                            auto length_segmentX = length(m_particules.at(X-1 + count*Y).m_pos - m_particules.at(X+count*Y).m_pos);
                            m_links.push_back(Link(&m_particules.at(X-1 + count*Y), &m_particules.at(X+count*Y), LinkType::damped_hook, length_segmentX, k, z, 0));
                        }
                        if(Y > 0) {
                            auto length_segmentY = length(m_particules.at(X+count*(Y - 1)).m_pos - m_particules.at(X+count*Y).m_pos);
                            m_links.push_back(Link(&m_particules.at(X+count*(Y - 1)), &m_particules.at(X+count*Y), LinkType::damped_hook, length_segmentY, k, z, 0));
                        }
                    }
                }
                
            }

            void update(float h) {
                for (auto link: m_links) {
                    link.update();
                }
                for(uint index = 0; index < m_particules.size(); index++) {
                    m_particules.at(index).m_forces_acc += vec3(0, -0.25, 0);
                    m_particules.at(index).update(h);
                    m_instance.get()->updatePosition(index, m_particules.at(index).m_pos);
                    m_instance.get()->compute(index);
                }
            }

            void setPos(vec3 pos) {
                m_particules.back().m_pos = pos;
            }
            
        private:
            AnimType m_type;
            std::shared_ptr<Instance> m_instance;
            std::vector<Particule> m_particules;
            std::vector<Link> m_links;
    };
}
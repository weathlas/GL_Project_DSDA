#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <vector>

#include <glimac/Particule.hpp>
#include <glimac/Link.hpp>
#include <glimac/Field.hpp>
#include <glimac/Instance.hpp>

#include "common.hpp"
#include "glm.hpp"

using namespace glm;

namespace glimac {

    enum AnimType {
        none,
        rope,
        grid,
        cube
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
                // m_count_dimension = count;
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
                    if(i==1) {
                        m_particules.push_back(Particule(mass, pos, ParticuleComputeType::fixed));    
                    }
                    else {
                        m_particules.push_back(Particule(mass, pos, ParticuleComputeType::leapfrog));
                    }
                    std::cout << "particule at " << pos << std::endl;
                }
                m_instance.get()->add(Transform(p2, vec3(), vec3(particuleSize)));
                // m_particules.push_back(Particule(mass, p2, ParticuleComputeType::fixed));
                m_particules.push_back(Particule(mass, p2, ParticuleComputeType::leapfrog));
                std::cout << "particule at " << p2 << std::endl;

                for (uint i = 1; i < count; i++) {
                    m_links.push_back(Link(&m_particules.at(i-1), &m_particules.at(i), LinkType::damped_hook, length_segment, k, z, 0));
                }
                for (uint i = 2; i < count; i++) {
                    m_links.push_back(Link(&m_particules.at(i-2), &m_particules.at(i), LinkType::damped_hook, length_segment*2, k*3, z*0.1, 0));
                }
                
            }

            void make_grid(vec3 p1, vec3 p2, vec3 p3, vec3 p4, uint count, float mass, float k, float z) {
                if(m_type != AnimType::none) {
                    std::cout << "Anim is already set (" << m_type << ")" << std::endl;
                    return;
                }
                m_type = AnimType::grid;
                // m_count_dimension = count;
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
                        if(/*(X == 0 && Y == 0) ||*/ (X == count-1 && Y == 0) /*|| (X == 0 && Y == count-1)*/ || (X == count-1 && Y == count-1)) {
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

                        if(X > 1) {
                            auto length_segmentX = length(m_particules.at(X-2 + count*Y).m_pos - m_particules.at(X+count*Y).m_pos);
                            m_links.push_back(Link(&m_particules.at(X-2 + count*Y), &m_particules.at(X+count*Y), LinkType::damped_hook, length_segmentX, k, z*0.0, 0));
                        }
                        if(Y > 1) {
                            auto length_segmentY = length(m_particules.at(X+count*(Y - 2)).m_pos - m_particules.at(X+count*Y).m_pos);
                            m_links.push_back(Link(&m_particules.at(X+count*(Y - 2)), &m_particules.at(X+count*Y), LinkType::damped_hook, length_segmentY, k, z*0.0, 0));
                        }
                        
                        if(X > 0 && Y > 0) {
                            auto length_segment = length(m_particules.at((X-1) + count*(Y-1)).m_pos - m_particules.at(X+count*Y).m_pos);
                            m_links.push_back(Link(&m_particules.at((X-1) + count*(Y-1)), &m_particules.at(X+count*Y), LinkType::damped_hook, length_segment, k, z*0.0, 0));
                        }
                        if(X <count && Y > 0) {
                            auto length_segment = length(m_particules.at((X+1) + count*(Y-1)).m_pos - m_particules.at(X+count*Y).m_pos);
                            m_links.push_back(Link(&m_particules.at((X+1) + count*(Y-1)), &m_particules.at(X+count*Y), LinkType::damped_hook, length_segment, k, z*0.0, 0));
                        }
                    }
                }
                
            }

            void make_cube(vec3 center, vec3 dimensions, uint count, float mass, float k, float z) {
                if(m_type != AnimType::none) {
                    std::cout << "Anim is already set (" << m_type << ")" << std::endl;
                    return;
                }
                m_type = AnimType::cube;
                // m_count_dimension = count;
                // m_dimensions = dimensions;
                // m_center = center;

                vec3 offsets = dimensions * (1.0f/(count-1));
                vec3 origin = center - dimensions * 0.5f;

                for (float Z = 0; Z < count; Z++) {
                    for (float Y = 0; Y < count; Y++) {
                        for (float X = 0; X < count; X++) {
                            auto pos = origin + vec3(X*offsets.x, Y*offsets.y, Z*offsets.z);

                            auto type = ParticuleComputeType::leapfrog;
                            // if(X==0 && Y==0 && Z==0) {
                            //     type = ParticuleComputeType::fixed;
                            // }
                            // if(X == 0 || X == count-1) {
                            //     if(Y == 0 /*|| Y == count-1*/) {
                            //         if(Z == 0 || Z == count-1) {
                            //             type = ParticuleComputeType::fixed;
                            //         }
                            //     }
                            // }
                            // if((X == 0 && Y == 0 && Z == 0) || (X == count-1 && Y == 0 && Z == 0) || (X == 0 && Y == count-1 && Z == 0) || (X == count-1 && Y == count-1 && Z == 0) || (X == 0 && Y == 0 && Z == count-1) || (X == count-1 && Y == 0 && Z == count-1) || (X == 0 && Y == count-1 && Z == count-1) || (X == count-1 && Y == count-1 && Z == count-1)) {
                            //     type = ParticuleComputeType::fixed;
                            // }

                            std::cout << "Pos " << pos << " type " << type << std::endl;

                            m_instance.get()->add(Transform(pos, vec3(), vec3(particuleSize)));
                            m_particules.push_back(Particule(mass, pos, vec3(-0.645, 0.12, 0.35), type));

                            // m_particules.back().m_speed = ;
                        }
                    }
                }

                if(true) {
                    for (uint Z = 0; Z < count; Z++) {
                        for (uint Y = 0; Y < count; Y++) {
                            for (uint X = 0; X < count; X++) {
                                if(X > 0) {
                                    auto length_segmentX = length(m_particules.at(X-1 + count*Y + count*count*Z).m_pos - m_particules.at(X+count*Y + count*count*Z).m_pos);
                                    m_links.push_back(Link(&m_particules.at(X-1 + count*Y + count*count*Z), &m_particules.at(X+count*Y + count*count*Z), LinkType::damped_hook, length_segmentX, k, z, 0));
                                }
                                if(Y > 0) {
                                    auto length_segmentY = length(m_particules.at(X+count*(Y - 1) + count*count*Z).m_pos - m_particules.at(X+count*Y + count*count*Z).m_pos);
                                    m_links.push_back(Link(&m_particules.at(X+count*(Y - 1) + count*count*Z), &m_particules.at(X+count*Y + count*count*Z), LinkType::damped_hook, length_segmentY, k, z, 0));
                                }
                                if(Z > 0) {
                                    auto length_segmentZ = length(m_particules.at(X+count*Y + count*count*(Z-1)).m_pos - m_particules.at(X+count*Y + count*count*Z).m_pos);
                                    m_links.push_back(Link(&m_particules.at(X+count*Y + count*count*(Z-1)), &m_particules.at(X+count*Y + count*count*Z), LinkType::damped_hook, length_segmentZ, k, z, 0));
                                }

                                if(X > 1) {
                                    auto length_segmentX = length(m_particules.at(X-2 + count*Y + count*count*Z).m_pos - m_particules.at(X+count*Y + count*count*Z).m_pos);
                                    m_links.push_back(Link(&m_particules.at(X-2 + count*Y + count*count*Z), &m_particules.at(X+count*Y + count*count*Z), LinkType::damped_hook, length_segmentX, k, z*0.0, 0));
                                }
                                if(Y > 1) {
                                    auto length_segmentY = length(m_particules.at(X+count*(Y - 2) + count*count*Z).m_pos - m_particules.at(X+count*Y + count*count*Z).m_pos);
                                    m_links.push_back(Link(&m_particules.at(X+count*(Y - 2) + count*count*Z), &m_particules.at(X+count*Y + count*count*Z), LinkType::damped_hook, length_segmentY, k, z*0.0, 0));
                                }
                                if(Z > 1) {
                                    auto length_segmentZ = length(m_particules.at(X+count*Y + count*count*(Z-2)).m_pos - m_particules.at(X+count*Y + count*count*Z).m_pos);
                                    m_links.push_back(Link(&m_particules.at(X+count*Y + count*count*(Z-2)), &m_particules.at(X+count*Y + count*count*Z), LinkType::damped_hook, length_segmentZ, k, z*0.0, 0));
                                }

                                if(X > 0 && Y > 0) {
                                    auto length_segment = length(m_particules.at((X-1) + count*(Y-1) + count*count*(Z)).m_pos - m_particules.at(X+count*Y + count*count*Z).m_pos);
                                    m_links.push_back(Link(&m_particules.at((X-1) + count*(Y-1) + count*count*(Z)), &m_particules.at(X+count*Y + count*count*Z), LinkType::damped_hook, length_segment, k, z*0.0, 0));
                                }
                                if(X > 0 && Z > 0) {
                                    auto length_segment = length(m_particules.at((X-1) + count*(Y) + count*count*(Z-1)).m_pos - m_particules.at(X+count*Y + count*count*Z).m_pos);
                                    m_links.push_back(Link(&m_particules.at((X-1) + count*(Y) + count*count*(Z-1)), &m_particules.at(X+count*Y + count*count*Z), LinkType::damped_hook, length_segment, k, z*0.0, 0));
                                }
                                if(Y > 0 && Z > 0) {
                                    auto length_segment = length(m_particules.at((X) + count*(Y-1) + count*count*(Z-1)).m_pos - m_particules.at(X+count*Y + count*count*Z).m_pos);
                                    m_links.push_back(Link(&m_particules.at((X) + count*(Y-1) + count*count*(Z-1)), &m_particules.at(X+count*Y + count*count*Z), LinkType::damped_hook, length_segment, k, z*0.0, 0));
                                }

                                if(X <count && Y > 0) {
                                    auto length_segment = length(m_particules.at((X+1) + count*(Y-1) + count*count*(Z)).m_pos - m_particules.at(X+count*Y + count*count*Z).m_pos);
                                    m_links.push_back(Link(&m_particules.at((X+1) + count*(Y-1) + count*count*(Z)), &m_particules.at(X+count*Y + count*count*Z), LinkType::damped_hook, length_segment, k, z*0.0, 0));
                                }
                                if(X <count && Z > 0) {
                                    auto length_segment = length(m_particules.at((X+1) + count*(Y) + count*count*(Z-1)).m_pos - m_particules.at(X+count*Y + count*count*Z).m_pos);
                                    m_links.push_back(Link(&m_particules.at((X+1) + count*(Y) + count*count*(Z-1)), &m_particules.at(X+count*Y + count*count*Z), LinkType::damped_hook, length_segment, k, z*0.0, 0));
                                }
                                if(Y <count && Z > 0) {
                                    auto length_segment = length(m_particules.at((X) + count*(Y+1) + count*count*(Z-1)).m_pos - m_particules.at(X+count*Y + count*count*Z).m_pos);
                                    m_links.push_back(Link(&m_particules.at((X) + count*(Y+1) + count*count*(Z-1)), &m_particules.at(X+count*Y + count*count*Z), LinkType::damped_hook, length_segment, k, z*0.0, 0));
                                }
                            }
                        }
                    }
                }
            }

            void reset() {

                for(uint index = 0; index < m_particules.size(); index++) {
                    m_particules.at(index).reset();
                }

                // switch (m_type)
                // {
                // case AnimType::cube:
                //     reset_cube();
                //     break;
                // default:
                //     break;
                // }
            }

            void addField(FieldType type, vec3 coords, float k) {
                m_fields.push_back(Field());
                switch (type)
                {
                case FieldType::field_directional:
                    m_fields.back().make_directional(coords, k);
                    break;
                case FieldType::field_point:
                    m_fields.back().make_point(coords, k);
                    break;
                default:
                    break;
                }
                for(uint index = 0; index < m_particules.size(); index++) {
                    m_fields.back().connect(&m_particules.at(index));
                }
            }

            void update(float h) {
                for (auto link: m_links) {
                    link.update();
                }
                for (auto field: m_fields) {
                    field.update();
                }
                for(uint index = 0; index < m_particules.size(); index++) {
                    // m_particules.at(index).m_forces_acc += vec3(0, -0.25, 0);
                    m_particules.at(index).update(h);
                    m_instance.get()->updatePosition(index, m_particules.at(index).m_pos);
                    m_instance.get()->compute(index);
                }
                // if(m_type == AnimType::cube) {
                //     std::cout << "CUBE Pos " << m_particules.at(1).m_pos << std::endl;
                // }
            }

            void setPos(vec3 pos) {
                m_particules.back().m_pos = pos;
            }

            void setPosFirst(vec3 pos) {
                m_particules.front().m_pos = pos;
            }

            void setTypeFirst(ParticuleComputeType type) {
                m_particules.front().setType(type);
            }
            
        private:
            AnimType m_type;
            std::shared_ptr<Instance> m_instance;
            std::vector<Particule> m_particules;
            std::vector<Link> m_links;
            std::vector<Field> m_fields;

            // int m_count_dimension;
            // vec3 m_dimensions;
            // vec3 m_center;

            // void reset_cube() {
            //     vec3 offsets = m_dimensions * (1.0f/(m_count_dimension-1));
            //     vec3 origin = m_center - m_dimensions * 0.5f;
            //     int index = 0;
            //     for (float Z = 0; Z < m_count_dimension; Z++) {
            //         for (float Y = 0; Y < m_count_dimension; Y++) {
            //             for (float X = 0; X < m_count_dimension; X++) {
            //                 auto pos = origin + vec3(X*offsets.x, Y*offsets.y, Z*offsets.z);

                            
            //                 m_particules.at(index).m_pos = pos;
            //                 m_particules.at(index).m_speed = vec3(0);
            //                 // push_back(Particule(mass, pos, type));
            //                 index++;
            //             }
            //         }
            //     }
            // }

    };
}
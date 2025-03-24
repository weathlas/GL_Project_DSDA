#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <vector>

#include <glimac/BBox.hpp>

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
        point,
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

            ~Animation() {
                auto size = m_particules.size();
                for (auto* p: m_particules) {
                    delete p;
                }
                m_instance.get()->~Instance();
                std::cout << "Deleted " << size << " particules." << std::endl;
            }

            std::shared_ptr<Instance> getInstance() {
                return m_instance;
            }

            void make_point(vec3 p, float mass) {
                if(m_type != AnimType::none) {
                    std::cout << "Anim is already set (" << m_type << ")" << std::endl;
                    return;
                }
                m_type = AnimType::point;
                m_instance.get()->add(Transform(p, vec3(), vec3(particuleSize)));
                m_particules.push_back(new Particule(mass, p, ParticuleComputeType::leapfrog));
                std::cout << "New Point Anim with " << m_particules.size() << " particule and " << m_links.size() << " links" << std::endl;
            }

            void make_rope(vec3 p1, vec3 p2, uint count, float mass, float k, float z) {
                if(m_type != AnimType::none) {
                    std::cout << "Anim is already set (" << m_type << ")" << std::endl;
                    return;
                }
                m_type = AnimType::rope;
                m_count_dimension = count;
                // m_instance.get()->add(Transform(p1, vec3(), vec3(1)));
                // m_instance.get()->add(Transform(p2, vec3(), vec3(1)));
                auto diff = p2 - p1;
                auto length_segment = length(diff * (1.0f/(count-1)));
                m_instance.get()->add(Transform(p1, vec3(), vec3(particuleSize)));
                m_particules.push_back(new Particule(mass, p1, ParticuleComputeType::fixed));
                for (uint i = 1; i < count-1; i++) {
                    auto pos = p1 + diff * (1.0f*i/(count-1));
                    m_instance.get()->add(Transform(pos, vec3(), vec3(particuleSize)));
                    if(i==1) {
                        m_particules.push_back(new Particule(mass, pos+vec3(0, 2.0, 0), ParticuleComputeType::leapfrog));    
                    }
                    else {
                        m_particules.push_back(new Particule(mass, pos, ParticuleComputeType::leapfrog));
                    }
                }
                m_instance.get()->add(Transform(p2, vec3(), vec3(particuleSize)));
                m_particules.push_back(new Particule(mass, p2, ParticuleComputeType::fixed));
                // m_particules.push_back(new Particule(mass, p2, ParticuleComputeType::leapfrog));

                for (uint i = 1; i < count; i++) {
                    m_links.push_back(Link(m_particules.at(i-1), m_particules.at(i), LinkType::damped_hook, length_segment, k, z, 0));
                }
                // for (uint i = 2; i < count; i++) {
                //     m_links.push_back(Link(m_particules.at(i-2), m_particules.at(i), LinkType::damped_hook, length_segment*2, k*3, z*0.1, 0));
                // }
                std::cout << "New Rope Anim with " << m_particules.size() << " particule and " << m_links.size() << " links" << std::endl;
            }

            void make_grid(vec3 p1, vec3 p2, vec3 p3, vec3 p4, uint count, float mass, float k, float z) {
                if(m_type != AnimType::none) {
                    std::cout << "Anim is already set (" << m_type << ")" << std::endl;
                    return;
                }
                m_type = AnimType::grid;
                m_count_dimension = count;
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
                        // if(/*(X == 0 && Y == 0) ||*/ (X == count-1 && Y == 0) /*|| (X == 0 && Y == count-1)*/ || (X == count-1 && Y == count-1)) {
                        // if(X == count-1) {
                        //     type = ParticuleComputeType::fixed;
                        // }

                        // WIREFRAME IS STATIC
                        // if(X == 0 || X == count-1 || Y == 0 || Y == count-1) {
                        //         type = ParticuleComputeType::fixed;
                        // }

                        // std::cerr << "GRID: ADDING Particule " << m_particules.size() << std::endl;
                        m_instance.get()->add(Transform(pos, vec3(), vec3(particuleSize)));
                        m_particules.push_back(new Particule(mass, pos, type));
                        // std::cerr << "GRID: Particule Added" << std::endl;
                    }
                }

                for (uint Y = 0; Y < count; Y++) {
                    for (uint X = 0; X < count; X++) {
                        if(X > 0) {
                            auto length_segmentX = length(m_particules.at(X-1 + count*Y)->m_pos - m_particules.at(X+count*Y)->m_pos);
                            m_links.push_back(Link(m_particules.at(X-1 + count*Y), m_particules.at(X+count*Y), LinkType::damped_hook, length_segmentX, k, z, 0));
                        }
                        if(Y > 0) {
                            auto length_segmentY = length(m_particules.at(X+count*(Y - 1))->m_pos - m_particules.at(X+count*Y)->m_pos);
                            m_links.push_back(Link(m_particules.at(X+count*(Y - 1)), m_particules.at(X+count*Y), LinkType::damped_hook, length_segmentY, k, z, 0));
                        }

                        if(X > 1) {
                            auto length_segmentX = length(m_particules.at(X-2 + count*Y)->m_pos - m_particules.at(X+count*Y)->m_pos);
                            m_links.push_back(Link(m_particules.at(X-2 + count*Y), m_particules.at(X+count*Y), LinkType::hook_spring, length_segmentX, k, z, 0));
                        }
                        if(Y > 1) {
                            auto length_segmentY = length(m_particules.at(X+count*(Y - 2))->m_pos - m_particules.at(X+count*Y)->m_pos);
                            m_links.push_back(Link(m_particules.at(X+count*(Y - 2)), m_particules.at(X+count*Y), LinkType::hook_spring, length_segmentY, k, z, 0));
                        }

                        // if(X > 2) {
                        //     auto length_segmentX = length(m_particules.at(X-3 + count*Y)->m_pos - m_particules.at(X+count*Y)->m_pos);
                        //     m_links.push_back(Link(m_particules.at(X-3 + count*Y), m_particules.at(X+count*Y), LinkType::hook_spring, length_segmentX, k, z, 0));
                        // }
                        // if(Y > 2) {
                        //     auto length_segmentY = length(m_particules.at(X+count*(Y - 3))->m_pos - m_particules.at(X+count*Y)->m_pos);
                        //     m_links.push_back(Link(m_particules.at(X+count*(Y - 3)), m_particules.at(X+count*Y), LinkType::hook_spring, length_segmentY, k, z, 0));
                        // }

                        // if(X > 3) {
                        //     auto length_segmentX = length(m_particules.at(X-4 + count*Y)->m_pos - m_particules.at(X+count*Y)->m_pos);
                        //     m_links.push_back(Link(m_particules.at(X-4 + count*Y), m_particules.at(X+count*Y), LinkType::hook_spring, length_segmentX, k, z, 0));
                        // }
                        // if(Y > 3) {
                        //     auto length_segmentY = length(m_particules.at(X+count*(Y - 4))->m_pos - m_particules.at(X+count*Y)->m_pos);
                        //     m_links.push_back(Link(m_particules.at(X+count*(Y - 4)), m_particules.at(X+count*Y), LinkType::hook_spring, length_segmentY, k, z, 0));
                        // }
                        
                        if(X > 0 && Y > 0) {
                            auto length_segment = length(m_particules.at((X-1) + count*(Y-1))->m_pos - m_particules.at(X+count*Y)->m_pos);
                            m_links.push_back(Link(m_particules.at((X-1) + count*(Y-1)), m_particules.at(X+count*Y), LinkType::hook_spring, length_segment, k, z, 0));
                        }
                        if(X <count && Y > 0) {
                            auto length_segment = length(m_particules.at((X+1) + count*(Y-1))->m_pos - m_particules.at(X+count*Y)->m_pos);
                            m_links.push_back(Link(m_particules.at((X+1) + count*(Y-1)), m_particules.at(X+count*Y), LinkType::hook_spring, length_segment, k, z, 0));
                        }
                    }
                }
                std::cout << "New Grid Anim with " << m_particules.size() << " particule and " << m_links.size() << " links" << std::endl;
            }

            void make_cube(vec3 center, vec3 dimensions, uint count, float mass, float k, float z) {
                if(m_type != AnimType::none) {
                    std::cout << "Anim is already set (" << m_type << ")" << std::endl;
                    return;
                }
                m_type = AnimType::cube;
                m_count_dimension = count;
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

                            // std::cout << "Pos " << pos << " type " << type << std::endl;

                            if(X == 0 || X == count-1 || Y == 0 || Y == count-1 || Z == 0 || Z == count-1) {
                                m_instance.get()->add(Transform(pos, vec3(), vec3(particuleSize)));
                                m_indexes.push_back(m_particules.size());
                            }

                            // m_particules.push_back(new Particule(mass, pos, vec3(-0.645, 0.12, 0.35), type));
                            m_particules.push_back(new Particule(mass, pos, vec3(0), type));

                            // m_particules.back().m_speed = ;
                        }
                    }
                }

                if(true) {
                    for (uint Z = 0; Z < count; Z++) {
                        for (uint Y = 0; Y < count; Y++) {
                            for (uint X = 0; X < count; X++) {

                                // GRID LINKS
                                if(true) {
                                    if(X > 0) {
                                        auto length_segmentX = length(m_particules.at(X-1 + count*Y + count*count*Z)->m_pos - m_particules.at(X+count*Y + count*count*Z)->m_pos);
                                        m_links.push_back(Link(m_particules.at(X-1 + count*Y + count*count*Z), m_particules.at(X+count*Y + count*count*Z), LinkType::damped_hook, length_segmentX, k, z, 0));
                                    }
                                    if(Y > 0) {
                                        auto length_segmentY = length(m_particules.at(X+count*(Y - 1) + count*count*Z)->m_pos - m_particules.at(X+count*Y + count*count*Z)->m_pos);
                                        m_links.push_back(Link(m_particules.at(X+count*(Y - 1) + count*count*Z), m_particules.at(X+count*Y + count*count*Z), LinkType::damped_hook, length_segmentY, k, z, 0));
                                    }
                                    if(Z > 0) {
                                        auto length_segmentZ = length(m_particules.at(X+count*Y + count*count*(Z-1))->m_pos - m_particules.at(X+count*Y + count*count*Z)->m_pos);
                                        m_links.push_back(Link(m_particules.at(X+count*Y + count*count*(Z-1)), m_particules.at(X+count*Y + count*count*Z), LinkType::damped_hook, length_segmentZ, k, z, 0));
                                    }
                                }

                                // DOUBLE LINKS
                                if(true) {
                                    if(X > 1) {
                                        auto length_segmentX = length(m_particules.at(X-2 + count*Y + count*count*Z)->m_pos - m_particules.at(X+count*Y + count*count*Z)->m_pos);
                                        m_links.push_back(Link(m_particules.at(X-2 + count*Y + count*count*Z), m_particules.at(X+count*Y + count*count*Z), LinkType::hook_spring, length_segmentX, k*2, 0, 0));
                                    }
                                    if(Y > 1) {
                                        auto length_segmentY = length(m_particules.at(X+count*(Y - 2) + count*count*Z)->m_pos - m_particules.at(X+count*Y + count*count*Z)->m_pos);
                                        m_links.push_back(Link(m_particules.at(X+count*(Y - 2) + count*count*Z), m_particules.at(X+count*Y + count*count*Z), LinkType::hook_spring, length_segmentY, k*2, 0, 0));
                                    }
                                    if(Z > 1) {
                                        auto length_segmentZ = length(m_particules.at(X+count*Y + count*count*(Z-2))->m_pos - m_particules.at(X+count*Y + count*count*Z)->m_pos);
                                        m_links.push_back(Link(m_particules.at(X+count*Y + count*count*(Z-2)), m_particules.at(X+count*Y + count*count*Z), LinkType::hook_spring, length_segmentZ, k*2, 0, 0));
                                    }
                                }

                                // if(X > 2) {
                                //     auto length_segmentX = length(m_particules.at(X-3 + count*Y + count*count*Z)->m_pos - m_particules.at(X+count*Y + count*count*Z)->m_pos);
                                //     m_links.push_back(Link(m_particules.at(X-3 + count*Y + count*count*Z), m_particules.at(X+count*Y + count*count*Z), LinkType::damped_hook, length_segmentX, k*3, z, 0));
                                // }
                                // if(Y > 2) {
                                //     auto length_segmentY = length(m_particules.at(X+count*(Y - 3) + count*count*Z)->m_pos - m_particules.at(X+count*Y + count*count*Z)->m_pos);
                                //     m_links.push_back(Link(m_particules.at(X+count*(Y - 3) + count*count*Z), m_particules.at(X+count*Y + count*count*Z), LinkType::damped_hook, length_segmentY, k*3, z, 0));
                                // }
                                // if(Z > 2) {
                                //     auto length_segmentZ = length(m_particules.at(X+count*Y + count*count*(Z-3))->m_pos - m_particules.at(X+count*Y + count*count*Z)->m_pos);
                                //     m_links.push_back(Link(m_particules.at(X+count*Y + count*count*(Z-3)), m_particules.at(X+count*Y + count*count*Z), LinkType::damped_hook, length_segmentZ, k*3, z, 0));
                                // }

                                // DIAGONAL LINKS
                                if(true) {
                                    if(X > 0 && Y > 0) {
                                        auto length_segment = length(m_particules.at((X-1) + count*(Y-1) + count*count*(Z))->m_pos - m_particules.at(X+count*Y + count*count*Z)->m_pos);
                                        m_links.push_back(Link(m_particules.at((X-1) + count*(Y-1) + count*count*(Z)), m_particules.at(X+count*Y + count*count*Z), LinkType::hook_spring, length_segment, k*1.4, 0, 0));
                                    }
                                    if(X > 0 && Z > 0) {
                                        auto length_segment = length(m_particules.at((X-1) + count*(Y) + count*count*(Z-1))->m_pos - m_particules.at(X+count*Y + count*count*Z)->m_pos);
                                        m_links.push_back(Link(m_particules.at((X-1) + count*(Y) + count*count*(Z-1)), m_particules.at(X+count*Y + count*count*Z), LinkType::hook_spring, length_segment, k*1.4, 0, 0));
                                    }
                                    if(Y > 0 && Z > 0) {
                                        auto length_segment = length(m_particules.at((X) + count*(Y-1) + count*count*(Z-1))->m_pos - m_particules.at(X+count*Y + count*count*Z)->m_pos);
                                        m_links.push_back(Link(m_particules.at((X) + count*(Y-1) + count*count*(Z-1)), m_particules.at(X+count*Y + count*count*Z), LinkType::hook_spring, length_segment, k*1.4, 0, 0));
                                    }

                                    if(X <count-1 && Y > 0) {
                                        auto length_segment = length(m_particules.at((X+1) + count*(Y-1) + count*count*(Z))->m_pos - m_particules.at(X+count*Y + count*count*Z)->m_pos);
                                        m_links.push_back(Link(m_particules.at((X+1) + count*(Y-1) + count*count*(Z)), m_particules.at(X+count*Y + count*count*Z), LinkType::hook_spring, length_segment, k*1.4, 0, 0));
                                    }
                                    if(X <count-1 && Z > 0) {
                                        auto length_segment = length(m_particules.at((X+1) + count*(Y) + count*count*(Z-1))->m_pos - m_particules.at(X+count*Y + count*count*Z)->m_pos);
                                        m_links.push_back(Link(m_particules.at((X+1) + count*(Y) + count*count*(Z-1)), m_particules.at(X+count*Y + count*count*Z), LinkType::hook_spring, length_segment, k*1.4, 0, 0));
                                    }
                                    if(Y <count-1 && Z > 0) {
                                        auto length_segment = length(m_particules.at((X) + count*(Y+1) + count*count*(Z-1))->m_pos - m_particules.at(X+count*Y + count*count*Z)->m_pos);
                                        m_links.push_back(Link(m_particules.at((X) + count*(Y+1) + count*count*(Z-1)), m_particules.at(X+count*Y + count*count*Z), LinkType::hook_spring, length_segment, k*1.4, 0, 0));
                                    }
                                }

                                //
                                if(true) {
                                    std::vector<vec3> offsets = {
                                        vec3(-1, -1, -1),
                                        vec3(-1, -1,  1),
                                        vec3(-1,  1, -1),
                                        vec3(-1,  1,  1),
                                        vec3( 1, -1, -1),
                                        vec3( 1, -1,  1),
                                        vec3( 1,  1, -1),
                                        vec3( 1,  1,  1)
                                    };
                                    for (auto offset: offsets) {
                                        auto x = offset.x;
                                        auto y = offset.y;
                                        auto z = offset.z;
                                        if(x==0 || y==0 || z==0) continue;
                                        if(X+x < 0 || X+x >= count) continue;
                                        if(Y+y < 0 || Y+y >= count) continue;
                                        if(Z+z < 0 || Z+z >= count) continue;
                                        auto length_segment = length(m_particules.at((X+x) + count*(Y+y) + count*count*(Z+z))->m_pos - m_particules.at(X+count*Y + count*count*Z)->m_pos);
                                        m_links.push_back(Link(m_particules.at((X+x) + count*(Y+y) + count*count*(Z+z)), m_particules.at(X+count*Y + count*count*Z), LinkType::hook_spring, length_segment, k*1.0, 0, 0));
                                    }
                                }
                            }
                        }
                    }
                }
                std::cout << "New Cube Anim with " << m_particules.size() << " particule and " << m_links.size() << " links" << std::endl;
            }

            void reset() {

                for(uint index = 0; index < m_particules.size(); index++) {
                    m_particules.at(index)->reset();
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

            void addField(FieldType type, rigidBody *rb, float k) {
                if(rb == nullptr) return;
                m_fields.push_back(Field());
                switch (type)
                {
                case FieldType::field_convex:
                    m_fields.back().make_convex(rb, k);
                    break;
                default:
                    break;
                }
            }

            void addField(FieldType type, BBox3f box, float k) {
                m_fields.push_back(Field());
                switch (type)
                {
                case FieldType::field_cube:
                    m_fields.back().make_cube(box, k);
                    break;
                default:
                    break;
                }
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
                case FieldType::field_fluid: // TO DO ADD fluid movement
                    m_fields.back().make_fluid(k);
                    break;
                case FieldType::field_wall:
                    m_fields.back().make_wall(coords, k);
                    break;
                case FieldType::field_cube:
                    m_fields.back().make_cube(coords, k);
                    break;
                default:
                    break;
                }
            }

            void update_links(uint start, uint end) {
                for (uint index = start; index < end; index++) {
                    m_links.at(index).update();
                }
            }

            void update_links() {update_links(0, m_links.size());}

            void update_particules(float h, uint start, uint end) {

                for(uint index = start; index < end; index++) {
                    for (auto field: m_fields) {
                        field.update(m_particules.at(index), h);
                    }
                    m_particules.at(index)->update(h);
                    // m_instance.get()->updatePosition(index, m_particules.at(index)->m_pos);
                    // m_instance.get()->compute(index);
                }

                // std::cout << "PARTICULE POSITION { " << m_particules.at(0)->m_pos.x << ", " << m_particules.at(0)->m_pos.y << ", " << m_particules.at(0)->m_pos.z << " } " << std::endl;
            }

            void update(float h) {
                update_links();
                update_particules(h, 0, m_particules.size());
            }

            void update_visual() {
                uint index = 0;
                switch (m_type)
                {
                case AnimType::cube:
                    for (auto particule_index: m_indexes) {
                        m_instance.get()->updatePosition(index, m_particules.at(particule_index)->m_pos);
                        m_instance.get()->compute(index);
                        index++;
                    }
                    // for (uint Z = 0; Z < m_count_dimension; Z++) {
                    //     for (uint Y = 0; Y < m_count_dimension; Y++) {
                    //         for (uint X = 0; X < m_count_dimension; X++) {
                    //             if(X == 0 || X == m_count_dimension-1 || Y == 0 || Y == m_count_dimension-1 || Z == 0 || Z == m_count_dimension-1) {
                    //                 m_instance.get()->updatePosition(index, m_particules.at(X+m_count_dimension*Y + m_count_dimension*m_count_dimension*Z)->m_pos);
                    //                 m_instance.get()->compute(index);
                    //                 index++;
                    //             }
                    //         }
                    //     }
                    // }
                    break;
                case AnimType::none:
                    break;
                case AnimType::point:
                case AnimType::rope:
                case AnimType::grid:
                default:
                    for(index = 0; index < m_particules.size(); index++) {
                        m_instance.get()->updatePosition(index, m_particules.at(index)->m_pos);
                        m_instance.get()->compute(index);
                    }
                    break;
                }
            }

            std::vector<Field>* getFields() {
                return &m_fields;
            }

            void setPos(vec3 pos) {
                m_particules.back()->m_pos = pos;
            }

            void setPosFirst(vec3 pos) {
                m_particules.front()->m_pos = pos;
            }

            void setTypeFirst(ParticuleComputeType type) {
                m_particules.front()->setType(type);
            }

            uint getParticulesCount() {
                return m_particules.size();
            }

            uint getLinksCount() {
                return m_links.size();
            }
            
        private:
            AnimType m_type;
            std::shared_ptr<Instance> m_instance;
            std::vector<Particule*> m_particules;
            std::vector<uint> m_indexes;
            std::vector<Link> m_links;
            std::vector<Field> m_fields;

            uint m_count_dimension;
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

                            
            //                 m_particules.at(index)->m_pos = pos;
            //                 m_particules.at(index).m_speed = vec3(0);
            //                 // push_back(Particule(mass, pos, type));
            //                 index++;
            //             }
            //         }
            //     }
            // }

    };
}
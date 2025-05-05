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

#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

using namespace glm;

namespace glimac {

    enum ThreadProcessState {
        NothingDone,
        LinksDone,
        ParticulesDone
    };
    
    enum ThreadProcessComparaison {
        Compute_1,
        Compute_2,
        Compute_3
    };

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

            Animation(GLuint baseTex, GLuint alternateTex, GLuint normalTex) {
                // m_instance = std::make_shared<Instance>(vertexCount, dataPointer, baseTex, alternateTex, normalTex);
                m_baseTex = baseTex;
                m_alternateTex = alternateTex;
                m_normalTex = normalTex;
                m_is_dynamic = true;
                m_type = AnimType::none;
            }

            ~Animation() {
                auto size = m_particules.size();
                for (auto* p: m_particules) {
                    delete p;
                }
                m_instance.get()->~Instance();
                std::cout << "Deleted " << size << " particules." << std::endl;

                stopMultithreads();
            }

            // The following allow to have the std::vector<std::thread> as a member
            Animation(const Animation&) = delete;
            Animation& operator=(const Animation&) = delete;


            Animation(Animation&& other) noexcept
                : m_computeAnim(other.m_computeAnim.load()), // atomic: load value
                m_killThreads(other.m_killThreads.load()), // atomic: load value
                m_threadList(std::move(other.m_threadList)) {}
                
            Animation& operator=(Animation&& other) noexcept {
                if (this != &other) {
                    m_computeAnim.store(other.m_computeAnim.load());
                    m_killThreads.store(other.m_killThreads.load());
                    m_threadList = std::move(other.m_threadList);
                }
                return *this;
            }
        
            // Animation(Animation&&) = default;
            // Animation& operator=(Animation&&) = default;

            std::shared_ptr<Instance> getInstance() {
                return m_instance;
            }

            BBox3f computeBB() {
                // simplified version, BB is just the extremums of all particules coordinates
                BBox3f bb;
                if(m_particules.size() == 0) {
                    return bb;
                }
                bool firstParticule = true;
                for(auto &particule: m_particules) {
                    if(firstParticule) {
                        firstParticule = false;
                        bb = BBox3f(particule->m_pos);
                        continue;
                    }
                    bb.grow(particule->m_pos);
                }
                return bb;
            }

            void make_point(vec3 p, float mass) {
                if(m_type != AnimType::none) {
                    std::cout << "Anim is already set (" << m_type << ")" << std::endl;
                    return;
                }
                m_type = AnimType::point;
                if(!m_is_dynamic)m_instance.get()->add(Transform(p, vec3(), vec3(particuleSize)));
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
                if(!m_is_dynamic)m_instance.get()->add(Transform(p1, vec3(), vec3(particuleSize)));
                m_particules.push_back(new Particule(mass, p1, ParticuleComputeType::fixed));
                for (uint i = 1; i < count-1; i++) {
                    auto pos = p1 + diff * (1.0f*i/(count-1));
                    if(!m_is_dynamic)m_instance.get()->add(Transform(pos, vec3(), vec3(particuleSize)));
                    if(i==1) {
                        m_particules.push_back(new Particule(mass, pos+vec3(0, 2.0, 0), ParticuleComputeType::leapfrog));    
                    }
                    else {
                        m_particules.push_back(new Particule(mass, pos, ParticuleComputeType::leapfrog));
                    }
                }
                if(!m_is_dynamic)m_instance.get()->add(Transform(p2, vec3(), vec3(particuleSize)));
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

                // std::cout << "START GRID INIT" << std::endl;

                for (float Y = 0; Y < count; Y++) {
                    for (float X = 0; X < count; X++) {
                        auto axisX = diff12 * (1.0f/count) * ((count-Y)/count) + diff34 * (1.0f/count) * (Y/count);
                        auto axisY = diff13 * (1.0f/count) * ((count-X)/count) + diff24 * (1.0f/count) * (X/count);
                        // auto pos = p1 + diff12 * X * (1.0f/count) + diff34 * (count - X) * (1.0f/count) + diff13 * Y * (1.0f/count) + diff24 * (count - Y) * (1.0f/count);
                        auto pos = p1 + axisX * X + axisY * Y;

                        auto type = ParticuleComputeType::leapfrog;
                        // if(/*(X == 0 && Y == 0) ||*/ (X == count-1 && Y == 0) /*|| (X == 0 && Y == count-1)*/ || (X == count-1 && Y == count-1)) {
                        // if(Y == 0 && (X==0 || X == count-1)) {
                        //     type = ParticuleComputeType::fixed;
                        // }

                        // WIREFRAME IS STATIC
                        // if(X == 0 || X == count-1 || Y == 0 || Y == count-1) {
                        //         type = ParticuleComputeType::fixed;
                        // }

                        // std::cerr << "GRID: ADDING Particule " << m_particules.size() << std::endl;
                        if(!m_is_dynamic)m_instance.get()->add(Transform(pos, vec3(), vec3(particuleSize)));
                        m_particules.push_back(new Particule(mass, pos, type));
                        // std::cerr << "GRID: Particule Added" << std::endl;
                    }
                }

                for (uint Y = 0; Y < count; Y++) {
                    for (uint X = 0; X < count; X++) {
                        if(X > 0) {
                            auto length_segmentX = length(m_particules.at(X-1 + count*Y)->m_pos - m_particules.at(X+count*Y)->m_pos);
                            m_links.push_back(Link(m_particules.at(X-1 + count*Y), m_particules.at(X+count*Y), LinkType::damped_hook, length_segmentX, k*1.75, z, 0));
                        }
                        if(Y > 0) {
                            auto length_segmentY = length(m_particules.at(X+count*(Y - 1))->m_pos - m_particules.at(X+count*Y)->m_pos);
                            m_links.push_back(Link(m_particules.at(X+count*(Y - 1)), m_particules.at(X+count*Y), LinkType::damped_hook, length_segmentY, k*1.75, z, 0));
                        }

                        if(X > 1) {
                            auto length_segmentX = length(m_particules.at(X-2 + count*Y)->m_pos - m_particules.at(X+count*Y)->m_pos);
                            m_links.push_back(Link(m_particules.at(X-2 + count*Y), m_particules.at(X+count*Y), LinkType::hook_spring, length_segmentX, k*0.25, 0, 0));
                        }
                        if(Y > 1) {
                            auto length_segmentY = length(m_particules.at(X+count*(Y - 2))->m_pos - m_particules.at(X+count*Y)->m_pos);
                            m_links.push_back(Link(m_particules.at(X+count*(Y - 2)), m_particules.at(X+count*Y), LinkType::hook_spring, length_segmentY, k*0.25, 0, 0));
                        }

                        if(X > 0 && Y > 0) {
                            auto length_segment = length(m_particules.at((X-1) + count*(Y-1))->m_pos - m_particules.at(X+count*Y)->m_pos);
                            m_links.push_back(Link(m_particules.at((X-1) + count*(Y-1)), m_particules.at(X+count*Y), LinkType::hook_spring, length_segment, k, 0, 0));
                        }
                        if(X <count && Y > 0) {
                            auto length_segment = length(m_particules.at((X+1) + count*(Y-1))->m_pos - m_particules.at(X+count*Y)->m_pos);
                            m_links.push_back(Link(m_particules.at((X+1) + count*(Y-1)), m_particules.at(X+count*Y), LinkType::hook_spring, length_segment, k, 0, 0));
                        }
                    }
                }

                if(m_is_dynamic) {
                    m_instance = std::make_shared<Instance>(count, DynamicType::dynamic_grid, m_baseTex, m_alternateTex, m_normalTex);
                    m_instance.get()->add();
                }
                std::cout << "New Grid Anim with " << m_particules.size() << " particule and " << m_links.size() << " links" << std::endl;
            }

            void make_flag(vec3 p1, vec3 p2, vec3 p3, vec3 p4, uint count, float mass, float k, float z) {
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

                // std::cout << "START GRID INIT" << std::endl;

                for (float Y = 0; Y < count; Y++) {
                    for (float X = 0; X < count; X++) {
                        auto axisX = diff12 * (1.0f/count) * ((count-Y)/count) + diff34 * (1.0f/count) * (Y/count);
                        auto axisY = diff13 * (1.0f/count) * ((count-X)/count) + diff24 * (1.0f/count) * (X/count);
                        // auto pos = p1 + diff12 * X * (1.0f/count) + diff34 * (count - X) * (1.0f/count) + diff13 * Y * (1.0f/count) + diff24 * (count - Y) * (1.0f/count);
                        auto pos = p1 + axisX * X + axisY * Y;

                        auto type = ParticuleComputeType::leapfrog;
                        // if(/*(X == 0 && Y == 0) ||*/ (X == count-1 && Y == 0) /*|| (X == 0 && Y == count-1)*/ || (X == count-1 && Y == count-1)) {
                        if(Y == 0 && (X==0 || X == count-1)) {
                            type = ParticuleComputeType::fixed;
                        }

                        // WIREFRAME IS STATIC
                        // if(X == 0 || X == count-1 || Y == 0 || Y == count-1) {
                        //         type = ParticuleComputeType::fixed;
                        // }

                        // std::cerr << "GRID: ADDING Particule " << m_particules.size() << std::endl;
                        if(!m_is_dynamic)m_instance.get()->add(Transform(pos, vec3(), vec3(particuleSize)));
                        m_particules.push_back(new Particule(mass, pos, type));
                        // std::cerr << "GRID: Particule Added" << std::endl;
                    }
                }

                for (uint Y = 0; Y < count; Y++) {
                    for (uint X = 0; X < count; X++) {
                        auto localStrength = k;
                        if (Y == 0) {
                            // is the first layer
                            localStrength*=250.0f;
                        }
                        // auto reductionCoef =  ((count-Y)/count)*0.2+0.8;
                        // localStrength*=reductionCoef;
                        if(X > 0) {
                            auto length_segmentX = length(m_particules.at(X-1 + count*Y)->m_pos - m_particules.at(X+count*Y)->m_pos);
                            m_links.push_back(Link(m_particules.at(X-1 + count*Y), m_particules.at(X+count*Y), LinkType::damped_hook, length_segmentX, localStrength, z, 0));
                        }
                        if(Y > 0) {
                            auto length_segmentY = length(m_particules.at(X+count*(Y - 1))->m_pos - m_particules.at(X+count*Y)->m_pos);
                            m_links.push_back(Link(m_particules.at(X+count*(Y - 1)), m_particules.at(X+count*Y), LinkType::damped_hook, length_segmentY, localStrength, z, 0));
                        }

                        if(X > 1) {
                            auto length_segmentX = length(m_particules.at(X-2 + count*Y)->m_pos - m_particules.at(X+count*Y)->m_pos);
                            m_links.push_back(Link(m_particules.at(X-2 + count*Y), m_particules.at(X+count*Y), LinkType::hook_spring, length_segmentX, localStrength, 0, 0));
                        }
                        if(Y > 1) {
                            auto length_segmentY = length(m_particules.at(X+count*(Y - 2))->m_pos - m_particules.at(X+count*Y)->m_pos);
                            m_links.push_back(Link(m_particules.at(X+count*(Y - 2)), m_particules.at(X+count*Y), LinkType::hook_spring, length_segmentY, localStrength, 0, 0));
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
                            m_links.push_back(Link(m_particules.at((X-1) + count*(Y-1)), m_particules.at(X+count*Y), LinkType::hook_spring, length_segment, k, 0, 0));
                        }
                        if(X <count && Y > 0) {
                            auto length_segment = length(m_particules.at((X+1) + count*(Y-1))->m_pos - m_particules.at(X+count*Y)->m_pos);
                            m_links.push_back(Link(m_particules.at((X+1) + count*(Y-1)), m_particules.at(X+count*Y), LinkType::hook_spring, length_segment, k, 0, 0));
                        }
                    }
                }


                if(m_is_dynamic) {
                    m_instance = std::make_shared<Instance>(count, DynamicType::dynamic_grid, m_baseTex, m_alternateTex, m_normalTex);
                    m_instance.get()->add();
                }

                std::cout << "New Flag Anim with " << m_particules.size() << " particule and " << m_links.size() << " links" << std::endl;
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
                                if(!m_is_dynamic)m_instance.get()->add(Transform(pos, vec3(), vec3(particuleSize)));
                                m_indexes.push_back(m_particules.size());
                            }

                            // m_particules.push_back(new Particule(mass, pos, vec3(-0.645, 0.12, 0.35), type));
                            m_particules.push_back(new Particule(mass, pos, vec3(0), type));

                            // m_particules.back().m_speed = ;
                        }
                    }
                }

                auto internalMultiplier = 1.0f;
                if(true) {
                    for (uint Z = 0; Z < count; Z++) {
                        for (uint Y = 0; Y < count; Y++) {
                            for (uint X = 0; X < count; X++) {

                                // std::cout<<"########## LOOP :"<<X<<" | "<<Y<<" | "<<Z<<std::endl;

                                // std::cout<<"#   # GRID :"<<std::endl;

                                // GRID LINKS
                                if(true) {
                                    auto multiplier = 1.0f;
                                    if(X > 0) {
                                        // std::cout<<X<<" | "<<Y<<" | "<<Z<<std::endl;
                                        multiplier = 1.0f;
                                        if(X>1&&X<count-1&&Y>0&&Y<count-1&&Z>0&&Z<count-1) {multiplier*=internalMultiplier;/* std::cout<<"GRID X  "<<X<<" | "<<Y<<" | "<<Z<<" : "<<k*multiplier<<std::endl; */}
                                        auto length_segmentX = length(m_particules.at(X-1 + count*Y + count*count*Z)->m_pos - m_particules.at(X+count*Y + count*count*Z)->m_pos);
                                        m_links.push_back(Link(m_particules.at(X-1 + count*Y + count*count*Z), m_particules.at(X+count*Y + count*count*Z), LinkType::damped_hook, length_segmentX, k*multiplier, z, 0));
                                    }
                                    if(Y > 0) {
                                        multiplier = 1.0f;
                                        if(X>0&&X<count-1&&Y>1&&Y<count-1&&Z>0&&Z<count-1) {multiplier*=internalMultiplier;/* std::cout<<"GRID Y  "<<X<<" | "<<Y<<" | "<<Z<<" : "<<k*multiplier<<std::endl; */}
                                        auto length_segmentY = length(m_particules.at(X+count*(Y - 1) + count*count*Z)->m_pos - m_particules.at(X+count*Y + count*count*Z)->m_pos);
                                        m_links.push_back(Link(m_particules.at(X+count*(Y - 1) + count*count*Z), m_particules.at(X+count*Y + count*count*Z), LinkType::damped_hook, length_segmentY, k*multiplier, z, 0));
                                    }

                                    if(Z > 0) {
                                        multiplier = 1.0f;
                                        if(X>0&&X<count-1&&Y>0&&Y<count-1&&Z>1&&Z<count-1) {multiplier*=internalMultiplier;/* std::cout<<"GRID Z  "<<X<<" | "<<Y<<" | "<<Z<<" : "<<k*multiplier<<std::endl; */}
                                        auto length_segmentZ = length(m_particules.at(X+count*Y + count*count*(Z-1))->m_pos - m_particules.at(X+count*Y + count*count*Z)->m_pos);
                                        m_links.push_back(Link(m_particules.at(X+count*Y + count*count*(Z-1)), m_particules.at(X+count*Y + count*count*Z), LinkType::damped_hook, length_segmentZ, k*multiplier, z, 0));
                                    }
                                }

                                // std::cout<<"#   # LONG :"<<std::endl;

                                // DOUBLE LINKS
                                if(true) {
                                    auto multiplier = 2.0f;
                                    if(X > 1) {
                                        multiplier = 2.0f;
                                        if(X>2&&X<count-1&&Y>1&&Y<count-1&&Z>1&&Z<count-1) {multiplier*=internalMultiplier;/* std::cout<<"LONG X  "<<X<<" | "<<Y<<" | "<<Z<<" : "<<k*multiplier<<std::endl; */}
                                        auto length_segmentX = length(m_particules.at(X-2 + count*Y + count*count*Z)->m_pos - m_particules.at(X+count*Y + count*count*Z)->m_pos);
                                        m_links.push_back(Link(m_particules.at(X-2 + count*Y + count*count*Z), m_particules.at(X+count*Y + count*count*Z), LinkType::hook_spring, length_segmentX, k*multiplier, 0, 0));
                                    }
                                    if(Y > 1) {
                                        multiplier = 2.0f;
                                        if(X>1&&X<count-1&&Y>2&&Y<count-1&&Z>1&&Z<count-1) {multiplier*=internalMultiplier;/* std::cout<<"LONG Y  "<<X<<" | "<<Y<<" | "<<Z<<" : "<<k*multiplier<<std::endl; */}
                                        auto length_segmentY = length(m_particules.at(X+count*(Y - 2) + count*count*Z)->m_pos - m_particules.at(X+count*Y + count*count*Z)->m_pos);
                                        m_links.push_back(Link(m_particules.at(X+count*(Y - 2) + count*count*Z), m_particules.at(X+count*Y + count*count*Z), LinkType::hook_spring, length_segmentY, k*multiplier, 0, 0));
                                    }
                                    if(Z > 1) {
                                        multiplier = 2.0f;
                                        if(X>1&&X<count-1&&Y>1&&Y<count-1&&Z>2&&Z<count-1) {multiplier*=internalMultiplier;/* std::cout<<"LONG Z  "<<X<<" | "<<Y<<" | "<<Z<<" : "<<k*multiplier<<std::endl; */}
                                        auto length_segmentZ = length(m_particules.at(X+count*Y + count*count*(Z-2))->m_pos - m_particules.at(X+count*Y + count*count*Z)->m_pos);
                                        m_links.push_back(Link(m_particules.at(X+count*Y + count*count*(Z-2)), m_particules.at(X+count*Y + count*count*Z), LinkType::hook_spring, length_segmentZ, k*multiplier, 0, 0));
                                    }
                                }

                                // MAX LINKS
                                if(false) {
                                    auto multiplier = 3.0f;
                                    if(X >= count-1) {
                                        auto length_segmentX = length(m_particules.at(0 + count*Y + count*count*Z)->m_pos - m_particules.at(X+count*Y + count*count*Z)->m_pos);
                                        m_links.push_back(Link(m_particules.at(0 + count*Y + count*count*Z), m_particules.at(X+count*Y + count*count*Z), LinkType::damped_hook, length_segmentX, k*multiplier, z, 0));
                                    }
                                    if(Y >= count-1) {
                                        auto length_segmentY = length(m_particules.at(X+count*(0) + count*count*Z)->m_pos - m_particules.at(X+count*Y + count*count*Z)->m_pos);
                                        m_links.push_back(Link(m_particules.at(X+count*(0) + count*count*Z), m_particules.at(X+count*Y + count*count*Z), LinkType::damped_hook, length_segmentY, k*multiplier, z, 0));
                                    }
                                    if(Z >= count-1) {
                                        auto length_segmentZ = length(m_particules.at(X+count*Y + count*count*(0))->m_pos - m_particules.at(X+count*Y + count*count*Z)->m_pos);
                                        m_links.push_back(Link(m_particules.at(X+count*Y + count*count*(0)), m_particules.at(X+count*Y + count*count*Z), LinkType::damped_hook, length_segmentZ, k*multiplier, z, 0));
                                    }
                                }

                                // std::cout<<"# 1 # DIAG :"<<std::endl;

                                // DIAGONAL LINKS
                                if(false) {
                                    auto multiplier = 1.4f;
                                    {
                                        if(X > 0 && Y > 0) {
                                            multiplier = 1.4f;
                                            if(X>1&&X<count-1&&Y>1&&Y<count-1&&Z>0&&Z<count-1) {multiplier*=internalMultiplier;/* std::cout<<"DIAG XY "<<X<<" | "<<Y<<" | "<<Z<<" : "<<k*multiplier<<std::endl; */}
                                            auto length_segment = length(m_particules.at((X-1) + count*(Y-1) + count*count*(Z))->m_pos - m_particules.at(X+count*Y + count*count*Z)->m_pos);
                                            m_links.push_back(Link(m_particules.at((X-1) + count*(Y-1) + count*count*(Z)), m_particules.at(X+count*Y + count*count*Z), LinkType::hook_spring, length_segment, k*multiplier, 0, 0));
                                        }
                                        if(X > 0 && Z > 0) {
                                            multiplier = 1.4f;
                                            if(X>1&&X<count-1&&Y>0&&Y<count-1&&Z>1&&Z<count-1) {multiplier*=internalMultiplier;/* std::cout<<"DIAG XZ "<<X<<" | "<<Y<<" | "<<Z<<" : "<<k*multiplier<<std::endl; */}
                                            auto length_segment = length(m_particules.at((X-1) + count*(Y) + count*count*(Z-1))->m_pos - m_particules.at(X+count*Y + count*count*Z)->m_pos);
                                            m_links.push_back(Link(m_particules.at((X-1) + count*(Y) + count*count*(Z-1)), m_particules.at(X+count*Y + count*count*Z), LinkType::hook_spring, length_segment, k*multiplier, 0, 0));
                                        }
                                        if(Y > 0 && Z > 0) {
                                            multiplier = 1.4f;
                                            if(X>0&&X<count-1&&Y>1&&Y<count-1&&Z>1&&Z<count-1) {multiplier*=internalMultiplier;/* std::cout<<"DIAG YZ "<<X<<" | "<<Y<<" | "<<Z<<" : "<<k*multiplier<<std::endl; */}
                                            auto length_segment = length(m_particules.at((X) + count*(Y-1) + count*count*(Z-1))->m_pos - m_particules.at(X+count*Y + count*count*Z)->m_pos);
                                            m_links.push_back(Link(m_particules.at((X) + count*(Y-1) + count*count*(Z-1)), m_particules.at(X+count*Y + count*count*Z), LinkType::hook_spring, length_segment, k*multiplier, 0, 0));
                                        }
                                    }

                                    // std::cout<<"# 2 # DIAG :"<<std::endl;
                                    {
                                        if(X <count-1 && Y > 0) {
                                            multiplier = 1.4f;
                                            if(X>0&&X<count-2&&Y>1&&Y<count-1&&Z>0&&Z<count-1) {multiplier*=internalMultiplier;/* std::cout<<"GIAD  "<<X<<" | "<<Y<<" | "<<Z<<" : "<<k*multiplier<<std::endl; */}
                                            auto length_segment = length(m_particules.at((X+1) + count*(Y-1) + count*count*(Z))->m_pos - m_particules.at(X+count*Y + count*count*Z)->m_pos);
                                            m_links.push_back(Link(m_particules.at((X+1) + count*(Y-1) + count*count*(Z)), m_particules.at(X+count*Y + count*count*Z), LinkType::hook_spring, length_segment, k*multiplier, 0, 0));
                                        }
                                        if(X <count-1 && Z > 0) {
                                            multiplier = 1.4f;
                                            if(X>0&&X<count-2&&Y>0&&Y<count-1&&Z>1&&Z<count-1) {multiplier*=internalMultiplier;/* std::cout<<"GIAD  "<<X<<" | "<<Y<<" | "<<Z<<" : "<<k*multiplier<<std::endl; */}
                                            auto length_segment = length(m_particules.at((X+1) + count*(Y) + count*count*(Z-1))->m_pos - m_particules.at(X+count*Y + count*count*Z)->m_pos);
                                            m_links.push_back(Link(m_particules.at((X+1) + count*(Y) + count*count*(Z-1)), m_particules.at(X+count*Y + count*count*Z), LinkType::hook_spring, length_segment, k*multiplier, 0, 0));
                                        }
                                        if(Y <count-1 && Z > 0) {
                                            multiplier = 1.4f;
                                            if(X>0&&X<count-1&&Y>0&&Y<count-2&&Z>1&&Z<count-1) {multiplier*=internalMultiplier;/* std::cout<<"GIAD  "<<X<<" | "<<Y<<" | "<<Z<<" : "<<k*multiplier<<std::endl; */}
                                            auto length_segment = length(m_particules.at((X) + count*(Y+1) + count*count*(Z-1))->m_pos - m_particules.at(X+count*Y + count*count*Z)->m_pos);
                                            m_links.push_back(Link(m_particules.at((X) + count*(Y+1) + count*count*(Z-1)), m_particules.at(X+count*Y + count*count*Z), LinkType::hook_spring, length_segment, k*multiplier, 0, 0));
                                        }
                                    }
                                }

                                // MAX DIAGONAL LINKS
                                if(false) {
                                    auto multiplier = 1.4f;
                                    multiplier = 1.4f;
                                    if(X == 0 && Y == 0) {
                                        auto length_segment = length(m_particules.at((0) + count*(0) + count*count*(Z))->m_pos - m_particules.at(X+count*Y + count*count*Z)->m_pos);
                                        m_links.push_back(Link(m_particules.at((0) + count*(0) + count*count*(Z)), m_particules.at(X+count*Y + count*count*Z), LinkType::hook_spring, length_segment, k*multiplier, 0, 0));
                                    }
                                    if(X == 0 && Z == 0) {
                                        auto length_segment = length(m_particules.at((0) + count*(Y) + count*count*(0))->m_pos - m_particules.at(X+count*Y + count*count*Z)->m_pos);
                                        m_links.push_back(Link(m_particules.at((0) + count*(Y) + count*count*(0)), m_particules.at(X+count*Y + count*count*Z), LinkType::hook_spring, length_segment, k*multiplier, 0, 0));
                                    }
                                    if(Y == 0 && Z == 0) {
                                        auto length_segment = length(m_particules.at((X) + count*(0) + count*count*(0))->m_pos - m_particules.at(X+count*Y + count*count*Z)->m_pos);
                                        m_links.push_back(Link(m_particules.at((X) + count*(0) + count*count*(0)), m_particules.at(X+count*Y + count*count*Z), LinkType::hook_spring, length_segment, k*multiplier, 0, 0));
                                    }

                                    if(X ==count-1 && Y == 0) {
                                        auto length_segment = length(m_particules.at((count) + count*(0) + count*count*(Z))->m_pos - m_particules.at(X+count*Y + count*count*Z)->m_pos);
                                        m_links.push_back(Link(m_particules.at((count) + count*(0) + count*count*(Z)), m_particules.at(X+count*Y + count*count*Z), LinkType::hook_spring, length_segment, k*multiplier, 0, 0));
                                    }
                                    if(X ==count-1 && Z == 0) {
                                        auto length_segment = length(m_particules.at((count) + count*(Y) + count*count*(0))->m_pos - m_particules.at(X+count*Y + count*count*Z)->m_pos);
                                        m_links.push_back(Link(m_particules.at((count) + count*(Y) + count*count*(0)), m_particules.at(X+count*Y + count*count*Z), LinkType::hook_spring, length_segment, k*multiplier, 0, 0));
                                    }
                                    if(Y ==count-1 && Z == 0) {
                                        auto length_segment = length(m_particules.at((X) + count*(count) + count*count*(0))->m_pos - m_particules.at(X+count*Y + count*count*Z)->m_pos);
                                        m_links.push_back(Link(m_particules.at((X) + count*(count) + count*count*(0)), m_particules.at(X+count*Y + count*count*Z), LinkType::hook_spring, length_segment, k*multiplier, 0, 0));
                                    }
                                }

                                //
                                if(true) {
                                    auto multiplier = sqrt(3.0);
                                    // multiplier = 4.0f;
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
                                        m_links.push_back(Link(m_particules.at((X+x) + count*(Y+y) + count*count*(Z+z)), m_particules.at(X+count*Y + count*count*Z), LinkType::hook_spring, length_segment, k*multiplier, 0, 0));
                                    }
                                }
                            }
                        }
                    }
                }

                if(m_is_dynamic) {
                    m_instance = std::make_shared<Instance>(count, DynamicType::dynamic_cube, m_baseTex, m_alternateTex, m_normalTex);
                    m_instance.get()->add();
                }

                std::cout << "New Cube Anim with " << m_particules.size() << " particule and " << m_links.size() << " links" << std::endl;
            }

            void reset() {

                bool was_running = m_computeAnim.load(std::memory_order_seq_cst);

                // stop the computation
                setComputeState(false);

                // ensure all threads are stopped
                bool have_to_wait = true;
                while(have_to_wait) {
                    have_to_wait = false;
                    for (auto& state : m_threadStates) {
                        if(state!=ThreadProcessState::ParticulesDone) {
                            have_to_wait = true;
                            break;
                        }
                    }
                }
                
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
                    
                    if (was_running) {
                        setComputeState(was_running);
                    }
                }
            
            void addField(FieldType type, rigidBody *rb, float k) {
                if(rb == nullptr) return;
                m_fields.emplace_back(Field());
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
                m_fields.emplace_back(Field());
                switch (type)
                {
                    case FieldType::field_cube:
                    m_fields.back().make_cube(box, k);
                    break;
                    default:
                    break;
                }
            }


            void addField(FieldType type, const FilePath &applicationPath, WindowManager *window, vec3 direction, float k) {
                m_fields.emplace_back(Field());
                switch (type)
                {
                case FieldType::field_wind:
                    m_fields.back().make_wind(applicationPath, window, m_instance, normalize(direction), computeBB(), k);
                    break;
                default:
                    break;
                }
            }
            
            void addField(FieldType type, vec3 coords, float k) {
                m_fields.emplace_back(Field());
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

            void addField(FieldType type, float k) {
                m_fields.emplace_back(Field());
                switch (type)
                {
                case FieldType::field_fluid: // TO DO ADD fluid movement
                    m_fields.back().make_fluid(k);
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
                    for (auto &field: m_fields) {
                        field.update(m_particules.at(index), h);
                    }
                    m_particules.at(index)->update(h);
                    // m_instance.get()->updatePosition(index, m_particules.at(index)->m_pos);
                    // m_instance.get()->compute(index);
                }

                // std::cout << "PARTICULE POSITION { " << m_particules.at(0)->m_pos.x << ", " << m_particules.at(0)->m_pos.y << ", " << m_particules.at(0)->m_pos.z << " } " << std::endl;
            }

            void update(float h) {
                for(auto &f: m_fields) {
                    if(f.getType() == FieldType::field_wind)continue;
                    switch (f.getType())
                    {
                    case FieldType::field_wind:
                        f.change_box(computeBB());  
                    default:
                        f.update_field();
                        break;
                    }
                }
                update_links();
                update_particules(h, 0, m_particules.size());
            }

            std::vector<vec3> getParticulesPositions() {
                std::vector<vec3> tmp_vector;
                for(auto &particule: m_particules) {
                    tmp_vector.push_back(vec3(particule->m_pos));
                }
                return tmp_vector;
            }

            void update_visual() {
                if(m_type == AnimType::none) return;
                uint index = 0;

                if(m_is_dynamic) {
                    // SLOWWWWWWWWW
                    std::vector<vec3> tmp_vector = getParticulesPositions();
                    m_instance.get()->updateDynamicMesh(tmp_vector);
                }

                else {
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
            }

            void updateFields() {
                if(m_type == AnimType::none) return;
                for (size_t i = 0; i < m_fields.size(); i++)
                {
                    switch (m_fields.at(i).getType())
                    {
                    case FieldType::field_wind:
                        m_fields.at(i).change_box(computeBB());  
                        m_fields.at(i).update_field();
                        break;
                    default:
                        m_fields.at(i).update_field();
                        break;
                    }
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

            bool stopMultithreads() {
                if(!m_is_multithreaded) {
                    return false;
                }
                {
                    std::lock_guard<std::mutex> lock(m_mtx);
                    m_computeAnim.store(false, std::memory_order_seq_cst);
                }
                m_killThreads.store(true, std::memory_order_seq_cst);
                m_cv.notify_all();
                for (uint i = 0; i < m_threadList.size(); i++)
                {
                    if(m_threadList.at(i).joinable()) m_threadList.at(i).join();
                }
                m_threadList.clear();
                m_threadStates.clear();
                m_threadComparaison.clear();
                m_nb_threads = 0;
                return true;
            }
            
            bool activateMultithreaded(uint threads, FPSCamera *camera) {
                if(m_is_multithreaded) {
                    return false;
                }
                if (threads == 0) {
                    return false;
                }
                
                m_nb_threads = threads;
                m_killThreads.store(false, std::memory_order_seq_cst);
                m_computeAnim.store(false, std::memory_order_seq_cst);
                m_deltaTThreads = 0.001f;

                // m_threadStates.reserve(m_nb_threads);
                // m_threadComparaison.reserve(m_nb_threads);
                // m_threadList.reserve(m_nb_threads);

                auto m_animUpdateLambda = [&] (int index, FPSCamera *camera) {
                    uint startLinksIndex = (1.0f*index/m_nb_threads) * getLinksCount();
                    uint endLinksIndex = (1.0f*(index+1)/m_nb_threads) * getLinksCount();
                
                    uint startParticuleIndex = (1.0f*index/m_nb_threads) * getParticulesCount();
                    uint endParticuleIndex = (1.0f*(index+1)/m_nb_threads) * getParticulesCount();
                
                    m_threadStates.at(index) = NothingDone;
                    m_threadComparaison.at(index) = Compute_1;
                
                    auto canStartNextStep = false;
                    auto canUpdateParticule = false;

                    auto refreshVisualDelta = 1.0/60.0;
                    auto nextRefresh = 0.0;
                
                    auto timer = 0.0f;
                    auto oldTime = 0.0f;
                
                    while(!m_killThreads.load(std::memory_order_seq_cst)) {
                        // This block should wait for the other threads to have finished their computation
                        canStartNextStep = true;
                        switch (m_threadComparaison.at(index))
                        {
                        case Compute_1:
                            
                            for(auto state: m_threadComparaison) {
                                if(state == Compute_3) {
                                    canStartNextStep = false;
                                    break;
                                }
                            }
                            break;
                        case Compute_2:
                            for(auto state: m_threadComparaison) {
                                if(state == Compute_1) {
                                    canStartNextStep = false;
                                    break;
                                }
                            }
                            break;
                        case Compute_3:
                            for(auto state: m_threadComparaison) {
                                if(state == Compute_2) {
                                    canStartNextStep = false;
                                    break;
                                }
                            }
                            break;
                        }
                        if(!canStartNextStep) {
                            continue;
                        }

                        {
                            std::unique_lock<std::mutex> lock(m_mtx);
                            m_cv.wait(lock, [&] {
                                if(m_killThreads.load(std::memory_order_seq_cst)) {
                                    return true;
                                }
                                // std::cout<<"Bro Im in a lock "<<index<<std::endl;
                                return m_computeAnim.load(std::memory_order_seq_cst);
                            });  // Passive wait
                        }

                        if(m_killThreads.load(std::memory_order_seq_cst)) {
                            break;
                        }
                
                        if(m_computeAnim.load(std::memory_order_seq_cst)) {
                            // std::cout<<"Computing"<<std::endl;
                            timer = glfwGetTime();
                            m_threadStates.at(index) = NothingDone;
                
                            update_links(startLinksIndex, endLinksIndex);
                            m_threadStates.at(index) = LinksDone;
                            
                            while(!m_killThreads.load(std::memory_order_seq_cst)) {
                                
                                canUpdateParticule = true;
                                for (size_t i = 0; i < m_threadStates.size(); i++) {
                                    if(m_threadStates.at(i) == NothingDone) {
                                        canUpdateParticule = false;
                                        break;
                                    }
                                }   
                                if(canUpdateParticule) {
                                    break;
                                }
                            }
                            if(m_killThreads.load(std::memory_order_seq_cst)) {
                                return;
                            }
                            
                            update_particules(m_deltaTThreads, startParticuleIndex, endParticuleIndex);
                            // update_particules(0.0001, startParticuleIndex, endParticuleIndex);
                            m_threadStates.at(index) = ParticulesDone;
                            
                            
                            // tell the other thread that it has finished computing this frame
                            switch (m_threadComparaison.at(index))
                            {
                                case Compute_1:
                                m_threadComparaison.at(index) = Compute_2;
                                break;
                                case Compute_2:
                                m_threadComparaison.at(index) = Compute_3;
                                break;
                                case Compute_3:
                                m_threadComparaison.at(index) = Compute_1;
                                break;
                            }
                            if(index == 0) {

                                if(nextRefresh < timer) {
                                    nextRefresh = timer + refreshVisualDelta;
                                    // updateFields();
                                    // update_visual();
                                }


                                // std::cout << 1.0/(glfwGetTime() - timer) << std::endl;
                                // std::cout << "deltaTThreads " << deltaTThreads << " nb links: " << getLinksCount() << std::endl;
                                m_deltaTThreads = glfwGetTime() - timer;
                                // animFrameTime = glfwGetTime() - oldTime;
                                if(camera) {
                                    getFields()->back().make_cube(camera->getBBox(), 0);
                                }
                                // deltaTThreads = 0.1;
                            }
                        }

                        else {
                        }
                        // bool state = m_computeAnim.load(std::memory_order_acquire);
                        // std::cout << "compute anim state:" << state << std::endl;
                
                        while(oldTime+m_deltaTThreads > glfwGetTime()){}
                        oldTime = glfwGetTime();
                    }
                };

                for (uint i = 0; i < m_nb_threads; i++) {
                    m_threadStates.push_back(NothingDone);
                    m_threadComparaison.push_back(Compute_1);
                    m_threadList.emplace_back(std::thread(m_animUpdateLambda, i, camera));
                }

                m_is_multithreaded = true;

                return true;
            }

            bool setComputeState(bool state) {
                if(!m_is_multithreaded) return false;
                {
                    std::lock_guard<std::mutex> lock(m_mtx);
                    m_computeAnim.store(state, std::memory_order_seq_cst);
                }
                m_cv.notify_all();
                return true;
            }
            
            bool getComputeAnimState() {
                return m_computeAnim.load(std::memory_order_seq_cst);
            }

            float getDeltaTThreads() {
                return m_deltaTThreads;
            }

        private:
            AnimType m_type;
            std::shared_ptr<Instance> m_instance;
            std::vector<Particule*> m_particules;
            std::vector<uint> m_indexes;
            std::vector<Link> m_links;
            std::vector<Field> m_fields;
            uint m_count_dimension;

            bool m_is_dynamic=false;
            GLuint m_baseTex;
            GLuint m_alternateTex;
            GLuint m_normalTex;

            bool m_is_multithreaded=false;
            uint m_nb_threads;
            std::atomic<bool> m_computeAnim{false};
            std::atomic<bool> m_killThreads{false};
            volatile double m_deltaTThreads;
            std::vector<ThreadProcessState> m_threadStates;
            std::vector<ThreadProcessComparaison> m_threadComparaison;
            std::vector<std::thread> m_threadList;
            std::mutex m_mtx;
            std::condition_variable m_cv;
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
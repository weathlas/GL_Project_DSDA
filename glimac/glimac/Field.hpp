#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <vector>

#include <glimac/BBox.hpp>
#include <glimac/Particule.hpp>

#include "common.hpp"
#include "glm.hpp"

using namespace glm;

namespace glimac {

    enum FieldType {
        field_directional,
        field_point,
        field_fluid,
        field_wall,
        field_cube,
        field_convex,
        field_wind,
        field_magnet // might be too complex
    };

    const float gravityConstant = 6.67f;
    const float marginBBOX = 0.01f;

    const int DIM_TEXTURE = 1024;

    class Field {

        public:
            // std::vector<Particule*> m_particules;

            Field() {
            }
            
            Field(FieldType type, float k, float z, float s) {
                m_type = type;
                m_k = k;
                m_z = z;
                m_s = s;
            }

            ~Field() {
                if(m_shadowMap) {
                    m_shadowMap.get()->~ShadowMap();
                }
            }

            // void connect(Particule *M1) {
            //     m_particules.push_back(M1);
            // }

            void make_directional(vec3 direction, float k) {
                m_type = FieldType::field_directional;
                m_world_direction = direction;
                m_k = k;
            }

            void make_point(vec3 position, float k) {
                m_type = FieldType::field_point;
                m_world_pos = position;
                m_k = k;
            }
            
            void make_fluid(float k) {
                m_type = FieldType::field_fluid;
                m_k = k;
            }

            void make_wall(vec3 position, float k) {
                m_type = FieldType::field_wall;
                m_world_pos = position;
                m_k = k;
            }
            
            void make_cube(BBox3f box, float k) {
                m_type = FieldType::field_cube;
                m_bbox = box;
                m_k = k;
            }
            
            void make_convex(rigidBody* rb, float k) {
                if(rb == nullptr) return;
                m_type = FieldType::field_convex;
                convexHull = rb;
                m_k = k;
            }

            void make_wind(const FilePath &applicationPath, WindowManager *window, std::shared_ptr<Instance> &instance, vec3 direction, BBox3f bbox, float k) {
                m_type = FieldType::field_wind;
                m_world_direction = normalize(direction);
                m_bbox = bbox;
                m_k = k;
                m_window = window;
                m_shadowMap = std::make_shared<ShadowMap>(applicationPath, "src/shaders/utils/normal.vs.glsl", "src/shaders/utils/normal_normalized.fs.glsl");
                m_shadowMap.get()->init(DIM_TEXTURE, DIM_TEXTURE);
                m_shadowMap.get()->setOrthoRadius(length(m_bbox.size()));
                auto center = 0.5f*(vec3(m_bbox.upper)+vec3(m_bbox.lower));
                m_shadowMap.get()->computeTransforms(LightStruct(vec3(-m_world_direction), vec3(1), vec3(100, 0.85, LightType::directionalLight)), center);
                m_scene = Scene();
                m_scene.addInstance(instance);

                m_depthData = std::vector<float>(DIM_TEXTURE * DIM_TEXTURE);
                m_colorData = std::vector<vec3>(DIM_TEXTURE * DIM_TEXTURE);
                update_field();
            }
            
            void change_box(BBox3f box) {
                // std::cout << "Old BBOX: " << length(m_bbox.size()) << " New BBOX: " << length(box.size()) << std::endl;
                m_bbox = box;
            }
            
            void change_k(float k) {
                m_k = k;
            }
            
            void change_z(float z) {
                m_z = z;
            }
            
            void change_s(float s) {
                m_s = s;
            }
            
            void change_pos(vec3 pos) {
                m_world_pos = pos;
            }
            
            void change_direction(vec3 direction) {
                m_world_direction = normalize(direction);
            }
            
            void change_convexhull(rigidBody *rb) {
                if(rb == nullptr) return;
                convexHull = rb;
            }
            
            void update_field() {
                float value = 0.0f;
                switch(m_type)
                {
                case FieldType::field_wind:
                    // std::cout<<"updating wind"<<std::endl;
                    value = length(m_bbox.size());
                    m_shadowMap.get()->setPlanes(-value, value);
                    m_shadowMap.get()->setOrthoRadius(value/2);
                    m_shadowMap.get()->computeTransforms(LightStruct(vec3(-m_world_direction), vec3(1), vec3(100, 0.85, LightType::directionalLight)), 0.5f*(vec3(m_bbox.upper)+vec3(m_bbox.lower)));
                    m_shadowMap.get()->renderTexture(*m_window, m_scene);

                    glBindTexture(GL_TEXTURE_2D, m_shadowMap.get()->getDepthMap());
                    glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_FLOAT, m_depthData.data());

                    glBindTexture(GL_TEXTURE_2D, m_shadowMap.get()->getColorMap());
                    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, m_colorData.data());

                    break;
                default:
                    break;
                }
            }

            void update(Particule* p, float h) {
                // std::cout<<"Field Update " << m_type << std::endl;
                switch (m_type)
                {

                case FieldType::field_directional:
                    update_field_directional(p);
                    break;
                case FieldType::field_point:
                    update_field_point(p);
                    break;
                case FieldType::field_fluid:
                    update_field_fluid(p, h);
                    break;
                case FieldType::field_wall:
                    update_field_wall(p, h);
                    break;
                case FieldType::field_cube:
                    update_field_cube(p, h);
                    break;
                case FieldType::field_convex:
                    update_field_convex(p, h);
                    break;
                case FieldType::field_wind:
                    update_field_wind(p, h);
                    break;
                default:
                    break;
                }
            }

            FieldType getType() {
                return m_type;
            }

            GLuint getDebugColorTexture() {
                return m_shadowMap.get()->getColorMap();
            }

            GLuint getDebugDepthTexture() {
                return m_shadowMap.get()->getDepthMap();
            }
            
        private:
            FieldType m_type;
            BBox3f m_bbox;

            std::shared_ptr<ShadowMap> m_shadowMap;
            std::vector<float> m_depthData;
            std::vector<vec3> m_colorData;
            Scene m_scene;
            WindowManager *m_window;

            float m_k, m_z, m_s;
            vec3 m_world_pos, m_world_direction;
            rigidBody * convexHull;

            void update_field_directional(Particule* p) {
                p->m_forces_acc += m_k * m_world_direction * p->m_mass; 
            }

            void update_field_point(Particule* p) {
                vec3 v_diff = p->m_pos - m_world_pos;
                float d = length(v_diff);
                if(d < 0.25f) {
                    d = 0.25f;
                    // continue;
                }
                // if(v_diff.x == 0.0f && v_diff.y == 0.0f && v_diff.z == 0.0f) {
                //     continue;
                // }
                // float epsilon = 0.005;
                // if (d < epsilon)  {
                //     return;
                // }
                vec3 u = normalize(v_diff);

                p->m_forces_acc -= gravityConstant * ((p->m_mass * m_k) / (d*d)) * u;

                // p->m_forces_acc += m_k * u * (gravityConstant/d);
                // p->m_forces_acc -= 0.0001f * vec3(m_k/v_diff.x, m_k/v_diff.y, m_k/v_diff.z);
                // p->m_forces_acc += v_diff;
                // p->m_forces_acc += vec3(0, -0.1, 0);
                
                // for (uint i = 0; i < m_particules.size(); i++) {
                // }

            }

            void update_field_fluid(Particule* p, float h) {
                // float speed = length(p->m_speed);
                // if (speed == 0.0f) {
                //     return;
                // }
                // vec3 speed_unit_vector = p->m_speed / speed;
                // p->m_forces_acc -= (speed * speed * m_k) * speed_unit_vector;
                p->m_forces_acc += (-m_k * p->m_mass * (1/h)) * (p->m_speed);
            }

            void update_field_wall(Particule* p, float h) {
                auto diff = p->m_pos.y - m_world_pos.y-marginBBOX;
                if(diff < 0.0f) {
                    if(p->m_speed.y < 0.0f) {
                        // p->m_forces_acc += p->m_mass * (-p->m_speed.y) / h * vec3(0, 1, 0);
                    }
                    p->m_forces_acc += (p->m_mass * (-diff/(h*h))) * vec3(0, 1, 0) + (-m_k * p->m_mass * (1/h)) * (p->m_speed);
                    
                    // else {
                    //     p->m_forces_acc += p->m_mass * (p->m_speed.y) / h * vec3(0, 1, 0);
                    // }
                    // p->m_forces_acc += (diff*diff) * vec3(0, 1, 0);
                    // if(p->m_speed.y <= 0.0f) {
                    //     p->m_speed.y = 0.0f;
                    // }
                }
            }

            vec3 get_closest_edge(vec3 &pos) {
                auto left = pos.x - m_bbox.lower.x-marginBBOX;
                if(left <= 0) return vec3(0);
                auto right = m_bbox.upper.x+marginBBOX - pos.x;
                if(right <= 0) return vec3(0);
                auto up = m_bbox.upper.y+marginBBOX - pos.y;
                if(up <= 0) return vec3(0);
                auto down = pos.y - m_bbox.lower.y-marginBBOX;
                if(down <= 0) return vec3(0);
                auto front = m_bbox.upper.z+marginBBOX - pos.z;
                if(front <= 0) return vec3(0);
                auto back = pos.z - m_bbox.lower.z-marginBBOX;
                if(back <= 0) return vec3(0);
                auto x = (left <= right) ? -left : right;
                auto y = (down <= up) ? -down : up;
                auto z = (back <= front) ? -back : front;
                if(abs(x) < abs(y)) {
                    if(abs(x) < abs(z)) {
                        return vec3(x, 0, 0);
                    }
                    return vec3(0, 0, z);
                }
                if (abs(y) < abs(z)) {
                    return vec3(0, y, 0);
                }
                return vec3(0, 0, z);
            }

            void update_field_cube(Particule* p, float h) {
                vec3 offset = get_closest_edge(p->m_pos);

                // save one sqrt if the particule is not inside the cube (len == 0.0)
                if(offset.x == 0.0f && offset.y == 0.0f && offset.z == 0.0f) return;
                auto len = length(offset);
                vec3 norm = offset * (1.0f/len); // replace normalize
                auto friction_axis_speed = vec3(p->m_speed.x*(1-abs(norm.x)), p->m_speed.y*(1-abs(norm.y)), p->m_speed.z*(1-abs(norm.z)));
                p->m_forces_acc += (p->m_mass * (1.0f/(h))) * norm + (-m_k * p->m_mass * (1/h)) * (friction_axis_speed);
                //  - p->m_speed * p->m_mass;
                // p->m_mass * (-p->m_speed) / h;
            }

            void update_field_convex(Particule* p, float h) {
                vec3 offset = convexHull->getDisplacementPoint(p->m_pos);
                // std::cout << "Convex offset: { " << offset.x << ", " << offset.y << ", " << offset.z << " }" << std::endl;
                if(offset.x == 0.0f && offset.y == 0.0f && offset.z == 0.0f) {
                    // std::cout<<"Convex offset is zero" << std::endl;
                    return;
                };
                auto len = length(offset);
                if(len<0.0001f) {
                    // std::cout<<"Length for convex is too small " << len << std::endl;
                    return;
                }
                vec3 norm = offset * (1.0f/len); // replace normalize 
                // std::cout<<"############################################## Length for convex is " << len << std::endl;
                p->m_forces_acc += (p->m_mass * (len/(h*h))) * norm + (-m_k * p->m_mass * (1/h)) * (p->m_speed);
            }

            bool isVisibleFromDepthData(const glm::vec3& position)
            {
                glm::vec4 clip = m_shadowMap.get()->getLightProj() * m_shadowMap.get()->getModelToLight() * glm::vec4(position, 1.0f);
                glm::vec3 ndc = glm::vec3(clip) / clip.w;
            
                if (ndc.x < -1.0f || ndc.x > 1.0f || ndc.y < -1.0f || ndc.y > 1.0f || ndc.z < -1.0f || ndc.z > 1.0f)
                    return false;
            
                // Convert to texture space
                int u = static_cast<int>((ndc.x * 0.5f + 0.5f) * DIM_TEXTURE);
                int v = static_cast<int>((ndc.y * 0.5f + 0.5f) * DIM_TEXTURE);
            
                // Clamp to texture bounds
                u = clamp(u, 0, DIM_TEXTURE - 1);
                v = clamp(v, 0, DIM_TEXTURE - 1);
            
                // Depth map is stored row-major, bottom to top
                float storedDepth = m_depthData[v * DIM_TEXTURE + u];
            
                float pointDepth = ndc.z * 0.5f + 0.5f; // remap z from [-1, 1] to [0, 1]
            
                return pointDepth <= storedDepth + 0.001f;
            }

            vec3 getNormalFromColorData(const glm::vec3& position) {
                glm::vec4 clip = m_shadowMap.get()->getLightProj() * m_shadowMap.get()->getModelToLight() * glm::vec4(position, 1.0f);
                glm::vec3 ndc = glm::vec3(clip) / clip.w;
            
                if (ndc.x < -1.0f || ndc.x > 1.0f || ndc.y < -1.0f || ndc.y > 1.0f || ndc.z < -1.0f || ndc.z > 1.0f)
                    return vec3(0);
            
                int u = int(round((ndc.x * 0.5f + 0.5f) * DIM_TEXTURE));
                int v = int(round((ndc.y * 0.5f + 0.5f) * DIM_TEXTURE));
            
                u = clamp(u, 0, DIM_TEXTURE - 1);
                v = clamp(v, 0, DIM_TEXTURE - 1);
            
                vec3 storedColor = m_colorData[v * DIM_TEXTURE + u];

                if(storedColor.x == 0 && storedColor.y == 0 && storedColor.z == 0) {
                    return vec3(0);
                }

                vec3 storedNormal = (storedColor-0.5f)*2.0f; // remap from [0, 1] to [-1, 1]
                // std::cout << storedColor << std::endl;
                return storedNormal;
            }

            void update_field_wind(Particule* p, float h) {
                if(isVisibleFromDepthData(p->m_pos)) {

                    auto normal = getNormalFromColorData(p->m_pos);
                    if(normal.x == 0 && normal.y == 0 && normal.z == 0) {
                        return;
                    }
                    // std::cout<<normal<<std::endl;
                    // change the world normal to a wind space one
                    // auto windViewNormal = normalize(vec3(m_shadowMap.get()->getLightNormal() * vec4(normal, 0)));
                    auto dot_result = dot(m_world_direction, normal);
                    auto dot_exposure = abs(dot_result);

                    if(dot_result < 0) {
                        normal*=-1;
                    }
                    p->m_forces_acc += dot_exposure * m_k * normal * (1/p->m_mass);
                    // p->m_forces_acc += dot_exposure * m_k * vec3(0, 0, 1) * p->m_mass;
                }
            }

            // void update_field_cube(Particule* p, float h) {
            //     // BBox3f(vec3(-1, 0, -12), vec3(1, 3, -2))
            //     float xOverlap = std::max(0.0f, std::min(m_bbox.upper.x, p->m_pos.x) - std::max(m_bbox.lower.x, p->m_pos.x));
            //     float yOverlap = std::max(0.0f, std::min(m_bbox.upper.y, p->m_pos.y) - std::max(m_bbox.lower.y, p->m_pos.y));
            //     float zOverlap = std::max(0.0f, std::min(m_bbox.upper.z, p->m_pos.z) - std::max(m_bbox.lower.z, p->m_pos.z));
            //     float xOffset = 0.0f;
            //     float yOffset = 0.0f;
            //     float zOffset = 0.0f;

            //     if(xOverlap<=0 && yOverlap<=0 && zOverlap<=0) {
            //         return;
            //     }
            //     std::cout << "CASE 0, nothing"  << " | " << xOverlap << " | " << yOverlap << " | " << zOverlap << std::endl;

            //     if (xOverlap > 0) {
            //         auto left = m_bbox.upper.x - p->m_pos.x;
            //         auto right = p->m_pos.x - m_bbox.lower.x;
            //         xOffset = left < right ? -xOverlap : xOverlap;
            //     }

            //     if (yOverlap > 0) {
            //         auto bottom = m_bbox.upper.y - p->m_pos.y;
            //         auto top = p->m_pos.y - m_bbox.lower.y;
            //         yOffset = bottom < top ? -yOverlap : yOverlap;
            //     }

            //     if (zOverlap > 0) {
            //         auto back = m_bbox.upper.z - p->m_pos.z;
            //         auto front = p->m_pos.z - m_bbox.lower.z;
            //         zOffset = back < front ? -zOverlap : zOverlap;
            //     }

            //     // std::cout << xOverlap << " | " << zOverlap << " -> " << xOffset << " | " << zOffset << std::endl;

            //     if(xOverlap < zOverlap) {
            //         if (xOverlap < yOverlap) {
            //             p->m_forces_acc += p->m_mass * ((xOffset)/(h*h)) * vec3(1, 0, 0);
            //             std::cout << "CASE 1" << std::endl;
            //             return;
            //             // return vec3(xOffset, 0.0f, 0.0f);
            //         }
            //         if(yOffset > 0) {
            //             p->m_forces_acc += p->m_mass * ((yOffset)/(h*h)) * vec3(0, 1, 0);
            //             std::cout << "CASE 2" << std::endl;
            //             return;
            //             // return vec3(0.0f, yOffset, 0.0f);
            //         }
            //     }
            //     else if (yOverlap < zOverlap) {
            //         if(yOffset > 0) {
            //             p->m_forces_acc += p->m_mass * ((yOffset)/(h*h)) * vec3(0, 1, 0);
            //             std::cout << "CASE 3" << std::endl;
            //             return;
            //             // return vec3(0.0f, yOffset, 0.0f);
            //         }
            //     }
            //     p->m_forces_acc += p->m_mass * ((zOffset)/(h*h)) * vec3(0, 0, 1);
            //     std::cout << "CASE 4" << std::endl;
            //     return;
            //     // return vec3(0.0f, 0.0f, zOffset);
            // }
    };
}
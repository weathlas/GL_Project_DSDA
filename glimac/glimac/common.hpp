#pragma once

#include <algorithm>

#include <GLFW/glfw3.h>
#include "glm.hpp"

#include <klein/klein.hpp>

#include <iostream>

namespace glimac {

    template <typename T>
    struct tuple {
        T a;
        T b;
        bool operator==(const tuple& o) const {
            return a==o.a && b==o.b;
        };
        bool operator!=(const tuple& o) const {
            return a!=o.a || b!=o.b;
        };
        bool operator<(const tuple& o) const {
            return a<o.a || b<o.b;
        };
        bool operator>(const tuple& o) const {
            return a>o.a || b>o.b;
        };
        // bool operator<(const tuple& o) const {
        //     return a<o.a || (a==o.a && b<o.b);
        // };
        // bool operator>(const tuple& o) const {
        //     return a>o.a || (a==o.a && b>o.b);
        // };
        // bool operator<=(const tuple& o) const {
        //     return a<=o.a || (a==o.a && b<=o.b);
        // };
        // bool operator>=(const tuple& o) const {
        //     return a>=o.a || (a==o.a && b>=o.b);
        // };
    };

    template <typename T>
    struct triple {
        T a;
        T b;
        T c;
        bool operator==(const triple& o) const {
            return a==o.a && b==o.b && c==o.c;
        };
        bool operator!=(const triple& o) const {
            return a!=o.a || b!=o.b && c!=o.c;
        };
        bool operator<(const triple& o) const {
            return a<o.a || b<o.b || c<o.c;
        };
        bool operator>(const triple& o) const {
            return a>o.a || b>o.b || c>o.c;
        };
        // bool operator<(const triple& o) const {
        //     return a<o.a || (a==o.a && (b<o.b || (b==o.b && c<o.c)));
        // };
        // bool operator>(const triple& o) const {
        //     return a>o.a || (a==o.a && (b>o.b || (b==o.b && c>o.c)));
        // };
        // bool operator<=(const triple& o) const {
        //     return a<=o.a || (a==o.a && (b<=o.b || (b==o.b && c<=o.c)));
        // };
        // bool operator>=(const triple& o) const {
        //     return a>=o.a || (a==o.a && (b>=o.b || (b==o.b && c>=o.c)));
        // };
    };

    struct edge
    {
        kln::point p1;
        kln::point p2;
        kln::line l;
        float len;
        edge(kln::point a, kln::point b) : p1(a), p2(b), l(kln::line(p1 & p2)) {

            auto tmp1 = p1.normalized();
            auto tmp2 = p2.normalized();
            len = length(glm::vec3(tmp2.x(), tmp2.y(), tmp2.z()) - glm::vec3(tmp1.x(), tmp1.y(), tmp1.z()));
        }
    };

    struct triangle
    {
        kln::point p1;
        kln::point p2;
        kln::point p3;
        kln::plane plane;
        triangle() = delete;
        triangle(kln::point a, kln::point b, kln::point c) : p1(a), p2(b), p3(c), plane((p1 & p2 & p3)){}
    };

    struct collision
    {
        kln::point location;
        glm::vec3 offset;
        float distance = 0.0;
        bool collided;
        collision():location(kln::point(0, 0, 0)), offset(glm::vec3(0)), collided(false){}
        collision(bool collide):location(kln::point(0, 0, 0)), offset(glm::vec3(0)), collided(collide){}
        collision(kln::point loc, glm::vec3 off, float d, bool collide):location(loc), offset(off), distance(d), collided(collide) {}
    };

    struct rigidBody {
        std::vector<kln::point> points;
        std::vector<kln::point> points_computed;
        std::vector<tuple<unsigned int>> edges_index;
        std::vector<edge> edges_computed;
        std::vector<edge> edges_from_Instance;
        std::vector<triple<unsigned int>> triangles_index;
        std::vector<triangle> triangles_computed;
        std::vector<triangle> triangles_from_Instance;

        std::vector<collision> collisions;
        kln::point com; // used for collison resolving (rotation/translations)
        kln::motor motor;
        kln::motor motor_tick; // update to add at each physic step
        kln::rotor rotor_tick; // update to add at each physic step
        kln::translator translator_tick; // update to add at each physic step
        float saved_mass = 1.0f;
        float inverseMass;
        bool is_static;
        bool setMass(float mass) {
            if(mass == 0.0f) {
                return false;
            }
            saved_mass = mass;
            inverseMass = 1/mass;
            return true;
        }
        bool setStatic(bool value) {
            bool old = is_static;
            is_static = value;
            if(is_static) {
                inverseMass = 0.0f;
            }
            else {
                inverseMass = 1/saved_mass;
            }
            return old;
        }
        glm::vec3 getVelocity() {
            return glm::vec3(translator_tick.e01(), translator_tick.e02(), translator_tick.e03());
        }
        void setMotor(kln::motor& m) {
            motor = m;
            com = motor(kln::point{0, 0, 0});
            motor_tick = kln::motor(kln::translator(0, 1, 0, 0));
            translator_tick = kln::translator(0, 1, 0, 0);
        }
        void updateMotor(float deltaT) {
            motor = (translator_tick * deltaT) * motor;
            com = (translator_tick * deltaT)(com);

            // WARNING
            // com = motor(kln::point{0, 0, 0});
        }
        void computePoints(){
            points_computed.clear();
            for(auto &p: points) {
                points_computed.push_back(motor(p));
            }
        }
        void computeEdges(){
            edges_computed.clear();
            for(auto &e: edges_from_Instance) {
                edges_computed.push_back(edge{motor(e.p1), motor(e.p2)});
            }
            // edges_computed.clear();
            // for(auto &t: edges_index) {
            //     edges_computed.push_back(edge{kln::point(points_computed[t.a]), kln::point(points_computed[t.b])});
            // }
        }
        void computeTriangles(){
            // triangles_computed.clear();
            std::vector<triangle> tmp;

            // for (size_t i = 0; i < triangles_from_Instance.size(); i++)
            // {
            //     /* code */
            // }
            

            for(auto &t: triangles_from_Instance) {
                tmp.push_back(triangle(motor(t.p1), motor(t.p2), motor(t.p3)));
            }
            triangles_computed = tmp;
            // triangles_computed.clear();
            // for(auto &t: triangles_index) {
            //     // WARNING : A -> C -> B
            //     triangles_computed.push_back(triangle{kln::point(points_computed[t.a]), kln::point(points_computed[t.b]), kln::point(points_computed[t.c])});
            // }
        }
        void computeAll() {
            computePoints();
            computeEdges();
            computeTriangles();
        }
        // void reducePoints(){
        //     std::sort(points.begin(), points.end());
        //     auto it = unique(points.begin(), points.end());
        //     points.erase(it, points.end());
        // }
        void reduceEdges(){
            std::sort(edges_index.begin(), edges_index.end());
            auto it = unique(edges_index.begin(), edges_index.end());
            edges_index.erase(it, edges_index.end());
        }
        // void reduceTriangles(){
        //     std::sort(triangles_index.begin(), triangles_index.end());
        //     auto it = unique(triangles_index.begin(), triangles_index.end());
        //     triangles_index.erase(it, triangles_index.end());
        // }
        void reduceAll() {
            reduceEdges();
            // reduceTriangles();
        }
        void resolveCollisons() {

            auto accumulator = glm::vec3(0);
            for(auto &collision: collisions) {
                accumulator += collision.offset;
                // for each subsequent collision offset, we neet to offset the colliison.offset by the accumulator to check if the old collsion solved it
                if (accumulator != glm::vec3(0)) {
                    break;
                }
            }
            if(accumulator != glm::vec3(0)) {
                kln::translator tx(accumulator.x, 1.0, 0.0, 0.0);
                kln::translator ty(accumulator.y, 0.0, 1.0, 0.0);
                kln::translator tz(accumulator.z, 0.0, 0.0, 1.0);
                translator_tick = (tx * ty * tz) * translator_tick;
                // motor = (tx * ty * tz) * motor;
            }
            collisions.clear();
        }

        glm::vec3 translatorToVec3(kln::translator p) {
            float out[4];
            _mm_store_ps(out, p.p2_);
            return glm::vec3(out[1], out[2], out[3]);
        }
        
        collision projectToTriangle(triangle& t, kln::point& p) {
            // auto plane = (t.p1 & t.p2 & t.p3);
            auto projectedPoint = kln::project(p, t.plane).normalized();
            if((t.plane ^ p).e0123() > 0) {
                return collision(false);
            }
            auto offset = translatorToVec3(p * projectedPoint);

            // not the case anymore // made in a way to reduce the amount of computation is the point is not behind the triangle
            auto l12 = (t.p1 & t.p2).normalized();
            auto l13 = (t.p1 & t.p3).normalized();
            auto l1p = (t.p1 & projectedPoint).normalized();
            auto l23 = (t.p2 & t.p3).normalized();
            auto l2p = (t.p2 & projectedPoint).normalized();
            auto lp3 = (projectedPoint & t.p3).normalized();

            auto k11 = l12 | l13;
            auto k12 = l12 | l1p;
            auto k21 = l23 | l12.inverse();
            auto k22 = l23 | l2p;
            auto k31 = l13 | l23;
            auto k32 = l13 | lp3;

            if(k11 < k12 || k21 < k22 || k31 < k32) {
                return collision(projectedPoint, offset, glm::length(offset), false);
            }

            return collision(projectedPoint, offset, glm::length(offset), true);
        }

        glm::vec3 getDisplacementPoint(kln::point& p) {
            // auto transform = graphRender.get()->get(indexGraph);
            // auto pos = vec3(transform.m_Position);
            // pos = vec3(x, 0.12, z);
            // auto velocity = vec3(transform.m_Velocity);
            // velocity = vec3(0);
            // auto point = kln::point(pos.x, pos.y, pos.z);

            float smallestDistance;
            bool firstProjection = true;
            glm::vec3 displacement;

            for(auto &tr: triangles_computed) {
                auto result = projectToTriangle(tr, p);

                // std::cout <<"Collided "<< result.collided << " distance " << result.distance << " offset { " << result.offset.x << ", " << result.offset.y << ", " << result.offset.z << " } " << std::endl;

                // if collided is false, we will ignore the displacement
                // otherwise we will use it so no need for another conditional branch
                if(firstProjection) {
                    firstProjection = false;
                    smallestDistance = result.distance;
                    displacement = result.offset;
                    continue;
                }

                // new candidate for the closest plane from a triangle
                // if the closest plane is not in collision then there is no need for a collision
                // otherwise it is a smaller displacement that resolve the collision
                if(result.distance < smallestDistance) {
                    smallestDistance = result.distance;
                    displacement = result.offset;
                    continue;
                }
            }
            
            return displacement;
        }

        glm::vec3 getDisplacementPoint(glm::vec3& p) {
            auto point = kln::point{p.x, p.y, p.z}.normalized();
            return getDisplacementPoint(point);
        }
    };

    enum collision_state {
        collision_state_none,
        collision_state_closest,
    };
    struct manifold {
        rigidBody* R1, R2;
        glm::vec3 closestToR1, closestToR2;
        glm::vec3 localVelocity;
    };

    struct ShapeVertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoords;
        glm::vec3 tangeant;
    };

    struct Transform {
        glm::vec3 m_Position;
        glm::vec3 m_Angles;
        glm::vec3 m_Scales;
        glm::vec3 m_Velocity;
        Transform() : m_Position(glm::vec3(0)), m_Angles(glm::vec3(0)), m_Scales(glm::vec3(1))  {}
        Transform(glm::vec3 pos) : m_Position(pos), m_Angles(glm::vec3(0)), m_Scales(glm::vec3(1))  {}
        Transform(glm::vec3 pos, glm::vec3 angle) : m_Position(pos), m_Angles(angle), m_Scales(glm::vec3(1))  {}
        Transform(glm::vec3 pos, glm::vec3 angle, glm::vec3 scale) : m_Position(pos), m_Angles(angle), m_Scales(scale)  {}
    };

    const float degToRad = glm::pi<float>()/180;
    const float radToDeg = 180/glm::pi<float>();

    const float kmToMs = 1/3.6;

    const glm::vec3 X_Vector = glm::vec3(1, 0, 0);
    const glm::vec3 Y_Vector = glm::vec3(0, 1, 0);
    const glm::vec3 Z_Vector = glm::vec3(0, 0, 1);

}

#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <klein/klein.hpp>

#include "common.hpp"
#include "glm.hpp"

using namespace glm;

namespace glimac {

    void printFloat(float v) {
        std::cout << "v:" << v << std::endl;
    }

    void printVec(vec3& v) {
        std::cout << "{ " << v.x << ", " << v.y << ", " << v.z << " } vector" << std::endl;
    }

    void printBool(bool v) {
        std::cout << "bool: " << (v?"true":"false") << std::endl;
    }

    void printPoint(kln::point& p) {
        std::cout << "{ " << p.x() << ", " << p.y() << ", " << p.z() << " }" << std::endl;
    }

    void printTranslator(kln::translator& p) {
        std::cout << "{ " << p.e01() << ", " << p.e02() << ", " << p.e03() << ", " << p.e10() << ", " << p.e20() << ", " << p.e30() << " }" << std::endl;
    }

    void printDual(kln::dual& p) {
        std::cout << "{ value:" << p.e0123() << ", p:" << p.p << ", q:" << p.q << ", scalar:" << p.scalar() << " }" << std::endl;
    }

    void printMotor(kln::motor& p) {
        std::cout
        << "{ e12: " << p.e12()//
        << ", e13: " << p.e13()//
        << ", e23: " << p.e23()//
        << "} { e0123: " << p.e0123()
        << ", e01: " << p.e01()
        << ", e02: " << p.e02()
        << ", e03: " << p.e03()
        << ", e10: " << p.e10()
        << ", e12: " << p.e12()//
        << ", e13: " << p.e13()//
        << ", e20: " << p.e20()
        << ", e20: " << p.e20()
        << ", e21: " << p.e21()
        << ", e23: " << p.e23()//
        << ", e30: " << p.e30()
        << ", e31: " << p.e31()
        << ", e32: " << p.e32()
        << " }"
        << std::endl;
        // std::cout << "{ value:" << p.e0123() << ", p:" << p.p << ", q:" << p.q << ", scalar:" << p.scalar() << " }" << std::endl;
    }

    void printLine(kln::line& p) {
        std::cout
        << "{ "
        << "e01:" << p.e01()
        << ", e02:" << p.e02()
        << ", e03:" << p.e03()
        << ", e10:" << p.e10()
        << ", e12:" << p.e12()
        << ", e13:" << p.e13()
        << ", e20:" << p.e20()
        << ", e21:" << p.e21()
        << ", e23:" << p.e23()
        << ", e30:" << p.e30()
        << ", e31:" << p.e31()
        << ", e32:" << p.e32()
        << " }"
        << std::endl;
        // std::cout << "{ value:" << p.e0123() << ", p:" << p.p << ", q:" << p.q << ", scalar:" << p.scalar() << " }" << std::endl;
    }

    kln::point vecToPoint(const glm::vec3& v) {
        return kln::point(v.x, v.y, v.z);
    }

    glm::vec3 pointToVec(const kln::point& p) {
        auto tmp = p.normalized();
        return glm::vec3(tmp.x(), tmp.y(), tmp.z());
    }

    float signedVolume(kln::point& p1, kln::point& p2, kln::point& p3, kln::point& p4) {
        return (p4.normalized() & p1.normalized() & p2.normalized() & p3.normalized()).scalar() * (1.0/6);
    }

    vec3 translatorToVec3(kln::translator p) {
        float out[4];
        _mm_store_ps(out, p.p2_);
        return vec3(out[1], out[2], out[3]);
    }

    kln::rotor anglesToRotor(float ax, float ay, float az) {
        kln::rotor rx(-ax, 1, 0, 0);
        kln::rotor ry(-ay, 0, 1, 0);
        kln::rotor rz(-az, 0, 0, 1);
        return (ry * rx * rz).normalized();
    }

    kln::rotor anglesToRotor(vec3& angles) {
        return anglesToRotor(angles.x, angles.y, angles.z);
    }

    kln::translator posToTranslator(float px, float py, float pz) {
        kln::translator tx(px, 1.0, 0.0, 0.0);
        kln::translator ty(py, 0.0, 1.0, 0.0);
        kln::translator tz(pz, 0.0, 0.0, 1.0);
        return tx * ty * tz;
    }

    kln::translator posToTranslator(vec3& pos) {
        return posToTranslator(pos.x, pos.y, pos.z );
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
            return collision(projectedPoint, offset, length(offset), false);
        }

        return collision(projectedPoint, offset, length(offset), true);
    }

    int collideRigidBody(rigidBody* R1, rigidBody* R2) {
        // if faut prendre en compte les collisions plan/plan
        // cas ou 2 triangles se touches mais pas au niveau des sommets
        /*
            .
        ._____/_\_____.
        \    /   \    /
        \  /     \  /
        \/       \/
        /\       /\
        /  \     /  \
        .____\___/____.
            \ /
            *    
        */

    bool foundACollision = false;

    auto nbBefore = R1->collisions.size() + R2->collisions.size();

        for (size_t i = 0; i < R1->points_computed.size(); i++)
        {
        auto point = kln::point(R1->points_computed[i]);

        float smallestDistance;
        bool firstProjection = true;
        vec3 displacement;

        auto c = collision(false);
        for (auto &tr: R2->triangles_computed)
        {

            auto result = projectToTriangle(tr, point);

            // if collided is false, we will ignore the displacement
            // otherwise we will use it so no need for another conditional branch
            if(firstProjection) {
                firstProjection = false;
                smallestDistance = result.distance;
                displacement = result.offset;
                c = result;
                continue;
            }

            // new candidate for the closest plane from a triangle
            // if the closest plane is not in collision then there is no need for a collision
            // otherwise it is a smaller displacement that resolve the collision
            if(result.distance < smallestDistance) {
                smallestDistance = result.distance;
                displacement = result.offset;
                c = result;
                continue;
            }
        }

        if(c.collided) {
            R1->collisions.push_back(c);
            auto inverseCollision = c;
            inverseCollision.offset *= -1;
            R2->collisions.push_back(inverseCollision);
        }

        // velocity += displacement;
        // pos += velocity;

        // transform.m_Position = pos;
        // transform.m_Velocity = velocity;
        // graphRender.get()->update(transform, indexGraph);
        // // graphRender.get()->updatePosition(indexGraph, pos);
        // indexGraph++;
        
        }

        for (size_t i = 0; i < R2->points_computed.size(); i++)
        {
        auto point = kln::point(R2->points_computed[i]);

        float smallestDistance;
        bool firstProjection = true;
        vec3 displacement;

        auto c = collision(false);
        for (auto &tr: R1->triangles_computed)
        {

            auto result = projectToTriangle(tr, point);

            // if collided is false, we will ignore the displacement
            // otherwise we will use it so no need for another conditional branch
            if(firstProjection) {
                firstProjection = false;
                smallestDistance = result.distance;
                displacement = result.offset;
                c = result;
                continue;
            }

            // new candidate for the closest plane from a triangle
            // if the closest plane is not in collision then there is no need for a collision
            // otherwise it is a smaller displacement that resolve the collision
            if(result.distance < smallestDistance) {
                smallestDistance = result.distance;
                displacement = result.offset;
                c = result;
                continue;
            }
        }

        if(c.collided) {
            R2->collisions.push_back(c);
            auto inverseCollision = c;
            inverseCollision.offset *= -1;
            R1->collisions.push_back(inverseCollision);
        }

        // velocity += displacement;
        // pos += velocity;

        // transform.m_Position = pos;
        // transform.m_Velocity = velocity;
        // graphRender.get()->update(transform, indexGraph);
        // // graphRender.get()->updatePosition(indexGraph, pos);
        // indexGraph++;
        
        }

        if(!foundACollision) {
            // rebelote pour les edges
        }

        return R1->collisions.size() + R2->collisions.size() - nbBefore;
    }

    bool linesSameDirection(kln::line& L1, kln::line& L2) {
        float P1L1[4];
        float P2L1[4];
        _mm_store_ps(P1L1, L1.p1_);
        _mm_store_ps(P2L1, L1.p2_);
        float P1L2[4];
        float P2L2[4];
        _mm_store_ps(P1L2, L2.p1_);
        _mm_store_ps(P2L2, L2.p2_);

        // if(std::isnan(P1L1[0]) || std::isnan(P1L2[0])) {
        //     return false;
        // }

        if(
        (
        std::isnan(P1L1[0]) ||
        std::isnan(P1L2[0]) ||
        (
        P1L1[0] == P1L2[0] &&
        P1L1[1] == P1L2[1] &&
        P1L1[2] == P1L2[2] &&
        P1L1[3] == P1L2[3]
        &&
        P2L1[0] == P2L2[0] &&
        P2L1[1] == P2L2[1] &&
        P2L1[2] == P2L2[2] &&
        P2L1[3] == P2L2[3]
        )
        )
        )
        return true;
        return false;
    }

    bool linesSameDirection(kln::line& L1, kln::line& L2, kln::line& L3) {
        float P1L1[4];
        float P2L1[4];
        _mm_store_ps(P1L1, L1.p1_);
        _mm_store_ps(P2L1, L1.p2_);
        float P1L2[4];
        float P2L2[4];
        _mm_store_ps(P1L2, L2.p1_);
        _mm_store_ps(P2L2, L2.p2_);
        float P1L3[4];
        float P2L3[4];
        _mm_store_ps(P1L3, L3.p1_);
        _mm_store_ps(P2L3, L3.p2_);


        // we get nan if one of the line if from 2 points at the same position
        if(
        (
        std::isnan(P1L1[0]) |
        std::isnan(P1L2[0]) |
        (
        P1L1[0] == P1L2[0] &&
        P1L1[1] == P1L2[1] &&
        P1L1[2] == P1L2[2] &&
        P1L1[3] == P1L2[3]
        &&
        P2L1[0] == P2L2[0] &&
        P2L1[1] == P2L2[1] &&
        P2L1[2] == P2L2[2] &&
        P2L1[3] == P2L2[3]
        )
        )
        &&
        (
        std::isnan(P1L1[0]) |
        std::isnan(P1L3[0]) |
        (
        P1L1[0] == P1L3[0] &&
        P1L1[1] == P1L3[1] &&
        P1L1[2] == P1L3[2] &&
        P1L1[3] == P1L3[3]
        &&
        P2L1[0] == P2L3[0] &&
        P2L1[1] == P2L3[1] &&
        P2L1[2] == P2L3[2] &&
        P2L1[3] == P2L3[3]
        )
        )
        )
        return true;
        return false;
    }


    struct edgeCast {
        kln::point intersection;
        bool collided;
        edge e;
        edgeCast() : intersection(kln::point{0, 0, 0}) , collided(false), e(edge(kln::point{0, 0, 0}, kln::point{0, 0, 0})) {}
        edgeCast(bool b) : intersection(kln::point{0, 0, 0}) , collided(b), e(edge(kln::point{0, 0, 0}, kln::point{0, 0, 0})) {}
        edgeCast(bool b, kln::point p, edge& ee) : intersection(p) , collided(b), e(edge(ee.p1, ee.p2)) {}
    };

    edgeCast collideEdgeTriangle(triangle& t, edge& e) {
        auto intersection = (e.l ^ t.plane);

        // std::cout << "intersection: " << std::endl;
        // printPoint(intersection);

        if(
            std::isnan(intersection.e013()) ||
            std::isnan(intersection.e021()) ||
            std::isnan(intersection.e032()) ||
            std::isnan(intersection.e123())
        )
        return edgeCast(false);

        auto l12 = (t.p1 & t.p2).normalized();
        auto l13 = (t.p1 & t.p3).normalized();
        auto l1p = (t.p1 & intersection).normalized();
        auto l23 = (t.p2 & t.p3).normalized();
        auto l2p = (t.p2 & intersection).normalized();
        auto lp3 = (intersection & t.p3).normalized();

        auto k11 = l12 | l13;
        auto k12 = l12 | l1p;
        auto k21 = l23 | l12.inverse();
        auto k22 = l23 | l2p;
        auto k31 = l13 | l23;
        auto k32 = l13 | lp3;

        if(k11 < k12 || k21 < k22 || k31 < k32) {
            // std::cout << "Valeurs: " << (k11 < k12) << " : " << k11 << " < " << k12 << std::endl;
            // std::cout << "Valeurs: " << (k21 < k22) << " : " << k21 << " < " << k22 << std::endl;
            // std::cout << "Valeurs: " << (k31 < k32) << " : " << k31 << " < " << k32 << std::endl;
            return edgeCast(false);
        }
        auto L1 = (intersection.normalized() & e.p1).normalized();
        auto L2 = (intersection.normalized() & e.p2).normalized();

        L1 = kln::project(L1, kln::point{0, 0, 0}).normalized();
        L2 = kln::project(L2, kln::point{0, 0, 0}).normalized();

        if(linesSameDirection(L1, L2)) {
            return edgeCast(false);
        }
        // printPoint(intersection);
        // auto tmp = intersection.normalized();
        // std::cout << "Intersection:" << std::endl;
        // printPoint(tmp);
        // std::cout << "Edge:" << std::endl;
        // printPoint(e.p1);
        // printPoint(e.p2);
        // auto tmp2 = (intersection - e.p1.normalized());
        // auto tmp3 = (intersection - e.p2.normalized());
        // std::cout << "Diff:" << std::endl;
        // printPoint(tmp2);
        // printPoint(tmp3);
        // auto tmpVec1 = pointToVec(tmp2);
        // auto tmpVec2 = pointToVec(tmp3);
        // std::cout << "Vecteur diff:" << std::endl;
        // printVec(tmpVec1);
        // printVec(tmpVec2);
        // auto len1 = length(tmpVec1);
        // auto len2 = length(tmpVec2);
        // std::cout << "Longeurs:" << std::endl;
        // std::cout << "Len: " << e.len << " : " << (len1) << " | " << len2 << std::endl;

        vec3 inter = pointToVec(intersection);
        vec3 ppp1 = pointToVec(e.p1);
        vec3 ppp2 = pointToVec(e.p2);

        auto len1 = length(inter - ppp1);
        auto len2 = length(inter - ppp2);

        // std::cout << "Len: " << e.len << " : " << (len1) << " | " << len2 << std::endl;


        if(len1 > e.len || len2 > e.len) {
            // std::cout << "Len: " << e.len << " : " << (len1) << " | " << len2 << std::endl;
            return edgeCast(false);
        }

        return edgeCast(true, intersection.normalized(), e);
    }

    int RBCollide(rigidBody* R1, rigidBody* R2, std::vector<edgeCast> * lst, vec3* I1, vec3* I2, int* id, triangle* col_triangleR11, triangle* col_triangleR12, triangle* col_triangleR21, triangle* col_triangleR22) {
        auto nbCollide1 = 0;
        auto nbCollide2 = 0;
        auto foundPointInside1 = false;
        auto foundPointInside2 = false;
        triangle savedTriangle1(kln::point{0, 0, 0}, kln::point{0, 0, 0}, kln::point{0, 0, 0});
        triangle savedTriangle2(kln::point{0, 0, 0}, kln::point{0, 0, 0}, kln::point{0, 0, 0});
        triangle savedTriangle11(kln::point{0, 0, 0}, kln::point{0, 0, 0}, kln::point{0, 0, 0});
        triangle savedTriangle22(kln::point{0, 0, 0}, kln::point{0, 0, 0}, kln::point{0, 0, 0});
        std::vector<edgeCast> allIntersectionsR1;
        std::vector<edgeCast> allIntersectionsR2;
        *I1 = vec3(0);
        *I2 = vec3(0);
        *id = -1;

        for (size_t i = 0; i < R1->points_computed.size(); i++) {
            auto c = collision(false);
            auto point = kln::point(R1->points_computed[i]).normalized();

            float smallestDistance;
            bool firstProjection = true;
            vec3 displacement;

            for (auto &tr: R2->triangles_computed) {

                auto result = projectToTriangle(tr, point);

                if(firstProjection) {
                    firstProjection = false;
                    smallestDistance = result.distance;
                    displacement = result.offset;
                    c = result;
                    savedTriangle1 = triangle(tr.p1, tr.p2, tr.p3);
                    continue;
                }
                if(result.distance < smallestDistance) {
                    smallestDistance = result.distance;
                    displacement = result.offset;
                    c = result;
                    savedTriangle1 = triangle(tr.p1, tr.p2, tr.p3);
                    continue;
                }
            }

            if(displacement != vec3(0)) {
                nbCollide1++;
                foundPointInside1 = true;
                *I2 = pointToVec(point);
                // std::cout << "Point collided: 1" << std::endl;
                break;
            }
            // else {
            //     std::cout << "Point did not collide for 1: " << c.collided << " | " << c.distance << " | x:" << c.offset.x << " y:" << c.offset.x << " z:" << c.offset.x << std::endl;
            // }
        }
        for (size_t i = 0; i < R2->points_computed.size(); i++) {
            auto c = collision(false);
            auto point = kln::point(R2->points_computed[i]).normalized();

            float smallestDistance;
            bool firstProjection = true;
            vec3 displacement;

            for (auto &tr: R1->triangles_computed) {

                auto result = projectToTriangle(tr, point);

                if(firstProjection) {
                    firstProjection = false;
                    smallestDistance = result.distance;
                    displacement = result.offset;
                    c = result;
                    savedTriangle2 = triangle(tr.p1, tr.p2, tr.p3);
                    continue;
                }
                if(result.distance < smallestDistance) {
                    smallestDistance = result.distance;
                    displacement = result.offset;
                    c = result;
                    savedTriangle2 = triangle(tr.p1, tr.p2, tr.p3);
                    continue;
                }
            }

            if(displacement != vec3(0)) {
                nbCollide2++;
                foundPointInside2 = true;
                *I1 = pointToVec(point);
                // std::cout << "Point collided: 2" << std::endl;
                break;
            }
            // else {
            //     std::cout << "Point did not collide for 2: " << c.collided << " | " << c.distance << " | x:" << c.offset.x << " y:" << c.offset.x << " z:" << c.offset.x << std::endl;
            // }
        }

        if(!foundPointInside1) {
            for (size_t i = 0; i < R1->edges_computed.size(); i++) {
                for (auto &tr: R2->triangles_computed) {
                    auto result = collideEdgeTriangle(tr, R1->edges_computed[i]);
                    if (result.collided) {
                        if(lst != nullptr) {
                            allIntersectionsR1.push_back(result);
                            lst->push_back(result);
                        }
                        nbCollide1++;
                    }
                }
            }
            std::cout<<"nbCollide 1: "<< nbCollide1 <<std::endl;
            // if(nbCollide1 == 0)
        }
        
        if(!foundPointInside2) {
            for (size_t i = 0; i < R2->edges_computed.size(); i++) {
                for (auto &tr: R1->triangles_computed) {
                    auto result = collideEdgeTriangle(tr, R2->edges_computed[i]);
                    if (result.collided) {
                        if(lst != nullptr) {
                            allIntersectionsR2.push_back(result);
                            lst->push_back(result);
                        }
                        nbCollide2++;
                    }
                }
            }
            std::cout<<"nbCollide 2: "<< nbCollide2 <<std::endl;
        }
        
        if(foundPointInside1 && !foundPointInside2) {
            *id = 0;
            *col_triangleR11 = triangle(savedTriangle1.p1, savedTriangle1.p2, savedTriangle1.p3);
            return nbCollide1+nbCollide2;
        }
        if(!foundPointInside1 && foundPointInside2) {
            *id = 1;
            *col_triangleR21 = triangle(savedTriangle2.p1, savedTriangle2.p2, savedTriangle2.p3);
            return nbCollide1+nbCollide2;
        }

        *id = 2;

        if(nbCollide1>1) {
            for(auto& p: allIntersectionsR1) {
                *I2+= pointToVec(p.intersection);
            }
            *I2/=allIntersectionsR1.size();
        }
        if(nbCollide2>1) {
            for(auto& p: allIntersectionsR2) {
                *I1+= pointToVec(p.intersection);
            }
            *I1/=allIntersectionsR2.size();
        }

        // if(nbCollide1 > 0 || nbCollide2 > 0) {
        //     std::cout << "NB edge collide: " << (nbCollide1+nbCollide2) << " = " << nbCollide1 << " + " << nbCollide2 << std::endl;
        // }

        return nbCollide1+nbCollide2;
    }

    int RBCollideNOP(rigidBody* R1, rigidBody* R2, std::vector<edgeCast> * lst, vec3* I1, vec3* I2) {
        auto nbCollide1 = 0;
        auto nbCollide2 = 0;
        std::vector<edgeCast> allIntersectionsR1;
        std::vector<edgeCast> allIntersectionsR2;
        *I1 = vec3(0);
        *I2 = vec3(0);

        for (size_t i = 0; i < R1->points_computed.size(); i++) {
            auto c = collision(false);
            auto point = kln::point(R1->points_computed[i]).normalized();

            float smallestDistance;
            bool firstProjection = true;
            vec3 displacement;

            for (auto &tr: R2->triangles_computed) {

                auto result = projectToTriangle(tr, point);

                // if collided is false, we will ignore the displacement
                // otherwise we will use it so no need for another conditional branch
                if(firstProjection) {
                    firstProjection = false;
                    smallestDistance = result.distance;
                    displacement = result.offset;
                    c = result;
                    continue;
                }

                // new candidate for the closest plane from a triangle
                // if the closest plane is not in collision then there is no need for a collision
                // otherwise it is a smaller displacement that resolve the collision
                if(result.distance < smallestDistance) {
                    smallestDistance = result.distance;
                    displacement = result.offset;
                    c = result;
                    continue;
                }
            }

            if(displacement != vec3(0)) {
                nbCollide1++;
                *I2 = pointToVec(point);
                // std::cout << "Point collided: 1" << std::endl;
                break;
            }
            // else {
            //     std::cout << "Point did not collide for 1: " << c.collided << " | " << c.distance << " | x:" << c.offset.x << " y:" << c.offset.x << " z:" << c.offset.x << std::endl;
            // }
        }
        if(nbCollide1 == 0) {
            for (size_t i = 0; i < R1->edges_computed.size(); i++) {
                for (auto &tr: R2->triangles_computed) {
                    auto result = collideEdgeTriangle(tr, R1->edges_computed[i]);
                    if (result.collided) {
                        if(lst != nullptr) {
                            allIntersectionsR1.push_back(result);
                            lst->push_back(result);
                        }
                        nbCollide1++;
                    }
                }
            }
        }

        for (size_t i = 0; i < R2->points_computed.size(); i++) {
            auto c = collision(false);
            auto point = kln::point(R2->points_computed[i]).normalized();

            float smallestDistance;
            bool firstProjection = true;
            vec3 displacement;

            for (auto &tr: R1->triangles_computed) {

                auto result = projectToTriangle(tr, point);

                // if collided is false, we will ignore the displacement
                // otherwise we will use it so no need for another conditional branch
                if(firstProjection) {
                    firstProjection = false;
                    smallestDistance = result.distance;
                    displacement = result.offset;
                    c = result;
                    continue;
                }

                // new candidate for the closest plane from a triangle
                // if the closest plane is not in collision then there is no need for a collision
                // otherwise it is a smaller displacement that resolve the collision
                if(result.distance < smallestDistance) {
                    smallestDistance = result.distance;
                    displacement = result.offset;
                    c = result;
                    continue;
                }
            }

            if(displacement != vec3(0)) {
                nbCollide2++;
                *I1 = pointToVec(point);
                // std::cout << "Point collided: 2" << std::endl;
                break;
            }
            // else {
            //     std::cout << "Point did not collide for 2: " << c.collided << " | " << c.distance << " | x:" << c.offset.x << " y:" << c.offset.x << " z:" << c.offset.x << std::endl;
            // }
        }

        if(nbCollide2 == 0) {
            for (size_t i = 0; i < R2->edges_computed.size(); i++) {
                for (auto &tr: R1->triangles_computed) {
                    auto result = collideEdgeTriangle(tr, R2->edges_computed[i]);
                    if (result.collided) {
                        if(lst != nullptr) {
                            allIntersectionsR2.push_back(result);
                            lst->push_back(result);
                        }
                        nbCollide2++;
                    }
                }
            }
        }

        if(nbCollide1>1) {
            for(auto& p: allIntersectionsR1) {
                *I2+= pointToVec(p.intersection);
            }
            *I2/=allIntersectionsR1.size();
        }
        if(nbCollide2>1) {
            for(auto& p: allIntersectionsR2) {
                *I1+= pointToVec(p.intersection);
            }
            *I1/=allIntersectionsR2.size();
        }

        // if(nbCollide1 > 0 || nbCollide2 > 0) {
        //     std::cout << "NB edge collide: " << (nbCollide1+nbCollide2) << " = " << nbCollide1 << " + " << nbCollide2 << std::endl;
        // }

        return nbCollide1+nbCollide2;
    }

    vec3 resolveInternalPoint(rigidBody* R, vec3& p, bool caca) {
        // auto pos = pointToVec(R->com);
        if(caca)return vec3(0);
        auto velocity = R->getVelocity();
        auto point = kln::point(p.x, p.y, p.z);

        float smallestDistance;
        bool firstProjection = true;
        vec3 displacement;

        for(auto &tr: R->triangles_computed) {
            auto result = projectToTriangle(tr, point);

            // if collided is false, we will ignore the displacement
            // otherwise we will use it so no need for another conditional branch
            if(firstProjection) {
                firstProjection = false;
                smallestDistance = result.distance;
                displacement = -result.offset;
                continue;
            }

            // new candidate for the closest plane from a triangle
            // if the closest plane is not in collision then there is no need for a collision
            // otherwise it is a smaller displacement that resolve the collision
            if(result.distance < smallestDistance) {
                smallestDistance = result.distance;
                displacement = -result.offset;
                continue;
            }
        }
        
        velocity += displacement;
        // pos += velocity;

        // std::cout<< "displacement" << std::endl;
        // printVec(displacement);

        // auto translation = posToTranslator(displacement);
        // auto normalizedTranslation = normalize(displacement);

        // R->com = translation(R->com);
        // R->motor = translation * R->motor;
        // // R->com = R->com + vecToPoint(displacement);
        
        // R->translator_tick = (posToTranslator(normalizedTranslation)*1.0f) * R->translator_tick;

        return displacement;
    }

    void updatePhysic(rigidBody* R1, rigidBody* R2, vec3& p1,  vec3& p2, vec3& displacementR1, vec3& displacementR2, int id) {
        auto R1ContactPoint = p1 + -displacementR1;
        auto R2ContactPoint = p2 + -displacementR2;

        auto weirdNormal = normalize(displacementR2 - displacementR1);
        auto NORMAL = normalize(pointToVec(R1->com) - pointToVec(R2->com));

        auto normalR1 = normalize(pointToVec(R1->com) - p1);
        auto normalR2 = normalize(pointToVec(R2->com) - p2);

        auto restitutionA = 1.0f;
        auto restitutionB = 1.0f;

        auto normal = p2 - p1;
        auto depth = length(normal);
        normal*= 1/depth;
        float e = std::min(restitutionA, restitutionB);
        
        auto sumDisplacement = displacementR2 - displacementR1;

        switch (id)
        {

        case 0: // The point is inside R1 (displace 1)
            NORMAL = -normalize(displacementR1);
            break;
            case 1: // The point is inside R2 (displace 2)
            NORMAL = normalize(displacementR2);
            break;
        case 2: // EDGE VS EDGE
            NORMAL = -normalize(sumDisplacement);
            break;
        default:
            break;
        }
        
        
        
        
        
        vec3 relativeVelocity = R2->getVelocity() - R1->getVelocity();
        float j = -(1 + 0.8) * dot(relativeVelocity, NORMAL);
        j/=(R1->inverseMass + R2->inverseMass);

        std::cout << "Impulse: " << j << ", dot: " << dot(relativeVelocity, NORMAL) << " | ";
        printVec(NORMAL);

        {
            auto translation = posToTranslator(displacementR1);
            auto normalizedTranslation = normalize(displacementR1);

            if(!R1->is_static) {

                auto response = j * R1->inverseMass * NORMAL;

                R1->com = translation(R1->com);
                R1->motor = translation * R1->motor;
                R1->translator_tick = (posToTranslator(response)) * R1->translator_tick;
            }
        }

        {
            auto translation = posToTranslator(displacementR2);
            auto normalizedTranslation = normalize(displacementR2);

            if(!R2->is_static) {

                auto response = -j * R2->inverseMass * NORMAL;

                R2->com = translation(R2->com);
                R2->motor = translation * R2->motor;
                R2->translator_tick = (posToTranslator(response)) * R2->translator_tick;
            }
        }

    }

}
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
// #include <glad/glad.h>
#include <glimac/Program.hpp>
#include <glimac/FilePath.hpp>
#include <glimac/Sphere.hpp>
#include <glimac/glm.hpp>
#include <glimac/Image.hpp>
#include <glimac/FPSCamera.hpp>
#include <glimac/VertexData.hpp>
#include <glimac/Scene.hpp>
#include <glimac/BasicProgram.hpp>
#include <glimac/KeyCodes.hpp>
#include <glimac/Geometry.hpp>
#include <glimac/Instance.hpp>
#include <glimac/WindowManager.hpp>
#include <glimac/ShadowMap.hpp>
#include <glimac/Mirror.hpp>
#include <glimac/Button.hpp>
#include <glimac/Particule.hpp>
#include <glimac/Link.hpp>
#include <glimac/Field.hpp>
#include <glimac/Animation.hpp>
#include <glimac/common.hpp>
#include <glimac/PGA.hpp>
// #include <glimac/imgui.h>
// #include <glimac/GLFWHandle.hpp>

#include <klein/klein.hpp>

#include <thread>


using namespace std;
using namespace glimac;
using namespace glm;

GLuint bind_texture(glimac::FilePath path) {
    std::unique_ptr<Image> image = loadImage(path);

    GLuint out;
    glGenTextures(1, &out);
    glBindTexture(GL_TEXTURE_2D, out);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.get()->getWidth(), image.get()->getHeight(), 0, GL_RGBA, GL_FLOAT, image.get()->getPixels());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    return out;
}

void clear_screen() {
    glClearColor(.0f, .0f, .0f, 1.f);
    // glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

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

int RBCollide(rigidBody* R1, rigidBody* R2, std::vector<edgeCast> * lst, vec3* I1, vec3* I2) {
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

void updatePhysic(rigidBody* R1, rigidBody* R2, vec3& p1,  vec3& p2, vec3& displacementR1, vec3& displacementR2) {
    auto R1ContactPoint = p1 + -displacementR1;
    auto R2ContactPoint = p2 + -displacementR2;

    auto weirdNormal = normalize(displacementR2 - displacementR1);

    auto restitutionA = 1.0f;
    auto restitutionB = 1.0f;

    auto normal = p2 - p1;
    auto depth = length(normal);
    normal*= 1/depth;
    vec3 relativeVelocity = R2->getVelocity() - R1->getVelocity();
    float e = std::min(restitutionA, restitutionB);
    float j = -(1 + 0.1) * dot(relativeVelocity, weirdNormal);
    j/=(R1->inverseMass + R2->inverseMass);

    std::cout << "Impulse: " << j << ", dot: " << dot(relativeVelocity, weirdNormal) << " | ";
    printVec(normal);

    {
        auto translation = posToTranslator(displacementR1);
        auto normalizedTranslation = normalize(displacementR1);

        if(!R1->is_static) {

            auto response = j * R1->inverseMass * weirdNormal;

            R1->com = translation(R1->com);
            R1->motor = translation * R1->motor;
            R1->translator_tick = (posToTranslator(response)) * R1->translator_tick;
        }
    }

    {
        auto translation = posToTranslator(displacementR2);
        auto normalizedTranslation = normalize(displacementR2);

        if(!R2->is_static) {

            auto response = -j * R2->inverseMass * weirdNormal;

            R2->com = translation(R2->com);
            R2->motor = translation * R2->motor;
            R2->translator_tick = (posToTranslator(response)) * R2->translator_tick;
        }
    }

}

int main(int /*argc*/, char * argv[])
{

    /* Initialize the library */
    if (!glfwInit()) {
        return -1;
    }

    // Check points around line
    if(false){
        kln::line line = kln::point{0, 0, 0} & kln::point{1, 1, 0};

        kln::point P1{1, 1, 0};
        kln::point P2{2, 1, 0};
        kln::point P3{3, -1, 0};

        auto PP1 = project(P1, line);
        auto PP2 = project(P2, line);
        auto PP3 = project(P3, line);

        auto L1 = kln::project(P1 & PP1, kln::point{0, 0, 0}).normalized();
        auto L2 = kln::project(P2 & PP2, kln::point{0, 0, 0}).normalized();
        auto L3 = kln::project(P3 & PP3, kln::point{0, 0, 0}).normalized();

        printLine(L1);
        printLine(L2);
        printLine(L3);

        std::cout << "Same Side: " << linesSameDirection(L1, L2, L3) << std::endl;

        return 0;
    }
    
    // Test intersection plane and line (nan if the line is parallel)
    if(false){
        kln::plane plane = kln::point{-1, 0, 0} & kln::point{1, 0, 0} & kln::point{0, 1, 0};
        kln::line lineA = kln::point{0, -1, 0} & kln::point{0, 1, 0};

        auto intersection = (plane ^ lineA).normalized();

        printPoint(intersection);

        return 0;
    }

    if(false){
        triangle T(kln::point{-10, -10, 0}, kln::point{10, -10, 0}, kln::point{0, 10, 0});
        edge E(kln::point{0, 0, 1}, kln::point{0, 0, -1});

        // std::cout << "len edge: " << E.len << std::endl;

        edgeCast output = collideEdgeTriangle(T, E);

        std::cout << "Collided: " << output.collided << std::endl;
        printPoint(output.intersection);

        return 0;
    }

    if(false){
        kln::point P1{0, 0, 0};
        kln::point P2{1.23, 4.56, 7.89};
        auto translated = P1 * P2;
        printTranslator(translated);
        return 0;
    }


    /* Create a window and its OpenGL context */
#ifdef __APPLE__
    /* We need to explicitly ask for a 3.3 context on Mac */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
    
    WindowManager win;
    if(!win.init(1600, 900)) {
        return -1;
    }

    glfwSwapInterval(0);


    /*********************************
     * HERE SHOULD COME THE INITIALIZATION CODE
     *********************************/
    
/**/glimac::FilePath applicationPath(argv[0]);

/**/std::cout << "Compiling Shaders..." << std::endl;
/**/BasicProgram programRoom(applicationPath, "src/shaders/roomOne/ground.vs.glsl", "src/shaders/roomOne/ground.fs.glsl");
/**/BasicProgram programSky(applicationPath, "src/shaders/skybox/skybox_shader.vs.glsl", "src/shaders/skybox/skybox_shader.fs.glsl", ProgramType::TEXTURE);
/**/BasicProgram programLight(applicationPath, "src/shaders/light/light.vs.glsl", "src/shaders/light/light.fs.glsl", ProgramType::LIGHTS);
// /**/BasicProgram programVoronoi(applicationPath, "src/shaders/roomTwo/voronoi.vs.glsl", "src/shaders/roomTwo/voronoi.fs.glsl", ProgramType::LIGHTS);
/**/BasicProgram programVoronoi(applicationPath, "src/shaders/utils/normal.vs.glsl", "src/shaders/utils/normal.fs.glsl", ProgramType::LIGHTS);
/**/std::vector<BasicProgram*> allPrograms = {&programVoronoi, &programRoom, &programLight, &programSky};
/**/std::vector<BasicProgram*> allRoomTwoPrograms = {&programRoom, &programVoronoi};

/**/std::cout << "Loading Textures..." << std::endl;
/**/GLuint imageWhiteInt          = bind_texture(applicationPath.dirPath() + "/assets/textures/white.png");
/**/GLuint imageDefaultNormalInt  = bind_texture(applicationPath.dirPath() + "/assets/textures/normal.png");
/**/GLuint imageBrickDiffuseInt   = bind_texture(applicationPath.dirPath() + "/assets/textures/bricks_diffuse.jpg");
/**/GLuint imageBrickRoughnessInt = bind_texture(applicationPath.dirPath() + "/assets/textures/bricks_roughness.jpg");
/**/GLuint imageBrickNormalInt    = bind_texture(applicationPath.dirPath() + "/assets/textures/bricks_normal.jpg");
/**/GLuint imageGlassDiffuseInt   = bind_texture(applicationPath.dirPath() + "/assets/textures/glass_diffuse.png");
/**/GLuint imageGlassNormalInt    = bind_texture(applicationPath.dirPath() + "/assets/textures/glass_normal.jpg");
/**/GLuint imageSkyboxInt         = bind_texture(applicationPath.dirPath() + "/assets/textures/alpha-mayoris.jpg");
/**/std::vector<GLuint*> allTextures = {
        &imageWhiteInt,
        &imageDefaultNormalInt,
        &imageBrickDiffuseInt,
        &imageBrickRoughnessInt,
        &imageBrickNormalInt,
        &imageGlassDiffuseInt,
        &imageGlassNormalInt,
        &imageSkyboxInt
    };

/**/glimac::Sphere sphere = glimac::Sphere(1, 32, 16);
/**/glimac::Sphere sphereInverted = glimac::Sphere(-1, 32, 16);
/**/glimac::Sphere sphereLowPoly = glimac::Sphere(1, 8, 4);
/**/glimac::Sphere sphereLowPolyParticule = glimac::Sphere(1, 4, 2);

/**/std::vector<BBox3f> walls;

/**/// Add all the walls
/**/{
        const float wallThickness = 1.0f;
        walls.push_back(BBox3f(vec3(-220, -wallThickness*10, -130), vec3(220, -5, 130)));
    }

/**/std::cout << "Importing Objects..." << std::endl;
/**/auto skyboxInstances = std::make_shared<Instance>(sphereInverted.getVertexCount(), sphereInverted.getDataPointer(), imageSkyboxInt, 0, imageDefaultNormalInt);
/**/auto roomInstances = std::make_shared<Instance>(applicationPath.dirPath(), "dsda", imageBrickDiffuseInt, imageBrickRoughnessInt, imageBrickNormalInt);
/**/auto lightInstances = std::make_shared<Instance>(sphereLowPoly.getVertexCount(), sphereLowPoly.getDataPointer(), 0, 0, imageDefaultNormalInt);
/**/auto lightInstances2 = std::make_shared<Instance>(sphereLowPoly.getVertexCount(), sphereLowPoly.getDataPointer(), 0, 0, imageDefaultNormalInt);
/**/auto graphRender = std::make_shared<Instance>(sphere.getVertexCount(), sphere.getDataPointer(), imageBrickDiffuseInt, 0, imageDefaultNormalInt);
/**/auto cornerRender = std::make_shared<Instance>(sphereLowPoly.getVertexCount(), sphereLowPoly.getDataPointer(), imageSkyboxInt, 0, imageDefaultNormalInt);
/**/auto graphRenderStatic = std::make_shared<Instance>(sphereLowPolyParticule.getVertexCount(), sphereLowPolyParticule.getDataPointer(), imageDefaultNormalInt, 0, imageDefaultNormalInt);
// /**/auto simpleCube = std::make_shared<Instance>(applicationPath.dirPath(), "simpleCube2", imageGlassDiffuseInt, 0, imageGlassNormalInt);
// /**/auto simpleCube = std::make_shared<Instance>(applicationPath.dirPath(), "cylinder", imageGlassNormalInt, 0, imageGlassNormalInt);
/**/auto simpleCube = std::make_shared<Instance>(applicationPath.dirPath(), "ico", imageGlassNormalInt, 0, imageGlassNormalInt);
/**/auto edgeCollisionRender = std::make_shared<Instance>(sphere.getVertexCount(), sphere.getDataPointer(), imageGlassDiffuseInt, 0, imageGlassNormalInt);
/**/auto edgeCollisionLineRender = std::make_shared<Instance>(sphere.getVertexCount(), sphere.getDataPointer(), imageSkyboxInt, 0, imageDefaultNormalInt);
/**/auto edgeRender = std::make_shared<Instance>(sphere.getVertexCount(), sphere.getDataPointer(), imageBrickDiffuseInt, 0, imageDefaultNormalInt);

/**/Scene scene;
/**/Scene sceneProj;

/**/// Add all objects to the scene
/**/{
        scene.addInstance(roomInstances);
        sceneProj.addInstance(graphRender);
        sceneProj.addInstance(cornerRender);
        sceneProj.addInstance(graphRenderStatic);
        scene.addInstance(simpleCube);
        scene.addInstance(edgeCollisionRender);
        sceneProj.addInstance(edgeCollisionLineRender);
        sceneProj.addInstance(edgeRender);

        // edgeCollisionRender.get()->setBlendToTransparent();
        // simpleCube.get()->setBlendToTransparent();
    }

/**/// SHADERS INVARIANTS
/**/std::cout << "Placing all objects..." << std::endl;

/**/Light lightsRoomLeft;
/**/Light lightsRoomRight;

    // Add all the lights
/**/{
        const float lightSize = 0.05f;

        lightsRoomLeft.add(LightStruct(vec3(-1.25, 1.25, -3.65), vec3(1, 0.6, 0.2), vec3(7, 1, LightType::pointLight)));
        lightInstances.get()->add(Transform(vec3(-1.25, 1.25, -3.65), vec3(0), vec3(lightSize)));

        lightsRoomLeft.add(LightStruct(vec3(-13, 0.25, 3.4), vec3(1, 0.1, 0.1), vec3(7, 1, LightType::pointLight)));
        lightInstances.get()->add(Transform(vec3(-13, 0.25, 3.4), vec3(0), vec3(lightSize)));

        lightsRoomLeft.add( LightStruct(vec3(-10, 1, 0), vec3(0.2, 1, 0.5), vec3(5, 1, LightType::pointLight)));
        lightInstances.get()->add(Transform(vec3(-10, 1, 0), vec3(0), vec3(lightSize)));

        lightsRoomLeft.add( LightStruct(vec3(-0.5, 2, -1.85), vec3(1, 0.7, 0.4), vec3(3, 0.25, LightType::pointLight)));
        lightsRoomLeft.add( LightStruct(vec3(0.5, 2, -1.85), vec3(0.7, 0.4, 1), vec3(3, 0.25, LightType::pointLight)));
        lightsRoomLeft.add( LightStruct(vec3(-0.5, 2, 1.85), vec3(1, 0.7, 1), vec3(3, 0.25, LightType::pointLight)));
        lightsRoomLeft.add( LightStruct(vec3(0.5, 2, 1.85), vec3(0.4, 0.4, 0.9), vec3(3, 0.25, LightType::pointLight)));
        lightInstances.get()->add(Transform(vec3(0.5, 2, -1.85), vec3(0), vec3(lightSize)));
        lightInstances.get()->add(Transform(vec3(-0.5, 2, -1.85), vec3(0), vec3(lightSize)));
        lightInstances.get()->add(Transform(vec3(0.5, 2, 1.85), vec3(0), vec3(lightSize)));
        lightInstances.get()->add(Transform(vec3(-0.5, 2, 1.85), vec3(0), vec3(lightSize)));

        lightsRoomLeft.add( LightStruct(vec3(-16.4192, 10.25, 3.26514), vec3(1.0, 0.3, 0.2), vec3(10, 1, LightType::pointLight)));
        lightInstances.get()->add(Transform(vec3(-16.4192, 10.25, 3.26514), vec3(0), vec3(lightSize)));

        lightsRoomLeft.add( LightStruct(vec3(-16.4192, 10.25, -3.26514), vec3(0.3, 1.0, 0.2), vec3(10, 1, LightType::pointLight)));
        lightInstances.get()->add(Transform(vec3(-16.4192, 10.25, -3.26514), vec3(0), vec3(lightSize)));

        lightsRoomLeft.add( LightStruct(vec3(-9.85781, 10.25, -3.26514), vec3(0.2, 0.3, 1.0), vec3(10, 1, LightType::pointLight)));
        lightInstances.get()->add(Transform(vec3(-9.85781, 10.25, -3.26514), vec3(0), vec3(lightSize)));

        lightsRoomLeft.add( LightStruct(vec3(-9.85781, 10.25, 3.26514), vec3(0.8, 0.8, 1.0), vec3(10, 1, LightType::pointLight)));
        lightInstances.get()->add(Transform(vec3(-9.85781, 10.25, 3.26514), vec3(0), vec3(lightSize)));

        lightsRoomLeft.add(LightStruct(vec3(-10.5, 2.5, -11.25), vec3(0.8, 0.9, 1), vec3(15, 1, LightType::pointLight)));
        lightInstances.get()->add(Transform(vec3(-10.5, 2.5, -11.25), vec3(0), vec3(lightSize)));

        for (int y = 1; y < 3; y++) {
            for (int x = -20; x < -1; x++) {
                lightsRoomLeft.add(LightStruct(vec3(x, y, 11.5), vec3(-x/20.0, y/2.0, 1), vec3(4, 0.4, lightSize)));
                lightInstances.get()->add(Transform(vec3(x, y, 11.5), vec3(0), vec3(lightSize)));
            }
        }

        int nbRandLight = 15;

        for (int i = 0; i < nbRandLight; i++) {
            vec3 pos = vec3(linearRand(1.0f, 20.0f), linearRand(.1f, 1.0f), linearRand(-11.0f, 11.0f));
            pos = vec3(pos.x, pos.y/20, pos.z);
            vec3 color = vec3(linearRand(.1f, 1.0f), linearRand(.1f, 1.0f), linearRand(.1f, 1.0f));
            lightsRoomRight.add(LightStruct(pos, color, vec3(5, 1, lightSize)));
            lightInstances2.get()->add(Transform(pos, vec3(0), vec3(lightSize)));
        }
    }

/**/std::cout << "Initializing variables..." << std::endl;

/**/unsigned int currentRoom = 0;
/**/BasicProgram *currentProgram = allRoomTwoPrograms.at(currentRoom);


/**/double oldTime = -1.0f;
/**/double deltaT = 0;
/**/bool animateSwitch = false;
/**/double timer = 0.0f;
/**/double animateTimer = 0.0f;
/**/bool computeNextFrame = false;
/**/bool boolRightRoom = false;
/**/vec2 mousePos = win.mouse();

/**/glm::mat4 shadowMatrix;

/**/vec3 currentCamPos;
/**/FPSCamera fpsCam = FPSCamera(win.width(), win.height());

/**/auto startPoint = vec3(0, 0.25, 0);
/**/auto startLookPoint = vec3(0, 0.25, -1);
/**/fpsCam.makeLookAt(startPoint, startLookPoint);
/**/fpsCam.update(win, walls, deltaT);
/**/fpsCam.resetMouse(mousePos);

/**/
/**/ShadowMap shadowMap(applicationPath);
/**/shadowMap.init();

/**/shadowMap.computeTransforms(LightStruct(vec3(40, 40, -40), vec3(1), vec3(100, 0.85, LightType::directionalLight)));

/**/GLuint depthMapId;
/**/depthMapId = shadowMap.getDepthMap();
/**/shadowMatrix = shadowMap.getShadowMatrix();
/**/shadowMap.renderTexture(win, scene);
/**/vec3 sunPos = shadowMap.getLightPos();

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

    auto resolution = 15;
    auto maxLen = 1.0f;
    auto min = -maxLen;
    auto max = maxLen;
    auto range = max-min;
    auto offset = range / resolution;



    // auto ax = -30.f*degToRad;
    // auto ay = 45.f*degToRad;
    // auto az = 7.0f*degToRad;

    // auto ax2 = 60.f*degToRad;
    // auto ay2 = 40.f*degToRad;
    // auto az2 = 110.0f*degToRad;
    // auto ax = -45.f*degToRad;
    // auto ay = 90.f*degToRad;
    // auto az = 0.f*degToRad;
    auto ax = 0.f*degToRad;
    auto ay = 0.f*degToRad;
    auto az = 0.f*degToRad;

    // auto ax2 = 45.f*degToRad;
    // auto ay2 = 0.f*degToRad;
    // auto az2 = 36.6f*degToRad;
    auto ax2 = 0.f*degToRad;
    auto ay2 = 0.f*degToRad;
    auto az2 = 0.f*degToRad;
    

    
    // // could be optimized with the length as 1.0 and x/y/z as the position
    // // or not since there is a disvision by the length of x/y/z



    rigidBody rb;
    rigidBody rb2;

    // rb.com = vecToPoint(pos);
    auto rb1Pos = vec3(0.0, 0.0, 0.0);
    auto rb1Motor = posToTranslator(rb1Pos) * anglesToRotor(ax, ay, az);
    rb.setMass(1000.0f);
    rb.setStatic(true);
    rb.setMotor(rb1Motor);
    simpleCube.get()->getVertex(&rb.points);
    simpleCube.get()->getEdgesIndex(&rb.edges_index);
    simpleCube.get()->getTrianglesIndex(&rb.triangles_index);
    simpleCube.get()->generateTriangles(&rb.triangles_from_Instance);
    simpleCube.get()->generateEdges(&rb.edges_from_Instance);
    rb.reduceAll();
    rb.computeAll();

    std::cout << "Nb points for rb: " << rb.points.size() << " computed: " << rb.points_computed.size() << " edges: " << rb.edges_computed.size() << std::endl;

    // rb2.com = vecToPoint(pos+vec3(0, 1.3, 0));
    auto rb2Pos = vec3(0.0, 4.0, 0.25);
    auto rb2Motor = posToTranslator(rb2Pos) * anglesToRotor(ax2, ay2, az2);
    rb2.setMass(1.0f);
    rb2.setStatic(true);
    rb2.setMotor(rb2Motor);
    // rb2.translator_tick = posToTranslator(0.0f, -2.0f, 0.0f);
    // rb2.motor = kln::motor(posToTranslator(0.0f, -0.1f, 0.0f));
    simpleCube.get()->getVertex(&rb2.points);
    simpleCube.get()->getEdgesIndex(&rb2.edges_index);
    simpleCube.get()->getTrianglesIndex(&rb2.triangles_index);
    simpleCube.get()->generateTriangles(&rb2.triangles_from_Instance);
    simpleCube.get()->generateEdges(&rb2.edges_from_Instance);
    rb2.reduceAll();
    rb2.computeAll();

    std::cout << "Nb points for rb2: " << rb2.points.size() << " computed: " << rb2.points_computed.size() << " edges: " << rb2.edges_computed.size() << std::endl;

    // std::vector<triangle> cubeTriangles;
    // simpleCube.get()->getTrianglesIndex(&cubeTriangles);

    simpleCube.get()->add(Transform(rb1Pos, vec3(ax, ay, az)));
    simpleCube.get()->add(Transform(rb2Pos, vec3(ax2, ay2, az2)));

    // for(auto &point: rb.points_computed) {
    //     cornerRender.get()->add(Transform(pointToVec(point), vec3(0), vec3(0.0625)));
    // }

    // for(auto &point: rb2.points_computed) {
    //     cornerRender.get()->add(Transform(pointToVec(point), vec3(0), vec3(0.0625)));
    // }

    // Add all objects positions
    {
        roomInstances.get()->add(Transform(vec3(0, -5, 0)));
        skyboxInstances.get()->add(Transform(vec3(0), vec3(135*degToRad, -105*degToRad, 15*degToRad), vec3(5000)));

        // for (float z = max; z >= min; z=z-offset)
        // {
        //     for (float x = min; x <= max; x=x+offset) {
        //         graphRender.get()->add(Transform(vec3(x, 0, z), vec3(0, 45*degToRad, 0), vec3((1.0/cos(45*degToRad))*(offset/2.0))));
        //     }
        // }
        // for (float z = max; z >= min; z=z-offset)
        // {
        //     for (float x = min; x <= max; x=x+offset) {
        //         graphRenderStatic.get()->add(Transform(vec3(x, 0, z), vec3(0, 45*degToRad, 0), vec3(0.00625f)));
        //     }
        // }
    }

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// PARTICULES

auto firstGrid = Animation(sphereLowPolyParticule.getVertexCount(), sphereLowPolyParticule.getDataPointer(), imageWhiteInt, 0, imageDefaultNormalInt);
scene.addInstance(firstGrid.getInstance());
// firstGrid.make_grid(vec3(-6, 12, -6), vec3(6, 12, -6), vec3(-6, 12, 6), vec3(6, 12, 6), 1, 3.0, 100, 1);
// firstGrid.make_point(vec3(0, 6, 0), 1.0);
float radiusCube = 1.0f;
float height = 1.5f;
// firstGrid.make_grid(vec3(-radiusCube, height, -radiusCube), vec3(radiusCube, height, -radiusCube), vec3(-radiusCube, height, radiusCube), vec3(radiusCube, height, radiusCube), 25, 1.0, 10000, 50);
firstGrid.make_grid(vec3(-radiusCube, height, -radiusCube), vec3(radiusCube, height, -radiusCube), vec3(-radiusCube, height, radiusCube), vec3(radiusCube, height, radiusCube), 15, 1.0, 1000, 50);
firstGrid.addField(FieldType::field_directional, vec3(0, -1, 0), 9.81);
// firstGrid.addField(FieldType::field_directional, vec3(0, -1, 0), 1.0f);
const float wallFriction = 0.68f;

firstGrid.addField(FieldType::field_convex, &rb, wallFriction);

firstGrid.addField(FieldType::field_cube, BBox3f(vec3(-50, -15, -50), vec3(50, -5, 50)), wallFriction);

// const float radiusCube = 3.0f;
// firstGrid.addField(FieldType::field_cube, BBox3f(vec3(-radiusCube, -radiusCube+6, -radiusCube), vec3(radiusCube, radiusCube+6, radiusCube)), wallFriction);

// player BBox
// firstGrid.addField(FieldType::field_cube, BBox3f(), 0.1);


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

uint nb_threads = 11;
volatile bool killThreads = false;
volatile bool computeAnim = false;
volatile double animFrameTime = 0.0;
volatile auto deltaTThreads = 0.010f;
std::vector<ThreadProcessState> threadStates;
std::vector<ThreadProcessComparaison> threadComparaison;
std::vector<std::thread> threadList;

auto animUpdateLambda = [&] (int index, Animation* animation) {
    uint startLinksIndex = (1.0f*index/nb_threads) * animation->getLinksCount();
    uint endLinksIndex = (1.0f*(index+1)/nb_threads) * animation->getLinksCount();

    uint startParticuleIndex = (1.0f*index/nb_threads) * animation->getParticulesCount();
    uint endParticuleIndex = (1.0f*(index+1)/nb_threads) * animation->getParticulesCount();

    threadStates.at(index) = NothingDone;
    threadComparaison.at(index) = Compute_1;

    auto canStartNextStep = false;
    auto canUpdateParticule = false;

    auto timer = 0.0f;
    auto oldTime = 0.0f;

    while(!killThreads) {
        // This block should wait for the other threads to have finished their computation
        canStartNextStep = true;
        switch (threadComparaison.at(index))
        {
        case Compute_1:
            
            for(auto state: threadComparaison) {
                if(state == Compute_3) {
                    canStartNextStep = false;
                    break;
                }
            }
            break;
        case Compute_2:
            for(auto state: threadComparaison) {
                if(state == Compute_1) {
                    canStartNextStep = false;
                    break;
                }
            }
            break;
        case Compute_3:
            for(auto state: threadComparaison) {
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

        if(computeAnim) {
            // std::cout<<"Computing"<<std::endl;
            timer = glfwGetTime();
            threadStates.at(index) = NothingDone;

            // animation->getFields()->back().make_cube(fpsCam.getBBox(), 0);
            animation->update_links(startLinksIndex, endLinksIndex);
            threadStates.at(index) = LinksDone;

            while(!killThreads) {
                
                canUpdateParticule = true;
                for (size_t i = 0; i < threadStates.size(); i++) {
                    if(threadStates.at(i) == NothingDone) {
                        canUpdateParticule = false;
                        break;
                    }
                }   
                if(canUpdateParticule) {
                    break;
                }
            }
            if(killThreads) {
                return;
            }

            animation->update_particules(deltaTThreads, startParticuleIndex, endParticuleIndex);
            // animation->update_particules(0.0001, startParticuleIndex, endParticuleIndex);
            threadStates.at(index) = ParticulesDone;


            // tell the other thread that it has finished computing this frame
            switch (threadComparaison.at(index))
            {
            case Compute_1:
                threadComparaison.at(index) = Compute_2;
                break;
            case Compute_2:
                threadComparaison.at(index) = Compute_3;
                break;
            case Compute_3:
                threadComparaison.at(index) = Compute_1;
                break;
            }
            if(index == 0) {
                // std::cout << 1.0/(glfwGetTime() - timer) << std::endl;
                // std::cout << "deltaTThreads " << deltaTThreads << " nb links: " << animation->getLinksCount() << std::endl;
                animFrameTime = glfwGetTime() - timer;
                // animFrameTime = glfwGetTime() - oldTime;
                deltaTThreads = animFrameTime;
                // deltaTThreads = 0.1;
            }
        }

        while(oldTime+deltaTThreads > glfwGetTime()){}
        oldTime = glfwGetTime();
    }
};

for (uint i = 0; i < nb_threads; i++) {
    threadStates.push_back(NothingDone);
    threadComparaison.push_back(Compute_1);
    threadList.push_back(std::thread(animUpdateLambda, i, &firstGrid));
}

computeAnim = false;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**/std::cout << "Main loop" << std::endl;

// // GUI code:
// imguiNewFrame();
// {
//     ImGui::Begin("GUI");
//     ImGui::Text("Application");
//     if (ImGui::CollapsingHeader("Camera", 32)) {
//     ImGui::Text("nice");
//     }
//     if (ImGui::CollapsingHeader(
//             "Directional Light", 32)) {
//     }
//     if (ImGui::CollapsingHeader(
//             "Render Options", 32)) {
//     }
//     ImGui::End();
// }
// imguiRenderFrame();

/**//* Loop until the user closes the window */
    uint refreshTitle = 0;
    while (win.running()) {

        { // INIT CODE
            clear_screen();
            fpsCam.update(win, walls, deltaT);
            timer = glfwGetTime();
            deltaT = std::min(timer - oldTime, 1.0/24);
            if(oldTime < 0) { // first frame
                deltaT = 1/60.0f;
            }
            if (animateSwitch || computeNextFrame) { // the animation is running
                animateTimer += deltaT;
            }
            oldTime = timer;
            currentCamPos = fpsCam.getPos();
        }

        { // UPDATES

            firstGrid.update_visual();

            if(animateSwitch || computeNextFrame) {

                // auto ppp1 = rb.getVelocity();
                // // printVec(ppp);
                // auto ppp2 = rb2.getVelocity();
                // // printVec(ppp);

                // std::cout << "{ " << ppp1.x << ", " << ppp1.y << ", " << ppp1.z << " } vector" << "{ " << ppp2.x << ", " << ppp2.y << ", " << ppp2.z << " } vector" << std::endl;
                

                auto scaleTime = 1.0f;

                ax = 30.f*degToRad*sin(animateTimer*scaleTime)*3.5;
                az = 0.0f*degToRad;
                ay = 15.f*degToRad*animateTimer*scaleTime;

                rb1Pos = vec3(-0.5+0.5*cos(animateTimer*scaleTime), 0.0, 0.5*sin(animateTimer*scaleTime));

                rb1Motor = posToTranslator(rb1Pos) * anglesToRotor(ax, ay, az);
                // simpleCube.get()->generateTriangles(&rb.triangles_from_Instance);


                rb.setMotor(rb1Motor);
                // rb.updateMotor(deltaT);
                rb.computeAll();

                // simpleCube.get()->generateTriangles(&rb2.triangles_from_Instance);
                rb2.updateMotor(deltaT);
                rb2.computeAll();

                // auto count = collideRigidBody(&rb, &rb2);
            }

            vec3 centerCollisionRB1;
            vec3 centerCollisionRB2;
            std::vector<edgeCast> edgeCollision;
            // auto nbIntersections = RBCollide(&rb, &rb2, &edgeCollision, &centerCollisionRB1, &centerCollisionRB2);
            auto nbIntersections = 0;
            edgeCollisionRender.get()->clear();
            edgeCollisionLineRender.get()->clear();

            if(false && (animateSwitch || computeNextFrame)) {
                if(nbIntersections>0) {

                    auto displace1 = resolveInternalPoint(&rb, centerCollisionRB1, false);
                    auto displace2 = resolveInternalPoint(&rb2, centerCollisionRB2, false);
                    updatePhysic(&rb, &rb2, centerCollisionRB1, centerCollisionRB2, displace1, displace2);
                    edgeCollisionRender.get()->add(Transform(displace1 + centerCollisionRB1, vec3(0), vec3(0.300)));
                    edgeCollisionRender.get()->add(Transform(displace2 + centerCollisionRB2, vec3(0), vec3(0.300)));
                }
            }


            if(nbIntersections>0) {
                // edgeCollisionRender.get()->add(Transform(centerCollision, vec3(0), vec3(0.125)));
                // edgeCollisionRender.get()->add(Transform(centerCollision, vec3(0), vec3(0.125)));

                // std::cout << "RB2: "<< std::endl;



                // for(auto &p: edgeCollision) {
                //     auto contactPoint = pointToVec(p.intersection);
                //     // edgeCollisionRender.get()->add(Transform(contactPoint, vec3(0), vec3(0.125)));
                //     // std::cout << "Pos intersection: { " << contactPoint.x << ", " << contactPoint.y << ", " << contactPoint.z << " }" << std::endl;
                //     size_t count = 10;
                //     auto len = length(pointToVec(p.e.p2) - pointToVec(p.e.p1));
                //     for (size_t i = 0; i <= count; i++)
                //     {
                //         auto point = (1.0f*(count - i)/(count)) * (p.e.p1) + (1.0f*(i)/(count)) * (p.e.p2);
                //         edgeCollisionLineRender.get()->add(Transform(pointToVec(point), vec3(0), vec3(0.5f*len/count)));
                //     }                    
                // }
            }

                                                                                    // graphRenderStatic.get()->clear();
                                                                                    // graphRenderStatic.get()->add(Transform(pointToVec(rb.com), vec3(0), vec3(0.25)));
                                                                                    // graphRenderStatic.get()->add(Transform(pointToVec(rb2.com), vec3(0), vec3(0.25)));


            // if(animateSwitch) {
                // std::cout << "Collision: " << count << std::endl;
                // std::cout << "Nb collisions: " << rb.collisions.size() << ", " << rb2.collisions.size() << std::endl;

                // rb.collisions.clear();
                // rb2.collisions.clear();

                // rb.resolveCollisons();
                // rb2.resolveCollisons();

                // printMotor(rb2.motor);



            // }

            simpleCube.get()->updatePosition(0, pointToVec(rb.com));
            simpleCube.get()->updateAngles(0, vec3(ax, ay, az));
            simpleCube.get()->computeAll();

            simpleCube.get()->updatePosition(1, pointToVec(rb2.com));
            // simpleCube.get()->updateAngles(1, vec3(0));
            simpleCube.get()->compute(1);

            // debug render
            if(false) {

                edgeRender.get()->clear();
                for(auto &e: rb.edges_computed) {
                    size_t count = 20;
                    auto len = length(pointToVec(e.p2) - pointToVec(e.p1));
                    for (size_t i = 0; i <= count; i++)
                    {
                        auto point = (1.0f*(count - i)/(count)) * (e.p1) + (1.0f*(i)/(count)) * (e.p2);
                        edgeRender.get()->add(Transform(pointToVec(point), vec3(0), vec3(0.5f*len/count)));
                    } 
                }
                for(auto &e: rb2.edges_computed) {
                    size_t count = 20;
                    auto len = length(pointToVec(e.p2) - pointToVec(e.p1));
                    for (size_t i = 0; i <= count; i++)
                    {
                        auto point = (1.0f*(count - i)/(count)) * (e.p1) + (1.0f*(i)/(count)) * (e.p2);
                        edgeRender.get()->add(Transform(pointToVec(point), vec3(0), vec3(0.5f*len/count)));
                    } 
                }
                // std::cout << "COM: " << pointToVec(rb.com) << ", " << pointToVec(rb2.com) << std::endl;

                auto indexCorner = 0;
                for(auto &point: rb.points_computed) {
                    cornerRender.get()->updatePosition(indexCorner, pointToVec(point));
                    indexCorner++;
                }

                for(auto &point: rb2.points_computed) {
                    cornerRender.get()->updatePosition(indexCorner, pointToVec(point));
                    indexCorner++;
                }
                cornerRender.get()->computeAll();
            }


            if(false && (animateSwitch || computeNextFrame)) {
                auto indexGraph = 0;

                for (float z = max; z >= min; z=z-offset)
                {
                    for (float x = min; x <= max; x=x+offset) {
                        auto transform = graphRender.get()->get(indexGraph);
                        auto pos = vec3(transform.m_Position);
                        pos = vec3(x, 0.12, z);
                        auto velocity = vec3(transform.m_Velocity);
                        velocity = vec3(0);
                        auto point = kln::point(pos.x, pos.y, pos.z);

                        float smallestDistance;
                        bool firstProjection = true;
                        vec3 displacement;

                        for(auto &tr: rb.triangles_computed) {
                            auto result = projectToTriangle(tr, point);

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
                        
                        velocity += displacement;
                        pos += velocity;

                        transform.m_Position = pos;
                        transform.m_Velocity = velocity;
                        graphRender.get()->update(transform, indexGraph);
                        indexGraph++;
                    }
                }   
                graphRender.get()->computeAll();
            }

            

            if(depthMapId != 0) {
                shadowMap.renderTexture(win, scene);
            }


            computeNextFrame = false;
        }

        { // RENDERING

            {

                programSky.activate(win, fpsCam, shadowMatrix, lightsRoomRight, sunPos);
                glCullFace(GL_FRONT);
                skyboxInstances.get()->drawAll(programSky, fpsCam.getViewMatrix(), fpsCam.getProjMatrix(), 0);
                glCullFace(GL_BACK);

                if(currentCamPos.x < -0.1) {
                    boolRightRoom = false;
                }
                if(currentCamPos.x > 0.1) {
                    boolRightRoom = true;
                }
                
                if(boolRightRoom) {
                    scene.drawScene(win, *currentProgram, fpsCam, shadowMatrix, lightsRoomRight, depthMapId, sunPos);
                    sceneProj.drawScene(win, programSky, fpsCam, shadowMatrix, lightsRoomRight, depthMapId, sunPos);
                    programLight.activate(win, fpsCam, shadowMatrix, lightsRoomRight, sunPos);
                    lightInstances2.get()->drawAll(programLight, fpsCam.getViewMatrix(), fpsCam.getProjMatrix(), depthMapId);
                }
                else {
                    scene.drawScene(win, programRoom, fpsCam, shadowMatrix, lightsRoomLeft, depthMapId, sunPos);
                    sceneProj.drawScene(win, programSky, fpsCam, shadowMatrix, lightsRoomLeft, depthMapId, sunPos);
                    programLight.activate(win, fpsCam, shadowMatrix, lightsRoomLeft, sunPos);
                    lightInstances.get()->drawAll(programLight, fpsCam.getViewMatrix(), fpsCam.getProjMatrix(), depthMapId);
                }
            }

        }

        /* Swap front and back buffers */
        win.display();

        auto keys = win.events();

        /* Poll for and process events */
        if (win.isFocused()) { // EVENTS
            if (keys & keyTab) {
                win.mouseCapture(!win.isMouseCaptured());
                mousePos = win.mouse();
                fpsCam.resetMouse(mousePos);
            }
            
            if(win.isMouseCaptured()) {
                win.hideCursor();
                mousePos = win.mouse();
            }
            else {
                win.showCursor();
            }

            fpsCam.updateKeys(keys, mousePos, deltaT);

            if (keys & scrollDown) {
                currentRoom = (currentRoom+1) % allRoomTwoPrograms.size();
                currentProgram = allRoomTwoPrograms.at(currentRoom);
            }

            if (keys & scrollUp) {
                animateSwitch = !animateSwitch;
                computeAnim = !computeAnim;
            }

            if (keys & keyNextFrame) {
                computeNextFrame = true;
            }

            
            if (keys & keySun) {
                if (depthMapId != 0) {
                    depthMapId = 0;
                }
                else {
                    depthMapId = shadowMap.getDepthMap();
                }
            }
            
            if (keys & keyDebug) {
                firstGrid.reset();
                // auto indexGraph = 0;
                // for (float z = max; z >= min; z=z-offset)
                // {
                //     for (float x = min; x <= max; x=x+offset) {
                //         auto transform = graphRender.get()->get(indexGraph);
                //         transform.m_Position = vec3(x, 0.12, z);
                //         transform.m_Velocity = vec3(0);
                //         graphRender.get()->update(transform, indexGraph);
                //         // graphRender.get()->updatePosition(indexGraph, pos);
                //         indexGraph++;
                //     }
                // }
                // graphRender.get()->computeAll();


                rb.setMotor(rb1Motor);
                rb.computeAll();
                rb2.setMotor(rb2Motor);
                // rb2.translator_tick = posToTranslator(0.0f, -2.0f, 0.0f);
                rb2.computeAll();

                simpleCube.get()->updatePosition(1, pointToVec(rb2.com));
                simpleCube.get()->updateAngles(1, vec3(ax2, ay2, az2));
                simpleCube.get()->compute(1);
            }

            if (keys & switchMode) {
                GLint polygonMode[2];
                glGetIntegerv(GL_POLYGON_MODE, polygonMode);
                if(polygonMode[1] == GL_LINE) {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                }
                else {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                }
                glGetIntegerv(GL_POLYGON_MODE, polygonMode);
                std::cout << "Switched to " << polygonMode[1] << std::endl;
            }
            

        }
        else {
            win.mouseCapture(false);
            win.showCursor();
            win.flushKeys();
            fpsCam.resetMouse(mousePos);
        }

        if(refreshTitle == 10) {
            // std::cout << 1.0f/deltaT << std::endl;
            refreshTitle = 0;
            win.updateTitle(pointToVec(rb2.com), deltaT, deltaTThreads);
        }
        else {
            refreshTitle++;
        }
    }

    killThreads = true;
    for (uint i = 0; i < threadList.size(); i++)
    {
        threadList.at(i).join();
    }
    std::cout << "Thread joined" << std::endl;

    skyboxInstances.get()->~Instance();
    lightInstances.get()->~Instance();
    lightInstances2.get()->~Instance();
    scene.~Scene();
    sceneProj.~Scene();

    for(auto prgm : allPrograms) {
        prgm->~BasicProgram();
    }

    shadowMap.~ShadowMap();

    for(auto texture : allTextures) {
        glDeleteTextures(1, texture);
    }

    win.close();

    std::cout << "Programm Finished" << std::endl;
    return 0;
}

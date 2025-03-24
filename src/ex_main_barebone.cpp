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

struct collision
{
    kln::point location;
    vec3 offset;
    float distance = 0.0;
    bool collided;
    collision():location(kln::point(0, 0, 0)), offset(vec3(0)), collided(false){}
    collision(bool collide):location(kln::point(0, 0, 0)), offset(vec3(0)), collided(collide){}
    collision(kln::point loc, vec3 off, float d, bool collide):location(loc), offset(off), distance(d), collided(collide) {}
};

bool geometryToTriangles(Geometry& g, std::vector<triangle> lst) {
    auto vc = g.getVertexCount();
    auto vb = g.getVertexBuffer();
    auto ic = g.getIndexCount();
    auto ib = g.getIndexBuffer();
    auto mc = g.getMeshCount();
    auto mb = g.getMeshBuffer();

    // vb[0].
    // ib[0]
    // mb[0].

}

float sameSign(float a, float b, float c) {
    return (a >= 0 && b >= 0 && c >= 0) || (a <= 0 && b <= 0 && c <= 0);
    // return (a > 0 && b > 0 && c > 0) || (a < 0 && b < 0 && c < 0);
}

float sameSign(float a, float b) {
    return (a >= 0 && b >= 0) || (a <= 0 && b <= 0);
    // return (a > 0 && b > 0) || (a < 0 && b < 0);
}

float sameSignMotor(kln::motor& m1, kln::motor& m2, kln::motor& m3) {
    return
    sameSign(m1.e12(), m2.e12(), m3.e12()) && 
    sameSign(m1.e13(), m2.e13(), m3.e13()) &&
    sameSign(m1.e23(), m2.e23(), m3.e23());
}

vec3 translatorToVec3(kln::translator p) {
    float out[4];
    _mm_store_ps(out, p.p2_);
    return vec3(out[1], out[2], out[3]);
}

collision projectToTriangle(triangle& t, kln::point& p) {
    auto plane = (t.p1 & t.p2 & t.p3);
    auto projectedPoint = kln::project(p, plane).normalized();
    if((plane ^ p).e0123() > 0) {
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
        // return collision(false);
    }

    return collision(projectedPoint, offset, length(offset), true);
}

collision collide(triangle& t, kln::point& p, bool showData) {
    // kln::point zero{0, 0, 0};
    // auto plane = (t.p1 & t.p2 & t.p3).normalized();
    // auto dual = (p ^ plane);
    // auto distance = dual.e0123();
    auto l12 = (t.p1 & t.p2).normalized();
    auto plane = (t.p1.normalized() & t.p2.normalized() & t.p3.normalized()).normalized();
    auto distance = (plane ^ p).e0123();
    auto pp = kln::project(p, plane).normalized();

    // auto diff = (p * pp);

    // auto trans = pp.normalized();
    // float out[4];
    // _mm_store_ps(out, trans.p3_);
    // auto diffLength = vec3(out[1], out[2], out[3]);
    // vec3 outVector = vec3(trans.x(), trans.y(), trans.z());
    // if(distance != length(diffLength)) {
    //     std::cout << "diff length: " << distance << " != " << length(diffLength) << std::endl;
    // }
    // return collision(pp, diffLength, distance, true);
    // return collision(pp, vec3(out[1], out[2], out[3]), distance, true);
    if (distance >= 0.0) {
        // std::cout << "NOP 1" << std::endl;
        return collision(false);
    }
    // return sameSign(((t.p1 & t.p2) ^ (t.p1 & p)).scalar(), ((t.p2 & t.p3) ^ (t.p2 & p)).scalar(), ((t.p3 & t.p1) ^ (t.p3 & p)).scalar());


    // auto m1 = (l1 * (t.p1 & pp).normalized());
    // auto m2 = ((t.p2 & t.p3).normalized() * (t.p2 & pp).normalized());
    // auto m3 = ((t.p3 & t.p1).normalized() * (t.p3 & pp).normalized());


    // auto d1 = (l1 ^ (t.p1 & pp).normalized());
    // auto d2 = ((t.p2 & t.p3).normalized() ^ (t.p2 & pp).normalized());
    // auto d3 = ((t.p3 & t.p1).normalized() ^ (t.p3 & pp).normalized());

    // auto t1 = ((t.p2 - t.p1).normalized() * (pp - t.p1).normalized());
    // auto t2 = ((t.p3 - t.p2).normalized() * (pp - t.p2).normalized());
    // auto t3 = ((t.p1 - t.p3).normalized() * (pp - t.p3).normalized());

    // auto l12 = (t.p1 & t.p2).normalized();
    auto l13 = (t.p1 & t.p3).normalized();
    auto l23 = (t.p2 & t.p3).normalized();

    auto k11 = l12 | l13;
    auto k12 = l12 | (t.p1 & pp).normalized();

    auto k21 = l23 | l12.inverse();
    auto k22 = l23 | (t.p2 & pp).normalized();

    auto k31 = l13 | l23;
    auto k32 = l13 | (pp & t.p3).normalized();

    // auto k11 = (t.p1 & t.p2).normalized() | (t.p1 & t.p3).normalized();
    // auto k12 = (t.p1 & t.p2).normalized() | (t.p1 & pp).normalized();
    // auto k21 = (t.p2 & t.p3).normalized() | (t.p2 & t.p1).normalized();
    // auto k22 = (t.p2 & t.p3).normalized() | (t.p2 & pp).normalized();
    // auto k31 = (t.p1 & t.p3).normalized() | (t.p2 & t.p3).normalized();
    // auto k32 = (t.p1 & t.p3).normalized() | (pp & t.p3).normalized();

    if(showData) {
        // printMotor(m1);
        // printMotor(m2);
        // printMotor(m3);

        // printDual(d1);
        // printDual(d2);
        // printDual(d3);

        // printTranslator(t1);
        // printTranslator(t2);
        // printTranslator(t3);

        
        printFloat(k11);
        printFloat(k12);
        printFloat(k21);
        printFloat(k22);
        printFloat(k31);
        printFloat(k32);
    }

    if(k11 > k12 && k21 > k22 && k31 > k32) {
        // auto pp = kln::project(p, plane).normalized();

        auto plane = (t.p1 & t.p2 & t.p3);
        auto projectedPoint = kln::project(p, plane).normalized();

        auto diff = (p * projectedPoint);
        float out[4];
        _mm_store_ps(out, diff.p2_);
        auto offset = vec3(out[1], out[2], out[3]);
        return collision(projectedPoint, offset, length(offset), true);

        // auto diff = (p * pp);
        // float out[4];
        // _mm_store_ps(out, diff.p2_);
        // return collision(pp, vec3(out[1], out[2], out[3]), distance, true);
    }

    // if(sameSignMotor(m1, m2, m3)) {
    // }


    // auto v1 = (l1 | (t.p1 & p).normalized());
    // auto v2 = ((t.p2 & t.p3).normalized() | (t.p2 & p).normalized());
    // if(sameSign(v1, v2)) {
    //     auto v3 = ((t.p3 & t.p1).normalized() | (t.p3 & p).normalized());
    //     // if(sameSign(v1, v2, v3)) {
    //     // }
    //         auto pp = kln::project(p, plane).normalized();
    //         auto diff = (p * pp);
    //         float out[4];
    //         _mm_store_ps(out, diff.p2_);
    //         // std::cout << "YEP" << std::endl;
    //         if(showData) {
    //             std::cout << "v1:" << v1 << " v2:" << v2 << " v3:" << v3 << std::endl;
    //         }

    //         return collision(pp, vec3(out[1], out[2], out[3]), distance, true);
    //         // return collision(pp, vec3(diff.x(), diff.y(), diff.z()), distance, true);
    // }
    // if(sameSign(, , ((t.p3 & t.p1) | (t.p3 & p)))) {
    //     // auto pp = project(p, plane).normalized();
    //     // auto diff = p - pp;
    //     // auto output = length(vec3(diff.x(), diff.y(), diff.z()));
    //     // if(display) {
    //     //     printFloat(distance);
    //     // }
    //     // return output;
    //     return distance;
    // }
    // std::cout << "NOP 2" << std::endl;
    return collision(false);

    // slower
    // if (signedVolume(t.p1, t.p2, t.p3, p) != 0) {
    //     return false;
    // }

    // auto pp = project(p, plane);
    // auto v1 = (t.p1 & t.p2) ^ (t.p1 & p);
    // auto v2 = (t.p2 & t.p3) ^ (t.p2 & p);
    // auto v3 = (t.p3 & t.p1) ^ (t.p3 & p);
    // // printDual(v1);
    // // printDual(v2);
    // // printDual(v3);
    // return sameSign(v1.scalar(), v2.scalar(), v3.scalar());
    // printFloat(v1);
    // printFloat(v2);
    // printFloat(v3);
    // return sameSign(v1, v2, v3);
}

collision collide(triangle& t, kln::point& p){return collide(t, p, false);}

int sign(float a, float b) {
    return (a*b >= 0) - (a*b < 0);// + ((a==0)*((b>0) - (b<0))) - ((b==0)*((a>0) - (a<0)));
}

int main(int /*argc*/, char * argv[])
{

    // for (int i = -10; i < 10; i++)
    // {
    //     auto result = sign(i, 1);
    //     std::cout << i << ": " << result << std::endl;
    // }

    // std::cout << 84156 << ": " << sign(sign(0, 1), 0) << std::endl;
    // return 0;

    /* Initialize the library */
    if (!glfwInit()) {
        return -1;
    }
    // kln::point p1{1.0f, 0, 0};
    // kln::point p2{0, 1.0f, 0};
    // kln::point p3{0, 0, 1.0f};
    // kln::point p4{0, 0, 0};
    // // kln::point p1{-0.5, 0, -1};
    // // kln::point p2{0.5, 0, -1};
    // // kln::point p3{0, -0.5, +1};
    // // kln::point p4{0, 0.5, +1};
    // auto vol = ((p4.normalized() & p1.normalized() & p2.normalized() & p3.normalized()));
    // auto volume = vol.scalar() * (1.0/6);
    // printDual(vol);
    // printFloat(volume);

    if(false){

        vec4 vp1(+1, +1, 0, 1);
        vec4 vp2(-1, +1, 0, 1);
        vec4 vp3( 0, -1, 0, 1);


        float degree = 1;
        float a = degree * pi<float>() / 180;
        auto rotationMatrix = rotate(mat4(1), a, vec3(0, 0, 1));

        vp1 = rotationMatrix * vp1;
        vp2 = rotationMatrix * vp2;
        vp3 = rotationMatrix * vp3;

        // cos(a)*vp1.x + sin(a)*vp1.y, sin(a)*vp1.x + cos(a)*vp1.y

        kln::point p1{vp1.x, vp1.y, 0};
        kln::point p2{vp2.x, vp2.y, 0};
        kln::point p3{vp3.x, vp3.y, 0};
        triangle t{p1, p2, p3};

        // auto p8 = kln::point(0, 0, -1);
        // auto alors = collide(t, p8, true);
        // std::cout << "{ " << p8.x() << ", " << p8.y() << ", " << p8.z() << " }" << ": collision:" << alors.collided << ", distance:" << alors.distance << ", at: " << "{ " << alors.location.x() << ", " << alors.location.y() << ", " << alors.location.z() << " }" << " offset:" << "{ " << alors.offset.x << ", " << alors.offset.y << ", " << alors.offset.z << " }" << std::endl;
        // return 0;










        // kln::point q1{-1, -1, 0};
        // kln::point q2{-1, -1, 0};
        // kln::point q1{1, 1, 1684136};
        // kln::point q2{0, 0, 0};




        // auto v1 = (t.p1 & t.p2) | (t.p1 & p);
        // auto v2 = (t.p2 & t.p3) | (t.p2 & p);
        // auto v3 = (t.p3 & t.p1) | (t.p3 & p);
        // printFloat(v1);
        // printFloat(v2);
        // printFloat(v3);

        // // auto sign = sameSign(v1.scalar(), v2.scalar(), v3.scalar());
        // auto signr = sameSign(v1, v2, v3);

        // printFloat(signr);

        // return 0;

        auto start = glfwGetTime();
        auto duration = 1.0/60;
        auto end = start + duration;
        size_t count = 0ull;
        size_t safeGuard = ~0ull;
        // std::cout << "Sign: " << std::endl;
        // while(glfwGetTime() < end) {
        //     // collide(t, point);
        //     kln::point p{1, 1, 1684136};
        //     sameSign(((t.p1 & t.p2) | (t.p1 & p)), ((t.p2 & t.p3) | (t.p2 & p)), ((t.p3 & t.p1) | (t.p3 & p)));
        //     count++;
        //     if (count == safeGuard) {
        //         std::cout << "Safeguard reached: " << count << std::endl;
        //         break;
        //     }
        // }
        // std::cout << "Time limit reached: " << count << std::endl;
        // count = 0ull;
        // end = glfwGetTime() + duration;
        // std::cout << "Distance: " << std::endl;
        // while(glfwGetTime() < end) {
        //     // collide(t, point);
        //     kln::point p{1, 1, 1684136};
        //     (p ^ (t.p1 & t.p2 & t.p3)).e0123();
        //     count++;
        //     if (count == safeGuard) {
        //         std::cout << "Safeguard reached: " << count << std::endl;
        //         break;
        //     }
        // }
        // std::cout << "Time limit reached: " << count << std::endl;
        count = 0ull;
        end = glfwGetTime() + duration; 
        kln::point point{0, 0, +0.25};
        std::cout << "Collide: " << std::endl;
        while(glfwGetTime() < end) {
            // collide(t, point);
            projectToTriangle(t, point);
            // std::abs((p ^ (t.p1 & t.p2 & t.p3)).e0123());
            count++;
            if (count == safeGuard) {
                std::cout << "Safeguard reached: " << count << std::endl;
                break;
            }
        }
        std::cout << "Time limit reached: " << count << std::endl;

        for (int i = -10; i < 10; i++)
        {
            // auto p = kln::point(0.1561, 0.75, i*0.1f);
            auto p = kln::point(0, i*0.2f, -1);
            auto result = collide(t, p);
            std::cout << i << ": { " << p.x() << ", " << p.y() << ", " << p.z() << " }" << ": collision:" << result.collided << ", distance:" << result.distance << ", at: " << "{ " << result.location.x() << ", " << result.location.y() << ", " << result.location.z() << " }" << " offset:" << "{ " << result.offset.x << ", " << result.offset.y << ", " << result.offset.z << " }" << std::endl;
            // printFloat(result.distance);
        }

        // auto p = kln::point(0, 1.5, -100);
        // auto result = collide(t, p, true);
        // std::cout << "{ " << p.x() << ", " << p.y() << ", " << p.z() << " }" << ": collision:" << result.collided << ", distance:" << result.distance << ", at: " << "{ " << result.location.x() << ", " << result.location.y() << ", " << result.location.z() << " }" << " offset:" << "{ " << result.offset.x << ", " << result.offset.y << ", " << result.offset.z << " }" << std::endl;
        
        // auto min = -4.0f;
        // auto max = 4.0f;
        // auto resolution = 50;
        // auto range = max-min;
        // auto epsilon = 0.1f;
        // auto offset = range / resolution;
        // for (float y = max; y >= min; y=y-offset)
        // {
        //     for (float x = min; x <= max; x=x+offset) {
        //         if((std::abs(x-t.p1.x())<epsilon && std::abs(y-t.p1.y())<epsilon) || (std::abs(x-t.p2.x())<epsilon && std::abs(y-t.p2.y())<epsilon) || (std::abs(x-t.p3.x())<epsilon && std::abs(y-t.p3.y())<epsilon)) {
        //             std::cout << "<>";
        //             continue;
        //         }
        //         // auto p = kln::point(0.1561, 0.75, i*0.1f);
        //         auto p = kln::point(x, y, -1);
        //         auto result = collide(t, p);
        //         auto falseTxt = (std::abs(x) > 1.0 || std::abs(y) > 1.0) ? "  ":"__";

        //         std::cout << ((result.distance==-1.0f)?"##":falseTxt);
        //     }
        //     std::cout << std::endl;
        // }



        return 0;

        // printBool(output);





        // // diff sign
        // auto s1 = signedVolume(q1, p1, p2, p3);
        // auto s2 = signedVolume(q2, p1, p2, p3);

        // // AND

        // auto s3 = signedVolume(q1, q2, p1, p2);
        // auto s4 = signedVolume(q1, q2, p2, p3);
        // auto s5 = signedVolume(q1, q2, p3, p1);

        // if((s1 > 0 && s2 < 0) || (s1 < 0 && s2 > 0)) {
        //     if((s3 > 0 && s4 > 0 && s5 > 0) || (s3 < 0 && s4 < 0 && s5 < 0)) {
        //         std::cout << "Intersecion" << std::endl;
        //     }
        //     else {
        //         std::cout << "No Intersecion 2" << std::endl;
        //     }
        // }
        // else {
        //     std::cout << "No Intersecion 1" << std::endl;
        // }
        
        // kln::point p1{2, 0, 0};
        // kln::point p2{4, 0, 0};
        // kln::plane plane1{1, 0, 0, 0};
        // // kln::line myLine(0, 0, 0, 1, 0, 0);

        // auto wedge = p1 * p2;

        // auto exterior = plane1 ^ p1;

        // auto inner = p1 | p2;

        // auto reg = p1 & p2; // line passing by p1 and p2

        // kln::point p3{3, 0, 0};

        // auto p4 = project(p3, reg);

        // auto test = p3 | p4;

        // float distplaneToP1 = std::abs((p3 ^ plane1).e0123());
        // printFloat(distplaneToP1);

        // printDual(p3 ^ plane1);


        // // auto tmp = (p3|reg);
        // // auto pp = reg * tmp;
        // float distance = kln::plane{reg & p3}.norm();

        // printFloat(distance);

        // printPoint(p3);
        // printPoint(p4);
        // printFloat(test);


        // kln::point res = kln::project(p1, plane1);

        // printFloat(test);

        // printFloat(inner);

        // printPoint(res);

        // printTranslator(wedge);

        // printLine(reg);


        // for (int i = -10; i < 10; i++)
        // {
        //     auto value = plane1 ^ kln::point(i*0.1f, 56, 0);
        //     printDual(value);
        // }
        // printDual(exterior);

        // kln::point center{0, 0, 0};
        // for (int i = -10; i <= 10; i++)
        // {
        //     auto value = center | kln::point(i*0.1f, 0, 0);
        //     printFloat(value);
        // }

        // return 0;
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
    
    glimac::FilePath applicationPath(argv[0]);

    std::cout << "Compiling Shaders..." << std::endl;
    BasicProgram programRoom(applicationPath, "src/shaders/roomOne/ground.vs.glsl", "src/shaders/roomOne/ground.fs.glsl");
    BasicProgram programSky(applicationPath, "src/shaders/skybox/skybox_shader.vs.glsl", "src/shaders/skybox/skybox_shader.fs.glsl", ProgramType::TEXTURE);
    BasicProgram programLight(applicationPath, "src/shaders/light/light.vs.glsl", "src/shaders/light/light.fs.glsl", ProgramType::LIGHTS);
    BasicProgram programVoronoi(applicationPath, "src/shaders/roomTwo/voronoi.vs.glsl", "src/shaders/roomTwo/voronoi.fs.glsl", ProgramType::LIGHTS);
    std::vector<BasicProgram*> allPrograms = {&programVoronoi, &programRoom, &programLight, &programSky};
    std::vector<BasicProgram*> allRoomTwoPrograms = {&programRoom, &programVoronoi};

    std::cout << "Loading Textures..." << std::endl;
    GLuint imageWhiteInt = bind_texture(applicationPath.dirPath() + "/assets/textures/white.png");
    GLuint imageDefaultNormalInt = bind_texture(applicationPath.dirPath() + "/assets/textures/normal.png");
    GLuint imageBrickDiffuseInt   = bind_texture(applicationPath.dirPath() + "/assets/textures/bricks_diffuse.jpg");
    GLuint imageBrickRoughnessInt = bind_texture(applicationPath.dirPath() + "/assets/textures/bricks_roughness.jpg");
    GLuint imageBrickNormalInt    = bind_texture(applicationPath.dirPath() + "/assets/textures/bricks_normal.jpg");
    GLuint imageGlassDiffuseInt    = bind_texture(applicationPath.dirPath() + "/assets/textures/glass_diffuse.png");
    GLuint imageGlassNormalInt    = bind_texture(applicationPath.dirPath() + "/assets/textures/glass_normal.jpg");
    GLuint imageSkyboxInt = bind_texture(applicationPath.dirPath() + "/assets/textures/alpha-mayoris.jpg");
    std::vector<GLuint*> allTextures = {
        &imageWhiteInt,
        &imageDefaultNormalInt,
        &imageBrickDiffuseInt,
        &imageBrickRoughnessInt,
        &imageBrickNormalInt,
        &imageGlassDiffuseInt,
        &imageGlassNormalInt,
        &imageSkyboxInt
    };

    glimac::Sphere sphere = glimac::Sphere(1, 32, 16);
    glimac::Sphere sphereInverted = glimac::Sphere(-1, 32, 16);
    glimac::Sphere sphereLowPoly = glimac::Sphere(1, 8, 4);
    glimac::Sphere sphereLowPolyParticule = glimac::Sphere(1, 4, 2);

    std::vector<BBox3f> walls;

    // Add all the walls
    {
        const float wallThickness = 1.0f;
        walls.push_back(BBox3f(vec3(-220, -wallThickness*10, -130), vec3(220, -5, 130)));
        // walls.push_back(BBox3f(vec3(-1, -1, -12), vec3(1, 3, -2)));
        // walls.push_back(BBox3f(vec3(-1, -1, 2), vec3(1, 3, 12)));
        // walls.push_back(BBox3f(vec3(-22, -1, 12), vec3(22, 3, 12+wallThickness)));
        // walls.push_back(BBox3f(vec3(-22, -1, -12-wallThickness), vec3(22, 3, -12)));
        // walls.push_back(BBox3f(vec3(-21-wallThickness, -1, -13), vec3(-21, 3, 13)));
        // walls.push_back(BBox3f(vec3(21, -1, -13), vec3(21+wallThickness, 3, 13)));

    }

    std::cout << "Importing Objects..." << std::endl;
    auto skyboxInstances = std::make_shared<Instance>(sphereInverted.getVertexCount(), sphereInverted.getDataPointer(), imageSkyboxInt, 0, imageDefaultNormalInt);
    auto roomInstances = std::make_shared<Instance>(applicationPath.dirPath(), "dsda", imageBrickDiffuseInt, imageBrickRoughnessInt, imageBrickNormalInt);
    auto lightInstances = std::make_shared<Instance>(sphereLowPoly.getVertexCount(), sphereLowPoly.getDataPointer(), 0, 0, imageDefaultNormalInt);
    auto lightInstances2 = std::make_shared<Instance>(sphereLowPoly.getVertexCount(), sphereLowPoly.getDataPointer(), 0, 0, imageDefaultNormalInt);
    auto graphRender = std::make_shared<Instance>(sphere.getVertexCount(), sphere.getDataPointer(), imageBrickDiffuseInt, 0, imageDefaultNormalInt);
    auto cornerRender = std::make_shared<Instance>(sphereLowPoly.getVertexCount(), sphereLowPoly.getDataPointer(), imageDefaultNormalInt, 0, imageDefaultNormalInt);
    auto graphRenderStatic = std::make_shared<Instance>(sphereLowPolyParticule.getVertexCount(), sphereLowPolyParticule.getDataPointer(), imageDefaultNormalInt, 0, imageDefaultNormalInt);
    // auto simpleCube = std::make_shared<Instance>(applicationPath.dirPath(), "simpleCube2", imageGlassDiffuseInt, 0, imageGlassNormalInt);
    // auto simpleCube = std::make_shared<Instance>(applicationPath.dirPath(), "ico", imageDefaultNormalInt, 0, imageGlassNormalInt);
    // auto simpleCube = std::make_shared<Instance>(applicationPath.dirPath(), "cylinder", imageDefaultNormalInt, 0, imageGlassNormalInt);
    auto simpleCube = std::make_shared<Instance>(applicationPath.dirPath(), "disc", imageDefaultNormalInt, 0, imageGlassNormalInt);

    std::cout << "Particules Initialisation..." << std::endl;
    std::cout << "Rope..." << std::endl;
    auto firstRope = Animation(sphereLowPolyParticule.getVertexCount(), sphereLowPolyParticule.getDataPointer(), imageWhiteInt, 0, imageDefaultNormalInt);
    // firstRope.addField(FieldType::field_directional, vec3(0, -1, 0), 9.81);
    firstRope.make_rope(vec3(-10, 6, 0), vec3(10, 6, 0), 7, 1.0, 16000, 0);
    firstRope.addField(FieldType::field_cube, BBox3f(), 0.1);

    std::cout << "Grid..." << std::endl;
    auto firstGrid = Animation(sphereLowPolyParticule.getVertexCount(), sphereLowPolyParticule.getDataPointer(), imageWhiteInt, 0, imageDefaultNormalInt);
    // firstGrid.make_grid(vec3(-2, 6, -2), vec3(2, 6, -2), vec3(-2, 6, 2), vec3(2, 6, 2), 7, 0.005, 48.0, 0.65);
    if (true) { // work with 7 threads
        // firstGrid.make_grid(vec3(-2, 8, 0), vec3(2, 8, 0), vec3(-2, 4, 0), vec3(2, 4, 0), 70, 1.0, 8192.0, 10);
        firstGrid.make_grid(vec3(-6, 12, -6), vec3(6, 12, -6), vec3(-6, 12, 6), vec3(6, 12, 6), 70, 3.0, 16000, 10);
        // firstGrid.make_grid(vec3(-21, 4, -12), vec3(21, 4, -12), vec3(-21, 4, 12), vec3(21, 4, 12), 80, 50.0, 1024.0, 60);
        firstGrid.addField(FieldType::field_directional, vec3(0, -1, 0), 9.81);
        // firstGrid.addField(FieldType::field_directional, vec3(0, 0, -1), 6.3);
        // firstGrid.addField(FieldType::field_wall, vec3(0, 3, 0), 0.1);
        const float wallFriction = 0.33f;

        const float radiusCube = 3.0f;
        firstGrid.addField(FieldType::field_cube, BBox3f(vec3(-radiusCube, -radiusCube+6, -radiusCube), vec3(radiusCube, radiusCube+6, radiusCube)), wallFriction);
        for(auto wall: walls) {
            firstGrid.addField(FieldType::field_cube, wall, wallFriction);
        }

        // player BBox
        firstGrid.addField(FieldType::field_cube, BBox3f(), 0.1);
    }
    if (false) { // work with 15 threads
        firstGrid.make_grid(vec3(-2, 5, 0), vec3(-6, 5, 0), vec3(-2, 1, 0), vec3(-6, 1, 0), 70, 40.0, 2048.0, 60);
        firstGrid.addField(FieldType::field_directional, vec3(0, -1, 0), 9.81);
        firstGrid.addField(FieldType::field_directional, vec3(0, 0, -1), 6.3);
        firstGrid.addField(FieldType::field_wall, vec3(0, 0, 0), 0.1);
    }
    if (false) { // work with 15 threads, no sub connection
        firstGrid.make_grid(vec3(-2, 5, 0), vec3(-6, 5, 0), vec3(-2, 1, 0), vec3(-6, 1, 0), 450, 40.0, 2048.0, 60);
        firstGrid.addField(FieldType::field_directional, vec3(0, -1, 0), 9.81);
        firstGrid.addField(FieldType::field_directional, vec3(0, 0, -1), 6.3);
        firstGrid.addField(FieldType::field_wall, vec3(0, 0, 0), 0.1);
    }


    std::cout << "Cube..." << std::endl;
    auto firstCube = Animation(sphereLowPolyParticule.getVertexCount(), sphereLowPolyParticule.getDataPointer(), imageWhiteInt, 0, imageDefaultNormalInt);
    // firstCube.make_cube(vec3(0, 10, 0), vec3(4, 4, 4), 13, 5.5, 1000.0, 100);
    
    if (false) {
        firstCube.make_cube(vec3(-11, 7, 0), vec3(4, 4, 4), 25, 50, 3000, 30);
        firstCube.addField(FieldType::field_directional, vec3(0, -1, 0), 9.81);
        firstCube.addField(FieldType::field_wall, vec3(0, 0, 0), 0.1);
    }
    
    if (true) {
        firstCube.make_cube(vec3(0, 7, -1.9), vec3(2), 15, 1, 15000, 3);
        firstCube.addField(FieldType::field_directional, vec3(0, -1, 0), 9.81);
        // firstCube.addField(FieldType::field_fluid, vec3(0), 0.001);
        // firstCube.addField(FieldType::field_wall, vec3(0, 0, 0), 0.1);
        const float wallFriction = 0.45f;

        for(auto wall: walls) {
            firstCube.addField(FieldType::field_cube, wall, wallFriction);
        }

        // Player Bbox
        firstCube.addField(FieldType::field_cube, BBox3f(), 0.1);
    }

    auto secondCube = Animation(sphereLowPolyParticule.getVertexCount(), sphereLowPolyParticule.getDataPointer(), imageWhiteInt, 0, imageDefaultNormalInt);
    secondCube.make_cube(vec3(0, 7, -1.9), vec3(0.5), 2, 0.1, 400, 0.3);
    secondCube.addField(FieldType::field_directional, vec3(0, -1, 0), 9.81);
    for(auto wall: walls) {
        secondCube.addField(FieldType::field_cube, wall, 0.05f);
    }
    secondCube.addField(FieldType::field_cube, BBox3f(), 0.1);


    std::vector<Animation*> allAnimations = {
        &firstRope,
        &firstGrid,
        &firstCube,
        &secondCube
    };


    bool cubeInsteadOfFlag = false;

    Scene scene;

    // Add all objects to the scene
    {
        scene.addInstance(roomInstances);
        scene.addInstance(graphRender);
        scene.addInstance(cornerRender);
        scene.addInstance(graphRenderStatic);
        scene.addInstance(simpleCube);
        // scene.addInstance(transparentInstances);

        // simpleCube.get()->setBlendToTransparent();

        // Animation objects
        // scene.addInstance(firstRope.getInstance());
        // if(cubeInsteadOfFlag) {
        //     scene.addInstance(firstCube.getInstance());
        // }
        // else {
        //     scene.addInstance(firstGrid.getInstance());
        //     // scene.addInstance(firstRope.getInstance());
        // }
        // scene.addInstance(secondCube.getInstance());

        // transparentInstances.get()->setBlendToTransparent();
    }

    // SHADERS INVARIANTS
    std::cout << "Placing all objects..." << std::endl;

    // vec4 vp1(+1, 0, +1, 1);
    // vec4 vp2(-1, 0, +1, 1);
    // vec4 vp3( 0, 0.6f, -1, 1);
    // vec4 vp1(+4,   0, 0, 1);
    // vec4 vp2(-4,   0, 0, 1);
    // vec4 vp3( 0, 4,  4, 1);
    vec4 vp1(-0.5, 0, -0.5, 1);
    vec4 vp2(-0.5, 0,  0.5, 1);
    vec4 vp3(+0.5, 0,  0.5, 1);
    vec4 vp4(+0.5, 0, -0.5, 1);

    auto resolution = 15;
    auto len1 = length(vec3(vp1.x, vp1.y, vp1.z));
    auto len2 = length(vec3(vp2.x, vp2.y, vp2.z));
    auto len3 = length(vec3(vp3.x, vp3.y, vp3.z));

    auto maxLen = 0.95 * std::max(std::max(len1, len2), len3);

    auto min = -maxLen;
    auto max = maxLen;
    auto range = max-min;
    auto offset = range / resolution;

    float degree = 0.0f;
    float a = degree * pi<float>() / 180;
    auto rotationMatrix = rotate(mat4(1), a, vec3(0, 1, 0));

    auto vrp1 = rotationMatrix * vp1;
    auto vrp2 = rotationMatrix * vp2;
    auto vrp3 = rotationMatrix * vp3;
    auto vrp4 = rotationMatrix * vp4;

    kln::point p1{vrp1.x, vrp1.y, vrp1.z};
    kln::point p2{vrp2.x, vrp2.y, vrp2.z};
    kln::point p3{vrp3.x, vrp3.y, vrp3.z};
    kln::point p4{vrp4.x, vrp4.y, vrp4.z};

    triangle t{p1, p2, p3};
    triangle t2{p3, p4, p1};

    auto ax = -30.f*degToRad;
    auto ay = 45.f*degToRad;
    auto az = 7.0f*degToRad;
    // auto ax = 0.f*degToRad;
    // auto ay = 0.f*degToRad;
    // auto az = 35.f*degToRad;
    // auto ax = 0.0f*degToRad;
    // auto ay = 0.0f*degToRad;
    // auto az = 0.0f*degToRad;

    // kln::rotor rx(-az, 1, 0, 0);
    // kln::rotor ry(ay, 0, 1, 0);
    // kln::rotor rz(ax, 0, 0, 1);
    // kln::rotor rx(-ax, 1, 0, 0);
    // kln::rotor ry(ay, 0, 1, 0);
    // kln::rotor rz(-az, 0, 0, 1);

    kln::rotor rx(-ax, 1, 0, 0);
    kln::rotor ry(-ay, 0, 1, 0);
    kln::rotor rz(-az, 0, 0, 1);
    auto finalRotor = ry * rx * rz;
    finalRotor.normalize();
    

    // could be optimized with the length as 1.0 and x/y/z as the position
    // or not since there is a disvision by the length of x/y/z
    auto pos = vec3(0.0, 0.0, 0.0);
    kln::translator tx(pos.x, 1.0, 0.0, 0.0);
    kln::translator ty(pos.y, 0.0, 1.0, 0.0);
    kln::translator tz(pos.z, 0.0, 0.0, 1.0);
    auto finalTranslator = tx * ty * tz;

    auto finalMotor = finalTranslator * finalRotor;

    rigidBody rb;
    rigidBody rb2;

    rb.com = vecToPoint(pos);
    rb.motor = finalMotor;
    simpleCube.get()->getTriangles(rb.motor, &rb.triangles);

    rb2.com = vecToPoint(pos+vec3(0, 2.5, 0));
    rb2.motor = finalMotor;
    simpleCube.get()->getTriangles(rb2.motor, &rb2.triangles);


    std::vector<triangle> cubeTriangles;
    simpleCube.get()->getTriangles(finalMotor, &cubeTriangles);

    simpleCube.get()->add(Transform(pos, vec3(ax, ay, az)));
    simpleCube.get()->add(Transform(pos+vec3(0, 1, 0)));

    // Add all objects positions
    {
        roomInstances.get()->add(Transform(vec3(0, -5, 0)));
        // transparentInstances.get()->add(Transform(vec3(0), vec3(0), vec3(50, 6, 25)));
        // transparentInstances.get()->add(Transform(vec3(0, 6, 0), vec3(0), vec3(50, 6, 25)));

        skyboxInstances.get()->add(Transform(vec3(0), vec3(135*degToRad, -105*degToRad, 15*degToRad), vec3(5000)));

        for (float z = max; z >= min; z=z-offset)
        {
            for (float x = min; x <= max; x=x+offset) {
                graphRender.get()->add(Transform(vec3(x, 0, z), vec3(0, 45*degToRad, 0), vec3((1.0/cos(45*degToRad))*(offset/2.0))));
            }
        }
        for (float z = max; z >= min; z=z-offset)
        {
            for (float x = min; x <= max; x=x+offset) {
                graphRenderStatic.get()->add(Transform(vec3(x, 0, z), vec3(0, 45*degToRad, 0), vec3(0.00625f)));
            }
        }
        cornerRender.get()->add(Transform(vec3(vrp1), vec3(), vec3(offset)));
        cornerRender.get()->add(Transform(vec3(vrp2), vec3(), vec3(offset)));
        cornerRender.get()->add(Transform(vec3(vrp3), vec3(), vec3(offset)));
        cornerRender.get()->add(Transform(vec3(vrp4), vec3(), vec3(offset)));
    }

    Light lightsRoomLeft;
    Light lightsRoomRight;

    // Add all the lights
    {
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

    std::cout << "Initializing variables..." << std::endl;

    unsigned int currentRoom = 0;
    BasicProgram *currentProgram = allRoomTwoPrograms.at(currentRoom);


    double oldTime = -1.0f;
    double deltaT = 0;
    bool animateSwitch = false;
    double timer = 0.0f;
    double animateTimer = 0.0f;
    bool boolRightRoom = false;
    vec2 mousePos = win.mouse();

    glm::mat4 shadowMatrix;

    vec3 currentCamPos;
    FPSCamera fpsCam = FPSCamera(win.width(), win.height());

    auto startPoint = vec3(0, 0.25, 0);
    auto startLookPoint = vec3(0, 0.25, -1);
    fpsCam.makeLookAt(startPoint, startLookPoint);
    fpsCam.update(win, walls, deltaT);
    fpsCam.resetMouse(mousePos);

    
    ShadowMap shadowMap(applicationPath);
    shadowMap.init();

    shadowMap.computeTransforms(LightStruct(vec3(40, 40, -40), vec3(1), vec3(100, 0.85, LightType::directionalLight)));

    GLuint depthMapId;
    depthMapId = shadowMap.getDepthMap();
    shadowMatrix = shadowMap.getShadowMatrix();
    shadowMap.renderTexture(win, scene);
    vec3 sunPos = shadowMap.getLightPos();


    // bool ropeFollowPlayer = false;

    std::cout << "Main loop" << std::endl;

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

    uint nb_threads = 1;
    volatile bool killThreads = false;
    volatile bool computeAnim = false;
    volatile double animFrameTime = 0.0;
    volatile auto deltaTThreads = 0.010f;
    std::vector<ThreadProcessState> threadStates;
    std::vector<ThreadProcessComparaison> threadComparaison;
    std::vector<std::thread> threadList;

    std::vector<ThreadProcessState> threadStates_flag;
    std::vector<ThreadProcessComparaison> threadComparaison_flag;
    std::vector<std::thread> threadList_flag;

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
                timer = glfwGetTime();
                threadStates.at(index) = NothingDone;

                animation->getFields()->back().make_cube(fpsCam.getBBox(), 0);
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
                    animFrameTime = glfwGetTime() - timer;
                    // animFrameTime = glfwGetTime() - oldTime;
                    deltaTThreads = animFrameTime;
                }
            }

            while(oldTime+deltaTThreads > glfwGetTime()){}
            oldTime = glfwGetTime();
        }
    };
    std::cerr << "Before Threads shenanigans" << std::endl;
    // for (uint i = 0; i < nb_threads; i++) {
    //     threadStates.push_back(NothingDone);
    //     threadComparaison.push_back(Compute_1);
    //     if(cubeInsteadOfFlag) {
    //         threadList.push_back(std::thread(animUpdateLambda, i, &firstCube));
    //     }
    // }
    // for (uint i = 0; i < nb_threads; i++) {
    //     threadStates_flag.push_back(NothingDone);
    //     threadComparaison_flag.push_back(Compute_1);
    //     if(!cubeInsteadOfFlag) {
    //         // threadList_flag.push_back(std::thread(animUpdateLambda, i, &firstRope));
    //         threadList_flag.push_back(std::thread(animUpdateLambda, i, &firstGrid));
    //     }
    // }
    computeAnim = false;

    /* Loop until the user closes the window */
    uint refreshTitle = 0;
    while (win.running()) {

        { // INIT CODE
            clear_screen();
            fpsCam.update(win, walls, deltaT);
            timer = glfwGetTime();
            deltaT = std::min(timer - oldTime, 1.0/24);
            // deltaT = timer - oldTime;
            if(oldTime < 0) { // first frame
                deltaT = 0.0f;
            }
            if (animateSwitch) { // the animation is running
                animateTimer += deltaT;
            }
            oldTime = timer;
            // oldMouse = win.mouse();
            currentCamPos = fpsCam.getPos();
        }

        { // UPDATES

            // if(cubeInsteadOfFlag) {
            //     firstCube.update_visual();
            // }
            // else {
            //     firstGrid.update_visual();
            //     // firstRope.update_visual();
            // }

            // if(computeAnim) {
            //     secondCube.getFields()->back().make_cube(fpsCam.getBBox(), 0);
            //     secondCube.update(deltaT);
            // }
            // secondCube.update_visual();
            if(animateSwitch) {
                // degree+=36.0f * 0.001f;
                // a = degree * pi<float>() / 180;
                // auto rotationMatrix = rotate(mat4(1), a, vec3(0, 1, 0));

                // vp3.y = 4*cos(animateTimer);
                // vp3.z = 4*sin(animateTimer);
                // auto timedOffset = vec4(0, 0, 1.64817*cos(animateTimer), 0);

                // vrp1 = rotationMatrix * (vp1 + timedOffset);
                // vrp2 = rotationMatrix * (vp2 + timedOffset);
                // vrp3 = rotationMatrix * (vp3 + timedOffset);

                // p1 = kln::point{vrp1.x, vrp1.y, vrp1.z};
                // p2 = kln::point{vrp2.x, vrp2.y, vrp2.z};
                // p3 = kln::point{vrp3.x, vrp3.y, vrp3.z};

                // t = triangle{p1, p2, p3};
                float degree = 36.f;
                float a = degree * pi<float>() / 180;
                auto rotationMatrix = rotate(rotate(mat4(1), 45.0f, vec3(0, 1, 0)), a, vec3(0, 0, 1));

                auto vrp1 = rotationMatrix * vp1;
                auto vrp2 = rotationMatrix * vp2;
                auto vrp3 = rotationMatrix * vp3;
                auto vrp4 = rotationMatrix * vp4;

                kln::point p1{vrp1.x, vrp1.y, vrp1.z};
                kln::point p2{vrp2.x, vrp2.y, vrp2.z};
                kln::point p3{vrp3.x, vrp3.y, vrp3.z};
                kln::point p4{vrp4.x, vrp4.y, vrp4.z};

                t = triangle{p1, p2, p3};
                t2 = triangle{p3, p4, p1};



                ax = -30.f*degToRad*cos(animateTimer)*3.5;
                // ay = 45.f*degToRad;
                az = 0.0f*degToRad;
                // ax = 0.f*degToRad;
                ay = 15.f*degToRad*animateTimer;
                rx = kln::rotor(-ax, 1, 0, 0);
                ry = kln::rotor(-ay, 0, 1, 0);
                rz = kln::rotor(-az, 0, 0, 1);

                finalRotor = ry * rx * rz;
                finalRotor.normalize();

                pos = vec3(0.5*cos(animateTimer), 0.0, 0.5*sin(animateTimer));
                tx = kln::translator(pos.x, 1.0, 0.0, 0.0);
                ty = kln::translator(pos.y, 0.0, 1.0, 0.0);
                tz = kln::translator(pos.z, 0.0, 0.0, 1.0);
                finalTranslator = tx * ty * tz;

                finalMotor = finalTranslator * finalRotor;
                cubeTriangles.clear();
                simpleCube.get()->getTriangles(finalMotor, &cubeTriangles);

                simpleCube.get()->updatePosition(0, pos);
                simpleCube.get()->updateAngles(0, vec3(ax, ay, az));
                simpleCube.get()->computeAll();



                auto indexGraph = 0;
                for (float z = max; z >= min; z=z-offset)
                {
                    for (float x = min; x <= max; x=x+offset) {
                        // auto pos = vec3(x, 1.2*(0.25+0.75*cos(animateTimer*1.1486)), z);
                        auto pos = vec3(x, 0.12, z);
                        auto point = kln::point(pos.x, pos.y, pos.z);

                        float smallestDistance;
                        bool firstProjection = true;
                        bool collided = false;
                        vec3 displacement;

                        for(auto &tr: cubeTriangles) {
                            auto result = projectToTriangle(tr, point);

                            if(firstProjection) {
                                firstProjection = false;
                                smallestDistance = result.distance;

                                // if collided is false, we will ignore the displacement
                                // otherwise we will use it so no need for another conditional branch
                                // collided = result.collided;
                                // collided = true;
                                displacement = result.offset;
                                continue;
                            }

                            // new candidate for the closest plane from a triangle
                            // if the closest plane is not in collision then there is no need for a collision
                            // otherwise it is a smaller displacement that resolve the collision
                            if(result.distance < smallestDistance) {
                                // if(result.collided) {
                                // }
                                smallestDistance = result.distance;
                                collided = result.collided;
                                // collided = true;
                                displacement = result.offset;
                                continue;
                            }
                            // break;
                        }

                        // with hack from the nature of a cube
                        // for(auto &tr: cubeTriangles) {
                        //     auto result = projectToTriangle(tr, point);
                        //     if(result.collided) {
                        //         if(result.distance > 0.5) { // hack because its a cube of size 0.5
                        //             continue;
                        //         }
                        //         if(!collided) {
                        //             displacement = result.offset;
                        //             smallestDistance = result.distance;
                        //             collided = true;
                        //             // break;
                        //             continue;
                        //         }
                        //         if(result.distance < smallestDistance) {
                        //             displacement = result.offset;
                        //             smallestDistance = result.distance;
                        //             // break;
                        //             continue;
                        //         }
                        //     }
                        //     // break;
                        // }
                        if(true) {
                            pos += displacement;
                        }
                        graphRender.get()->updatePosition(indexGraph, pos);
                        indexGraph++;







                        // auto result = projectToTriangle(t, point);
                        // if(result.collided) {
                        //     pos += vec3(result.offset.x, result.offset.y, result.offset.z);
                        // }
                        // auto point2 = kln::point(pos.x, pos.y, pos.z);
                        // auto result2 = projectToTriangle(t2, point2);
                        // if(result2.collided) {
                        //     pos += vec3(result2.offset.x, result2.offset.y, result2.offset.z);
                        // }
                        // auto result = collide(t, point);
                        // if(std::abs(z) < 0.1 || std::abs(x) < 0.1) {
                        // // std::cout << offset.x << " " << offset.y << " " << offset.z << std::endl;
                        // std::cout << offset << std::endl;
                        // // if(offset == vec3(0)) {
                        // // }
                        // }
                    }
                }
                graphRender.get()->computeAll();
                

                // for (size_t i = 0; i < graphRender.get()->size(); i++)
                // {
                //     auto pos = graphRender.get()->get(i).m_Position;
                //     auto point = kln::point(pos.x, pos.z, -0.2);
                //     pos.y = collide(t, point).distance + 4;
                //     graphRender.get()->updatePosition(i, pos);
                // }
                // graphRender.get()->computeAll();

                cornerRender.get()->updatePosition(0, vec3(vrp1.x, vrp1.y, vrp1.z));
                cornerRender.get()->updatePosition(1, vec3(vrp2.x, vrp2.y, vrp2.z));
                cornerRender.get()->updatePosition(2, vec3(vrp3.x, vrp3.y, vrp3.z));
                cornerRender.get()->updatePosition(3, vec3(vrp4.x, vrp4.y, vrp4.z));
                cornerRender.get()->computeAll();
            }

            

            if(depthMapId != 0) {
                shadowMap.renderTexture(win, scene);
            }

            // if((ropeFollowPlayer && false) || depthMapId == 0) {

            //     // firstRope.setPosFirst(fpsCam.getPos() + vec3(0, -0.1, 0));
            //     firstGrid.setPosFirst(fpsCam.getPos() + vec3(0, 0, 0));
            //     // firstCube.setPosFirst(fpsCam.getPos() + vec3(0, 0, 0));
            //     firstGrid.setTypeFirst(ParticuleComputeType::fixed);
            //     // firstCube.setTypeFirst(ParticuleComputeType::fixed);
            // }
            // else {
            //     firstGrid.setTypeFirst(ParticuleComputeType::leapfrog);
            //     // firstCube.setTypeFirst(ParticuleComputeType::leapfrog);
            // }

            // Ka = h^2 * k/m
            // Za = h * z/m

            // particules: 20525
            // links: 264966

            // auto timeStartOneIteration = glfwGetTime();
            // firstRope.update(deltaT/animIterPerFrame);
            // firstGrid.update(deltaT);
            // firstCube.update(deltaT/animIterPerFrame);

            
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
                    programLight.activate(win, fpsCam, shadowMatrix, lightsRoomRight, sunPos);
                    lightInstances2.get()->drawAll(programLight, fpsCam.getViewMatrix(), fpsCam.getProjMatrix(), depthMapId);
                    // programParticules.activate(win, fpsCam, shadowMatrix, lightsRoomLeft, sunPos);
                    // firstGrid.getInstance().get()->drawAll(programParticules, fpsCam.getViewMatrix(), fpsCam.getProjMatrix(), depthMapId);
                }
                else {
                    scene.drawScene(win, programRoom, fpsCam, shadowMatrix, lightsRoomLeft, depthMapId, sunPos);
                    programLight.activate(win, fpsCam, shadowMatrix, lightsRoomLeft, sunPos);
                    lightInstances.get()->drawAll(programLight, fpsCam.getViewMatrix(), fpsCam.getProjMatrix(), depthMapId);
                    // programParticules.activate(win, fpsCam, shadowMatrix, lightsRoomLeft, sunPos);
                    // firstGrid.getInstance().get()->drawAll(programParticules, fpsCam.getViewMatrix(), fpsCam.getProjMatrix(), depthMapId);
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
                // std::cout << "currentRoom " << currentRoom << std::endl;
            }

            if (keys & scrollUp) {
                animateSwitch = !animateSwitch;
                computeAnim = !computeAnim;
                // std::cout << "animate room 1 " << animateSwitch << std::endl;
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
                firstRope.reset();
                firstGrid.reset();
                firstCube.reset();
                secondCube.reset();
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
            win.updateTitle(currentCamPos, deltaT, animFrameTime);
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
    for (uint i = 0; i < threadList_flag.size(); i++)
    {
        threadList_flag.at(i).join();
    }
    std::cout << "Thread joined" << std::endl;
    
    // Make the audio lag
    // for(auto anim : allAnimations) {
    //     anim->~Animation();
    // }

    lightInstances.get()->~Instance();
    lightInstances2.get()->~Instance();
    scene.~Scene();

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

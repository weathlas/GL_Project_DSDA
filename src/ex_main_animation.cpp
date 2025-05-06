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
#include <glimac/Texture.hpp>
#include <glimac/common.hpp>
#include <glimac/Physic.hpp>
// #include <glimac/PGA.hpp>
// #include <glimac/imgui.h>
// #include <glimac/GLFWHandle.hpp>

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

int main(int argc, char * argv[])
{

    /* Initialize the library */
    if (!glfwInit()) {
        return -1;
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

    const bool SUPERFLUX = true;


    /*********************************
     * HERE SHOULD COME THE INITIALIZATION CODE
     *********************************/
    
/**/glimac::FilePath applicationPath(argv[0]);

/**/std::cout << "Compiling Shaders..." << std::endl;
/**/BasicProgram programRoom(applicationPath, "src/shaders/roomOne/ground.vs.glsl", "src/shaders/roomOne/ground.fs.glsl");
/**/BasicProgram programSky(applicationPath, "src/shaders/skybox/skybox_shader.vs.glsl", "src/shaders/skybox/skybox_shader.fs.glsl", ProgramType::TEXTURE);
/**/BasicProgram programLight(applicationPath, "src/shaders/light/light.vs.glsl", "src/shaders/light/light.fs.glsl", ProgramType::LIGHTS);
/**/BasicProgram programVoronoi(applicationPath, "src/shaders/roomTwo/voronoi.vs.glsl", "src/shaders/roomTwo/voronoi.fs.glsl", ProgramType::LIGHTS);
// /**/BasicProgram programNormal(applicationPath, "src/shaders/utils/normal.vs.glsl", "src/shaders/utils/normal.fs.glsl", ProgramType::LIGHTS);

    BasicProgram programSun(applicationPath, "src/shaders/utils/white.vs.glsl", "src/shaders/utils/white.fs.glsl", ProgramType::NONE);
    BasicProgram programMirror(applicationPath, "src/shaders/mirror/mirror.vs.glsl", "src/shaders/mirror/mirror.fs.glsl");
    BasicProgram programButtons(applicationPath, "src/shaders/utils/buttons.vs.glsl", "src/shaders/utils/buttons.fs.glsl");
    std::vector<BasicProgram*> allPrograms = {&programRoom, &programVoronoi, &programSky, &programLight, &programButtons, &programSun, &programMirror};

/**/std::vector<BasicProgram*> allRoomTwoPrograms = {&programRoom, &programVoronoi};

    std::cout << "Loading Textures..." << std::endl;

    GLuint imageEarthInt = bind_texture(applicationPath.dirPath() + "/assets/textures/EarthMap.jpg");
    GLuint imageCloudInt = bind_texture(applicationPath.dirPath() + "/assets/textures/CloudMap.jpg");
    GLuint imageMoonInt  = bind_texture(applicationPath.dirPath() + "/assets/textures/MoonMap.jpg");

    GLuint imageSkyboxInt = bind_texture(applicationPath.dirPath() + "/assets/textures/alpha-mayoris.jpg");

    GLuint imageBrickDiffuseInt   = bind_texture(applicationPath.dirPath() + "/assets/textures/bricks_diffuse.jpg");
    GLuint imageBrickRoughnessInt = bind_texture(applicationPath.dirPath() + "/assets/textures/bricks_roughness.jpg");
    GLuint imageBrickNormalInt    = bind_texture(applicationPath.dirPath() + "/assets/textures/bricks_normal.jpg");

    GLuint imageHouseDiffuseInt   = bind_texture(applicationPath.dirPath() + "/assets/textures/cottage_diffuse.png");
    GLuint imageHouseRoughnessInt = bind_texture(applicationPath.dirPath() + "/assets/textures/cottage_roughness.jpg");
    GLuint imageHouseNormalInt    = bind_texture(applicationPath.dirPath() + "/assets/textures/cottage_normal.jpg");

    GLuint imageStatueInt = bind_texture(applicationPath.dirPath() + "/assets/textures/statue.jpg");

    GLuint imageCollumnDiffuseInt   = bind_texture(applicationPath.dirPath() + "/assets/textures/collumn_diffuse.jpg");
    GLuint imageCollumnRoughnessInt = bind_texture(applicationPath.dirPath() + "/assets/textures/collumn_roughness.jpg");
    GLuint imageCollumnNormalInt    = bind_texture(applicationPath.dirPath() + "/assets/textures/collumn_normal.jpg");

    GLuint imageFootDiffuseInt   = bind_texture(applicationPath.dirPath() + "/assets/textures/foot_diffuse.jpg");
    GLuint imageFootRoughnessInt = bind_texture(applicationPath.dirPath() + "/assets/textures/foot_rougness.jpg");
    GLuint imageFootNormalInt    = bind_texture(applicationPath.dirPath() + "/assets/textures/foot_normal.jpg");

    GLuint imageGlassDiffuseInt = bind_texture(applicationPath.dirPath() + "/assets/textures/glass_diffuse.png");
    GLuint imageGlassNormalInt  = bind_texture(applicationPath.dirPath() + "/assets/textures/glass_normal.jpg");

    GLuint imageBenchDiffuseInt   = bind_texture(applicationPath.dirPath() + "/assets/textures/bench_diffuse.jpg");
    GLuint imageBenchRoughnessInt = bind_texture(applicationPath.dirPath() + "/assets/textures/bench_roughness.jpg");
    GLuint imageBenchNormalInt    = bind_texture(applicationPath.dirPath() + "/assets/textures/bench_normal.jpg");


    GLuint imageFlagFrenchInt    = bind_texture(applicationPath.dirPath() + "/assets/textures/Flag_of_France.jpg");
    GLuint imageApertureInt    = bind_texture(applicationPath.dirPath() + "/assets/textures/Companion_Cube.jpg");

    GLuint imageWhiteInt = bind_texture(applicationPath.dirPath() + "/assets/textures/white.png");
    GLuint imageDefaultNormalInt = bind_texture(applicationPath.dirPath() + "/assets/textures/normal.png");

    std::vector<GLuint*> allTextures = {
        &imageEarthInt,
        &imageCloudInt,
        &imageMoonInt,
        &imageSkyboxInt,

        &imageBrickDiffuseInt,
        &imageBrickRoughnessInt,
        &imageBrickNormalInt,

        &imageHouseDiffuseInt,
        &imageHouseRoughnessInt,
        &imageHouseNormalInt,

        &imageStatueInt,

        &imageCollumnDiffuseInt,
        &imageCollumnRoughnessInt,
        &imageCollumnNormalInt,

        &imageFootDiffuseInt,
        &imageFootRoughnessInt,
        &imageFootNormalInt,

        &imageGlassDiffuseInt,
        &imageGlassNormalInt,

        &imageBenchDiffuseInt,
        &imageBenchRoughnessInt,
        &imageBenchNormalInt,

        &imageFlagFrenchInt,
        &imageApertureInt,

        &imageWhiteInt,
        &imageDefaultNormalInt
    };

/**/glimac::Sphere sphere = glimac::Sphere(1, 32, 16);
/**/glimac::Sphere sphereInverted = glimac::Sphere(-1, 32, 16);
/**/glimac::Sphere sphereLowPoly = glimac::Sphere(1, 8, 4);
/**/glimac::Sphere sphereLowPolyParticule = glimac::Sphere(1, 4, 2);

/**/std::cout << "Importing Objects..." << std::endl;
/**/auto skyboxInstances = std::make_shared<Instance>(sphereInverted.getVertexCount(), sphereInverted.getDataPointer(), imageSkyboxInt, 0, imageDefaultNormalInt);
/**/auto roomInstances = std::make_shared<Instance>(applicationPath.dirPath(), "dsda", imageBrickDiffuseInt, imageBrickRoughnessInt, imageBrickNormalInt);
/**/auto lightInstances = std::make_shared<Instance>(sphereLowPoly.getVertexCount(), sphereLowPoly.getDataPointer(), 0, 0, imageDefaultNormalInt);
/**/auto lightInstances2 = std::make_shared<Instance>(sphereLowPoly.getVertexCount(), sphereLowPoly.getDataPointer(), 0, 0, imageDefaultNormalInt);
    auto earthInstances = std::make_shared<Instance>(sphere.getVertexCount(), sphere.getDataPointer(), imageEarthInt, imageCloudInt, imageDefaultNormalInt);
    auto sunInstances = std::make_shared<Instance>(sphere.getVertexCount(), sphere.getDataPointer(), imageWhiteInt, 0, imageDefaultNormalInt);

    auto particulesInstances = std::make_shared<Instance>(sphereLowPolyParticule.getVertexCount(), sphereLowPolyParticule.getDataPointer(), imageMoonInt, 0, imageDefaultNormalInt);

    auto collumnInstances = std::make_shared<Instance>(applicationPath.dirPath(), "collumn3", imageCollumnDiffuseInt, imageCollumnRoughnessInt, imageCollumnNormalInt);
    auto houseInstances = std::make_shared<Instance>(applicationPath.dirPath(), "cottage", imageHouseDiffuseInt, imageHouseRoughnessInt, imageHouseNormalInt);
    auto statueInstances = std::make_shared<Instance>(applicationPath.dirPath(), "statue", imageStatueInt, 0, imageDefaultNormalInt);
    auto footInstances = std::make_shared<Instance>(applicationPath.dirPath(), "foot", imageFootDiffuseInt, imageFootRoughnessInt, imageFootNormalInt);
    auto cubeInstances = std::make_shared<Instance>(applicationPath.dirPath(), "simpleCube2", imageGlassDiffuseInt, 0, imageGlassNormalInt);
    auto buttonInstances = std::make_shared<Instance>(sphere.getVertexCount(), sphere.getDataPointer(), imageWhiteInt, imageWhiteInt, imageDefaultNormalInt);
    auto benchInstances = std::make_shared<Instance>(applicationPath.dirPath(), "bench", imageBenchDiffuseInt, imageBenchRoughnessInt, imageBenchNormalInt);

    auto domeGlassInstances = std::make_shared<Instance>(applicationPath.dirPath(), "domeGlass", imageGlassDiffuseInt, 0, imageGlassNormalInt);
    auto domeWireInstances = std::make_shared<Instance>(applicationPath.dirPath(), "domeWire", imageWhiteInt, 0, imageDefaultNormalInt);

    auto shelveInstances = std::make_shared<Instance>(applicationPath.dirPath(), "shelve", imageMoonInt, 0, imageDefaultNormalInt);
    auto titleInstances = std::make_shared<Instance>(applicationPath.dirPath(), "title", imageBrickDiffuseInt, 0, imageDefaultNormalInt);
    auto spaceShipInstances = std::make_shared<Instance>(applicationPath.dirPath(), "Farragut", imageBrickDiffuseInt, 0, imageDefaultNormalInt);
    auto planeInstances = std::make_shared<Instance>(applicationPath.dirPath(), "plane", imageWhiteInt, 0, imageDefaultNormalInt);
    auto citadelInstances = std::make_shared<Instance>(applicationPath.dirPath(), "citadel", imageStatueInt, 0, imageDefaultNormalInt);

    auto mirrorInstances = std::make_shared<Instance>(applicationPath.dirPath(), "circle", imageGlassDiffuseInt, imageWhiteInt, imageGlassNormalInt);
    auto arrowInstances = std::make_shared<Instance>(applicationPath.dirPath(), "arrow", imageWhiteInt, imageWhiteInt, imageDefaultNormalInt);






/**/Scene scene;
/**/Scene sceneProj;
/**/Scene sceneUnused;

/**/// Add all objects to the scene
/**/{
        scene.addInstance(roomInstances);
        // edgeCollisionRender.get()->setBlendToTransparent();
        // simpleCube.get()->setBlendToTransparent();

        scene.addInstance(earthInstances);
        scene.addInstance(particulesInstances);
        scene.addInstance(shelveInstances);
        scene.addInstance(collumnInstances);
        scene.addInstance(titleInstances);
        scene.addInstance(spaceShipInstances);
        scene.addInstance(planeInstances);
        scene.addInstance(statueInstances);
        scene.addInstance(citadelInstances);
        scene.addInstance(footInstances);
        scene.addInstance(houseInstances);
        scene.addInstance(domeWireInstances);
        scene.addInstance(benchInstances);
        scene.addInstance(arrowInstances);

        // transparents objects
        scene.addInstance(cubeInstances);
        scene.addInstance(domeGlassInstances);

        cubeInstances.get()->setBlendToTransparent();
        domeGlassInstances.get()->setBlendToTransparent();
    }

    buttonInstances.get()->setBlendToTransparent();

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

        int nbRandLight = 40;

        for (int i = 0; i < nbRandLight; i++) {
            vec3 pos = vec3(linearRand(1.0f, 20.0f), linearRand(.1f, 1.0f), linearRand(-11.0f, 11.0f));
            pos = vec3(pos.x, pos.y/20, pos.z);
            vec3 color = vec3(linearRand(.1f, 1.0f), linearRand(.1f, 1.0f), linearRand(.1f, 1.0f));
            lightsRoomRight.add(LightStruct(pos, color, vec3(2, 0.4, lightSize)));
            lightInstances2.get()->add(Transform(pos, vec3(0), vec3(lightSize)));
        }
    }
    
    /**/std::vector<BBox3f> walls;

    // Add all objects positions
    {
        const float heightFoot = footInstances.get()->getDefautBBox().size().y;

        sunInstances.get()->add(Transform(vec3(2500, 2500*cos(45*degToRad), -2500), vec3(0), vec3(10)));

        earthInstances.get()->add(Transform(vec3(2500, 2500, 2500), vec3(0), vec3(125)));

        // skyboxInstances.get()->add(Transform(vec3(0), vec3(135*degToRad, -135*degToRad, 0), vec3(5000)));
        skyboxInstances.get()->add(Transform(vec3(0), vec3(135*degToRad, -105*degToRad, 15*degToRad), vec3(5000)));
        // skyboxInstances.get()->add(Transform(vec3(0), vec3(0), vec3(5000)));
        // 136.273, -105.463,   15.575

        // skyboxInstances.get()->add(Transform(vec3(0, 1, 0), vec3(135*degToRad, -135*degToRad, 0), vec3(1)));

        roomInstances.get()->add();
        spaceShipInstances.get()->add(Transform(vec3(10.5, 5, 0), vec3(75*degToRad, 0, 15*degToRad), vec3(0.1f)));

        if(SUPERFLUX) {
            titleInstances.get()->add(Transform(vec3(-11, 0, 12), vec3(0), vec3(3)));
            titleInstances.get()->add(Transform(vec3(0, 1.5, -1.75), vec3(0, glm::pi<float>(), 0), vec3(0.4, 0.4, 1.0)));

            statueInstances.get()->add(Transform(vec3(-10.5, heightFoot, -11.25), vec3(0, 180*degToRad, 0)));
            statueInstances.get()->add(Transform(vec3(-1.75, heightFoot, -5), vec3(0, 180*degToRad, 0)));
    
            statueInstances.get()->add(Transform(vec3(-1.75, heightFoot, -2), vec3(0, 210*degToRad, 0)));
            statueInstances.get()->add(Transform(vec3(-1.75, heightFoot, 2), vec3(0, -30*degToRad, 0)));
            statueInstances.get()->add(Transform(vec3(+1.75, heightFoot, -2), vec3(0, 150*degToRad, 0)));
            statueInstances.get()->add(Transform(vec3(+1.75, heightFoot, 2), vec3(0, 30*degToRad, 0)));
    
            statueInstances.get()->add(Transform(vec3(+11, heightFoot, -11.25), vec3(0, 180*degToRad, 0)));
    
            citadelInstances.get()->add(Transform(vec3(0, 1440, -1200), vec3(), vec3(0.48)));
    
            // spaceShipInstances.get()->add(Transform(vec3(-80, 30, 128), vec3(0, 60*degToRad, 15*degToRad), vec3(3.0f)));
            spaceShipInstances.get()->add(Transform(vec3(0, 100, 1030), vec3(0, 30*degToRad, 0), vec3(20.0f)));
    
            // shipInstances.get()->add(Transform(vec3(0, 3.5, 0), vec3(-60*degToRad, 215*degToRad, 0), vec3(0.3f)));
    
            domeGlassInstances.get()->add(Transform());
            domeWireInstances.get()->add(Transform());
    
            benchInstances.get()->add(Transform(vec3(1.75, 0.0, -4), vec3(0, 90*degToRad, 0)));
            benchInstances.get()->add(Transform(vec3(1.75, 0.0, -7), vec3(0, 90*degToRad, 0)));
            benchInstances.get()->add(Transform(vec3(1.75, 0.0, -10), vec3(0, 90*degToRad, 0)));
            benchInstances.get()->add(Transform(vec3(3.0, 0.0, -11.25), vec3(0, 0*degToRad, 0)));
            benchInstances.get()->add(Transform(vec3(6.0, 0.0, -11.25), vec3(0, 0*degToRad, 0)));
            benchInstances.get()->add(Transform(vec3(9.0, 0.0, -11.25), vec3(0, 0*degToRad, 0)));
            benchInstances.get()->add(Transform(vec3(13.0, 0.0, -11.25), vec3(0, 0*degToRad, 0)));
            benchInstances.get()->add(Transform(vec3(16.0, 0.0, -11.25), vec3(0, 0*degToRad, 0)));
            benchInstances.get()->add(Transform(vec3(19.0, 0.0, -11.25), vec3(0, 0*degToRad, 0)));
    
            benchInstances.get()->add(Transform(vec3(19.0, 0.0, 11.25), vec3(0, 0*degToRad, 0)));
            benchInstances.get()->add(Transform(vec3(13.0, 0.0, 11.25), vec3(0, 0*degToRad, 0)));
            benchInstances.get()->add(Transform(vec3(16.0, 0.0, 11.25), vec3(0, 0*degToRad, 0)));
            benchInstances.get()->add(Transform(vec3(9.0, 0.0, 11.25), vec3(0, 0*degToRad, 0)));
            benchInstances.get()->add(Transform(vec3(6.0, 0.0, 11.25), vec3(0, 0*degToRad, 0)));
            benchInstances.get()->add(Transform(vec3(3.0, 0.0, 11.25), vec3(0, 0*degToRad, 0)));
            benchInstances.get()->add(Transform(vec3(1.75, 0.0, 10), vec3(0, 90*degToRad, 0)));
            benchInstances.get()->add(Transform(vec3(1.75, 0.0, 7), vec3(0, 90*degToRad, 0)));
            benchInstances.get()->add(Transform(vec3(1.75, 0.0, 4), vec3(0, 90*degToRad, 0)));
            
            const uint amountParticules = 250;
            const float particuleSize = 0.01f;
            for (uint i = 0; i < amountParticules; i++) {
                // particulesInstances.get()->add(Transform(vec3(linearRand(-21.0f, 0.0f), linearRand(-0.5f, 10.0f), linearRand(-12.0f, 12.0f)), vec3(), vec3(particuleSize)));
                particulesInstances.get()->add(Transform(vec3(linearRand(-21.0f, 21.0f), linearRand(-0.5f, 10.0f), linearRand(-12.0f, 12.0f)), vec3(), vec3(particuleSize)));
                // particulesInstances.get()->add(Transform(vec3(linearRand(-0.1f, 0.1f), linearRand(-0.5f, 3.0f), linearRand(-2.0f, 2.0f)), vec3(), vec3(particuleSize)));
            }
        }

        // at each walls of the map
        for (int z = -12; z <= 12; z++) {
            for (int x = -21; x <= 21; x++) {
                if((x == -1 && z == -4) || (x == -1 && z == -3) || (x == 10 && z == 12) || (x == 11 && z == 12) || (x == 12 && z == 12)) {
                    continue;
                }

                if (x == -21 || x == 21) {
                    collumnInstances.get()->add(Transform(vec3(x, 0, z)));
                }

                if (x != 0 && (z == -12 || z == 12)) {
                    collumnInstances.get()->add(Transform(vec3(x, 0, z)));
                }

                if ((x == -1 || x == 1) &&(z <= -2 || z >= 2)) {
                    collumnInstances.get()->add(Transform(vec3(x, 0, z)));
                }
                if (x == 0 && (z == -2 || z == 2)) {
                    collumnInstances.get()->add(Transform(vec3(x, 0, z)));
                }
            } 
        }


        shelveInstances.get()->add(Transform(vec3(-1, 0, -4), vec3(0, glm::pi<float>()/2, 0)));

        houseInstances.get()->add(Transform(vec3(-10.5, heightFoot, 0), vec3(0), vec3(0.2)));

        planeInstances.get()->add(Transform(vec3(1000, 15, -25), vec3(0, 0, -5*degToRad), vec3(1)));

        arrowInstances.get()->add();
    }

    // Add all walls
    {
        const float wallThickness = 1.0f;
        const float AddWallHeight = 0.0f;
        // walls.push_back(BBox3f(vec3(-220, -wallThickness*10, -130), vec3(220, -5, 130)));
        walls.push_back(BBox3f(vec3(-220, -wallThickness*10, -130), vec3(220, 0, 130)));
        walls.push_back(BBox3f(vec3(-1, -1, -12), vec3(1, 3+AddWallHeight, -2)));
        walls.push_back(BBox3f(vec3(-1, -1, 2), vec3(1, 3+AddWallHeight, 12)));
        walls.push_back(BBox3f(vec3(-22, -1, 12), vec3(22, 3+AddWallHeight, 12+wallThickness)));
        walls.push_back(BBox3f(vec3(-22, -1, -12-wallThickness), vec3(22, 3+AddWallHeight, -12)));
        walls.push_back(BBox3f(vec3(-21-wallThickness, -1, -13), vec3(-21, 3+AddWallHeight, 13)));
        walls.push_back(BBox3f(vec3(21, -1, -13), vec3(21+wallThickness, 3+AddWallHeight, 13)));
        {
            auto bbox = spaceShipInstances.get()->getBBox().at(0);
            vec3 size = bbox.size();
            auto fooCenter = center(bbox);
            cubeInstances.get()->add(Transform(vec3(center(bbox)), vec3(), size));

            footInstances.get()->add(Transform(vec3(fooCenter.x, 0, fooCenter.z), vec3(0), vec3(size.x*(1/0.9), 1, size.z*(1/0.9))));

            walls.push_back(bbox);
        }
        for (auto bbox : houseInstances.get()->getBBox()) {
            vec3 size = bbox.size();
            // cubeInstances.get()->add(Transform(vec3(center(bbox)), vec3(), size));

            auto fooCenter = center(bbox);
            footInstances.get()->add(Transform(vec3(fooCenter.x, 0, fooCenter.z), vec3(0), vec3(size.x*(1/0.9), 1, size.z*(1/0.9))));

            walls.push_back(bbox);
        }
        for (auto bbox : statueInstances.get()->getBBox()) {
            vec3 size = bbox.size();
            cubeInstances.get()->add(Transform(vec3(center(bbox)), vec3(), size));

            auto fooCenter = center(bbox);
            footInstances.get()->add(Transform(vec3(fooCenter.x, 0, fooCenter.z), vec3(0), vec3(size.x*(1/0.9), 1, size.z*(1/0.9))));


            walls.push_back(bbox);
        }

        for (auto bbox : footInstances.get()->getBBox()) {
            walls.push_back(bbox);
        }

        for (auto bbox : shelveInstances.get()->getBBox()) {
            walls.push_back(bbox);
        }

        for (auto bbox : benchInstances.get()->getBBox()) {
            walls.push_back(bbox);
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
// /**/bool computeNextFrame = false;
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

    Mirror mirror(vec3(11, 1.51, 11.99), vec3(0, 0, -1), applicationPath);
    mirror.init(win.width(), win.height());
    mirrorInstances.get()->add(Transform(vec3(11, 1.51, 11.99), vec3(-degToRad*90, 0, 0), vec3(3.0f, 1.0f, 6.0f)));
    // mirrorInstances.get()->add(Transform(vec3(11, 1.5, 11.999), vec3(-degToRad*90, 0, degToRad*180), vec3(5.0f, 3.0f, 3.0f)));
    GLuint mirrorMapId = mirror.getMirrorMap();

    mirror.computeCamera(fpsCam);

    std::vector<Button> buttons;

    for (size_t i = 0; i < allRoomTwoPrograms.size(); i++) {
        // -1.25, 1.25, -3.65
        buttons.push_back(Button(vec3(-1.14, 0.24+0.372*i, -3.78), 0.05));
        buttonInstances.get()->add(Transform(buttons.at(i).pos(), vec3(), vec3(0.05)));
    }

    Texture renderTexture(applicationPath);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// PARTICULES

std::vector<Animation*> allAnimations;

vec3 flagRootCenter(-11, 2.0, 6);
float windTheta = 0.0f;
float windPhy = 0.0f;
vec3 windDir = vec3(cos(windTheta)*sin(windPhy), sin(windTheta), cos(windTheta)*cos(windPhy));

arrowInstances.get()->updatePosition(0, vec3(flagRootCenter.x, 0.25, flagRootCenter.z));
arrowInstances.get()->updateAngles(0, vec3(-windTheta, windPhy, 0));
arrowInstances.get()->computeAll();

auto flagWindPower = 50.0f;
auto flagAnimation = Animation(imageFlagFrenchInt, imageWhiteInt, imageDefaultNormalInt);
auto count = 25;
// vec3 flagRootCenter(0, 0, 0);
{
    // vec3(1, 5+height-radiusCube, -radiusCube*1.5), vec3(1, 5+height+radiusCube, -radiusCube*1.5), vec3(1, 5+height-radiusCube, radiusCube*1.5), vec3(1, 5+height+radiusCube, radiusCube*1.5)
    allAnimations.push_back(&flagAnimation);
    float radiusCube = 2.0f;
    flagAnimation.make_flag(
        flagRootCenter + vec3(0, 0, 0),
        flagRootCenter + vec3(0, radiusCube, 0),
        flagRootCenter + vec3(-radiusCube*1.5, 0, 0),
        flagRootCenter + vec3(-radiusCube*1.5, radiusCube, 0),
        count, 0.5, 2000, 15);

    scene.addInstance(flagAnimation.getInstance());
    flagAnimation.getFields()->reserve(walls.size()+10);
    flagAnimation.addField(FieldType::field_directional, vec3(0, -1, 0), 9.81f);
    flagAnimation.addField(FieldType::field_directional, vec3(-1, 0, 0), 0.0f);
    // flagAnimation.addField(FieldType::field_convex, &allRigidBodies.at(0), wallFriction);
    {
        const float wallFriction = 0.1f;
        // for(auto &wall: walls) {
            //     flagAnimation.addField(FieldType::field_cube, wall, wallFriction);
            // }
        flagAnimation.addField(FieldType::field_wall, vec3(0, 0, 0), wallFriction);
    }
    flagAnimation.addField(FieldType::field_wind, applicationPath, &win, windDir, flagWindPower);
    
    // player BBox
    flagAnimation.addField(FieldType::field_cube, BBox3f(), 0.1);
    flagAnimation.activateMultithreaded(4, &fpsCam);

}
auto flagInstance = std::make_shared<Instance>(count, DynamicType::dynamic_grid, imageFlagFrenchInt, imageWhiteInt, imageDefaultNormalInt);
// flagInstance.get()->add(Transform(vec3(10, 10, 10)));
{
    auto miniFlagsScale = 0.25f;
    for (int z = -12; z <= 12; z++) {
        for (int x = -21; x <= 21; x++) {
            if((x == -1 && z == -4) || (x == -1 && z == -3) || (x == 10 && z == 12) || (x == 11 && z == 12) || (x == 12 && z == 12)) {
                continue;
            }
            if (x == -21 || x == 21) {
                flagInstance.get()->add(Transform(vec3(x, 3.5, z)-(miniFlagsScale*flagRootCenter), vec3(), vec3(miniFlagsScale)));
            }
    
            if (x != 0 && (z == -12 || z == 12)) {
                flagInstance.get()->add(Transform(vec3(x, 3.5, z)-(miniFlagsScale*flagRootCenter), vec3(), vec3(miniFlagsScale)));
            }
    
            if ((x == -1 || x == 1) &&(z <= -2 || z >= 2)) {
                flagInstance.get()->add(Transform(vec3(x, 3.5, z)-(miniFlagsScale*flagRootCenter), vec3(), vec3(miniFlagsScale)));
            }
            if (x == 0 && (z == -2 || z == 2)) {
                flagInstance.get()->add(Transform(vec3(x, 3.5, z)-(miniFlagsScale*flagRootCenter), vec3(), vec3(miniFlagsScale)));
            }
        } 
    }
}

{
    auto tmp_pos = flagAnimation.getParticulesPositions();
    flagInstance.get()->updateDynamicMesh(tmp_pos);
}
scene.addInstance(flagInstance);

auto blancketWindPower = 25.0f;

auto blancketAnimation = Animation(imageFlagFrenchInt, imageWhiteInt, imageDefaultNormalInt);
{
    allAnimations.push_back(&blancketAnimation);
    vec3 center(-10.5, 4, 0);
    vec2 dimensions(6, 4);
    dimensions*=0.8;
    blancketAnimation.make_grid(
        center + vec3(-dimensions.x, 0, -dimensions.y),
        center + vec3(-dimensions.x, 0,  dimensions.y),
        center + vec3( dimensions.x, 0, -dimensions.y),
        center + vec3( dimensions.x, 0,  dimensions.y),
        30, 1.0, 1000, 35
    );
    scene.addInstance(blancketAnimation.getInstance());
    blancketAnimation.getFields()->reserve(walls.size()+10);
    blancketAnimation.addField(FieldType::field_directional, vec3(0, -1, 0), 9.81);
    blancketAnimation.addField(FieldType::field_directional, vec3(-1, 0, 0), 0.0f);
    {
        const float wallFriction = 0.85f;
        for(auto &wall: walls) {
            blancketAnimation.addField(FieldType::field_cube, wall, wallFriction);
        }
    }
    blancketAnimation.addField(FieldType::field_wind, applicationPath, &win, vec3(0, 1, 0), 0.0f);
    blancketAnimation.addField(FieldType::field_cube, BBox3f(), 0.1);
    blancketAnimation.activateMultithreaded(2, &fpsCam);
}

auto cubeAnimation = Animation(imageApertureInt, 0, imageDefaultNormalInt);
{
    allAnimations.push_back(&cubeAnimation);
    vec3 center(-8, 4, -8);
    vec3 dimensions(1, 1, 1);
    dimensions*=0.8;
    cubeAnimation.make_cube(
        center, dimensions,
        4, 0.1, 10000, 3
    );
    scene.addInstance(cubeAnimation.getInstance());
    cubeAnimation.getFields()->reserve(walls.size()+10);
    cubeAnimation.addField(FieldType::field_directional, vec3(0, -1, 0), 9.81);
    cubeAnimation.addField(FieldType::field_directional, vec3(-1, 0, 0), 0.0f);
    {
        const float wallFriction = 0.1f;
        for(auto &wall: walls) {
            cubeAnimation.addField(FieldType::field_cube, wall, wallFriction);
        }
    }
    cubeAnimation.addField(FieldType::field_cube, BBox3f(), 0.0);
    cubeAnimation.activateMultithreaded(1, &fpsCam);
}

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

bool showtexture = false;
bool activateFan = false;
bool flagWindActivated = true;

/**//* Loop until the user closes the window */
    uint refreshTitle = 0;
    while (win.running()) {

        { // INIT CODE
            win.clear();
            fpsCam.update(win, walls, deltaT);
            mirror.computeCamera(fpsCam);
            timer = glfwGetTime();
            deltaT = std::min(timer - oldTime, 1.0/24);
            if(oldTime < 0) { // first frame
                deltaT = 1/60.0f;
            }
            if (animateSwitch) { // the animation is running
                animateTimer += deltaT;
            }
            oldTime = timer;
            currentCamPos = fpsCam.getPos();
        }

        { // UPDATES

            // Skyboc objects update
            {
                earthInstances.get()->updatePosition(0, vec3(1500, 1500, 1500) + fpsCam.getPos());
                earthInstances.get()->computeLast();

                sunInstances.get()->updatePosition(0, vec3(1500, 1500, -1500) + fpsCam.getPos());
                sunInstances.get()->computeLast();

                skyboxInstances.get()->updatePosition(0, fpsCam.getPos());
                skyboxInstances.get()->computeAll();
                
                if(SUPERFLUX) {
                    citadelInstances.get()->updatePosition(0, fpsCam.getPos() + vec3(0, 1440, -1200));
                    citadelInstances.get()->computeAll();
                }
            }

            for (size_t i = 0; i < particulesInstances.get()->size(); i++) {
                particulesInstances.get()->updateAngles(i, vec3(0, timer, 0));
                float initialHeight = particulesInstances.get()->get(i).m_Position.y;
                float newHeight = initialHeight + 0.89*deltaT;
                if (newHeight > 10.0f) {
                    newHeight -= (10.0f - -0.5f);
                }
                particulesInstances.get()->addPosition(i, vec3(0, newHeight-initialHeight, 0));

            }
            particulesInstances.get()->computeAll();

            for (size_t i = 0; i < lightInstances2.get()->size(); i++) {
                vec3 oldPos = lightInstances2.get()->get(i).m_Position;
                float t = (int(oldPos.x*oldPos.z)%7);
                vec3 newPos = vec3(oldPos.x, (2+cos(timer*(1+t))+cos(timer))*0.85, oldPos.z);
                lightInstances2.get()->updatePosition(i, newPos);
                lightInstances2.get()->compute(i);
                lightsRoomRight.updatePosition(i, newPos);
            }

            earthInstances.get()->updateAngles(0, vec3(0, timer*15.0, 0)*degToRad);

            lightsRoomLeft.updatePosition(2, vec3(-10.5+5*cos(timer/1.0), 1, 5*sin(timer/1.0)));
            lightInstances.get()->updatePosition(2, vec3(-10.5+5*cos(timer/1.0), 1, 5*sin(timer/1.0)));
            lightInstances.get()->compute(2);

            fpsCam.shake(0.0f);
            if (animateSwitch) {

                vec3 planePos = planeInstances.get()->get(0).m_Position;
                planePos += 931 * kmToMs * vec3(-1, 0, 0) * float(deltaT);

                if (planePos.x < -2000) {
                    planePos = vec3(1000, linearRand(9.0f, 15.0f), linearRand(-30.0f, 30.0f));
                }

                planeInstances.get()->updatePosition(0, planePos);
                planeInstances.get()->updateAngles(0, vec3(sin(animateTimer)*10.0f*degToRad, 0, -5*degToRad));
                planeInstances.get()->computeLast();


                float distanceToPlane = length(planeInstances.get()->get(0).m_Position - currentCamPos);

                if (distanceToPlane < 450) {
                    // clamp((distanceToPlane-450)/450, 0.0, 1.0);
                    // fpsCam.shake(1/(1+distanceToPlane));
                    auto shakeAmount = 0.1 * pow((450.0-distanceToPlane)/450, 4);
                    fpsCam.shake(shakeAmount);
                }
                
                distanceToPlane = abs(planeInstances.get()->get(0).m_Position.x);
                if (distanceToPlane < 450) {
                    auto shakeAmount = 45.0f * pow((450.0-distanceToPlane)/450, 4);
                    // std::cout << shakeAmount << std::endl;
                    blancketAnimation.getFields()->at(1).change_k(shakeAmount);
                    flagAnimation.getFields()->at(1).change_k(shakeAmount);
                    cubeAnimation.getFields()->at(1).change_k(shakeAmount);
                }
                else {
                    blancketAnimation.getFields()->at(1).change_k(0.0f);
                    flagAnimation.getFields()->at(1).change_k(0.0f);
                    cubeAnimation.getFields()->at(1).change_k(0.0f);
                }

            }

            if(activateFan) {
                blancketAnimation.getFields()->at(blancketAnimation.getFields()->size()-2).change_k(blancketWindPower);
            }
            else {
                blancketAnimation.getFields()->at(blancketAnimation.getFields()->size()-2).change_k(0.0f);
            }

            if(flagWindActivated) {
                flagAnimation.getFields()->at(flagAnimation.getFields()->size()-2).change_k(flagWindPower);
            }
            else {
                flagAnimation.getFields()->at(flagAnimation.getFields()->size()-2).change_k(0.0f);
            }
            // Important otherwise would not render asynchronously
            // std::cout<<windTheta<<" | "<<windPhy<<std::endl;
            windDir = vec3(cos(windTheta)*sin(windPhy), sin(windTheta), cos(windTheta)*cos(windPhy));
            arrowInstances.get()->updateAngles(0, vec3(-windTheta, windPhy, 0));
            arrowInstances.get()->computeAll();
            // std::cout<<windDir<<std::endl;
            flagAnimation.getFields()->at(flagAnimation.getFields()->size()-2).change_direction(windDir);
            for (size_t i = 0; i < allAnimations.size(); i++)
            {
                // std::cout<<"Update visual"<<std::endl;
                allAnimations.at(i)->updateFields();
                allAnimations.at(i)->update_visual();
            }
            {
                auto tmp_pos = flagAnimation.getParticulesPositions();
                flagInstance.get()->updateDynamicMesh(tmp_pos);
            }
            
            

            if(depthMapId != 0) {
                shadowMap.renderTexture(win, scene);
            }

        }

        if(showtexture) {
            renderTexture.render(flagAnimation.getFields()->at(flagAnimation.getFields()->size()-2).getDebugColorTexture());
            // renderTexture.render(shadowMap.getColorMap());
        }
        
        else { // RENDERING

            // mirroir
            if (boolRightRoom) {
                auto mirrorCam = mirror.getCamera();
                mirror.startRenderMirrorTexture();

                programSky.activate(win, mirrorCam, shadowMatrix, lightsRoomRight, sunPos);
                programSky.sendClippingPlane(mirror.getPlaneEquation());
                glCullFace(GL_FRONT);
                skyboxInstances.get()->drawAll(programSky, mirrorCam.getViewMatrix(), mirrorCam.getProjMatrix(), 0);
                glCullFace(GL_BACK);
                programSky.removeClippingPlane();


                // if the sun is active
                if (depthMapId != 0) {
                    programSun.activate(win, mirrorCam, shadowMatrix, lightsRoomRight, sunPos);
                    programSun.sendClippingPlane(mirror.getPlaneEquation());
                    sunInstances.get()->drawAll(programSun, mirrorCam.getViewMatrix(), mirrorCam.getProjMatrix(), 0);
                    programSun.removeClippingPlane();
                }

                if(mirrorCam.getPos().x < -0.1) {
                    boolRightRoom = false;
                }
                if(mirrorCam.getPos().x > 0.1) {
                    boolRightRoom = true;
                }
                
                if(boolRightRoom) {
                    currentProgram->activate(win, mirrorCam, shadowMatrix, lightsRoomRight, sunPos);
                    currentProgram->sendClippingPlane(mirror.getPlaneEquation());
                    scene.drawScene(win, *currentProgram, mirrorCam, shadowMatrix, lightsRoomRight, depthMapId, sunPos);
                    currentProgram->removeClippingPlane();
                    programLight.activate(win, mirrorCam, shadowMatrix, lightsRoomRight, sunPos);
                    programLight.sendClippingPlane(mirror.getPlaneEquation());
                    lightInstances2.get()->drawAll(programLight, mirrorCam.getViewMatrix(), mirrorCam.getProjMatrix(), depthMapId);
                    programLight.removeClippingPlane();
                }
                else {
                    programRoom.activate(win, mirrorCam, shadowMatrix, lightsRoomRight, sunPos);
                    programRoom.sendClippingPlane(mirror.getPlaneEquation());
                    scene.drawScene(win, programRoom, mirrorCam, shadowMatrix, lightsRoomLeft, depthMapId, sunPos);
                    programRoom.removeClippingPlane();
                    programLight.activate(win, mirrorCam, shadowMatrix, lightsRoomLeft, sunPos);
                    programLight.sendClippingPlane(mirror.getPlaneEquation());
                    lightInstances.get()->drawAll(programLight, mirrorCam.getViewMatrix(), mirrorCam.getProjMatrix(), depthMapId);
                    programLight.removeClippingPlane();
                }
                mirror.stopRenderMirrorTexture();
            }
            
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

            programMirror.activate(win, fpsCam, mirror.getMirrorProj(), lightsRoomRight, sunPos);

            GLint polygonMode[2];
            glGetIntegerv(GL_POLYGON_MODE, polygonMode);
            if(polygonMode[1] == GL_LINE) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
                    mirrorInstances.get()->drawAll(programMirror, fpsCam.getViewMatrix(), fpsCam.getProjMatrix(), mirrorMapId);
            if(polygonMode[1] == GL_LINE) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            }
            programButtons.activate(win, fpsCam, shadowMatrix, lightsRoomLeft, sunPos);
            buttonInstances.get()->drawAll(programButtons, fpsCam.getViewMatrix(), fpsCam.getProjMatrix(), depthMapId);
            
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
                for (size_t i = 0; i < allAnimations.size(); i++)
                {
                    allAnimations.at(i)->setComputeState(animateSwitch);
                }
                // computeAnim = !computeAnim;
            }

            if(keys & rotateLeft) {
                windPhy+=degToRad * 90 * deltaT;
            }
            if(keys & rotateRight) {
                windPhy-=degToRad * 90 * deltaT;
            }
            if(keys & rotateUp) {
                windTheta+=degToRad * 90 * deltaT;
                if(windTheta>glm::half_pi<float>()) {
                    windTheta = glm::half_pi<float>();
                }
            }
            if(keys & rotateDown) {
                windTheta-=degToRad * 90 * deltaT;
                if(windTheta<-glm::half_pi<float>()) {
                    windTheta = -glm::half_pi<float>();
                }
            }

            // if(keys & (rotateDown|rotateLeft|rotateRight|rotateUp)) {
            //     std::cout<<vec3(cos(windTheta)*sin(windPhy), sin(windTheta), cos(windTheta)*cos(windPhy))<<std::endl;
            // }

            if (keys & keyWrap) {
                fpsCam.makeLookAt(startPoint, startLookPoint);
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
                for (size_t i = 0; i < allAnimations.size(); i++)
                {
                    allAnimations.at(i)->reset();
                }
            }
            
            showtexture = keys & keyTexture;
            activateFan = keys & keyFan;
            if (keys & keyFlagWind) {
                flagWindActivated = !flagWindActivated;
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

            for (size_t i = 0; i < buttons.size(); i++) {
                if(buttons.at(i).click(fpsCam, keys & leftClick)) {
                    // currentRoom = (currentRoom+1) % allRoomTwoPrograms.size();
                    currentRoom = i;
                    currentProgram = allRoomTwoPrograms.at(currentRoom);
                    // win.quit();
                }
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
            win.updateTitle(fpsCam.getPos(), deltaT, flagAnimation.getDeltaTThreads());
        }
        else {
            refreshTitle++;
        }
    }

    // killThreads = true;
    // for (uint i = 0; i < threadList.size(); i++)
    // {
    //     threadList.at(i).join();
    // }
    for (size_t i = 0; i < allAnimations.size(); i++)
    {
        allAnimations.at(i)->stopMultithreads();
    }
    std::cout << "Thread joined" << std::endl;

    skyboxInstances.get()->~Instance();
    lightInstances.get()->~Instance();
    lightInstances2.get()->~Instance();
    sunInstances.get()->~Instance();
    scene.~Scene();
    sceneProj.~Scene();
    sceneUnused.~Scene();

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

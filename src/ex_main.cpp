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
#include <glimac/common.hpp>


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

int main(int /*argc*/, char * argv[])
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


    /*********************************
     * HERE SHOULD COME THE INITIALIZATION CODE
     *********************************/
    
    glimac::FilePath applicationPath(argv[0]);

    BasicProgram programRoom(applicationPath, "src/shaders/ground/ground.vs.glsl", "src/shaders/ground/ground.fs.glsl");
    // BasicProgram programRoom(applicationPath, "src/shaders/utils/simple_depth.vs.glsl", "src/shaders/utils/simple_depth.fs.glsl");
    BasicProgram programSimpleDepth(applicationPath, "src/shaders/utils/simple_depth.vs.glsl", "src/shaders/utils/simple_depth.fs.glsl");
    BasicProgram programMirrorTex(applicationPath, "src/shaders/utils/mirror_texture_test.vs.glsl", "src/shaders/utils/mirror_texture_test.fs.glsl");
    // BasicProgram programRoom(applicationPath, "src/shaders/utils/pattern.vs.glsl", "src/shaders/utils/pattern.fs.glsl");
    BasicProgram programShadow(applicationPath, "src/shaders/utils/shadow.vs.glsl", "src/shaders/utils/shadow.fs.glsl", ProgramType::DEPTH_COMPUTE);
    BasicProgram programShadowTest(applicationPath, "src/shaders/utils/check_shadow.vs.glsl", "src/shaders/utils/check_shadow.fs.glsl");
    BasicProgram programSky(applicationPath, "src/shaders/skybox/skybox_shader.vs.glsl", "src/shaders/skybox/skybox_shader.fs.glsl", ProgramType::TEXTURE);
    BasicProgram programNormal(applicationPath, "src/shaders/utils/normal.vs.glsl", "src/shaders/utils/normal.fs.glsl", ProgramType::NONE);
    BasicProgram programDepth(applicationPath, "src/shaders/utils/depth.vs.glsl", "src/shaders/utils/depth.fs.glsl", ProgramType::NONE);
    BasicProgram programLight(applicationPath, "src/shaders/light/light.vs.glsl", "src/shaders/light/light.fs.glsl", ProgramType::LIGHTS);
    BasicProgram programVoronoi(applicationPath, "src/shaders/utils/voronoi.vs.glsl", "src/shaders/utils/voronoi.fs.glsl", ProgramType::LIGHTS);
    BasicProgram programSun(applicationPath, "src/shaders/utils/white.vs.glsl", "src/shaders/utils/white.fs.glsl", ProgramType::NONE);
    BasicProgram programMirror(applicationPath, "src/shaders/utils/mirror.vs.glsl", "src/shaders/utils/mirror.fs.glsl");
    BasicProgram programButtons(applicationPath, "src/shaders/utils/buttons.vs.glsl", "src/shaders/utils/buttons.fs.glsl");
    std::vector<BasicProgram*> allPrograms = {&programVoronoi, &programMirrorTex, &programSimpleDepth, &programShadow, &programShadowTest, &programRoom, &programSky, &programNormal, &programDepth, &programLight};

    std::vector<BasicProgram*> allRoomTwoPrograms = {&programVoronoi, &programRoom, &programNormal, &programDepth, &programShadow};
    // std::vector<BasicProgram*> allRoomTwoPrograms = {&programVoronoi, &programMirrorTex, &programSimpleDepth, &programShadow, &programShadowTest, &programRoom, &programSky, &programNormal, &programDepth, &programLight};

    GLuint imageEarthInt = bind_texture(applicationPath.dirPath() + "/assets/textures/EarthMap.jpg");
    GLuint imageCloudInt = bind_texture(applicationPath.dirPath() + "/assets/textures/CloudMap.jpg");
    GLuint imageMoonInt  = bind_texture(applicationPath.dirPath() + "/assets/textures/MoonMap.jpg");

    GLuint imageSkyboxInt = bind_texture(applicationPath.dirPath() + "/assets/textures/alpha-mayoris.jpg");

    GLuint imageBrickDiffuseInt   = bind_texture(applicationPath.dirPath() + "/assets/textures/bricks_internet.jpg");
    GLuint imageBrickRoughnessInt = bind_texture(applicationPath.dirPath() + "/assets/textures/bricks_roughness.jpg");
    GLuint imageBrickNormalInt    = bind_texture(applicationPath.dirPath() + "/assets/textures/bricks_normal.jpg");

    GLuint imageHouseDiffuseInt   = bind_texture(applicationPath.dirPath() + "/assets/textures/cottage_diffuse.png");
    GLuint imageHouseRoughnessInt = bind_texture(applicationPath.dirPath() + "/assets/textures/cottage_roughness.jpg");
    GLuint imageHouseNormalInt    = bind_texture(applicationPath.dirPath() + "/assets/textures/cottage_normal.jpg");

    GLuint imageStatueInt = bind_texture(applicationPath.dirPath() + "/assets/textures/statue.jpg");

    GLuint imageCollumnDiffuseInt   = bind_texture(applicationPath.dirPath() + "/assets/textures/marble.jpg");
    GLuint imageCollumnRoughnessInt = bind_texture(applicationPath.dirPath() + "/assets/textures/collumn_roughness.jpg");
    GLuint imageCollumnNormalInt    = bind_texture(applicationPath.dirPath() + "/assets/textures/collumn_normal.jpg");


    GLuint imageFootDiffuseInt   = bind_texture(applicationPath.dirPath() + "/assets/textures/MarbleFoot.jpg");
    GLuint imageFootRoughnessInt = bind_texture(applicationPath.dirPath() + "/assets/textures/foot_rougness.jpg");
    GLuint imageFootNormalInt    = bind_texture(applicationPath.dirPath() + "/assets/textures/foot_normal.jpg");

    GLuint imageGlassDiffuseInt = bind_texture(applicationPath.dirPath() + "/assets/textures/frosted.png");
    GLuint imageGlassNormalInt = bind_texture(applicationPath.dirPath() + "/assets/textures/glass.jpg");

    GLuint imageWhiteInt = bind_texture(applicationPath.dirPath() + "/assets/textures/white.png");
    GLuint imageBenchInt = bind_texture(applicationPath.dirPath() + "/assets/textures/bench.jpg");
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
        &imageWhiteInt,
        &imageBenchInt,
        &imageDefaultNormalInt
    };


    glimac::Sphere sphere = glimac::Sphere(1, 32, 16);
    glimac::Sphere sphereInverted = glimac::Sphere(-1, 32, 16);
    glimac::Sphere sphereLowPoly = glimac::Sphere(1, 8, 4);
    glimac::Sphere sphereLowPolyParticule = glimac::Sphere(1, 4, 2);

    auto skyboxInstances = std::make_shared<Instance>(sphereInverted.getVertexCount(), sphereInverted.getDataPointer(), imageSkyboxInt, 0, imageDefaultNormalInt);
    auto earthInstances = std::make_shared<Instance>(sphere.getVertexCount(), sphere.getDataPointer(), imageEarthInt, imageCloudInt, imageDefaultNormalInt);
    auto sunInstances = std::make_shared<Instance>(sphere.getVertexCount(), sphere.getDataPointer(), imageWhiteInt, 0, imageDefaultNormalInt);

    auto particulesInstances = std::make_shared<Instance>(sphereLowPolyParticule.getVertexCount(), sphereLowPolyParticule.getDataPointer(), imageMoonInt, 0, imageDefaultNormalInt);
    auto lightInstances = std::make_shared<Instance>(sphereLowPoly.getVertexCount(), sphereLowPoly.getDataPointer(), 0, 0, imageDefaultNormalInt);
    auto lightInstances2 = std::make_shared<Instance>(sphereLowPoly.getVertexCount(), sphereLowPoly.getDataPointer(), 0, 0, imageDefaultNormalInt);

    auto roomInstances = std::make_shared<Instance>(applicationPath.dirPath(), "dsda", imageBrickDiffuseInt, imageBrickRoughnessInt, imageBrickNormalInt);
    auto collumnInstances = std::make_shared<Instance>(applicationPath.dirPath(), "collumn3", imageCollumnDiffuseInt, imageCollumnRoughnessInt, imageCollumnNormalInt);
    auto houseInstances = std::make_shared<Instance>(applicationPath.dirPath(), "cottage", imageHouseDiffuseInt, imageHouseRoughnessInt, imageHouseNormalInt);
    auto statueInstances = std::make_shared<Instance>(applicationPath.dirPath(), "statue", imageStatueInt, 0, imageDefaultNormalInt);
    auto footInstances = std::make_shared<Instance>(applicationPath.dirPath(), "foot", imageFootDiffuseInt, imageFootRoughnessInt, imageFootNormalInt);
    auto cubeInstances = std::make_shared<Instance>(applicationPath.dirPath(), "simpleCube2", imageGlassDiffuseInt, 0, imageGlassNormalInt);
    auto buttonInstances = std::make_shared<Instance>(sphere.getVertexCount(), sphere.getDataPointer(), imageWhiteInt, imageWhiteInt, imageDefaultNormalInt);
    auto benchInstances = std::make_shared<Instance>(applicationPath.dirPath(), "bench", imageBenchInt, 0, imageDefaultNormalInt);

    auto domeGlassInstances = std::make_shared<Instance>(applicationPath.dirPath(), "domeGlass", imageGlassDiffuseInt, 0, imageGlassNormalInt);
    auto domeWireInstances = std::make_shared<Instance>(applicationPath.dirPath(), "domeWire", imageWhiteInt, 0, imageDefaultNormalInt);
    // auto debugCubeInstances = std::make_shared<Instance>(applicationPath.dirPath(), "simpleCube", 0, 0);
    // auto cubeInstances = std::make_shared<Instance>(applicationPath.dirPath(), "cube", imageWhiteInt, 0);

    auto shelveInstances = std::make_shared<Instance>(applicationPath.dirPath(), "shelve", imageMoonInt, 0, imageDefaultNormalInt);
    auto titleInstances = std::make_shared<Instance>(applicationPath.dirPath(), "title", imageBrickDiffuseInt, 0, imageDefaultNormalInt);
    auto spaceShipInstances = std::make_shared<Instance>(applicationPath.dirPath(), "Farragut", imageBrickDiffuseInt, 0, imageDefaultNormalInt);
    auto planeInstances = std::make_shared<Instance>(applicationPath.dirPath(), "plane", imageWhiteInt, 0, imageDefaultNormalInt);
    auto citadelInstances = std::make_shared<Instance>(applicationPath.dirPath(), "citadel", imageStatueInt, 0, imageDefaultNormalInt);
    auto mirrorInstances = std::make_shared<Instance>(applicationPath.dirPath(), "mirrorGeometry", imageWhiteInt, 0, imageDefaultNormalInt);

    auto shipInstances = std::make_shared<Instance>(applicationPath.dirPath(), "ship", imageWhiteInt, 0, imageDefaultNormalInt);

    Scene scene;

    // Add all objects to the scene
    {
        scene.addInstance(roomInstances);
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
        // scene.addInstance(buttonInstances);
        scene.addInstance(domeWireInstances);
        scene.addInstance(benchInstances);
        scene.addInstance(shipInstances);
        // scene.addInstance(mirrorInstances);

        // transparents objects
        scene.addInstance(cubeInstances);
        scene.addInstance(domeGlassInstances);

        cubeInstances.get()->setBlendToTransparent();
        domeGlassInstances.get()->setBlendToTransparent();
        // houseInstances.get()->setBlendToTransparent();
    }

    buttonInstances.get()->setBlendToTransparent();

    // SHADERS INVARIANTS


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

        titleInstances.get()->add(Transform(vec3(-11, 0, 12), vec3(0), vec3(3)));
        titleInstances.get()->add(Transform(vec3(0, 1.5, -1.75), vec3(0, glm::pi<float>(), 0), vec3(0.4, 0.4, 1.0)));

        shelveInstances.get()->add(Transform(vec3(-1, 0, -4), vec3(0, glm::pi<float>()/2, 0)));

        houseInstances.get()->add(Transform(vec3(-10.5, heightFoot, 0), vec3(0), vec3(0.2)));

        planeInstances.get()->add(Transform(vec3(1000, 15, -25), vec3(0, 0, -5*degToRad), vec3(1)));

        statueInstances.get()->add(Transform(vec3(-10.5, heightFoot, -11.25), vec3(0, 180*degToRad, 0)));
        statueInstances.get()->add(Transform(vec3(-1.75, heightFoot, -5), vec3(0, 180*degToRad, 0)));

        statueInstances.get()->add(Transform(vec3(-1.75, heightFoot, -2), vec3(0, 210*degToRad, 0)));
        statueInstances.get()->add(Transform(vec3(-1.75, heightFoot, 2), vec3(0, -30*degToRad, 0)));
        statueInstances.get()->add(Transform(vec3(+1.75, heightFoot, -2), vec3(0, 150*degToRad, 0)));
        statueInstances.get()->add(Transform(vec3(+1.75, heightFoot, 2), vec3(0, 30*degToRad, 0)));

        statueInstances.get()->add(Transform(vec3(+11, heightFoot, -11.25), vec3(0, 180*degToRad, 0)));

        citadelInstances.get()->add(Transform(vec3(0, 1440, -1200), vec3(), vec3(0.48)));

        spaceShipInstances.get()->add(Transform(vec3(10.5, 5, 0), vec3(75*degToRad, 0, 15*degToRad), vec3(0.1f)));
        // spaceShipInstances.get()->add(Transform(vec3(-80, 30, 128), vec3(0, 60*degToRad, 15*degToRad), vec3(3.0f)));
        spaceShipInstances.get()->add(Transform(vec3(0, 100, 1030), vec3(0, 30*degToRad, 0), vec3(20.0f)));

        shipInstances.get()->add(Transform(vec3(0, 3.5, 0), vec3(-60*degToRad, 215*degToRad, 0), vec3(0.3f)));

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


        // buttonInstances.get()->add(Transform(vec3(0, 1.78, 0), vec3(0), vec3(0.05)));

        // debugCubeInstances.get()->add(Transform(vec3(0, 3, 0)));
    }

    std::vector<BBox3f> walls;

    // Add all the walls
    {
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

        // for (auto bbox : collumnInstances.get()->getBBox()) {

        //     vec3 center = bbox.lower + bbox.size()/2.0f;
        //     vec3 size = bbox.size();
        //     std::cout << bbox.lower << " " << bbox.lower + vec3(size.x, 0.35, size.z) << std::endl;

        //     auto baseBBox = BBox3f(bbox.lower, bbox.lower + vec3(size.x, 0.35, size.z));
        //     auto collumnBBox = BBox3f(center + vec3(-0.25, -bbox.size().y/2.0, -0.25), center + vec3(0.25, bbox.size().y/2.0, 0.25));
        //     auto topBBox = BBox3f(bbox.upper - vec3(size.x, 0.35, size.z) + vec3(size.x, 0.35, size.z), bbox.upper);

        //     cubeInstances.get()->add(Transform(vec3(bbox.lower)+vec3(0.5*size.x, 0.5*0.35, 0.5*size.z), vec3(), vec3(size.x, 0.35, size.z)));
        //     cubeInstances.get()->add(Transform(center, vec3(), vec3(0.5, bbox.size().y, 0.5)));
        //     cubeInstances.get()->add(Transform(vec3(bbox.upper)-vec3(0.5*size.x, 0.5*0.35, 0.5*size.z), vec3(), vec3(size.x, 0.35, size.z)));
        //     walls.push_back(baseBBox);
        //     walls.push_back(collumnBBox);
        //     walls.push_back(topBBox);
        // }

        const float wallThickness = 1.0f;
        walls.push_back(BBox3f(vec3(-21, 0, 12), vec3(21, 3, 12+wallThickness)));
        walls.push_back(BBox3f(vec3(-21, 0, -12-wallThickness), vec3(21, 3, -12)));
        walls.push_back(BBox3f(vec3(-21-wallThickness, 0, -12), vec3(-21, 3, 12)));
        walls.push_back(BBox3f(vec3(21, 0, -12), vec3(21+wallThickness, 3, 12)));
        walls.push_back(BBox3f(vec3(-21, -wallThickness*10, -12), vec3(21, 0, 12)));
        walls.push_back(BBox3f(vec3(-1, 0, 2), vec3(1, 3, 12)));
        walls.push_back(BBox3f(vec3(-1, 0, -12), vec3(1, 3, -2)));
    }

    std::cout << "Scene instances " << scene.count() << std::endl;

    Light lightsRoomLeft;
    Light lightsRoomRight;

    // Add all the lights
    {
        const float lightSize = 0.05f;

        lightsRoomLeft.add(LightStruct(vec3(-1.25, 1.25, -3.65), vec3(1, 0.6, 0.2), vec3(15, 1, LightType::pointLight)));
        lightInstances.get()->add(Transform(vec3(-1.25, 1.25, -3.65), vec3(0), vec3(lightSize)));

        lightsRoomLeft.add(LightStruct(vec3(-13, 0.25, 3.4), vec3(1, 0.1, 0.1), vec3(15, 1, LightType::pointLight)));
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

        // on the one statue in room 1
        lightsRoomLeft.add(LightStruct(vec3(-10.5, 2.5, -11.25), vec3(0.8, 0.9, 1), vec3(15, 1, LightType::pointLight)));
        lightInstances.get()->add(Transform(vec3(-10.5, 2.5, -11.25), vec3(0), vec3(lightSize)));

        for (int y = 1; y < 3; y++) {
            for (int x = -20; x < -1; x++) {
                lightsRoomLeft.add(LightStruct(vec3(x, y, 11.5), vec3(-x/20.0, y/2.0, 1), vec3(4, 0.4, lightSize)));
                lightInstances.get()->add(Transform(vec3(x, y, 11.5), vec3(0), vec3(lightSize)));
            }
        }
        
        // lightsRoomRight.add(LightStruct(vec3(2, 1, 1), vec3(1, 1, 1), vec3(15, 1, lightSize)));
        // lightsRoomRight.add(LightStruct(vec3(5, 5, 5), vec3(1, 0.8, 0.4), vec3(15, 1, lightSize)));
        // lightsRoomRight.add(LightStruct(vec3(18, 3.5, 9), vec3(0.4, 0.6, 1), vec3(15, 1, lightSize)));

        // lightInstances.get()->add(Transform(vec3(2, 1, 1), vec3(0), vec3(lightSize)));
        // lightInstances.get()->add(Transform(vec3(5, 5, 5), vec3(0), vec3(lightSize)));
        // lightInstances.get()->add(Transform(vec3(18, 3.5, 9), vec3(0), vec3(lightSize)));

        // int numberBeforeRandLights = lightInstances.get()->size();

        // int nbRandLight = 1;
        int nbRandLight = 200;

        for (int i = 0; i < nbRandLight; i++) {
            vec3 pos = vec3(linearRand(1.0f, 20.0f), linearRand(.1f, 1.0f), linearRand(-11.0f, 11.0f));
            // vec3 pos = ballRand(10.0f) + vec3(10.0f, 10.0f, 0);
            pos = vec3(pos.x, pos.y/20, pos.z);
            vec3 color = vec3(linearRand(.1f, 1.0f), linearRand(.1f, 1.0f), linearRand(.1f, 1.0f));
            // vec3 color = vec3(1.0, 1.0, 1.0);
            lightsRoomRight.add(LightStruct(pos, color, vec3(3, 0.5, lightSize)));
            lightInstances2.get()->add(Transform(pos, vec3(0), vec3(lightSize)));
        }
        

        // lightsRoomLeft.add( LightStruct(vec3(0, 5, 0), vec3(1), vec3(20, 1, lightSize)));
        // lightsRoomRight.add(LightStruct(vec3(40, 40, -40), vec3(1), vec3(20, 1, lightSize)));
        // lightInstances2.get()->add(Transform(vec3(40, 40, -40), vec3(0), vec3(lightSize*10)));
    }


    unsigned int currentRoom = 0;
    BasicProgram *currentProgram = allPrograms.at(currentRoom);

    std::cout << "Starting main loop" << std::endl;

    float oldTime = -1.0f;
    float deltaT = 0;
    bool animateSwitch = true;
    float timer = 0.0f;
    float animateTimer = 0.0f;
    bool boolRightRoom = false;

    // glm::mat4 projMatrix;
    // glm::mat4 ModelToViewVMatrix;
    // glm::mat4 NormalMatrix;

    glm::mat4 shadowMatrix;

    vec3 currentCamPos;
    vec2 oldMouse;
    FPSCamera fpsCam = FPSCamera(win.width(), win.height());
    fpsCam.resetMouse(win.mouse());

    
    ShadowMap shadowMap(applicationPath);
    shadowMap.init();

    shadowMap.computeTransforms(LightStruct(vec3(40, 40, -40), vec3(1), vec3(100, 0.85, LightType::directionalLight)));

    GLuint depthMapId;
    depthMapId = shadowMap.getDepthMap();
    shadowMatrix = shadowMap.getShadowMatrix();
    shadowMap.renderTexture(win, scene);
    vec3 sunPos = shadowMap.getLightPos();

    Mirror mirror(vec3(11, 1.51, 11.99), vec3(0, 0, -1), applicationPath);
    mirror.init(win.width(), win.height());
    mirrorInstances.get()->add(Transform(vec3(11, 1.51, 11.99), vec3(-degToRad*90, 0, 0), vec3(3.0f, 1.0f, 6.0f)));
    // mirrorInstances.get()->add(Transform(vec3(11, 1.5, 11.999), vec3(-degToRad*90, 0, degToRad*180), vec3(5.0f, 3.0f, 3.0f)));
    GLuint mirrorMapId = mirror.getMirrorMap();

    mirror.computeCamera(fpsCam);

    bool fromMirror = false;

    std::vector<Button> buttons;

    for (int i = 0; i < 5; i++) {
        // -1.25, 1.25, -3.65
        buttons.push_back(Button(vec3(-1.14, 0.24+0.372*i, -3.78), 0.05));
        buttonInstances.get()->add(Transform(buttons.at(i).pos(), vec3(), vec3(0.05)));
    }
    

    // buttons.push_back(Button(vec3(0, 1.78, 0), 0.05));
    // buttons.push_back(Button(vec3(0, 1.78, 0), 0.05));
    // buttons.push_back(Button(vec3(0, 1.78, 0), 0.05));
    // buttons.push_back(Button(vec3(0, 1.78, 0), 0.05));

    vec3 skyBoxAngles = vec3(135, -105, 15);

    /* Loop until the user closes the window */
    while (win.running()) {

        { // INIT CODE
            clear_screen();
            fpsCam.update(win, walls, deltaT);
            mirror.computeCamera(fpsCam);
            timer = glfwGetTime();
            deltaT = std::min(timer - oldTime, 1.0f/24);
            if(oldTime < 0) { // first frame
                deltaT = 0.0f;
            }
            if (animateSwitch) { // the animation is running
                animateTimer += deltaT;
            }
            oldTime = timer;
            oldMouse = win.mouse();
            // ModelToViewVMatrix = fpsCam.getViewMatrix();
            // NormalMatrix = glm::transpose(glm::inverse(ModelToViewVMatrix));
            // currentCamPos = fpsCam.getPos();
            // projMatrix = fpsCam.getProjMatrix();
        }

        { // UPDATES

            // Skyboc objects update
            {
                earthInstances.get()->updatePosition(0, vec3(1500, 1500, 1500) + currentCamPos);
                earthInstances.get()->computeLast();

                sunInstances.get()->updatePosition(0, vec3(1500, 1500, -1500) + currentCamPos);
                sunInstances.get()->computeLast();

                skyboxInstances.get()->updatePosition(0, currentCamPos);
                skyboxInstances.get()->computeAll();
                
                citadelInstances.get()->updatePosition(0, currentCamPos + vec3(0, 1440, -1200));
                citadelInstances.get()->computeAll();
            }

            // std::cout << vec3(40*cos(timer*0.2), 40, -40*sin(timer*0.2)) << std::endl;

            // shadowMap.computeTransforms(LightStruct(vec3(40*cos(timer*0.2), 40*cos(timer*1.2), -40*sin(timer*0.2)), vec3(1), vec3(100, 0.85, LightType::directionalLight)), fpsCam);
            // shadowMatrix = shadowMap.getShadowMatrix();
            // sunPos = shadowMap.getLightPos();

            // sunPos = vec3(1, 1, -1);
            // std::cout << sunPos << std::endl;

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

            // if (fromMirror) {
            //     lightsRoomLeft.updatePosition(1, vec3(fpsCam.getPos().x, 0.25, fpsCam.getPos().z));
            //     lightInstances.get()->updatePosition(1, vec3(fpsCam.getPos().x, 0.25, fpsCam.getPos().z));
            //     lightInstances.get()->compute(1);
            // }

            fpsCam.shake(0.0f);
            if (animateSwitch) {

                vec3 planePos = planeInstances.get()->get(0).m_Position;
                planePos += 931 * kmToMs * vec3(-1, 0, 0) * deltaT;

                if (planePos.x < -2000) {
                    planePos = vec3(1000, linearRand(9.0f, 20.0f), linearRand(-30.0f, 30.0f));
                }

                planeInstances.get()->updatePosition(0, planePos);
                planeInstances.get()->updateAngles(0, vec3(sin(animateTimer)*10.0f*degToRad, 0, -5*degToRad));
                planeInstances.get()->computeLast();


                float distanceToPlane = length(planeInstances.get()->get(0).m_Position - currentCamPos);

                if (distanceToPlane < 450) {
                    // clamp((distanceToPlane-450)/450, 0.0, 1.0);
                    // fpsCam.shake(1/(1+distanceToPlane));
                    fpsCam.shake(0.1 * pow((450.0-distanceToPlane)/450, 4));
                }

            }

            skyboxInstances.get()->updateAngles(0, vec3(skyBoxAngles.x*degToRad, skyBoxAngles.y*degToRad, skyBoxAngles.z*degToRad));
            skyboxInstances.get()->computeAll();
        }

        { // RENDERING

            if(depthMapId != 0) {
                shadowMap.renderTexture(win, scene);
            }

            // mirroir
            if (true) {
                // auto mirrorCamPos = mirror.getMirrorCamPos();
                // auto mirrorModelToViewMatrix = mirror.getViewMatrix();
                // auto mirrorTexjMatrix = mirror.getMirrorProj();
                // auto mirrorNormalMatrix = mirror.getNormalMatrix();
                // auto mirrorProjMatrix = mirror.getMirrorProj();
                auto mirrorCam = mirror.getCamera();
                // mirror.drawStencil(fpsCam);
                mirror.startRenderMirrorTexture();
                // mirror.drawStencil(win, mirrorCam);

                // glViewport(0, 0, 1600, 900);
                // glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mirror.getMirrorFBO());
                // glEnable(GL_DEPTH_TEST);

                // glBindRenderbuffer(GL_RENDERBUFFER, mirror.getMirrorDepthFBO());
                // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

                // glReadBuffer(GL_NONE);

                // glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
                // glBindFramebuffer(GL_READ_FRAMEBUFFER, mirror.getMirrorFBO());

                // glActiveTexture(GL_TEXTURE2);
                // glBindTexture(GL_TEXTURE_2D, mirror.getMirrorMap());

                // glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 0, 0, 1600, 900, 0);

                // glBlitFramebuffer(0, 0, 1600, 900, 0, 0, 1600, 900, GL_COLOR_BUFFER_BIT, GL_NEAREST);

                // glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

                // glBindFramebuffer(GL_FRAMEBUFFER, 0);

                // // glBindRenderbuffer(GL_RENDERBUFFER, 0);

                // glViewport(0, 0, 1600, 900);
                // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                mirror.stopRenderMirrorTexture();
                // mirror.deactivate();
            }




            if (fromMirror) {
                auto mirrorCam = mirror.getCamera();
                auto mirrorTexjMatrix = mirror.getMirrorProj();

                programSky.activate(win, mirrorCam, shadowMatrix, lightsRoomRight, sunPos);
                glCullFace(GL_FRONT);
                skyboxInstances.get()->drawAll(programSky, mirrorCam.getViewMatrix(), mirrorCam.getProjMatrix(), 0);
                glCullFace(GL_BACK);

                // if the sun is active
                if (depthMapId != 0) {
                    programSun.activate(win, mirrorCam, shadowMatrix, lightsRoomRight, sunPos);
                    sunInstances.get()->drawAll(programSun, mirrorCam.getViewMatrix(), mirrorCam.getProjMatrix(), 0);
                }

                if(mirrorCam.getPos().x < -0.1) {
                    boolRightRoom = false;
                }
                if(mirrorCam.getPos().x > 0.1) {
                    boolRightRoom = true;
                }
                
                if(boolRightRoom) {
                    scene.drawScene(win, *currentProgram, mirrorCam, shadowMatrix, lightsRoomRight, depthMapId, sunPos);
                    programLight.activate(win, mirrorCam, shadowMatrix, lightsRoomRight, sunPos);
                    lightInstances2.get()->drawAll(programLight, mirrorCam.getViewMatrix(), mirrorCam.getProjMatrix(), depthMapId);
                }
                else {
                    scene.drawScene(win, programRoom, mirrorCam, shadowMatrix, lightsRoomLeft, depthMapId, sunPos);
                    programLight.activate(win, mirrorCam, shadowMatrix, lightsRoomLeft, sunPos);
                    lightInstances.get()->drawAll(programLight, mirrorCam.getViewMatrix(), mirrorCam.getProjMatrix(), depthMapId);
                }

                programMirror.activate(win, mirrorCam, mirrorTexjMatrix, lightsRoomRight, sunPos);
                mirrorInstances.get()->drawAll(programMirror, mirrorCam.getViewMatrix(), mirrorCam.getProjMatrix(), mirrorMapId);
            }

            else {
                programSky.activate(win, fpsCam, shadowMatrix, lightsRoomRight, sunPos);
                glCullFace(GL_FRONT);
                skyboxInstances.get()->drawAll(programSky, fpsCam.getViewMatrix(), fpsCam.getProjMatrix(), 0);
                glCullFace(GL_BACK);

                // if the sun is active
                if (depthMapId != 0) {
                    programSun.activate(win, fpsCam, shadowMatrix, lightsRoomRight, sunPos);
                    sunInstances.get()->drawAll(programSun, fpsCam.getViewMatrix(), fpsCam.getProjMatrix(), 0);
                }

                if(currentCamPos.x < -0.1) {
                    boolRightRoom = false;
                }
                if(currentCamPos.x > 0.1) {
                    boolRightRoom = true;
                }
                
                if(boolRightRoom) {
                    scene.drawScene(win, *currentProgram, fpsCam, shadowMatrix, lightsRoomRight, depthMapId, sunPos);
                    // scene.drawScene(*currentProgram, ModelToViewVMatrix, projMatrix, NormalMatrix, currentCamPos, shadowMatrix, lightsRoomRight, mirrorMapId);
                    programLight.activate(win, fpsCam, shadowMatrix, lightsRoomRight, sunPos);
                    lightInstances2.get()->drawAll(programLight, fpsCam.getViewMatrix(), fpsCam.getProjMatrix(), depthMapId);
                }
                else {
                    scene.drawScene(win, programRoom, fpsCam, shadowMatrix, lightsRoomLeft, depthMapId, sunPos);
                    // scene.drawScene(programSimpleDepth, ModelToViewVMatrix, projMatrix, NormalMatrix, currentCamPos, shadowMatrix, lightsRoomLeft, depthMapId);
                    programLight.activate(win, fpsCam, shadowMatrix, lightsRoomLeft, sunPos);
                    lightInstances.get()->drawAll(programLight, fpsCam.getViewMatrix(), fpsCam.getProjMatrix(), depthMapId);
                }
                // auto tempProg = mirror.getProgram();
                // tempProg->activateSimple(mirrorCamPos, mirrorNormalMatrix);
                // programmMirror.activate(win, currentCamPos, NormalMatrix, mirrorModelToViewMatrix, lightsRoomRight);
                programMirror.activate(win, fpsCam, mirror.getMirrorProj(), lightsRoomRight, sunPos);
                // programmMirror.activate(win, currentCamPos, mirrorNormalMatrix, mirrorModelToViewMatrix, lightsRoomRight);
                // auto mirrorCamPos = mirror.getMirrorCamPos();
                // auto mirrorModelToViewMatrix = mirror.getViewMatrix();
                // auto mirrorNormalMatrix = mirror.getNormalMatrix();
                // auto mirrorProjMatrix = mirror.getMirrorProj();
                // GLint polygonMode[2];
                // glGetIntegerv(GL_POLYGON_MODE, polygonMode);
                // if(polygonMode[1] == GL_LINE) {
                //     glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                // }
                mirrorInstances.get()->drawAll(programMirror, fpsCam.getViewMatrix(), fpsCam.getProjMatrix(), mirrorMapId);
                // if(polygonMode[1] == GL_LINE) {
                //     glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                // }

                programButtons.activate(win, fpsCam, shadowMatrix, lightsRoomLeft, sunPos);
                buttonInstances.get()->drawAll(programButtons, fpsCam.getViewMatrix(), fpsCam.getProjMatrix(), depthMapId);
            }

        }

        /* Swap front and back buffers */
        win.display();

        auto keys = win.events();

        /* Poll for and process events */
        if (win.isFocused()) { // EVENTS
            win.hideCursor();

            fpsCam.updateKeys(keys, win.mouse(), deltaT);

            // auto speed = 5.0f;

            // if(keys & rotateLeft) {
            //     skyBoxAngles.y += speed * deltaT;
            // }
            // if(keys & rotateRight) {
            //     skyBoxAngles.y -= speed * deltaT;
            // }
            // if(keys & rotateUp) {
            //     skyBoxAngles.x += speed * deltaT;
            // }
            // if(keys & rotateDown) {
            //     skyBoxAngles.x -= speed * deltaT;
            // }
            // if(keys & shiftKey) {
            //     skyBoxAngles.z += speed * deltaT;
            // }
            // if(keys & leftClick) {
            //     skyBoxAngles.z -= speed * deltaT;
            // }


            if (keys & scrollDown) {
                currentRoom = (currentRoom+1) % allRoomTwoPrograms.size();
                currentProgram = allRoomTwoPrograms.at(currentRoom);
                // std::cout << "currentRoom " << currentRoom << std::endl;
            }

            if (keys & scrollUp) {
                animateSwitch = !animateSwitch;
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
                fromMirror = !fromMirror;
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
            win.showCursor();
            win.flushKeys();
            fpsCam.resetMouse(win.mouse());
        }


    
        win.updateTitle(fpsCam.getPos(), deltaT, false);
    }

    lightInstances.get()->~Instance();
    skyboxInstances.get()->~Instance();
    lightInstances2.get()->~Instance();
    sunInstances.get()->~Instance();
    scene.~Scene();

    for(auto prgm : allPrograms) {
        prgm->~BasicProgram();
    }

    shadowMap.~ShadowMap();

    for(auto texture : allTextures) {
        glDeleteTextures(1, texture);
    }

    // glDeleteTextures(1, &imageEarthInt);
    // glDeleteTextures(1, &imageCloudInt);
    // glDeleteTextures(1, &imageMoonInt);
    // glDeleteTextures(1, &imageSkyboxInt);
    // glDeleteTextures(1, &imageBrickDiffuseInt);
    // glDeleteTextures(1, &imageBrickRoughnessInt);
    // glDeleteTextures(1, &imageHouseDiffuseInt);
    // glDeleteTextures(1, &imageHouseRoughnessInt);
    // glDeleteTextures(1, &imageStatueInt);
    // glDeleteTextures(1, &imageCollumnDiffuseInt);
    // glDeleteTextures(1, &imageFootDiffuseInt);
    // glDeleteTextures(1, &imageWhiteInt);
    // glDeleteTextures(1, &imageGlassDiffuseInt);

    win.close();
    return 0;
}

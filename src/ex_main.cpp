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
    BasicProgram programmMirror(applicationPath, "src/shaders/utils/mirror.vs.glsl", "src/shaders/utils/mirror.fs.glsl");
    std::vector<BasicProgram*> allPrograms = {&programVoronoi, &programMirrorTex, &programSimpleDepth, &programShadow, &programShadowTest, &programRoom, &programSky, &programNormal, &programDepth, &programLight};

    std::vector<BasicProgram*> allRoomTwoPrograms = {&programVoronoi, &programRoom, &programNormal, &programDepth};
    // std::vector<BasicProgram*> allRoomTwoPrograms = {&programVoronoi, &programMirrorTex, &programSimpleDepth, &programShadow, &programShadowTest, &programRoom, &programSky, &programNormal, &programDepth, &programLight};

    GLuint imageEarthInt = bind_texture(applicationPath.dirPath() + "/assets/textures/EarthMap.jpg");
    GLuint imageCloudInt = bind_texture(applicationPath.dirPath() + "/assets/textures/CloudMap.jpg");
    GLuint imageMoonInt = bind_texture(applicationPath.dirPath() + "/assets/textures/MoonMap.jpg");
    GLuint imageSkyboxInt = bind_texture(applicationPath.dirPath() + "/assets/textures/hl2.png");
    GLuint imageBrickInt = bind_texture(applicationPath.dirPath() + "/assets/textures/bricks_internet.jpg");
    GLuint imageBrickRoughInt = bind_texture(applicationPath.dirPath() + "/assets/textures/bricks_roughness2.jpg");
    GLuint imageHouseDiffuseInt = bind_texture(applicationPath.dirPath() + "/assets/textures/cottage_diffuse.png");
    GLuint imageHouseRoughnessInt = bind_texture(applicationPath.dirPath() + "/assets/textures/cottage_roughness.jpg");
    GLuint imageStatueInt = bind_texture(applicationPath.dirPath() + "/assets/textures/statue.jpg");
    GLuint imageMarbleInt = bind_texture(applicationPath.dirPath() + "/assets/textures/marble.jpg");
    GLuint imageMarbleFootDiffuseInt = bind_texture(applicationPath.dirPath() + "/assets/textures/MarbleFoot.jpg");
    GLuint imageWhiteInt = bind_texture(applicationPath.dirPath() + "/assets/textures/white.png");
    GLuint imageGlassInt = bind_texture(applicationPath.dirPath() + "/assets/textures/GlassFinalAlpha.png");

    glimac::Sphere sphere = glimac::Sphere(1, 32, 16);
    glimac::Sphere sphereInverted = glimac::Sphere(-1, 32, 16);
    glimac::Sphere sphereLowPoly = glimac::Sphere(1, 8, 4);
    glimac::Sphere sphereLowPolyParticule = glimac::Sphere(1, 4, 2);
    auto earthInstances = std::make_shared<Instance>(sphere.getVertexCount(), sphere.getDataPointer(), imageEarthInt, imageCloudInt);
    auto particulesInstances = std::make_shared<Instance>(sphereLowPolyParticule.getVertexCount(), sphereLowPolyParticule.getDataPointer(), imageMoonInt, 0);
    auto lightInstances = std::make_shared<Instance>(sphereLowPoly.getVertexCount(), sphereLowPoly.getDataPointer(), 0, 0);
    auto lightInstances2 = std::make_shared<Instance>(sphereLowPoly.getVertexCount(), sphereLowPoly.getDataPointer(), 0, 0);
    auto skyboxInstances = std::make_shared<Instance>(sphereInverted.getVertexCount(), sphereInverted.getDataPointer(), imageSkyboxInt, 0);
    auto roomInstances = std::make_shared<Instance>(applicationPath.dirPath(), "dsda", imageBrickInt, imageBrickRoughInt);
    auto collumnInstances = std::make_shared<Instance>(applicationPath.dirPath(), "collumn3", imageMarbleInt, 0);
    auto houseInstances = std::make_shared<Instance>(applicationPath.dirPath(), "cottage", imageHouseDiffuseInt, imageHouseRoughnessInt);
    auto statueInstances = std::make_shared<Instance>(applicationPath.dirPath(), "statue", imageStatueInt, 0);
    auto footInstances = std::make_shared<Instance>(applicationPath.dirPath(), "foot", imageMarbleFootDiffuseInt, 0);
    auto cubeInstances = std::make_shared<Instance>(applicationPath.dirPath(), "simpleCube", imageGlassInt, 0);
    auto sunInstances = std::make_shared<Instance>(sphere.getVertexCount(), sphere.getDataPointer(), imageWhiteInt, 0);

    // auto debugCubeInstances = std::make_shared<Instance>(applicationPath.dirPath(), "simpleCube", 0, 0);
    // auto cubeInstances = std::make_shared<Instance>(applicationPath.dirPath(), "cube", imageWhiteInt, 0);

    auto shelveInstances = std::make_shared<Instance>(applicationPath.dirPath(), "shelve", imageMoonInt, 0);
    auto titleInstances = std::make_shared<Instance>(applicationPath.dirPath(), "title", imageBrickInt, 0);
    auto spaceShipInstances = std::make_shared<Instance>(applicationPath.dirPath(), "Farragut", imageBrickInt, 0);
    auto planeInstances = std::make_shared<Instance>(applicationPath.dirPath(), "plane", imageWhiteInt, 0);
    auto citadelInstances = std::make_shared<Instance>(applicationPath.dirPath(), "citadel", imageStatueInt, 0);
    auto mirrorInstances = std::make_shared<Instance>(applicationPath.dirPath(), "mirrorGeometry", imageWhiteInt, 0);

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
        // scene.addInstance(mirrorInstances);

        // transparents objects
        scene.addInstance(cubeInstances);

        cubeInstances.get()->setBlendToTransparent();
        // houseInstances.get()->setBlendToTransparent();
    }

    // SHADERS INVARIANTS


    // Add all objects positions
    {
        const float heightFoot = footInstances.get()->getDefautBBox().size().y;

        sunInstances.get()->add(Transform(vec3(100, 100, -100), vec3(0), vec3(1)));

        earthInstances.get()->add(Transform(vec3(100, 100, 100), vec3(0), vec3(50)));

        skyboxInstances.get()->add(Transform(vec3(0), vec3(0), vec3(800)));

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

        citadelInstances.get()->add(Transform(vec3(0, 180, -150), vec3(), vec3(0.06)));

        spaceShipInstances.get()->add(Transform(vec3(10, 5, 0), vec3(degToRad*75, 0, degToRad*15), vec3(0.1f)));
        spaceShipInstances.get()->add(Transform(vec3(-160, 60, 256), vec3(0, degToRad*60, degToRad*15), vec3(3.0f)));

        const uint amountParticules = 3200;
        const float particuleSize = 0.01f;
        for (uint i = 0; i < amountParticules; i++) {
            particulesInstances.get()->add(Transform(vec3(linearRand(-21.0f, 0.0f), linearRand(-0.5f, 10.0f), linearRand(-12.0f, 12.0f)), vec3(), vec3(particuleSize)));
        }

        // at each walls of the map
        for (int z = -12; z <= 12; z++) {
            for (int x = -21; x <= 21; x++) {
                if((x == -1 && z == -4) || (x == -1 && z == -3) || (x == 10 && z == 12) || (x == 11 && z == 12)) {
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
            cubeInstances.get()->add(Transform(vec3(center(bbox)), vec3(), size));

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
        int nbRandLight = 320;

        for (int i = 0; i < nbRandLight; i++) {
            vec3 pos = ballRand(10.0f) + vec3(10.0f, 10.0f, 0);
            pos = vec3(pos.x, pos.y/20, pos.z);
            vec3 color = vec3(linearRand(.1f, 1.0f), linearRand(.1f, 1.0f), linearRand(.1f, 1.0f));
            // vec3 color = vec3(1.0, 1.0, 1.0);
            lightsRoomRight.add(LightStruct(pos, color, vec3(3, 0.5, lightSize)));
            lightInstances2.get()->add(Transform(pos, vec3(0), vec3(lightSize)));
        }
        

        // lightsRoomLeft.add( LightStruct(vec3(0, 5, 0), vec3(1), vec3(20, 1, lightSize)));
        // lightsRoomRight.add(LightStruct(vec3(0, 5, 0), vec3(1), vec3(20, 1, lightSize)));
        // lightInstances.get()->add(Transform(vec3(0, 5, 0), vec3(0), vec3(lightSize)));
    }


    unsigned int currentRoom = 0;
    BasicProgram *currentProgram = allPrograms.at(currentRoom);

    std::cout << "Starting main loop" << std::endl;

    float oldTime = -1.0f;
    float deltaT = 0;
    bool colliding = false;
    bool animateSwitch = true;
    float timer = 0.0f;
    float animateTimer = 0.0f;
    bool boolRightRoom = false;

    glm::mat4 projMatrix;
    glm::mat4 ModelToViewVMatrix;
    glm::mat4 NormalMatrix;

    glm::mat4 shadowMatrix;
    glm::mat4 lightSpaceMatrix;
    glm::mat4 lightProj;
    glm::mat4 lightNormal;

    ShadowMap shadowMap(applicationPath);
    shadowMap.init();

    shadowMap.computeTransforms(LightStruct(vec3(40, 40, -40), vec3(1), vec3(100, 0.85, LightType::directionalLight)));

    GLuint depthMapId;
    depthMapId = shadowMap.getDepthMap();
    shadowMatrix = shadowMap.getShadowMatrix();
    shadowMap.renderTexture(scene);

    vec3 currentCamPos;
    vec2 oldMouse;
    FPSCamera fpsCam = FPSCamera(win.width(), win.height());
    fpsCam.resetMouse(win.mouse());

    Mirror mirror(vec3(10.5, 1.5, 9), vec3(0, 0, -1), applicationPath);
    mirror.init(win.width(), win.height());
    mirrorInstances.get()->add(Transform(vec3(10.5, 1.5, 9), vec3(-degToRad*90, 0, 0), vec3(3.0f, 3.0f, 3.0f)));
    mirrorInstances.get()->add(Transform(vec3(10.5, 1.5, 9), vec3(degToRad*90, 0, 0), vec3(3.0f, 3.0f, 3.0f)));
    GLuint mirrorMapId = mirror.getMirrorMap();

    mirror.computeCamera(fpsCam);

    bool fromMirror = false;

    /* Loop until the user closes the window */
    while (win.running()) {

        { // INIT CODE
            clear_screen();
            colliding = fpsCam.updatePhysic(walls, deltaT);
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
            ModelToViewVMatrix = fpsCam.getViewMatrix();
            NormalMatrix = glm::transpose(glm::inverse(ModelToViewVMatrix));
            currentCamPos = fpsCam.getPos();
            projMatrix = fpsCam.getProjMatrix();
        }

        { // UPDATES

            // Skyboc objects update
            {
                earthInstances.get()->updatePosition(0, vec3(100, 100, 100) + currentCamPos);
                earthInstances.get()->computeLast();

                sunInstances.get()->updatePosition(0, vec3(100, 100, -100) + currentCamPos);
                sunInstances.get()->computeLast();

                skyboxInstances.get()->updatePosition(0, currentCamPos);
                skyboxInstances.get()->computeAll();
                
                citadelInstances.get()->updatePosition(0, currentCamPos + vec3(0, 180, -150));
                citadelInstances.get()->computeAll();
            }

            shadowMap.computeTransforms(LightStruct(vec3(40, 40, -40), vec3(1), vec3(100, 0.85, LightType::directionalLight)));

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

            if (animateSwitch) {

                lightsRoomLeft.updatePosition(2, vec3(-10.5+5*cos(animateTimer/1.0), 1, 5*sin(animateTimer/1.0)));
                lightInstances.get()->updatePosition(2, vec3(-10.5+5*cos(animateTimer/1.0), 1, 5*sin(animateTimer/1.0)));
                lightInstances.get()->compute(2);

                vec3 planePos = planeInstances.get()->get(0).m_Position;
                planePos += 931 * kmToMs * vec3(-1, 0, 0) * deltaT;

                if (planePos.x < -2000) {
                    planePos = vec3(1000, linearRand(7.0f, 20.0f), linearRand(-30.0f, 30.0f));
                }

                planeInstances.get()->updatePosition(0, planePos);
                planeInstances.get()->updateAngles(0, vec3(sin(animateTimer)*10.0f*degToRad, 0, -5*degToRad));
                planeInstances.get()->computeLast();

                earthInstances.get()->updateAngles(0, vec3(0, animateTimer*15.0, 0)*degToRad);

                float distanceToPlane = length(planeInstances.get()->get(0).m_Position - currentCamPos);

                if (distanceToPlane < 450) {
                    fpsCam.shake(1/(1+distanceToPlane));
                }

            }
            else {
                fpsCam.shake(0.0f);
            }
        }

        { // RENDERING

            shadowMap.renderTexture(scene);

            // mirroir
            auto mirrorCamPos = mirror.getMirrorCamPos();
            auto mirrorModelToViewMatrix = mirror.getViewMatrix();
            auto mirrorTexjMatrix = mirror.getMirrorProj();
            auto mirrorNormalMatrix = mirror.getNormalMatrix();
            auto mirrorProjMatrix = mirror.getMirrorProj();
            mirror.computeCamera(fpsCam);
            if (false) {
                // mirror.drawStencil(fpsCam);
                // mirror.startRenderMirrorTexture();

                glViewport(0, 0, 1600, 900);
                glBindFramebuffer(GL_FRAMEBUFFER, mirror.getMirrorFBO());
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                programSky.activate(mirrorCamPos, mirrorNormalMatrix, shadowMatrix, lightsRoomRight);
                glCullFace(GL_FRONT);
                skyboxInstances.get()->drawAll(programSky, mirrorModelToViewMatrix, mirrorProjMatrix, 0);
                glCullFace(GL_BACK);

                // if the sun is active
                if (depthMapId != 0) {
                    programSun.activate(mirrorCamPos, mirrorNormalMatrix, shadowMatrix, lightsRoomRight);
                    sunInstances.get()->drawAll(programSun, mirrorModelToViewMatrix, mirrorProjMatrix, 0);
                }

                if(mirrorCamPos.x < -0.5) {
                    boolRightRoom = false;
                }
                if(mirrorCamPos.x > 0.5) {
                    boolRightRoom = true;
                }
                
                if(boolRightRoom) {
                    scene.drawScene(*currentProgram, mirrorModelToViewMatrix, mirrorProjMatrix, mirrorNormalMatrix, mirrorCamPos, shadowMatrix, lightsRoomRight, depthMapId);
                    programLight.activate(mirrorCamPos, mirrorNormalMatrix, shadowMatrix, lightsRoomRight);
                    lightInstances2.get()->drawAll(programLight, mirrorModelToViewMatrix, mirrorProjMatrix, depthMapId);
                }
                else {
                    scene.drawScene(programRoom, mirrorModelToViewMatrix, mirrorProjMatrix, mirrorNormalMatrix, mirrorCamPos, shadowMatrix, lightsRoomLeft, depthMapId);
                    programLight.activate(mirrorCamPos, mirrorNormalMatrix, shadowMatrix, lightsRoomLeft);
                    lightInstances.get()->drawAll(programLight, mirrorModelToViewMatrix, mirrorProjMatrix, depthMapId);
                }


                glReadBuffer(GL_NONE);

                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
                glBindFramebuffer(GL_READ_FRAMEBUFFER, mirror.getMirrorFBO());

                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, mirror.getMirrorMap());

                glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 0, 0, 1600, 900, 0);

                glBlitFramebuffer(0, 0, 1600, 900, 0, 0, 1600, 900, GL_COLOR_BUFFER_BIT, GL_NEAREST);

                glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

                glBindFramebuffer(GL_FRAMEBUFFER, 0);

                glViewport(0, 0, 1600, 900);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                // mirror.stopRenderMirrorTexture();
                // mirror.deactivate();
            }




            if (fromMirror) {
                programSky.activate(mirrorCamPos, mirrorNormalMatrix, shadowMatrix, lightsRoomRight);
                glCullFace(GL_FRONT);
                skyboxInstances.get()->drawAll(programSky, mirrorModelToViewMatrix, mirrorProjMatrix, 0);
                glCullFace(GL_BACK);

                // if the sun is active
                if (depthMapId != 0) {
                    programSun.activate(mirrorCamPos, mirrorNormalMatrix, shadowMatrix, lightsRoomRight);
                    sunInstances.get()->drawAll(programSun, mirrorModelToViewMatrix, mirrorProjMatrix, 0);
                }

                if(mirrorCamPos.x < -0.5) {
                    boolRightRoom = false;
                }
                if(mirrorCamPos.x > 0.5) {
                    boolRightRoom = true;
                }
                
                if(boolRightRoom) {
                    scene.drawScene(*currentProgram, mirrorModelToViewMatrix, mirrorProjMatrix, mirrorNormalMatrix, mirrorCamPos, shadowMatrix, lightsRoomRight, depthMapId);
                    programLight.activate(mirrorCamPos, mirrorNormalMatrix, shadowMatrix, lightsRoomRight);
                    lightInstances2.get()->drawAll(programLight, mirrorModelToViewMatrix, mirrorProjMatrix, depthMapId);
                }
                else {
                    scene.drawScene(programRoom, mirrorModelToViewMatrix, mirrorProjMatrix, mirrorNormalMatrix, mirrorCamPos, shadowMatrix, lightsRoomLeft, depthMapId);
                    programLight.activate(mirrorCamPos, mirrorNormalMatrix, shadowMatrix, lightsRoomLeft);
                    lightInstances.get()->drawAll(programLight, mirrorModelToViewMatrix, mirrorProjMatrix, depthMapId);
                }
            }

            else {
                programSky.activate(currentCamPos, NormalMatrix, shadowMatrix, lightsRoomRight);
                glCullFace(GL_FRONT);
                skyboxInstances.get()->drawAll(programSky, ModelToViewVMatrix, projMatrix, 0);
                glCullFace(GL_BACK);

                // if the sun is active
                if (depthMapId != 0) {
                    programSun.activate(currentCamPos, NormalMatrix, shadowMatrix, lightsRoomRight);
                    sunInstances.get()->drawAll(programSun, ModelToViewVMatrix, projMatrix, 0);
                }

                if(currentCamPos.x < -0.5) {
                    boolRightRoom = false;
                }
                if(currentCamPos.x > 0.5) {
                    boolRightRoom = true;
                }
                
                if(boolRightRoom) {
                    scene.drawScene(*currentProgram, ModelToViewVMatrix, projMatrix, NormalMatrix, currentCamPos, shadowMatrix, lightsRoomRight, depthMapId);
                    // scene.drawScene(*currentProgram, ModelToViewVMatrix, projMatrix, NormalMatrix, currentCamPos, shadowMatrix, lightsRoomRight, mirrorMapId);
                    programLight.activate(currentCamPos, NormalMatrix, shadowMatrix, lightsRoomRight);
                    lightInstances2.get()->drawAll(programLight, ModelToViewVMatrix, projMatrix, depthMapId);
                }
                else {
                    scene.drawScene(programRoom, ModelToViewVMatrix, projMatrix, NormalMatrix, currentCamPos, shadowMatrix, lightsRoomLeft, depthMapId);
                    // scene.drawScene(programSimpleDepth, ModelToViewVMatrix, projMatrix, NormalMatrix, currentCamPos, shadowMatrix, lightsRoomLeft, depthMapId);
                    programLight.activate(currentCamPos, NormalMatrix, shadowMatrix, lightsRoomLeft);
                    lightInstances.get()->drawAll(programLight, ModelToViewVMatrix, projMatrix, depthMapId);
                }
                // auto tempProg = mirror.getProgram();
                // tempProg->activateSimple(mirrorCamPos, mirrorNormalMatrix);
                // programmMirror.activate(currentCamPos, NormalMatrix, mirrorModelToViewMatrix, lightsRoomRight);
                programmMirror.activate(currentCamPos, NormalMatrix, mirrorTexjMatrix, lightsRoomRight);
                // programmMirror.activate(currentCamPos, mirrorNormalMatrix, mirrorModelToViewMatrix, lightsRoomRight);
                // auto mirrorCamPos = mirror.getMirrorCamPos();
                // auto mirrorModelToViewMatrix = mirror.getViewMatrix();
                // auto mirrorNormalMatrix = mirror.getNormalMatrix();
                // auto mirrorProjMatrix = mirror.getMirrorProj();
                mirrorInstances.get()->drawAll(programmMirror, ModelToViewVMatrix, projMatrix, mirrorMapId);
            }

        }

        /* Swap front and back buffers */
        win.display();

        /* Poll for and process events */
        if (win.isFocused()) { // EVENTS
            auto keys = win.events();

            fpsCam.updateKeys(keys, win.mouse(), deltaT);


            if (keys & scrollDown) {
                // keys &= ~scrollDown;
                currentRoom = (currentRoom+1) % allRoomTwoPrograms.size();
                currentProgram = allRoomTwoPrograms.at(currentRoom);
                // std::cout << "currentRoom " << currentRoom << std::endl;
            }

            if (keys & scrollUp) {
                // keys &= ~scrollUp;
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
                // keys &= ~switchMode;
                glGetIntegerv(GL_POLYGON_MODE, polygonMode);
                std::cout << "Switched to " << polygonMode[1] << std::endl;
            }
        }
    
        char title [256];
        title [255] = '\0';

        snprintf ( title, 255,
                    "%s %s - [FPS: %3.2f] - x:%3.0f y:%3.0f z:%3.0f, collision: %d",
                    "Deux Salles, deux Ambiances", "v0.1", 1.0f / (float)deltaT , currentCamPos.x, currentCamPos.y, currentCamPos.z, colliding);

        win.setTitle(title);
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

    glDeleteTextures(1, &imageEarthInt);
    glDeleteTextures(1, &imageCloudInt);
    glDeleteTextures(1, &imageMoonInt);
    glDeleteTextures(1, &imageSkyboxInt);
    glDeleteTextures(1, &imageBrickInt);
    glDeleteTextures(1, &imageBrickRoughInt);
    glDeleteTextures(1, &imageHouseDiffuseInt);
    glDeleteTextures(1, &imageHouseRoughnessInt);
    glDeleteTextures(1, &imageStatueInt);
    glDeleteTextures(1, &imageMarbleInt);
    glDeleteTextures(1, &imageMarbleFootDiffuseInt);
    glDeleteTextures(1, &imageWhiteInt);
    glDeleteTextures(1, &imageGlassInt);

    win.close();
    return 0;
}
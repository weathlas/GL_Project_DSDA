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

    glfwSwapInterval(0);


    /*********************************
     * HERE SHOULD COME THE INITIALIZATION CODE
     *********************************/
    
    glimac::FilePath applicationPath(argv[0]);

    std::cout << "Compiling Shaders..." << std::endl;

    // BasicProgram programRoom(applicationPath, "src/shaders/roomOne/ground.vs.glsl", "src/shaders/roomOne/ground.fs.glsl");
    BasicProgram programRoom(applicationPath, "src/shaders/roomTwo/voronoi.vs.glsl", "src/shaders/roomTwo/voronoi.fs.glsl", ProgramType::LIGHTS);

    BasicProgram programLight(applicationPath, "src/shaders/light/light.vs.glsl", "src/shaders/light/light.fs.glsl", ProgramType::LIGHTS);
    BasicProgram programVoronoi(applicationPath, "src/shaders/roomTwo/voronoi.vs.glsl", "src/shaders/roomTwo/voronoi.fs.glsl", ProgramType::LIGHTS);
    std::vector<BasicProgram*> allPrograms = {&programVoronoi, &programRoom, &programLight};

    std::vector<BasicProgram*> allRoomTwoPrograms = {&programVoronoi, &programRoom};

    std::cout << "Loading Textures..." << std::endl;

    GLuint imageWhiteInt = bind_texture(applicationPath.dirPath() + "/assets/textures/white.png");
    GLuint imageDefaultNormalInt = bind_texture(applicationPath.dirPath() + "/assets/textures/normal.png");

    std::vector<GLuint*> allTextures = {

        &imageWhiteInt,
        &imageDefaultNormalInt
    };

    glimac::Sphere sphere = glimac::Sphere(1, 32, 16);
    glimac::Sphere sphereInverted = glimac::Sphere(-1, 32, 16);
    glimac::Sphere sphereLowPoly = glimac::Sphere(1, 8, 4);
    glimac::Sphere sphereLowPolyParticule = glimac::Sphere(1, 4, 2);

    std::cout << "Importing Objects..." << std::endl;

    auto roomInstances = std::make_shared<Instance>(applicationPath.dirPath(), "dsda", imageWhiteInt, imageWhiteInt, imageDefaultNormalInt);
    auto lightInstances = std::make_shared<Instance>(sphereLowPoly.getVertexCount(), sphereLowPoly.getDataPointer(), 0, 0, imageDefaultNormalInt);
    auto lightInstances2 = std::make_shared<Instance>(sphereLowPoly.getVertexCount(), sphereLowPoly.getDataPointer(), 0, 0, imageDefaultNormalInt);


    auto firstRope = Animation(sphereLowPolyParticule.getVertexCount(), sphereLowPolyParticule.getDataPointer(), imageWhiteInt, 0, imageDefaultNormalInt);
    firstRope.make_rope(vec3(0, 6, 0), vec3(-8, 6, 0), 30, 1.0, 100000000.0, 0);
    firstRope.addField(FieldType::field_directional, vec3(0, -1, 0), 9.81*0.1);
    uint animIterPerFrame = 1;

    auto firstGrid = Animation(sphereLowPolyParticule.getVertexCount(), sphereLowPolyParticule.getDataPointer(), imageWhiteInt, 0, imageDefaultNormalInt);
    // firstGrid.make_grid(vec3(-2, 6, -2), vec3(2, 6, -2), vec3(-2, 6, 2), vec3(2, 6, 2), 7, 0.005, 48.0, 0.65);
    firstGrid.make_grid(vec3(-2, 10, 0), vec3(2, 10, 0), vec3(-2, 6, 0), vec3(2, 6, 0), 50, 1.0, 128.0, 0.6);
    firstGrid.addField(FieldType::field_directional, vec3(0, -1, 0), 45.81);
    firstGrid.addField(FieldType::field_directional, vec3(0, 0, 1), 30);
    firstGrid.addField(FieldType::field_wall, vec3(0, 5.8, 0), 0.1);
    // firstGrid.addField(FieldType::field_fluid, vec3(0, 0, 0), 0.001f);
    // firstGrid.addField(FieldType::field_point, vec3(0, 6, 0), -8.5);

    auto firstCube = Animation(sphereLowPolyParticule.getVertexCount(), sphereLowPolyParticule.getDataPointer(), imageWhiteInt, 0, imageDefaultNormalInt);
    // firstCube.make_cube(vec3(0, 10, 0), vec3(4, 4, 4), 13, 5.5, 1000.0, 100);
            firstCube.make_cube(vec3(0, 10, 0), vec3(4, 4, 4), 10, 10, 10000, 300);
    // firstCube.make_cube(vec3(0, 10, 0), vec3(4, 4, 4), 10, 1, 100, 0.1);
    // firstCube.make_cube(vec3(0, 10, 0), vec3(4, 4, 4), 7, 5.5, 0, 0);
    // firstCube.addField(FieldType::field_directional, vec3(0, -1, 0), 9.81);
    firstCube.addField(FieldType::field_point, vec3(-10, 10, 0), 3.6);
    firstCube.addField(FieldType::field_fluid, vec3(0, 0, 0), 0.1f);

    Scene scene;

    // Add all objects to the scene
    {
        scene.addInstance(roomInstances);

        // Animation objects
        // scene.addInstance(firstRope.getInstance());
        scene.addInstance(firstGrid.getInstance());
        // scene.addInstance(firstCube.getInstance());
    }

    // SHADERS INVARIANTS
    std::cout << "Placing all objects..." << std::endl;

    // Add all objects positions
    {
        roomInstances.get()->add();
    }

    std::vector<BBox3f> walls;

    // Add all the walls
    {
        const float wallThickness = 1.0f;
        walls.push_back(BBox3f(vec3(-21, -wallThickness*10, -12), vec3(21, 0, 12)));
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
            lightsRoomRight.add(LightStruct(pos, color, vec3(3, 0.5, lightSize)));
            lightInstances2.get()->add(Transform(pos, vec3(0), vec3(lightSize)));
        }
    }

    std::cout << "Initializing variables..." << std::endl;

    unsigned int currentRoom = 0;
    BasicProgram *currentProgram = allPrograms.at(currentRoom);


    double oldTime = -1.0f;
    double deltaT = 0;
    bool animateSwitch = true;
    double timer = 0.0f;
    bool boolRightRoom = false;


    glm::mat4 shadowMatrix;

    vec3 currentCamPos;
    FPSCamera fpsCam = FPSCamera(win.width(), win.height());

    auto startPoint = vec3(0, 0.25, 0);
    auto startLookPoint = vec3(0, 0.25, -1);
    fpsCam.makeLookAt(startPoint, startLookPoint);
    fpsCam.update(win, walls, deltaT);
    fpsCam.resetMouse(win.mouse());

    
    ShadowMap shadowMap(applicationPath);
    shadowMap.init();

    shadowMap.computeTransforms(LightStruct(vec3(40, 40, -40), vec3(1), vec3(100, 0.85, LightType::directionalLight)));

    GLuint depthMapId;
    depthMapId = shadowMap.getDepthMap();
    shadowMatrix = shadowMap.getShadowMatrix();
    shadowMap.renderTexture(win, scene);
    vec3 sunPos = shadowMap.getLightPos();


    bool ropeFollowPlayer = false;

    std::cout << "Main loop" << std::endl;

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
            // if (animateSwitch) { // the animation is running
            //     animateTimer += deltaT;
            // }
            oldTime = timer;
            // oldMouse = win.mouse();
            // currentCamPos = fpsCam.getPos();
        }

        { // UPDATES

            if((ropeFollowPlayer && false) || depthMapId == 0) {

                // firstRope.setPosFirst(fpsCam.getPos() + vec3(0, -0.1, 0));
                firstGrid.setPosFirst(fpsCam.getPos() + vec3(0, 0, 0));
                // firstCube.setPosFirst(fpsCam.getPos() + vec3(0, 0, 0));
                firstGrid.setTypeFirst(ParticuleComputeType::fixed);
                // firstCube.setTypeFirst(ParticuleComputeType::fixed);
            }
            else {
                firstGrid.setTypeFirst(ParticuleComputeType::leapfrog);
                // firstCube.setTypeFirst(ParticuleComputeType::leapfrog);
            }

            for (uint i = 0; i < animIterPerFrame; i++)
            {

                // Ka = h^2 * k/m
                // Za = h * z/m

                // auto timeStartOneIteration = glfwGetTime();

                // firstRope.update(deltaT/animIterPerFrame);
                firstGrid.update(deltaT/animIterPerFrame);
                // firstCube.update(deltaT/animIterPerFrame);

            }
            
        }

        { // RENDERING

            {
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
                }
                else {
                    scene.drawScene(win, programRoom, fpsCam, shadowMatrix, lightsRoomLeft, depthMapId, sunPos);
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
            win.hideCursor();

            fpsCam.updateKeys(keys, win.mouse(), deltaT);

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
                firstRope.reset();
                firstGrid.reset();
                firstCube.reset();
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
            win.showCursor();
            win.flushKeys();
            fpsCam.resetMouse(win.mouse());
        }

        if(refreshTitle == 100) {
            // std::cout << 1.0f/deltaT << std::endl;
            refreshTitle = 0;
            win.updateTitle(vec3(0), deltaT, false);
        }
        refreshTitle++;
    }

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
    return 0;
}

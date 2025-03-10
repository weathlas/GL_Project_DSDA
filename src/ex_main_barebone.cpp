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
    BasicProgram programRoom(applicationPath, "src/shaders/roomOne/ground.vs.glsl", "src/shaders/roomOne/ground.fs.glsl");
    BasicProgram programSky(applicationPath, "src/shaders/skybox/skybox_shader.vs.glsl", "src/shaders/skybox/skybox_shader.fs.glsl", ProgramType::TEXTURE);
    BasicProgram programLight(applicationPath, "src/shaders/light/light.vs.glsl", "src/shaders/light/light.fs.glsl", ProgramType::LIGHTS);
    BasicProgram programVoronoi(applicationPath, "src/shaders/roomTwo/voronoi.vs.glsl", "src/shaders/roomTwo/voronoi.fs.glsl", ProgramType::LIGHTS);
    std::vector<BasicProgram*> allPrograms = {&programVoronoi, &programRoom, &programLight, &programSky};
    std::vector<BasicProgram*> allRoomTwoPrograms = {&programVoronoi, &programRoom};

    std::cout << "Loading Textures..." << std::endl;
    GLuint imageWhiteInt = bind_texture(applicationPath.dirPath() + "/assets/textures/white.png");
    GLuint imageDefaultNormalInt = bind_texture(applicationPath.dirPath() + "/assets/textures/normal.png");
    GLuint imageBrickDiffuseInt   = bind_texture(applicationPath.dirPath() + "/assets/textures/bricks_diffuse.jpg");
    GLuint imageBrickRoughnessInt = bind_texture(applicationPath.dirPath() + "/assets/textures/bricks_roughness.jpg");
    GLuint imageBrickNormalInt    = bind_texture(applicationPath.dirPath() + "/assets/textures/bricks_normal.jpg");
    GLuint imageSkyboxInt = bind_texture(applicationPath.dirPath() + "/assets/textures/alpha-mayoris.jpg");
    std::vector<GLuint*> allTextures = {
        &imageWhiteInt,
        &imageDefaultNormalInt,
        &imageBrickDiffuseInt,
        &imageBrickRoughnessInt,
        &imageBrickNormalInt,
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
        walls.push_back(BBox3f(vec3(-220, -wallThickness*10, -130), vec3(220, 0, 130)));
        walls.push_back(BBox3f(vec3(-1, -1, -12), vec3(1, 3, -2)));
        walls.push_back(BBox3f(vec3(-1, -1, 2), vec3(1, 3, 12)));
        walls.push_back(BBox3f(vec3(-22, -1, 12), vec3(22, 3, 12+wallThickness)));
        walls.push_back(BBox3f(vec3(-22, -1, -12-wallThickness), vec3(22, 3, -12)));
        walls.push_back(BBox3f(vec3(-21-wallThickness, -1, -13), vec3(-21, 3, 13)));
        walls.push_back(BBox3f(vec3(21, -1, -13), vec3(21+wallThickness, 3, 13)));

    }

    std::cout << "Importing Objects..." << std::endl;
    auto skyboxInstances = std::make_shared<Instance>(sphereInverted.getVertexCount(), sphereInverted.getDataPointer(), imageSkyboxInt, 0, imageDefaultNormalInt);
    auto roomInstances = std::make_shared<Instance>(applicationPath.dirPath(), "dsda", imageBrickDiffuseInt, imageBrickRoughnessInt, imageBrickNormalInt);
    auto lightInstances = std::make_shared<Instance>(sphereLowPoly.getVertexCount(), sphereLowPoly.getDataPointer(), 0, 0, imageDefaultNormalInt);
    auto lightInstances2 = std::make_shared<Instance>(sphereLowPoly.getVertexCount(), sphereLowPoly.getDataPointer(), 0, 0, imageDefaultNormalInt);

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


    bool cubeInsteadOfFlag = true;

    Scene scene;

    // Add all objects to the scene
    {
        scene.addInstance(roomInstances);
        // scene.addInstance(transparentInstances);

        // Animation objects
        // scene.addInstance(firstRope.getInstance());
        if(cubeInsteadOfFlag) {
            scene.addInstance(firstCube.getInstance());
        }
        else {
            // scene.addInstance(firstGrid.getInstance());
            scene.addInstance(firstRope.getInstance());
        }
        scene.addInstance(secondCube.getInstance());

        // transparentInstances.get()->setBlendToTransparent();
    }

    // SHADERS INVARIANTS
    std::cout << "Placing all objects..." << std::endl;

    // Add all objects positions
    {
        roomInstances.get()->add();
        // transparentInstances.get()->add(Transform(vec3(0), vec3(0), vec3(50, 6, 25)));
        // transparentInstances.get()->add(Transform(vec3(0, 6, 0), vec3(0), vec3(50, 6, 25)));

        skyboxInstances.get()->add(Transform(vec3(0), vec3(135*degToRad, -105*degToRad, 15*degToRad), vec3(5000)));
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
    BasicProgram *currentProgram = allPrograms.at(currentRoom);


    double oldTime = -1.0f;
    double deltaT = 0;
    bool animateSwitch = false;
    double timer = 0.0f;
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

    uint nb_threads = 6;
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
    for (uint i = 0; i < nb_threads; i++) {
        threadStates.push_back(NothingDone);
        threadComparaison.push_back(Compute_1);
        if(cubeInsteadOfFlag) {
            threadList.push_back(std::thread(animUpdateLambda, i, &firstCube));
        }
    }
    for (uint i = 0; i < nb_threads; i++) {
        threadStates_flag.push_back(NothingDone);
        threadComparaison_flag.push_back(Compute_1);
        if(!cubeInsteadOfFlag) {
            threadList_flag.push_back(std::thread(animUpdateLambda, i, &firstRope));
            // threadList_flag.push_back(std::thread(animUpdateLambda, i, &firstGrid));
        }
    }
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
            // if (animateSwitch) { // the animation is running
            //     animateTimer += deltaT;
            // }
            oldTime = timer;
            // oldMouse = win.mouse();
            currentCamPos = fpsCam.getPos();
        }

        { // UPDATES

            if(cubeInsteadOfFlag) {
                firstCube.update_visual();
            }
            else {
                // firstGrid.update_visual();
                firstRope.update_visual();
            }

            if(computeAnim) {
                secondCube.getFields()->back().make_cube(fpsCam.getBBox(), 0);
                secondCube.update(deltaT);
            }
            secondCube.update_visual();

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

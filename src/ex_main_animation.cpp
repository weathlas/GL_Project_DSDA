#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
// #include <glad/glad.h>
#include <glimac/Program.hpp>
#include <glimac/FilePath.hpp>
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
// #include <glimac/ShadowMap.hpp>
#include <glimac/Texture.hpp>
#include <glimac/TextureMini.hpp>
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
    
    glimac::FilePath applicationPath(argv[0]);

    std::cout << "Compiling Shaders..." << std::endl;
    BasicProgram programRoom(applicationPath, "src/shaders/utils/white.vs.glsl", "src/shaders/utils/white.fs.glsl");
    std::vector<BasicProgram*> allPrograms = {&programRoom};

    std::cout << "Loading Textures..." << std::endl;

    GLuint imageWhiteInt = bind_texture(applicationPath.dirPath() + "/assets/textures/white.png");
    GLuint imageDefaultNormalInt = bind_texture(applicationPath.dirPath() + "/assets/textures/normal.png");
    GLuint imageCameraInt = bind_texture(applicationPath.dirPath() + "/assets/textures/20210128_171918.jpg");
    GLuint imageEarthInt = bind_texture(applicationPath.dirPath() + "/assets/textures/EarthMap.jpg");

    std::vector<GLuint*> allTextures = {
        &imageWhiteInt,
        &imageDefaultNormalInt,
        &imageCameraInt,
        &imageEarthInt
    };

    std::cout << "Importing Objects..." << std::endl;
    auto roomInstances = std::make_shared<Instance>(applicationPath.dirPath(), "dsda", imageWhiteInt, 0, imageDefaultNormalInt);

    Scene scene;

    // Add all objects to the scene
    {
        scene.addInstance(roomInstances);
    }

    // SHADERS INVARIANTS
    std::cout << "Placing all objects..." << std::endl;

    Light lightsRoomLeft;

    // Add all objects positions
    {
        roomInstances.get()->add();
    }

    std::cout << "Initializing variables..." << std::endl;

    double oldTime = -1.0f;
    double deltaT = 0;
    double timer = 0.0f;
    vec2 mousePos = win.mouse();

    glm::mat4 shadowMatrix;

    FPSCamera fpsCam = FPSCamera(win.width(), win.height());

    auto startPoint = vec3(0, 0.25, 0);
    auto startLookPoint = vec3(0, 0.25, -1);
    fpsCam.makeLookAt(startPoint, startLookPoint);
    fpsCam.resetMouse(mousePos);

    ShadowMap shadowMap(applicationPath);
    shadowMap.init();
    shadowMap.computeTransforms(LightStruct(vec3(40, 40, -40), vec3(1), vec3(100, 0.85, LightType::directionalLight)));

    GLuint depthMapId;
    depthMapId = shadowMap.getDepthMap();
    shadowMatrix = shadowMap.getShadowMatrix();
    shadowMap.renderTexture(win, scene);
    vec3 sunPos = shadowMap.getLightPos();

    bool showtexture = false;
    Texture renderTexture(applicationPath);
    TextureMini renderTextureMini(applicationPath);

    std::cout << "Main loop" << std::endl;

    /* Loop until the user closes the window */
    uint refreshTitle = 0;
    while (win.running()) {

        { // INIT CODE
            win.clear();
            timer = glfwGetTime();
            deltaT = std::min(timer - oldTime, 1.0/24);
            if(oldTime < 0) { // first frame
                deltaT = 1/60.0f;
            }
            oldTime = timer;
        }


        if(showtexture) {
            renderTexture.render(imageCameraInt);
            renderTextureMini.render(imageEarthInt);
        }
        else {
            scene.drawScene(win, programRoom, fpsCam, shadowMatrix, lightsRoomLeft, depthMapId, sunPos);
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

            showtexture = keys & keyTexture;
            
            if(win.isMouseCaptured()) {
                win.hideCursor();
                mousePos = win.mouse();
            }
            else {
                win.showCursor();
            }
        }
        else {
            win.mouseCapture(false);
            win.showCursor();
            win.flushKeys();
            fpsCam.resetMouse(mousePos);
        }

        if(refreshTitle == 1000) {
            refreshTitle = 0;
            win.updateTitle(fpsCam.getPos(), deltaT, 1.0f);
        }
        else {
            refreshTitle++;
        }
    }

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

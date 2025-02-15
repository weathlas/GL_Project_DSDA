#include <glimac/WindowManager.hpp>
#include <iostream>

#define GLFW_INCLUDE_NONE
// #include <GLFW/glfw3.h>
// #include <glad/glad.h>

namespace glimac {

    static vec2 m_dimensions;
    static unsigned int m_keys;
    static vec3 m_MousePos;

    static bool m_windowInitialized;

    
    bool WindowManager::init(int win_width, int win_height) {
        if(m_windowInitialized) {
            return false;
        }
        m_dimensions = vec2(win_width, win_height);
        m_keys = 0u;
        m_title[255] = '\0';
        // glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_FALSE);
        m_Window = glfwCreateWindow(m_dimensions.x, m_dimensions.y, "Deux Salles, Deux Ambiances", nullptr, nullptr);
        if (!m_Window) {
            glfwTerminate();
            return false;
        }

        /* Make the window's context current */
        glfwMakeContextCurrent(m_Window);

        // glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        // glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        // hideCursor();

        /* Intialize glad (loads the OpenGL functions) */
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            return false;
        }

        glScissor(0, 0, win_width, win_height);

        glEnable(GL_DEPTH_TEST);
        // glPolygonMode(GL_BACK, GL_FILL);
        glCullFace(GL_FRONT_AND_BACK);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        // glPolygonMode(GL_BACK, GL_POINT);

        glDrawBuffer(GL_BACK);
        glDepthMask(GL_TRUE);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /* Hook input callbacks */
        glfwSetKeyCallback(m_Window, &key_callback);
        glfwSetMouseButtonCallback(m_Window, &mouse_button_callback);
        glfwSetScrollCallback(m_Window, &scroll_callback);
        glfwSetCursorPosCallback(m_Window, &cursor_position_callback);
        glfwSetWindowSizeCallback(m_Window, &size_callback);
        m_windowInitialized = true;
        return true;
    }

    unsigned int WindowManager::events() {
        glfwPollEvents();

        auto out = m_keys;
        // prevent the scrollwheel event to be duplicated
        m_keys &= ~(scrollDown | scrollUp | switchMode | keySun | keyDebug);
        return out;
    }

    unsigned int WindowManager::keys() {
        return m_keys;
    }

    int WindowManager::width() {
        return m_dimensions.x;
    }

    int WindowManager::height() {
        return m_dimensions.y;
    }

    const vec2 WindowManager::getDimensions() {
        return m_dimensions;
    }

    const vec2 WindowManager::mouse() {
        return vec2(m_MousePos);
    }

    bool WindowManager::isInitialized() {
        return m_windowInitialized;
    }

    bool WindowManager::close() {
        glfwTerminate();
        m_windowInitialized = false;
        return false;
    }

    bool WindowManager::running() {
        return !glfwWindowShouldClose(m_Window);
    }

    void WindowManager::display() {
        glfwSwapBuffers(m_Window);
        // glFlush();
    }

    void WindowManager::updateTitle(vec3 pos, float deltaT, bool colliding) {
        snprintf ( m_title, 255,
                    "%s %s - [FPS: %3.2f] - x:%3.0f y:%3.0f z:%3.0f, collision: %d",
                    "Deux Salles, Deux Ambiances", "v0.3", 1.0f / (float)deltaT , pos.x, pos.y, pos.z, colliding);

        glfwSetWindowTitle (m_Window, m_title);

    }

    bool WindowManager::isFocused() {
        return glfwGetWindowAttrib(m_Window, GLFW_FOCUSED);
    }

    void WindowManager::hideCursor() {
        glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    void WindowManager::showCursor() {
        glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    void WindowManager::flushKeys() {
        m_keys = 0u;
    }

    void WindowManager::quit() {
        glfwSetWindowShouldClose(m_Window, true);
    }

    void WindowManager::key_callback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/) {
        if (action == GLFW_PRESS) {
            switch (key)
            {
            case GLFW_KEY_Q:
                glfwSetWindowShouldClose(window, true);
                break;
            case GLFW_KEY_W:
                m_keys |= keyUp;
                break;
            case GLFW_KEY_S:
                m_keys |= keyDown;
                break;
            case GLFW_KEY_A:
                m_keys |= keyLeft;
                break;
            case GLFW_KEY_D:
                m_keys |= keyRight;
                break;
            case GLFW_KEY_LEFT_SHIFT:
                m_keys |= shiftKey;
                break;
            case GLFW_KEY_X:
                m_keys |= switchMode;
                break;

            case GLFW_KEY_LEFT:
                m_keys |= rotateLeft;
                break;
            case GLFW_KEY_RIGHT:
                m_keys |= rotateRight;
                break;
            case GLFW_KEY_UP:
                m_keys |= rotateUp;
                break;
            case GLFW_KEY_DOWN:
                m_keys |= rotateDown;
                break;
            case GLFW_KEY_LEFT_CONTROL:
                m_keys |= ctrlKey;
                break;
            case GLFW_KEY_SPACE:
                m_keys |= spacebar;
                break;
            case GLFW_KEY_E:
                m_keys |= keySun;
                break;
            case GLFW_KEY_R:
                m_keys |= keyDebug;
                break;
            
            default:
                break;
            }
            // cout << m_keys << endl;
        }
        else if (action == GLFW_RELEASE) {
            switch (key)
            {
            case GLFW_KEY_W:
                m_keys &= ~keyUp;
                break;
            case GLFW_KEY_S:
                m_keys &= ~keyDown;
                break;
            case GLFW_KEY_A:
                m_keys &= ~keyLeft;
                break;
            case GLFW_KEY_D:
                m_keys &= ~keyRight;
                break;
            case GLFW_KEY_LEFT_SHIFT:
                m_keys &= ~shiftKey;
                break;

            case GLFW_KEY_LEFT:
                m_keys &= ~rotateLeft;
                break;
            case GLFW_KEY_RIGHT:
                m_keys &= ~rotateRight;
                break;
            case GLFW_KEY_UP:
                m_keys &= ~rotateUp;
                break;
            case GLFW_KEY_DOWN:
                m_keys &= ~rotateDown;
                break;
            case GLFW_KEY_LEFT_CONTROL:
                m_keys &= ~ctrlKey;
                break;
            case GLFW_KEY_SPACE:
                m_keys &= ~spacebar;
                break;
            case GLFW_KEY_E:
                m_keys &= ~keySun;
                break;
            case GLFW_KEY_R:
                m_keys &= ~keyDebug;
                break;
            
            default:
                break;
            }
            // cout << m_keys << endl;
        }

    }

    void WindowManager::mouse_button_callback(GLFWwindow* /*window*/, int button, int action, int /*mods*/) {

        switch (action) {
            case GLFW_PRESS:
                switch (button)
                {
                case GLFW_MOUSE_BUTTON_2:
                    m_keys |= rightClick;
                    break;
                case GLFW_MOUSE_BUTTON_1:
                    m_keys |= leftClick;
                    break;
                default:
                    break;
                }
                break;
            case GLFW_RELEASE:
                switch (button)
                {
                case GLFW_MOUSE_BUTTON_2:
                    m_keys &= ~rightClick;
                    break;
                case GLFW_MOUSE_BUTTON_1:
                    m_keys &= ~leftClick;
                    break;
                default:
                    break;
                }
                break;
            
            default:
                break;
        }

        // std::cout << button << " " << action << " " << mods << std::endl;
    }

    void WindowManager::scroll_callback(GLFWwindow* /*window*/, double /*xoffset*/, double yoffset) {
        if (yoffset > 0) {
            m_keys |= scrollUp;
        }
        if (yoffset < 0) {
            m_keys |= scrollDown;
        }
    }

    void WindowManager::cursor_position_callback(GLFWwindow* /*window*/, double xpos, double ypos) {
        m_MousePos.x = xpos;
        m_MousePos.y = ypos;
    }

    void WindowManager::size_callback(GLFWwindow* /*window*/, int width, int height) {
        m_dimensions.x = width;
        m_dimensions.y = height;
        glScissor(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glViewport(0,0,width,height);
    }

}
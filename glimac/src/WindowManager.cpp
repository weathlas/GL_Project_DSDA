#include <glimac/WindowManager.hpp>
#include <iostream>

#define GLFW_INCLUDE_NONE
// #include <GLFW/glfw3.h>
// #include <glad/glad.h>

namespace glimac {

    static int m_Width;
    static int m_Height;
    static unsigned int m_keys;
    static vec3 m_MousePos;

    static bool m_windowInitialized;

    unsigned int WindowManager::events() {
        glfwPollEvents();

        auto out = m_keys;
        // prevent the scrollwheel event to be duplicated
        m_keys &= ~(scrollDown | scrollUp | switchMode);
        return out;
    }

    unsigned int WindowManager::keys() {
        return m_keys;
    }

    int WindowManager::width() {
        return m_Width;
    }

    int WindowManager::height() {
        return m_Height;
    }

    vec2 WindowManager::mouse() {
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
    }

    void WindowManager::setTitle(char* title) {
        glfwSetWindowTitle (m_Window, title);
    }

    bool WindowManager::init(float win_width, float win_height) {
        m_Width  = win_width;
        m_Height = win_height;
        m_keys = 0u;
        if(m_windowInitialized) {
            return false;
        }
        m_Window = glfwCreateWindow(m_Width, m_Height, "Deux Salles, Deux Ambiances", nullptr, nullptr);
        if (!m_Window) {
            glfwTerminate();
            return false;
        }

        /* Make the window's context current */
        glfwMakeContextCurrent(m_Window);

        /* Intialize glad (loads the OpenGL functions) */
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            return false;
        }

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
        m_Width  = width;
        m_Height = height;
    }

}
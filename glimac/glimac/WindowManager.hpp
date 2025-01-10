#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <vector>

#include <glimac/KeyCodes.hpp>

#include "common.hpp"
#include "glm.hpp"

using namespace glm;

namespace glimac {

    class WindowManager {


        private:
            GLFWwindow* m_Window;
            char m_title[256];

        public:
            WindowManager(){};
            
            bool init(int win_width, int win_height);

            unsigned int events();

            unsigned int keys();

            int width();

            int height();

            const vec2 getDimensions();

            const vec2 mouse();

            bool isInitialized();

            bool close();

            bool running();

            void display();

            void updateTitle(vec3 pos, float deltaT, bool colliding);

            bool isFocused();

            void showCursor();

            void hideCursor();

            void flushKeys();

            void quit();

            static void key_callback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/);

            static void mouse_button_callback(GLFWwindow* /*window*/, int button, int action, int /*mods*/);

            static void scroll_callback(GLFWwindow* /*window*/, double /*xoffset*/, double yoffset);

            static void cursor_position_callback(GLFWwindow* /*window*/, double xpos, double ypos);

            static void size_callback(GLFWwindow* /*window*/, int width, int height);
    };

}
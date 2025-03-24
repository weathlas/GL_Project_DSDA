#pragma once

#include "gl_debug_output.hpp"
// #include "glfw.hpp"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <iostream>
#include <stdexcept>

// Class responsible for initializing GLFW, creating a window, initializing
// OpenGL function pointers with GLAD library and initializing ImGUI
class IMGUIHandle
{
public:
  IMGUIHandle(int width, int height, const char *title, bool visible = true)
  {
    // Setup ImGui
    ImGui::CreateContext();
    // ImGui_ImplGlfw_InitForOpenGL(m_pWindow, true);
    const char *glsl_version = "#version 130";
    ImGui_ImplOpenGL3_Init(glsl_version);
  }

  ~IMGUIHandle()
  {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // glfwDestroyWindow(m_pWindow);
    glfwTerminate();
  }

  // Non-copyable class:
  IMGUIHandle(const IMGUIHandle &) = delete;
  IMGUIHandle &operator=(const IMGUIHandle &) = delete;
};

inline void imguiNewFrame()
{
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

inline void imguiRenderFrame()
{
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

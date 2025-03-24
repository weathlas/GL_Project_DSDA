cmake_minimum_required(VERSION 3.8)

add_library(glimac)

file(GLOB_RECURSE GLIMAC_SOURCES CONFIGURE_DEPENDS src/*)
target_sources(glimac PRIVATE ${GLIMAC_SOURCES})
target_include_directories(glimac PUBLIC ../glimac)

# ---Add GLFW---
add_subdirectory(third-party/glfw)
target_link_libraries(glimac PUBLIC glfw)
# ---Add glad---
add_library(glad third-party/glad/src/glad.c)
target_include_directories(glad PUBLIC third-party/glad/include)
target_link_libraries(glimac PUBLIC glad)
# ---Add glm---
add_subdirectory(third-party/glm)
target_link_libraries(glimac PUBLIC glm)
# ---Add klein---
add_subdirectory(third-party/klein)
target_link_libraries(glimac PUBLIC klein)
# ---Add IMGUI---
add_library(imgui third-party/imgui/imgui.cpp)
target_include_directories(imgui PUBLIC third-party/imgui)
target_include_directories(imgui PUBLIC third-party/imgui/backends)
target_link_libraries(glimac PUBLIC imgui)


# set(IMGUI_DIR imgui)

# set(
#     IMGUI_SRC_FILES
#     third-party/${IMGUI_DIR}/imgui.cpp
#     third-party/${IMGUI_DIR}/imgui_demo.cpp
#     third-party/${IMGUI_DIR}/imgui_draw.cpp
#     third-party/${IMGUI_DIR}/imgui_widgets.cpp
#     third-party/${IMGUI_DIR}/examples/imgui_impl_opengl3.cpp
#     third-party/${IMGUI_DIR}/examples/imgui_impl_glfw.cpp
# )

# set(
#     THIRD_PARTY_SRC_FILES
#     ${IMGUI_SRC_FILES}
# )

# # add_executable(
# #     ${THIRD_PARTY_SRC_FILES}
# # )

# target_include_directories(
#     src_ex_main_projection
#     PUBLIC
#     third-party/${IMGUI_DIR}
#     third-party/${IMGUI_DIR}/examples/
# )


# # ---Add imgui---
# add_library(imgui third-party/glad/src/glad.c)
# target_include_directories(glad PUBLIC third-party/glad/include)
# target_link_libraries(glimac PUBLIC glad)
#include "common.hpp"

/// STD Libraries
#include <stdlib.h>
#include <stdio.h>

#include "loadShaders.hpp"
#include "nativefiledialog/include/nfd.h"
#include "input/InputManager.hpp"
#include "dataProviders/vtkLegacyReader.hpp"
#include "rendering/PointRenderer.hpp"
#include "rendering/AxesRenderer.hpp"

// easy access to math functions defined in glm & CEGUI
using namespace glm;
using namespace CEGUI;

// This function is called any time GLFW encounters an error
static void error_callback(int error, const char* description)
{
    fputs(description, stderr);
}

// Do some general initialization stuff
// This gets us a window we can draw to
void init(GLFWwindow** window)
{
    glfwSetErrorCallback(error_callback); // if glfw hits an error, it should call error_callback

    // initialize GLFW; bail if it fails for some reason
    if (!glfwInit())
    {
        fprintf(stderr, "ERROR: Failed to initialize GLFW!\n");
        exit(EXIT_FAILURE);
    }

    // request certain attributes from the OpenGL context we create later
    glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // request openGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // don't give us old ogl 2.0 stuff

    *window = glfwCreateWindow(1024, 768, "FuLBLINKy 0.7b rc1", NULL, NULL);
    if (!*window)
    {
        fprintf(stderr, "ERROR: Failed to open GLFW Window!\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    // set our new window as the current GL context to render to
    glfwMakeContextCurrent(*window);

    glfwSwapInterval(1); // how many screen updates should occur before glfwSwapBuffers does its work

    // if two fragments overlap, only accept the one closer to the camera in the final image
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    glDepthFunc(GL_LESS);
    glPointSize(5);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_ARB_vertex_array_object);
}

int main(void)
{
    glm::mat4 MVP; // model-view-projection matrix passed to renderers
    GLFWwindow* window;  // the window we draw to

        /* ---------------- */
        /*  GENERAL SETUP   */
        /* ---------------- */
    init(&window);
    InputManager inputManager = InputManager(window);
    Compositor::Instance().Start();
    inputManager.UpdateCameraMatrices(0, 0); // ensure camera state is correctly initialized before we start rendering

        /* ---------------- */
        /*  RENDERING LOOP  */
        /* ---------------- */
    while (!glfwWindowShouldClose(window))
    {
        MVP = Compositor::Instance().GetProjectionMatrix() * Compositor::Instance().GetViewMatrix() * glm::mat4(1.0f);

        // Draw!
        Compositor::Instance().Render(MVP);

        // put whatever we've drawn on the screen
        glfwSwapBuffers(window);
        // check for new events (key presses, etc)
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

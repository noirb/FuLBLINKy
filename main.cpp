#define GL_GLEXT_PROTOTYPES // MUST be before any GL-related includes!

/// CEGUI -- Must be included before GLFW
#include <CEGUI/CEGUI.h>
#include <CEGUI/RendererModules/OpenGL/GL3Renderer.h>

/// GLFW -- Must be before anything else that touches OpenGL (except CEGUI)
#include <GLFW/glfw3.h>

/// GLM
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

/// STD Libraries
#include <stdlib.h>
#include <stdio.h>

#include "loadShaders.hpp"

// easy access to math functions defined in glm & CEGUI
using namespace glm;
using namespace CEGUI;

// this describes all the vertices which make up our cube
static const GLfloat g_vertex_buffer_data[] = {
    -1.0f, -1.0f, -1.0f, // triangle 1 : begin
    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f, // triangle 1 : end
     1.0f,  1.0f, -1.0f, // triangle 2 : begin
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f, // triangle 2 : end
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,   1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

// This function is called any time GLFW encounters an error
static void error_callback(int error, const char* description)
{
    fputs(description, stderr);
}

// This function is called any time GLFW detects a keypress
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // if the user presses Escape, close the window
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
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

    *window = glfwCreateWindow(640, 480, "Simple OpenGL Sample", NULL, NULL);
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
    glDepthFunc(GL_LESS);

    glfwSetKeyCallback(*window, key_callback); // key_callback will be called whenever a key is pressed
}

int main(void)
{
    CEGUI::OpenGL3Renderer* guiRenderer; // main GUI object
    CEGUI::Window* guiRoot;              // root window for GUI

    GLFWwindow* window;  // the window we draw to
    GLuint VertexArrayID;
    GLuint vertexBuffer; // integer ID of our vertex buffer

    double mouseX, mouseY; // used to store mosue cursor position when it's over the window

    // set up some matrices to handle projection from the model to the camera
    glm::mat4 Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f); // perspective projection
    glm::mat4 View = glm::lookAt( // camera matrix
            glm::vec3(4, 3, 3), // camera's location in space
            glm::vec3(0, 0, 0), // location camera is pointing at (origin in this case)
            glm::vec3(0, 1, 0)  // which direction is "up" from the camera's perspective
    );
    glm::mat4 Model = glm::mat4(1.0f); // Model matrix--different for every model we render
    glm::mat4 MVP = Projection * View * Model; // our final Model-View-Projection matrix! Da real MVP.

    // do some setup stuff, open a window, and configure our GL context to target it for drawing
    init(&window);

    /* ----------- */
    /* CEGUI SETUP */ // <-- must be done AFTER GL Context creation
    /* ----------- */
    guiRenderer = &CEGUI::OpenGL3Renderer::bootstrapSystem();
    // set default resource paths
    CEGUI::DefaultResourceProvider* rp = static_cast<CEGUI::DefaultResourceProvider*>(CEGUI::System::getSingleton().getResourceProvider());
    rp->setResourceGroupDirectory("schemes", "/usr/local/share/cegui-0/schemes/");
    rp->setResourceGroupDirectory("imagesets", "/usr/local/share/cegui-0/imagesets/");
    rp->setResourceGroupDirectory("fonts", "/usr/local/share/cegui-0/fonts/");
    rp->setResourceGroupDirectory("layouts", "/usr/local/share/cegui-0/layouts/");
    rp->setResourceGroupDirectory("looknfeels", "/usr/local/share/cegui-0/looknfeel");
    rp->setResourceGroupDirectory("lua_scripts", "/usr/local/share/cegui-0/lua_scripts");
    CEGUI::ImageManager::setImagesetDefaultResourceGroup("imagesets");
    CEGUI::Font::setDefaultResourceGroup("fonts");
    CEGUI::WidgetLookManager::setDefaultResourceGroup("looknfeels");
    CEGUI::WindowManager::setDefaultResourceGroup("layouts");
    CEGUI::ScriptModule::setDefaultResourceGroup("lua_scripts");
    CEGUI::Scheme::setDefaultResourceGroup("schemes");
    CEGUI::SchemeManager::getSingleton().createFromFile("TaharezLook.scheme");
    CEGUI::System::getSingleton().getDefaultGUIContext().setDefaultFont("DejaVuSans-12");
    CEGUI::System::getSingleton().getDefaultGUIContext().getMouseCursor().setDefaultImage("TaharezLook/MouseArrow");
    // set root window
    guiRoot = CEGUI::WindowManager::getSingleton().createWindow("DefaultWindow", "_MasterRoot");
    CEGUI::System::getSingleton().getDefaultGUIContext().setRootWindow(guiRoot);
    // create first 'real' window
    CEGUI::FrameWindow* fWnd = static_cast<CEGUI::FrameWindow*>(CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/FrameWindow", "testWindow"));
    guiRoot->addChild(fWnd);
    fWnd->setPosition( UVector2(UDim(0.2f, 0.0f), UDim(0.2f, 0.0f)));
    fWnd->setSize( USize(UDim(0.25f, 0.0f), UDim(0.25f, 0.0f)));
    fWnd->setText("Hello, CEGUI!");


    // set up & bind a vertex array object (VAO) -- must be done AFTER the context is created!
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // setup & bind our vertex buffer object (VBO)
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

    // copy our vertices into the vertex buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    // load our vertex & fragment shaders so they're ready & compiled when we need them
    GLuint programID = LoadShaders("simpleProjection.vertex", "simpleMouseColor.fragment");
        // replace the *.vertex or *.fragment strings with the filename of any vertex or fragment shader

    // get a handle for our MVP matrix so we can pass it to the shaders
    GLuint mvpID = glGetUniformLocation(programID, "MVP");

    // get a handle for our mouse coordinates as well
    GLuint mousePosID = glGetUniformLocation(programID, "mousePos"); // the strings refer to variable names in the shader

    // set a default background color for any pixels we don't draw to
    glClearColor(0.1f, 0.1f, 0.15f, 0.0f);

    // the main rendering loop
    while (!glfwWindowShouldClose(window))
    {
        // clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindVertexArray(VertexArrayID);
        // set which shaders we want to use for the next batch of vertices
        glUseProgram(programID);

        // rotate cube slightly and update MVP
        Model = glm::rotate(Model, 0.01f, glm::vec3(0.5f, 1.0f, -1.0f));
        MVP = Projection * View * Model;

        // send the MVP matrix to the shaders
        glUniformMatrix4fv(mvpID, 1, GL_FALSE, &MVP[0][0]);

        // send mouse coordinates to the shaders
        glUniform2f(mousePosID, mouseX, mouseY);

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glVertexAttribPointer(
            0,       // index of the vertex array object
            3,    // number of attributes
            GL_FLOAT,// type of attribute
            GL_FALSE,// is the contents normalized?
            0,       // stride ( 0 == 1 per vertex )
            (void*)0
        );

        // draw the cube, using 12*3 vertices starting at vertex 0 (12 triangles, 3 vertices each)
        glDrawArrays(GL_TRIANGLES, 0, 12*3);

         // --- cleanup after drawing 3D geometry --- //
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDisableVertexAttribArray(0);
        glUseProgram(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(0);

        // Draw GUI -- must be the LAST drawing call we do!
        CEGUI::System::getSingleton().renderAllGUIContexts();

        // put whatever we've drawn on the screen
        glfwSwapBuffers(window);
        // check for new events (key presses, etc)
        glfwPollEvents();
        // update mouse position
        glfwGetCursorPos(window, &mouseX, &mouseY);
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

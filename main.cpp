#include "common.hpp"

/// STD Libraries
#include <stdlib.h>
#include <stdio.h>

#include "loadShaders.hpp"
#include "input/InputManager.hpp"
#include "dataProviders/vtkLegacyReader.hpp"
#include "rendering/PointRenderer.hpp"

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

   // glewInit();

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

    *window = glfwCreateWindow(1024, 768, "Simple OpenGL Sample", NULL, NULL);
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
    glPointSize(5);
    glEnable(GL_ARB_vertex_array_object);
}

int main(void)
{
    double time = glfwGetTime(); // time elapsed since startup

    CEGUI::OpenGL3Renderer* guiRenderer; // main GUI object
    CEGUI::Window* guiRoot;              // root window for GUI

    GLFWwindow* window;  // the window we draw to

    double mouseX, mouseY; // used to store mosue cursor position when it's over the window

    // set up some matrices to handle projection from the model to the camera
    glm::mat4 Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 1000.0f); // perspective projection
    glm::mat4 View = glm::lookAt( // camera matrix
            glm::vec3(-12, 50, -8), // camera's location in space
            glm::vec3(15, 15, 10), // location camera is pointing at (origin in this case)
            glm::vec3(0, 1, 0)  // which direction is "up" from the camera's perspective
    );
    glm::mat4 Model = glm::mat4(1.0f); // Model matrix--different for every model we render
    glm::mat4 MVP = Projection * View * Model; // our final Model-View-Projection matrix! Da real MVP.

    // do some setup stuff, open a window, and configure our GL context to target it for drawing
    init(&window);
    InputManager inputManager = InputManager(window);

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

    // force CEGUI's mouse position to (0,0)     /// TODO: do this in InputManager
    CEGUI::Vector2<float> mousePos = CEGUI::System::getSingleton().getDefaultGUIContext().getMouseCursor().getPosition();  
    CEGUI::System::getSingleton().getDefaultGUIContext().injectMouseMove(-mousePos.d_x, -mousePos.d_y);

    // set root window
    guiRoot = CEGUI::WindowManager::getSingleton().createWindow("DefaultWindow", "_MasterRoot");
    CEGUI::System::getSingleton().getDefaultGUIContext().setRootWindow(guiRoot);
    // create first 'real' window
    CEGUI::FrameWindow* fWnd = static_cast<CEGUI::FrameWindow*>(CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/FrameWindow", "testWindow"));
    guiRoot->addChild(fWnd);
    fWnd->setPosition( UVector2(UDim(0.0f, 0.0f), UDim(0.2f, 0.0f)));
    fWnd->setSize( USize(UDim(0.1f, 0.0f), UDim(0.05f, 0.0f)));
    fWnd->setText("Hello, CEGUI!");

    /* --------------- */
    /* END CEGUI Setup */
    /* --------------- */

    // load our vertex & fragment shaders so they're ready & compiled when we need them
    GLuint programID = LoadShaders("simpleProjection.vertex", "simpleMouseColor.fragment");
        // replace the *.vertex or *.fragment strings with the filename of any vertex or fragment shader

    // get a handle for our MVP matrix so we can pass it to the shaders
    GLuint mvpID = glGetUniformLocation(programID, "MVP");

    // get a handle for our mouse coordinates as well
//    GLuint mousePosID = glGetUniformLocation(programID, "mousePos"); // the strings refer to variable names in the shader

    // set a default background color for any pixels we don't draw to
    glClearColor(0.1f, 0.1f, 0.15f, 0.0f);

    vtkLegacyReader vtkReader = vtkLegacyReader("test.vtk");
    DomainParameters domainParameters;
    vtkReader.getDomainParameters(&domainParameters);
    PointRenderer pointRenderer;
    pointRenderer.SetShader(programID);
    pointRenderer.PrepareGeometry(&(vtkReader.pointsField));

    // the main rendering loop
    while (!glfwWindowShouldClose(window))
    {
        double curTime = glfwGetTime();

        // clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        pointRenderer.Draw(MVP, mvpID);
            // send mouse coordinates to the shaders
            //glUniform2f(mousePosID, mouseX, mouseY);

        // Draw GUI -- must be the LAST drawing call we do!
        CEGUI::System::getSingleton().renderAllGUIContexts();

        // put whatever we've drawn on the screen
        glfwSwapBuffers(window);
        // check for new events (key presses, etc)
        glfwPollEvents();
        // update mouse position
        glfwGetCursorPos(window, &mouseX, &mouseY);

        // tell CEGUI how long its been since the last frame
        CEGUI::System::getSingleton().injectTimePulse(curTime - time);
        time = curTime; // update time
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

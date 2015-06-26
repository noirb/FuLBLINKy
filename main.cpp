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

/// TODO: This should not be global
vtkLegacyReader vtkReader;
bool vtkReaderHasNewData = false;

// This function is called any time GLFW encounters an error
static void error_callback(int error, const char* description)
{
    fputs(description, stderr);
}

bool handle_loadvtkbtn_press(const CEGUI::EventArgs &e)
{
    nfdchar_t* outPath = NULL;
    nfdresult_t result = NFD_OpenDialog("vtk", NULL, &outPath);

    if (result == NFD_OKAY)
    {
        std::cout << "Opening file: '" << outPath << "'" << std::endl;
        vtkReader.init(outPath);
        vtkReaderHasNewData = true;
    }
    else if (result == NFD_CANCEL)
    {
        std::cout << "User pressed Cancel..." << std::endl;
    }
    else
    {
        std::cout << "ERROR: " << NFD_GetError() << std::endl;
    }

    return true;
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

// setup GUI
void init_cegui(CEGUI::Window* guiRoot)
{
    // set default resource paths
    CEGUI::DefaultResourceProvider* rp = static_cast<CEGUI::DefaultResourceProvider*>(CEGUI::System::getSingleton().getResourceProvider());
    rp->setResourceGroupDirectory("schemes", "./cegui_layout/schemes/");
    rp->setResourceGroupDirectory("imagesets", "./cegui_layout/imagesets/");
    rp->setResourceGroupDirectory("fonts", "./cegui_layout/fonts/");
    rp->setResourceGroupDirectory("layouts", "./cegui_layout/layouts/");
    rp->setResourceGroupDirectory("looknfeels", "./cegui_layout/looknfeel");
    rp->setResourceGroupDirectory("lua_scripts", "./cegui_layout/lua_scripts");
    CEGUI::ImageManager::setImagesetDefaultResourceGroup("imagesets");
    CEGUI::Font::setDefaultResourceGroup("fonts");
    CEGUI::WidgetLookManager::setDefaultResourceGroup("looknfeels");
    CEGUI::WindowManager::setDefaultResourceGroup("layouts");
    CEGUI::ScriptModule::setDefaultResourceGroup("lua_scripts");
    CEGUI::Scheme::setDefaultResourceGroup("schemes");
    CEGUI::SchemeManager::getSingleton().createFromFile("AlfiskoSkin.scheme");
    CEGUI::System::getSingleton().getDefaultGUIContext().setDefaultFont("DejaVuSans-12");

    // force CEGUI's mouse position to (0,0)     /// TODO: do this in InputManager
    CEGUI::Vector2<float> mousePos = CEGUI::System::getSingleton().getDefaultGUIContext().getMouseCursor().getPosition();
    CEGUI::System::getSingleton().getDefaultGUIContext().injectMouseMove(-mousePos.d_x, -mousePos.d_y);

    // set root window
    CEGUI::System::getSingleton().getDefaultGUIContext().setRootWindow(guiRoot); 
    guiRoot->setMousePassThroughEnabled(true);
    // load default window layout
    CEGUI::Window* fWnd = CEGUI::WindowManager::getSingleton().loadLayoutFromFile("default.layout");
    guiRoot->addChild(fWnd);

    fWnd->getChildRecursive("LoadVTKbtn")->subscribeEvent(CEGUI::PushButton::EventClicked, handle_loadvtkbtn_press);
}

int main(void)
{
    double time = glfwGetTime(); // time elapsed since startup

    glm::mat4 MVP; // model-view-projection matrix passed to renderers

    CEGUI::OpenGL3Renderer* guiRenderer; // main GUI object
    CEGUI::Window* guiRoot;              // root window for GUI

    GLFWwindow* window;  // the window we draw to

    double mouseX, mouseY; // used to store mosue cursor position when it's over the window


        /* ---------------- */
        /*  GENERAL SETUP   */
        /* ---------------- */
    init(&window);
    InputManager inputManager = InputManager(window);

    guiRenderer = &CEGUI::OpenGL3Renderer::bootstrapSystem();
    guiRoot = CEGUI::WindowManager::getSingleton().createWindow("DefaultWindow", "_MasterRoot");
    init_cegui(guiRoot);

    // load our vertex & fragment shaders so they're ready & compiled when we need them
    GLuint programID = LoadShaders("simpleProjection.vertex", "simpleMouseColor.fragment");
    GLuint axesShader = LoadShaders("shaders/_coordinateAxes.vertex", "shaders/_coordinateAxes.fragment");

    // get a handle for our MVP matrix so we can pass it to the shaders
    GLuint mvpID = glGetUniformLocation(programID, "MVP");

    // get a handle for our mouse coordinates as well
    GLuint mousePosID = glGetUniformLocation(programID, "mousePos"); // the strings refer to variable names in the shader

    // set a default background color for any pixels we don't draw to
    glClearColor(0.1f, 0.1f, 0.15f, 0.0f);

    DomainParameters domainParameters;
    vtkReader.getDomainParameters(&domainParameters);

    PointRenderer pointRenderer;
    AxesRenderer axesRenderer;

    axesRenderer.SetShader(axesShader);
    axesRenderer.PrepareGeometry();
    pointRenderer.SetShader(programID);


    inputManager.UpdateCameraMatrices(0, 0); // ensure camera state is correctly initialized before we start rendering

    // the main rendering loop
    while (!glfwWindowShouldClose(window))
    {
        double curTime = glfwGetTime();
        
        if (vtkReaderHasNewData)
            pointRenderer.PrepareGeometry(&(vtkReader));    /// TODO: This should be handled in a separate class, not hard-coded here

        // clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        MVP = inputManager.GetProjectionMatrix() * inputManager.GetViewMatrix() * glm::mat4(1.0f);
        axesRenderer.Draw(MVP, mvpID);
        pointRenderer.Draw(MVP, mvpID, mouseX, mouseY, mousePosID);

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

    guiRenderer->destroySystem();
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

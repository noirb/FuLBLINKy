#include "InputManager.hpp"

        // default constructor, will leave all events unassigned
InputManager::InputManager()
{
    this->mouseX = 0;
    this->mouseY = 0;
}
        
        // will initialize InputManager with default events for the given window
InputManager::InputManager( GLFWwindow* window )
{
    this->mouseX = 0;
    this->mouseY = 0;
    this->mainWindow = window;

    this->addKeyCallback( window );
    this->addCharCallback( window );
    this->addMouseEnterCallback( window );
    this->addMouseMoveCallback( window );
    this->addMouseButtonCallback( window );
    this->addScrollCallback( window );
}

void InputManager::GetMousePosition(double* pos_x, double* pos_y)
{
    glfwGetCursorPos(this->mainWindow, pos_x, pos_y);
}

        // called when a non-text key is pressed or released
GLFWkeyfun InputManager::addKeyCallback( GLFWwindow* window )
{
    return glfwSetKeyCallback(window, key_callback);
}

        // called when a text character is entered from the keyboard
GLFWcharfun InputManager::addCharCallback( GLFWwindow* window )
{
    return glfwSetCharCallback(window, char_callback);
}

        // called when mouse enters or leaves the specified window
GLFWcursorenterfun InputManager::addMouseEnterCallback( GLFWwindow* window )
{
    return glfwSetCursorEnterCallback(window, mouseEnter_callback);
}

        // called when mouse moves over specified window
GLFWcursorposfun InputManager::addMouseMoveCallback( GLFWwindow* window )
{
    return glfwSetCursorPosCallback(window, mouseMove_callback);
}
 
        // called when any mouse button is pressed 
GLFWmousebuttonfun InputManager::addMouseButtonCallback( GLFWwindow* window )
{
    return glfwSetMouseButtonCallback(window, mouseButton_callback);
}
 
        // called when a scroll event (e.g. mouse wheel) occurs
GLFWscrollfun InputManager::addScrollCallback( GLFWwindow* window )
{
    return glfwSetScrollCallback(window, scroll_callback);
}


/* Private Methods */

// default callback implementations
void InputManager::key_callback( GLFWwindow* window, int key, int scancode, int action, int mods )
{
     // if the user presses Escape, close the window
     if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
     {
         glfwSetWindowShouldClose(window, GL_TRUE);
     }
 
     // pass through to CEGUI
     if (action == GLFW_PRESS)
     {
         CEGUI::System::getSingleton().getDefaultGUIContext().injectKeyDown( (CEGUI::Key::Scan)GlfwToCeguiKey(key) );
     }
     else if (action == GLFW_RELEASE)
     {
         CEGUI::System::getSingleton().getDefaultGUIContext().injectKeyUp( (CEGUI::Key::Scan)GlfwToCeguiKey(key) );
     }
}

void InputManager::char_callback( GLFWwindow* window, unsigned int codepoint )
{
    // pass through to CEGUI
    CEGUI::System::getSingleton().getDefaultGUIContext().injectChar(codepoint);
}

void InputManager::mouseEnter_callback( GLFWwindow* window, int entered )
{
    /// TODO
    return;
}

void InputManager::mouseMove_callback( GLFWwindow* window, double xpos, double ypos )
{
    static double old_xpos = 0.0; /// TODO: Should probably just use our member variables
    static double old_ypos = 0.0;

    // inject movement to CEGUI
    CEGUI::System::getSingleton().getDefaultGUIContext().injectMouseMove(xpos - old_xpos, ypos - old_ypos);

    // store new positions
    old_xpos = xpos;
    old_ypos = ypos;
}

void InputManager::mouseButton_callback( GLFWwindow* window, int button, int action, int mods )
{
     // pass through to CEGUI
     if (action == GLFW_PRESS)
     {
         CEGUI::System::getSingleton().getDefaultGUIContext().injectMouseButtonDown( GlfwToCeguiButton(button) );
     }
     else if (action == GLFW_RELEASE)
     {
         CEGUI::System::getSingleton().getDefaultGUIContext().injectMouseButtonUp( GlfwToCeguiButton(button) );
     }

}

void InputManager::scroll_callback( GLFWwindow* window, double xoffset, double yoffset )
{
    /// TODO
    return;
}


#include "InputManager.hpp"

        // default constructor, will leave all events unassigned
InputManager::InputManager()
{
    this->init();
}
        
        // will initialize InputManager with default events for the given window
InputManager::InputManager( GLFWwindow* window )
{
    this->init();

    this->mainWindow = window;

    this->addKeyCallback( window );
    this->addCharCallback( window );
    this->addMouseEnterCallback( window );
    this->addMouseMoveCallback( window );
    this->addMouseButtonCallback( window );
    this->addScrollCallback( window );
    this->addWindowSizeCallback( window );
    this->addFramebufferSizeCallback( window );

    glfwSetWindowUserPointer( window, this ); // give GLFW a reference to the Manager
}

void InputManager::init()
{
    this->mouseX = 0;
    this->mouseY = 0;
    this->_leftMouseDown = false;
    this->_rightMouseDown = false;
}

void InputManager::GetMousePosition(double* pos_x, double* pos_y)
{
    *pos_x = this->mouseX;
    *pos_y = this->mouseY;
}

bool InputManager::MousePressed()
{
    return this->_leftMouseDown || this->_rightMouseDown;
}

bool InputManager::LeftMousePressed()
{
    return this->_leftMouseDown;
}

bool InputManager::RightMousePressed()
{
    return this->_rightMouseDown;
}

// updates _projectionMatrix & _viewMatrix according to current input state
void InputManager::UpdateCameraMatrices(double dx, double dy)
{
    Compositor::Instance().UpdateCamera(dx, dy);
}

void InputManager::window_size_callback( GLFWwindow* window, int width, int height )
{
    Compositor::Instance().DisplayChanged(width, height);
}

void InputManager::framebuffer_size_callback( GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
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

GLFWwindowsizefun InputManager::addWindowSizeCallback( GLFWwindow* window )
{
    return glfwSetWindowSizeCallback(window, window_size_callback);
}

GLFWframebuffersizefun InputManager::addFramebufferSizeCallback( GLFWwindow* window )
{
    return glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
}

/* Private Methods */

// default callback implementations
void InputManager::key_callback( GLFWwindow* window, int key, int scancode, int action, int mods )
{
     // if the user presses Escape, close the window
     if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
     {
        glfwSetWindowShouldClose(window, GL_TRUE);
        Compositor::Instance().ShutDown();
     }
     else if (key == GLFW_KEY_UP && action == GLFW_PRESS)
     {
        Compositor::Instance().ZoomCamera(-10);
     }
     else if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
     {
        Compositor::Instance().ZoomCamera(10);
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
    InputManager* manager = static_cast<InputManager*>(glfwGetWindowUserPointer(window));

    manager->ResetCEGUIMousePos();
}

void InputManager::mouseMove_callback( GLFWwindow* window, double xpos, double ypos )
{
    // get a 'this' reference
    InputManager* manager = static_cast<InputManager*>(glfwGetWindowUserPointer(window));

    // update camera based on mouse movement if a mouse button is currently held
    if (manager->LeftMousePressed())
    {
        Compositor::Instance().UpdateCamera(xpos - manager->mouseX, ypos - manager->mouseY);
    }
    else if (manager->RightMousePressed())
    {
        Compositor::Instance().PanCamera(xpos - manager->mouseX, ypos - manager->mouseY);
    }
    else
    {
        manager->ResetCEGUIMousePos();
    }

    // store new positions
    manager->mouseX = xpos;
    manager->mouseY = ypos;
}

void InputManager::mouseButton_callback( GLFWwindow* window, int button, int action, int mods )
{
    InputManager* manager = static_cast<InputManager*>(glfwGetWindowUserPointer(window));

    // pass through to CEGUI
    if (action == GLFW_PRESS)
    {
       if ( CEGUI::System::getSingleton().getDefaultGUIContext().injectMouseButtonDown( GlfwToCeguiButton(button) ) )
       {
            return; // if CEGUI handled the input, don't do anything else with it
       }

       if (button == GLFW_MOUSE_BUTTON_RIGHT)
       {
            manager->_rightMouseDown = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // lock mouse to window
       }
       else if (button == GLFW_MOUSE_BUTTON_LEFT)
       {
            manager->_leftMouseDown = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
       }
    }
    else if (action == GLFW_RELEASE)
    {
       bool cegui_caught_input = false;
        // if we've captured input for rotation, don't talk to CEGUI
       if (!manager->MousePressed())
       {
          cegui_caught_input = CEGUI::System::getSingleton().getDefaultGUIContext().injectMouseButtonUp( GlfwToCeguiButton(button));
       }

       if (!cegui_caught_input && (button == GLFW_MOUSE_BUTTON_RIGHT))
       {
            manager->_rightMouseDown = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // unlock mouse
       }
       else if (!cegui_caught_input && (button == GLFW_MOUSE_BUTTON_LEFT))
       {
            manager->_leftMouseDown = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
       }

       manager->ResetCEGUIMousePos();
    }

}

void InputManager::scroll_callback( GLFWwindow* window, double xoffset, double yoffset )
{
    Compositor::Instance().ZoomCamera(yoffset * -2.0);
    return;
}

void InputManager::ResetCEGUIMousePos()
{
    // reset CEGUI's mouse position in case we're out of sync, now
    CEGUI::Vector2<float> cegMousePos = CEGUI::System::getSingleton().getDefaultGUIContext().getMouseCursor().getPosition();
    CEGUI::System::getSingleton().getDefaultGUIContext().injectMouseMove(this->mouseX - cegMousePos.d_x, this->mouseY - cegMousePos.d_y);
}


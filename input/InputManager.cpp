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

    glfwSetWindowUserPointer( window, this ); // give GLFW a reference to the Manager
}

void InputManager::init()
{
    this->mouseX = 0;
    this->mouseY = 0;
    this->_leftMouseDown = false;
    this->_rightMouseDown = false;

    this->_projectionMatrix = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 1000.0f); // perspective projection
    this->_viewMatrix = glm::lookAt( // camera matrix
        glm::vec3(-12, 50, -8), // camera's location in space
        glm::vec3(15, 15, 10), // location camera is pointing at (origin in this case)
        glm::vec3(0, 1, 0)  // which direction is "up" from the camera's perspective
    );

    this->cameraPos = glm::vec3(-12, 50, -8);
    this->horizontalAngle = 3.0 * 3.14f/2.0f;
    this->verticalAngle = 0.0f;
    this->initialFoV = 45.0f;
    this->speed = 3.0f;
    this->mouseSpeed = 0.005f;

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

glm::mat4 InputManager::GetProjectionMatrix()
{
    return this->_projectionMatrix;
}

glm::mat4 InputManager::GetViewMatrix()
{
    return this->_viewMatrix;
}

// updates _projectionMatrix & _viewMatrix according to current input state
void InputManager::UpdateCameraMatrices(double dx, double dy)
{
    // compute new camera orientation
    this->horizontalAngle += this->mouseSpeed * dx; /// TODO: Should include some kind of deltatime here..
    this->verticalAngle   -= this->mouseSpeed * dy;

    // compute new camera position
    this->cameraPos = glm::vec3(
        15.0 + 40.0 * cos(verticalAngle) * sin(horizontalAngle),    /// TODO: fix this
        15.0 + 40.0 * sin(verticalAngle),
        10.0 + 40.0 * cos(horizontalAngle)
    );

    this->_projectionMatrix = glm::perspective(this->initialFoV, 4.0f / 3.0f, 0.1f, 500.0f);
    this->_viewMatrix = glm::lookAt(
        this->cameraPos,
        glm::vec3(15.0, 15.0, 10.0),
        glm::vec3(0, 1, 0)
    );

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
    // get a 'this' reference
    InputManager* manager = static_cast<InputManager*>(glfwGetWindowUserPointer(window));

    // update camera based on mouse movement if a mouse button is currently held
    if (manager->MousePressed())
    {
        manager->UpdateCameraMatrices(xpos - manager->mouseX, ypos - manager->mouseY);
    }
    else
    {
        // inject movement to CEGUI
        CEGUI::System::getSingleton().getDefaultGUIContext().injectMouseMove(xpos - manager->mouseX, ypos - manager->mouseY);
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
        // if we've captured input for rotation, don't talk to CEGUI
       if (!manager->MousePressed())
       {
          if ( CEGUI::System::getSingleton().getDefaultGUIContext().injectMouseButtonUp( GlfwToCeguiButton(button) ) )
            return; // if CEGUI handled the input, don't do anything else with it
       }

       if (button == GLFW_MOUSE_BUTTON_RIGHT)
       {
            manager->_rightMouseDown = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // unlock mouse
       }
       else if (button == GLFW_MOUSE_BUTTON_LEFT)
       {
            manager->_leftMouseDown = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
       }

        // reset CEGUI's mouse position in case we're out of sync, now
        CEGUI::Vector2<float> cegMousePos = CEGUI::System::getSingleton().getDefaultGUIContext().getMouseCursor().getPosition();
        CEGUI::System::getSingleton().getDefaultGUIContext().injectMouseMove(manager->mouseX - cegMousePos.d_x, manager->mouseY - cegMousePos.d_y);
    }

}

void InputManager::scroll_callback( GLFWwindow* window, double xoffset, double yoffset )
{
    /// TODO
    return;
}


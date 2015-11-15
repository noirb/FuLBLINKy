#ifndef _INPUT_MANAGER_H
#define _INPUT_MANAGER_H

#include "../common.hpp"
#include "../rendering/Compositor.hpp"
#include "input-mapping.hpp"

class InputManager
{
    private:
        GLFWwindow* _mainWindow;
        double _mouseX;
        double _mouseY;

        bool _leftMouseDown;
        bool _rightMouseDown;

        // called by constructor to set up some basic stuff
        void init();

        // default callback implementations
        static void key_callback( GLFWwindow* window, int key, int scancode, int action, int mods );
        static void char_callback( GLFWwindow* window, unsigned int codepoint );
        static void mouseEnter_callback( GLFWwindow* window, int entered );
        static void mouseMove_callback( GLFWwindow* window, double xpos, double ypos );
        static void mouseButton_callback( GLFWwindow* window, int button, int action, int mods );
        static void scroll_callback( GLFWwindow* window, double xoffset, double yoffset );
        static void window_size_callback( GLFWwindow* window, int width, int height );
        static void framebuffer_size_callback( GLFWwindow* window, int width, int height );

        void ResetCEGUIMousePos();        
    public:
        // default constructor, will leave all events unassigned
        InputManager();

        // will initialize InputManager with default events for the given window
        InputManager( GLFWwindow* window );

        // retrieves mouse position from the mainWindow
        void GetMousePosition(double* pos_x, double* pos_y);

        // returns TRUE if the left or right mouse button is pressed
        bool MousePressed();

        // returns TRUE if the left mouse button is pressed
        bool LeftMousePressed();

        // returns TRUE if the right mouse button is pressed
        bool RightMousePressed();

        // updates _projectionMatrix & _viewMatrix based on mouse movement
        void UpdateCameraMatrices(double dx, double dy);

        // retrieves the Projection & View matrix
        glm::mat4 GetProjectionMatrix();
        glm::mat4 GetViewMatrix();

        // called when a non-text key is pressed or released
        GLFWkeyfun addKeyCallback( GLFWwindow* window );
        
        // called when a text character is entered from the keyboard
        GLFWcharfun addCharCallback( GLFWwindow* window );

        // called when mouse enters or leaves the specified window
        GLFWcursorenterfun addMouseEnterCallback( GLFWwindow* window );

        // called when mouse moves over specified window
        GLFWcursorposfun addMouseMoveCallback( GLFWwindow* window );

        // called when any mouse button is pressed
        GLFWmousebuttonfun addMouseButtonCallback( GLFWwindow* window );

        // called when a scroll event (e.g. mouse wheel) occurs
        GLFWscrollfun addScrollCallback( GLFWwindow* window );

        // called when the window changes size
        GLFWwindowsizefun addWindowSizeCallback( GLFWwindow* window );

        // called when the framebuffer changes size
        GLFWframebuffersizefun addFramebufferSizeCallback( GLFWwindow* window );
};

#endif


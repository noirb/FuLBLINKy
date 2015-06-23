#ifndef _INPUT_MAPPING_H
#define _INPUT_MAPPING_H

#include <CEGUI/CEGUI.h>
#include <GLFW/glfw3.h>

/* ------------------------------------------------------------------------- */
/* This just provides a mapping between the GLFW keycodes and the equivalent */
/* codes in CEGUI to make transferring user input between the two easier.    */
/* ------------------------------------------------------------------------- */

/// Given a GLFW keycode, return the corresponding CEGUI keycode
unsigned int GlfwToCeguiKey(int glfwKey);

/// Given a GLFW mouse button, return the corresponding CEGUI button
CEGUI::MouseButton GlfwToCeguiButton(int glfwButton);

#endif


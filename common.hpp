#ifndef __COMMON_H__
#define __COMMON_H__

#define GL_GLEXT_PROTOTYPES // MUST be before any GL-related includes!

/// GLEW -- Must be before GLFW!
//#include <GL/glew.h>

/// CEGUI -- Must be included before GLFW
#include <CEGUI/CEGUI.h>
#include <CEGUI/RendererModules/OpenGL/GL3Renderer.h>

/// GLFW -- Must be before anything else that touches OpenGL (except CEGUI and GLEW)
#include <GLFW/glfw3.h>

/// GLM
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

    /// FIXME: This should NOT be here, omg... -.-
enum Interpolation {
    LINEAR,
    SMOOTH,
    EXPONENTIAL
};


#endif

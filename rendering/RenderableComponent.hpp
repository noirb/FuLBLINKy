#ifndef _RENDERABLE_COMPONENT_H
#define _RENDERABLE_COMPONENT_H

#include "../common.hpp"
#include "../dataProviders/DataProvider.hpp"

/********************************************************************************/
/* Base class for all Renderable Component Objects                              */
/* Anything which contributes to the frame buffer must be a RenderableComponent */
/********************************************************************************/

class RenderableComponent
{
    public:
        ~RenderableComponent();

        // must be called before Draw(). Used to generate any needed vertex buffers, etc.
        virtual void PrepareGeometry(DataProvider*) = 0;

        virtual void Draw(glm::mat4, GLuint) = 0;

        void SetShader(GLuint programID);

    protected:
        GLfloat*  vertex_buffer_data;  // vertex data specific to this renderer
        GLfloat** vertex_attrib_data;  // collection of vertex attributes (e.g. color, density, etc)
        GLuint shaderProgram;         // shaders to use when we render
        GLuint VBO;                   // Vertex Buffer Object
        GLuint VAO;                   // Vertex Array Object
        int totalVertices = 0;        // Total # of vertices this object will draw each frame
        int totalAttributes = 0;
};

#endif

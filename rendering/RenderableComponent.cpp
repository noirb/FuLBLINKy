#include "RenderableComponent.hpp"


// Destructor: clean up now-unused objects
RenderableComponent::~RenderableComponent()
{
    if (vertex_buffer_data)
    {
        delete vertex_buffer_data;
    }

    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
}


void RenderableComponent::SetShader(GLuint programID)
{
    this->shaderProgram = programID;
}


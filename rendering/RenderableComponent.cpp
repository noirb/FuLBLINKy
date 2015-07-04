#include "RenderableComponent.hpp"
#include <iostream>

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


void RenderableComponent::SetShader(ShaderProgram* program)
{
    this->shaderProgram = program;
}

void RenderableComponent::SetMaxColor(float r, float g, float b, float a)
{
    std::cout << "New Max Color: " << r << "," << g << "," << b << "," << a << std::endl;
    this->maxColor[0] = r;
    this->maxColor[1] = g;
    this->maxColor[2] = b;
    this->maxColor[3] = a;
}

void RenderableComponent::SetMinColor(float r, float g, float b, float a)
{
    std::cout << "New Min Color: " << r << "," << g << "," << b << "," << a << std::endl;
    this->minColor[0] = r;
    this->minColor[1] = g;
    this->minColor[2] = b;
    this->minColor[3] = a;
}
void RenderableComponent::Enable()
{
    this->enabled = true;
}

void RenderableComponent::Disable()
{
    this->enabled = false;
}


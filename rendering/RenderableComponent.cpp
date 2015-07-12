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
    this->maxColor[0] = r;
    this->maxColor[1] = g;
    this->maxColor[2] = b;
    this->maxColor[3] = a;
}

void RenderableComponent::SetMinColor(float r, float g, float b, float a)
{
    this->minColor[0] = r;
    this->minColor[1] = g;
    this->minColor[2] = b;
    this->minColor[3] = a;
}

void RenderableComponent::SetInterpolator(Interpolation i)
{
    this->interpolator = i;
}

void RenderableComponent::SetInterpolationBias(float b)
{
    this->bias = b;
}

void RenderableComponent::SetColorField(std::string fieldName)
{
    this->colorParamField = fieldName;
}

void RenderableComponent::SetScaleField(std::string fieldName)
{
    this->scaleParamField = fieldName;
}

void RenderableComponent::SetAutoScale(bool b)
{
    this->autoScale = b;
}

void RenderableComponent::SetScale(double min, double max)
{
    this->scaleFactorMin = min;
    this->scaleFactorMax = max;
}

void RenderableComponent::Enable()
{
    this->enabled = true;
}

void RenderableComponent::Disable()
{
    this->enabled = false;
}


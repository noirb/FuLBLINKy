#include "RenderableComponent.hpp"
#include <iostream>

// Destructor: clean up now-unused objects
RenderableComponent::~RenderableComponent()
{
    if (_vertex_buffer_data)
    {
        delete _vertex_buffer_data;
    }

    glDeleteBuffers(1, &_VBO);
    glDeleteVertexArrays(1, &_VAO);
}


void RenderableComponent::SetShader(ShaderProgram* program)
{
    _shaderProgram = program;
}

void RenderableComponent::SetMaxColor(float r, float g, float b, float a)
{
    _maxColor[0] = r;
    _maxColor[1] = g;
    _maxColor[2] = b;
    _maxColor[3] = a;
}

void RenderableComponent::SetMinColor(float r, float g, float b, float a)
{
    _minColor[0] = r;
    _minColor[1] = g;
    _minColor[2] = b;
    _minColor[3] = a;
}

void RenderableComponent::SetInterpolator(Interpolation i)
{
    _interpolator = i;
}

void RenderableComponent::SetInterpolationBias(double b)
{
    _bias = b;
}

void RenderableComponent::SetColorField(std::string fieldName)
{
    _colorParamField = fieldName;
}

void RenderableComponent::SetScaleField(std::string fieldName)
{
    _scaleParamField = fieldName;
}

void RenderableComponent::SetAutoScale(bool b)
{
    _autoScale = b;
}

void RenderableComponent::SetScale(double min, double max)
{
    if (min >= 0)
        _scaleFactorMin = min;

    if (max >= 0)
        _scaleFactorMax = max;
}

void RenderableComponent::Enable()
{
    _enabled = true;
}

void RenderableComponent::Disable()
{
    _enabled = false;
}


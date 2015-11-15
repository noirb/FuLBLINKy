#include "GradientRenderer.hpp"
#include <iostream>

void GradientRenderer::PrepareGeometry(DataProvider *)
{
    // if we've already set up our vertices, do nothing
    if (_totalVertices > 0)
    {
        return;
    }

    _totalVertices = 3;
    _vertex_buffer_data = new GLfloat[3 * _totalVertices];
    GLfloat vertices[] = {
        -3.0f,-1.0f, 0.0f,
         1.0f,-1.0f, 0.0f,
         1.0f, 3.0f, 0.0f
    };
    for (unsigned int i = 0; i < _totalVertices*3; i++)
    {
        _vertex_buffer_data[i] = vertices[i];
    }

    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * _totalVertices * 3, _vertex_buffer_data, GL_STATIC_DRAW);

    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        0,
        (void*)0
    );
    glEnableVertexAttribArray(0);

    // reset GL state
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    _VAO = vao;
    _VBO = vbo;
}

void GradientRenderer::Draw(glm::mat4 MVP)
{
    if (!_enabled) { return; }

    if (_shaderProgram == NULL || _VBO <= 0 || _VAO <= 0)
    {
        std::cout << "GradientRenderer::Draw FAILED" << std::endl;
        return;
    }

    _shaderProgram->enable();

    glUniform4fv(_shaderProgram->getUniform("startColor"), 1, _startColor);
    glUniform4fv(_shaderProgram->getUniform("endColor"), 1, _endColor);

    glBindVertexArray(_VAO);
    glDrawArrays(GL_TRIANGLES, 0, _totalVertices);

    // cleanup
    _shaderProgram->disable();
    glBindVertexArray(0);
}

void GradientRenderer::SetStartColor(float * rgba)
{
    for (int i = 0; i < 4; i++)
    {
        _startColor[i] = rgba[i];
    }
}

void GradientRenderer::SetEndColor(float * rgba)
{
    for (int i = 0; i < 4; i++)
    {
        _endColor[i] = rgba[i];
    }
}

void GradientRenderer::SetColors(float * start, float * end)
{
    for (int i = 0; i < 4; i++)
    {
        _startColor[i] = start[i];
        _endColor[i] = end[i];
    }
}

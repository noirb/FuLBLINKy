#include "GradientRenderer.hpp"
#include <iostream>

void GradientRenderer::PrepareGeometry(DataProvider *)
{
    // if we've already set up our vertices, do nothing
    if (this->totalVertices > 0)
    {
        return;
    }

    this->totalVertices = 3;
    this->vertex_buffer_data = new GLfloat[3 * this->totalVertices];
    GLfloat vertices[] = {
        -3.0f,-1.0f, 0.0f,
         1.0f,-1.0f, 0.0f,
         1.0f, 3.0f, 0.0f
    };
    for (unsigned int i = 0; i < this->totalVertices*3; i++)
    {
        this->vertex_buffer_data[i] = vertices[i];
    }

    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * this->totalVertices * 3, this->vertex_buffer_data, GL_STATIC_DRAW);

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

    this->VAO = vao;
    this->VBO = vbo;
}

void GradientRenderer::Draw(glm::mat4 MVP)
{
    if (!this->enabled) { return; }

    if (this->shaderProgram == NULL || this->VBO <= 0 || this->VAO <= 0)
    {
        std::cout << "GradientRenderer::Draw FAILED" << std::endl;
        return;
    }

    this->shaderProgram->enable();

    glUniform4fv(this->shaderProgram->getUniform("startColor"), 1, this->_startColor);
    glUniform4fv(this->shaderProgram->getUniform("endColor"), 1, this->_endColor);

    glBindVertexArray(this->VAO);
    glDrawArrays(GL_TRIANGLES, 0, this->totalVertices);

    // cleanup
    this->shaderProgram->disable();
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

#include "AxesRenderer.hpp"
#include <iostream>

void AxesRenderer::PrepareGeometry(DataProvider* provider)
{
    // if we've already set up our vertices, do nothing
    if (_totalVertices > 0)
    {
        return;
    }

    _totalVertices = 6; // two vertices per line
    _vertex_buffer_data = new GLfloat[3 * _totalVertices];
    GLfloat vertices[] = { // this just saves some typing since we can't assign a list like this directly to an array
        0.0f, 0.0f, 0.0f,       // origin-to-x
        10.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,       // origin-to-y
        0.0f, 10.0f, 0.0f,
        0.0f, 0.0f, 0.0f,       // origin-to-z
        0.0f, 0.0f, 10.0f
    };

    for (unsigned int i = 0; i < _totalVertices*3; i++)       // copy vertex data from vertices to vertex_buffer_data
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

void AxesRenderer::Draw(glm::mat4 MVP)
{
    if (!_enabled) { return; }

    if (_shaderProgram == NULL || _VBO <= 0 || _VAO <= 0)
    {
        std::cout << "AxesRenderer::Draw FAILED" << std::endl;
        return;
    }

    _shaderProgram->enable();
    glUniformMatrix4fv(_shaderProgram->getUniform("MVP"), 1, GL_FALSE, &MVP[0][0]);

    glBindVertexArray(_VAO); 

    glLineWidth(1.0);

    // DRAW!
    glDrawArrays(GL_LINES, 0, _totalVertices);

    // cleanup
    _shaderProgram->disable();
    glBindVertexArray(0);
}

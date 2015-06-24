#include "PointRenderer.hpp"
#include <iostream>

void PointRenderer::PrepareGeometry()
{
    /// TODO: Finalize Interface & remove this dummy
}

void PointRenderer::PrepareGeometry(std::vector<std::vector<double> >* points)
{
    // if we previously allocated space for our vertices, clear it before continuing
    if (this->totalVertices > 0)
    {
        delete(this->vertex_buffer_data);
    }

    // determine needed number of vertices & allocate space for them
    // PointRenderer only needs ONE vertex per data point
    this->totalVertices = (*points).size();
    this->vertex_buffer_data = new GLfloat[3 * this->totalVertices]; // 3 floats per vertex

    // we want to render points exactly at the locations specified by points, so just copy them
    int i = 0;
    for (std::vector<std::vector<double> >::iterator it = (*points).begin(); it != (*points).end(); ++it)
    {
        // get coords for current point
        for (std::vector<double>::iterator it2 = (*it).begin(); it2 != (*it).end(); ++it2)
        {
            std::cout << *it2 << ", "; 
            this->vertex_buffer_data[i] = *it2;
            i++;
        }
        std::cout << std::endl;
    }

    GLuint vao, vbo;
    // copy vertex data to GPU & save VAO and VBO handles
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * this->totalVertices * 3, this->vertex_buffer_data, GL_STATIC_DRAW);

    // reset GL state
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    this->VAO = vao;
    this->VBO = vbo;
}

void PointRenderer::Draw()
{
    /// TODO: Finalize interface & remove this dummy
}

void PointRenderer::Draw(glm::mat4 MVP, GLuint MVP_ID)
{
    // if we have no shaders, vertices, etc., we can't render anything
    if (this->shaderProgram <= 0 || this->VBO <= 0 || this->VAO <= 0)
    {
        return; /// TODO: Log an error here!
    }

    glBindVertexArray(this->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);

    // set shaders
    glUseProgram(this->shaderProgram);


    glUniformMatrix4fv(MVP_ID, 1, GL_FALSE, &MVP[0][0]);

    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        0,
        (void*)0
    );

    glDrawArrays(GL_POINTS, 0, this->totalVertices);

    // unbind our vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, 0);    

    // unset shaders
    glUseProgram(0);

    glBindVertexArray(0);
}


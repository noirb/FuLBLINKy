#include "PointRenderer.hpp"
#include "Compositor.hpp"
#include <iostream>

void PointRenderer::PrepareGeometry(DataProvider* provider)
{
    std::vector<std::vector<double> >* points;
    std::vector<std::vector<double> >* densities;

    if ( provider->GetField("points", &points) != 0)
    {
        std::cout << "ERROR<PointRenderer::PrepareGeometry>: Points Field Could not be retrieved!" << std::endl;
        return;
    }
    if ( provider->GetField("density", &densities) != 0)
    {
        std::cout << "ERROR<PointRenderer::PrepareGeometry>: Density Field could not be retrieved!" << std::endl;
    }

    std::cout << "PointRenderer::PrepareGeometry -- processing " << (*points).size() << " points and " << (*densities).size() << " densities" << std::endl;
    // if we previously allocated space for our vertices, clear it before continuing
    if (this->totalVertices > 0)
    {
        delete(this->vertex_buffer_data);
    }
    if (this->totalAttributes > 0)
    {
        for (int i = 0; i < this->totalAttributes; i++)
        {
            delete(this->vertex_attrib_data[i]);
        }
    }


    /** Copy point data **/

    // determine needed number of vertices & allocate space for them
    // PointRenderer only needs ONE vertex per data point
    this->totalVertices = (*points).size();
    this->vertex_buffer_data = new GLfloat[3 * this->totalVertices]; // 3 floats per vertex

    // we want to render points exactly at the locations specified by points, so just copy them
    int i = 0;
    for (auto point : *points)
    {
        // get coords for current point
        for (auto component : point)
        {
            this->vertex_buffer_data[i] = component; // copy each x,y,z component from each point
            i++;
        }
    }

    /** Copy density data **/

    this->totalAttributes = 1; // TODO: don't hard-code this...
    this->vertex_attrib_data = new GLfloat*[this->totalAttributes];
    this->vertex_attrib_data[0] = new GLfloat[this->totalVertices]; // 1 density per vertex
    i = 0;
    for (auto density_vector : *densities)
    {
        for (auto density : density_vector)
        {
            this->vertex_attrib_data[0][i] = density; // density_vector should always just have 1 element, but this generalizes to any number of elements
            i++;
        }
    }

    GLuint vao, vbo, density_buf;

    /** copy vertex data to GPU & save VAO and VBO handles **/
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * this->totalVertices * 3, this->vertex_buffer_data, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        0,
        (void*)0
    );

    /** Buffer density data **/
    glGenBuffers(1, &density_buf);
    glBindBuffer(GL_ARRAY_BUFFER, density_buf);

    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * this->totalVertices, this->vertex_attrib_data[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, density_buf);
    glVertexAttribPointer(
        1,
        1,
        GL_FLOAT,
        GL_FALSE,
        0,
        (void*)0
    );
    
    // reset GL state
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    this->VAO = vao;
    this->VBO = vbo;

    // save min/max values for rendering colored gradients/scaling/etc
    this->maxGradientValue = provider->GetMaxValueFromField("density");
    this->minGradientValue = provider->GetMinValueFromField("density");
    std::cout << "PointRenderer: Max Density: " << this->maxGradientValue << ", Min: " << this->minGradientValue << std::endl;

    this->maxColorID = glGetUniformLocation(this->shaderProgram, "hotColor");
    this->minColorID = glGetUniformLocation(this->shaderProgram, "coldColor");
}

void PointRenderer::Draw(glm::mat4 MVP, GLuint MVP_ID)
{
    if (!this->enabled) { return; }

    // if we have no shaders, vertices, etc., we can't render anything
    if (this->shaderProgram <= 0 || this->VBO <= 0 || this->VAO <= 0)
    {
        return; /// TODO: Log an error here!
    }

    // set shaders
    glUseProgram(this->shaderProgram);
    glUniformMatrix4fv(MVP_ID, 1, GL_FALSE, &MVP[0][0]);
    glUniform1f(Compositor::Instance().scalarMaxID, this->maxGradientValue);
    glUniform1f(Compositor::Instance().scalarMinID, this->minGradientValue);
    glUniform4fv(this->maxColorID, 1, this->maxColor);
    glUniform4fv(this->minColorID, 1, this->minColor);
    glBindVertexArray(this->VAO);

    // DRAW!
    glDrawArrays(GL_POINTS, 0, this->totalVertices);

    // unset shaders
    glUseProgram(0);

    // unbind VAO
    glBindVertexArray(0);
}


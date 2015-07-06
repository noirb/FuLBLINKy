#include "LineRenderer.hpp"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

void LineRenderer::PrepareGeometry(DataProvider* provider)
{
    float velVectorScale = 1;
    std::vector<std::vector<double> >* points;
    std::vector<std::vector<double> >* velocities;

    if ( provider->GetField("points", &points) != 0)
    {
        std::cout << "ERROR<LineRenderer::PrepareGeometry>: Points Field Could not be retrieved!" << std::endl;
        return;
    }
    if ( provider->GetField("velocity", &velocities) != 0)
    {
        std::cout << "ERROR<LineRenderer::PrepareGeometry>: Velocity Field could not be retrieved!" << std::endl;
    }

    std::cout << "LineRenderer::PrepareGeometry -- processing " << (*points).size() << " points and " << (*velocities).size() << " velocities" << std::endl;
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
    // Each line requires 2 vertices, so wee need 2 * numPoints total
    this->totalVertices = (*points).size() * 2;
    this->vertex_buffer_data = new GLfloat[3 * this->totalVertices]; // 3 floats per vertex


    int i = 0;
    for (uint p = 0; p < (*points).size(); p++)
    {
        // get velocity at point p
        glm::vec3 velocity = glm::vec3(velocities->at(p)[0], velocities->at(p)[1], velocities->at(p)[2]);
        glm::vec3 point    = glm::vec3(points->at(p)[0], points->at(p)[1], points->at(p)[2]);
        glm::vec3 point_2  = point;
        if (glm::length(velocity) > 0)
        {
            point_2 += velVectorScale * glm::normalize(velocity);
        }
        else
        {
            point_2 += velVectorScale;
        }

        // base point of line should be exactly at p
        for (int c = 0; c < 3; c++)
        {
            this->vertex_buffer_data[i] = point[c];
            i++;
        }

        // next point should be offset in the direction of the velocity at p
        for (int c = 0; c < 3; c++)
        {
            this->vertex_buffer_data[i] = point_2[c];
            i++;
        }
    }
    std::cout << i << std::endl;


    /** Copy velocity data **/

    this->totalAttributes = 1; // TODO: don't hard-code this...
    this->vertex_attrib_data = new GLfloat*[this->totalAttributes];
    this->vertex_attrib_data[0] = new GLfloat[this->totalVertices]; // 1 velocity magnitude per *vertex*
    i = 0;
    for (auto velocity_vector : *velocities)
    {
        // compute velocity magnitude at this point
        glm::vec3 vel = glm::vec3(velocity_vector[0], velocity_vector[1], velocity_vector[2]);
        double mag = glm::length(vel);

        // for each vertex in the current line, store the velocity magnitude
        for (int j = 0; j < 2; j++)
        {
            this->vertex_attrib_data[0][i] = mag;
            i++;
        }
    }
    std::cout << i << std::endl;
    GLuint vao, vbo, velocity_buf;

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

    /** Buffer velocity data **/
    glGenBuffers(1, &velocity_buf);
    glBindBuffer(GL_ARRAY_BUFFER, velocity_buf);

    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * this->totalVertices, this->vertex_attrib_data[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, velocity_buf);
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

    // save velocity max/min for rendering  /// TODO: These could be moved?
    this->maxGradientValue = provider->GetMaxValueFromField("velocity");
    this->minGradientValue = provider->GetMinValueFromField("velocity");
    std::cout << "LineRenderer: Max Velocity: " << this->maxGradientValue << ", Min: " << this->minGradientValue << std::endl;
}

void LineRenderer::Draw(glm::mat4 MVP)
{
    if (!this->enabled) { return; }

    // if we have no shaders, vertices, etc., we can't render anything
    if (this->shaderProgram == NULL || this->VBO <= 0 || this->VAO <= 0)
    {
        return; /// TODO: Log an error here!
    }

    // set shaders
    this->shaderProgram->enable();

    // send uniforms to shaders
    glUniformMatrix4fv(shaderProgram->getUniform("MVP"), 1, GL_FALSE, &MVP[0][0]);
    glUniform1f(shaderProgram->getUniform("max_scalar"), this->maxGradientValue);
    glUniform1f(shaderProgram->getUniform("min_scalar"), this->minGradientValue);
    glUniform4fv(shaderProgram->getUniform("hotColor"), 1, this->maxColor);
    glUniform4fv(shaderProgram->getUniform("coldColor"), 1, this->minColor);
    glUniform1f(shaderProgram->getUniform("bias"), this->bias);
    glUniform1i(shaderProgram->getUniform("interpolator"), this->interpolator);

    // bind VAO
    glBindVertexArray(this->VAO);

    // DRAW!
    glDrawArrays(GL_LINES, 0, this->totalVertices);

    // unbind VAO
    glBindVertexArray(0);

    // unset shaders
    this->shaderProgram->disable();
}

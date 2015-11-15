#include "LineRenderer.hpp"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

void LineRenderer::PrepareGeometry(DataProvider* provider)
{
    if (!provider) { return; } // do not attempt to generate geometry without a provider!

    float velVectorScale = 1;
    std::vector<std::vector<double> >* points;
    std::vector<std::vector<double> >* velocities;
    std::vector<std::vector<double> >* color_scalarField;

    if ( provider->GetField("points", &points) != 0)
    {
        std::cout << "ERROR<LineRenderer::PrepareGeometry>: Points Field Could not be retrieved!" << std::endl;
        return;
    }
    if ( provider->GetField("velocity", &velocities) != 0)
    {
        std::cout << "ERROR<LineRenderer::PrepareGeometry>: Velocity Field could not be retrieved!" << std::endl;
        return;
    }
    if ( provider->GetField(_colorParamField, &color_scalarField) != 0)
    {
        std::cout << "ERROR<LineRenderer::PrepareGeometry>: " << _colorParamField << " Field could not be retrieved!" << std::endl;
        return;
    }

    // save scalar max/min for rendering  /// TODO: These could be moved?
    _maxGradientValue = provider->GetMaxValueFromField(_colorParamField);
    _minGradientValue = provider->GetMinValueFromField(_colorParamField);

    if (_autoScale)
    {
        _scaleFactorMin = _minGradientValue;
        _scaleFactorMax = _maxGradientValue;
    }

    // if we previously allocated space for our vertices, clear it before continuing
    if (_totalVertices > 0)
    {
        delete(_vertex_buffer_data);
    }
    if (_totalAttributes > 0)
    {
        for (unsigned int i = 0; i < _totalAttributes; i++)
        {
            delete(_vertex_attrib_data[i]);
        }
    }


    /** Copy point data **/

    // determine needed number of vertices & allocate space for them
    // Each line requires 2 vertices, so wee need 2 * numPoints total
    _totalVertices = (*points).size() * 2;
    _vertex_buffer_data = new GLfloat[3 * _totalVertices]; // 3 floats per vertex


    int i = 0;
    for (unsigned int p = 0; p < (*points).size(); p++)
    {
        // set scale for point p
        if (_scaleFactorMin == _scaleFactorMax)
        {
            velVectorScale = (float)glm::mix(0.0, 1.0, _scaleFactorMin);
        }
        else
        {
            velVectorScale = (float)glm::abs(glm::mix(0.0, 1.0, ((color_scalarField->at(p))[0] - _scaleFactorMin) / (_scaleFactorMax - _scaleFactorMin)));
        }

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
            _vertex_buffer_data[i] = point[c];
            i++;
        }

        // next point should be offset in the direction of the velocity at p
        for (int c = 0; c < 3; c++)
        {
            _vertex_buffer_data[i] = point_2[c];
            i++;
        }
    }


    /** Copy velocity data **/

    _totalAttributes = 1; // TODO: don't hard-code this...
    _vertex_attrib_data = new GLfloat*[_totalAttributes];
    _vertex_attrib_data[0] = new GLfloat[_totalVertices]; // 1 scalar value per *vertex*
    i = 0;
    for (auto scalar_vector : *color_scalarField)
    {
        double mag;
        if (scalar_vector.size() > 1)
        {
            // compute vector magnitude at this point
            glm::vec3 vel = glm::vec3(scalar_vector[0], scalar_vector[1], scalar_vector[2]);
            mag = glm::length(vel);
        }
        else
        {
            mag = scalar_vector[0];
        }

        // for each vertex in the current line, store the scalar value
        for (int j = 0; j < 2; j++)
        {
            _vertex_attrib_data[0][i] = (GLfloat)mag;
            i++;
        }
    }

    GLuint vao, vbo, scalar_buf;

    /** copy vertex data to GPU & save VAO and VBO handles **/
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * _totalVertices * 3, _vertex_buffer_data, GL_STATIC_DRAW);

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
    glGenBuffers(1, &scalar_buf);
    glBindBuffer(GL_ARRAY_BUFFER, scalar_buf);

    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * _totalVertices, _vertex_attrib_data[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, scalar_buf);
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

    _VAO = vao;
    _VBO = vbo;
}

void LineRenderer::Draw(glm::mat4 MVP)
{
    if (!_enabled) { return; }

    // if we have no shaders, vertices, etc., we can't render anything
    if (_shaderProgram == NULL || _VBO <= 0 || _VAO <= 0)
    {
        return; /// TODO: Log an error here!
    }

    // set shaders
    _shaderProgram->enable();

    glLineWidth(2.0);

    // send uniforms to shaders
    glUniformMatrix4fv(_shaderProgram->getUniform("MVP"), 1, GL_FALSE, &MVP[0][0]);
    glUniform1f(_shaderProgram->getUniform("max_scalar"), (GLfloat)_maxGradientValue);
    glUniform1f(_shaderProgram->getUniform("min_scalar"), (GLfloat)_minGradientValue);
    glUniform4fv(_shaderProgram->getUniform("hotColor"), 1, _maxColor);
    glUniform4fv(_shaderProgram->getUniform("coldColor"), 1, _minColor);
    glUniform1f(_shaderProgram->getUniform("bias"), (GLfloat)_bias);
    glUniform1i(_shaderProgram->getUniform("interpolator"), _interpolator);

    // bind VAO
    glBindVertexArray(_VAO);

    // DRAW!
    glDrawArrays(GL_LINES, 0, _totalVertices);

    // unbind VAO
    glBindVertexArray(0);

    // unset shaders
    _shaderProgram->disable();
}


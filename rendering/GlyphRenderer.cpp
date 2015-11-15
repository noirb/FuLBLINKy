#include "GlyphRenderer.hpp"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
void GlyphRenderer::PrepareGeometry(DataProvider* provider)
{
    if (!provider) { return; } // do not attempt to generate geometry without a provider!
    static const int ArrowGlyphSize = 60;//sizeof(g_arrow2d_vertex_buffer_data)/sizeof(float);
    std::vector<std::vector<double> >* points;
    std::vector<std::vector<double> >* color_scalarField;
    std::vector<std::vector<double> >* velocities;

    if ( provider->GetField("points", &points) != 0)
    {
        std::cout << "ERROR<GlyphRenderer::PrepareGeometry>: Points Field Could not be retrieved!" << std::endl;
        return;
    }
    if ( provider->GetField(_colorParamField, &color_scalarField) != 0)
    {
        std::cout << "ERROR<GlyphRenderer::PrepareGeometry>: " << _colorParamField << " Field could not be retrieved!" << std::endl;
        return;
    }
    if ( provider->GetField("velocity", &velocities) != 0)
    {
        std::cout << "ERROR<GlyphRenderer::PrepareGeometry>: Velocity Field could not be retrieved!" << std::endl;
        return;
    }

    // save velocity max/min for rendering  /// TODO: These could be moved?
    _maxGradientValue = provider->GetMaxValueFromField(_colorParamField);
    _minGradientValue = provider->GetMinValueFromField(_colorParamField);

    if (_autoScale)
    {
        _scaleFactorMin = _minGradientValue;
        _scaleFactorMax = _maxGradientValue;
    }

    std::cout << "GlyphRenderer::PrepareGeometry -- processing " << (*points).size() << " points and " << (*color_scalarField).size() << " scalars" << std::endl;
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
    // GlyphRenderer needs ArrowGlyphSize vertices per data point
    _totalVertices = (*points).size();
    _vertex_buffer_data = new GLfloat[ArrowGlyphSize * 3 * _totalVertices];


    // we want to render points exactly at the locations specified by points, so just copy them
    int i = 0;
    double max_velocity = provider->GetMaxValueFromField("velocity");
    for (unsigned int loopVarVertices = 0; loopVarVertices < _totalVertices; loopVarVertices++)
    {
        float velTemp[3];
        for (int loopVarComponents = 0; loopVarComponents < 3; loopVarComponents++)
        {
            velTemp[loopVarComponents] = (float)(velocities->at(loopVarVertices))[loopVarComponents];
        }

        double local_scaling;
           local_scaling = 0.1;

        glm::mat4 M = glm::mat4(1.0f);
        M = glm::translate(M,  glm::vec3((points->at(loopVarVertices))[0],    // translation matrix to current location in dataset
                                         (points->at(loopVarVertices))[1],
                                         (points->at(loopVarVertices))[2]));
        glm::vec3 source_vec = glm::normalize(glm::vec3(0.0, 0.0, 1.0));      // our current direction (all glyphs face +Z by default)
        glm::vec3 target_vec = glm::vec3(velTemp[0], velTemp[1], velTemp[2]); // vector facing direction we want to face
        if (glm::length(target_vec) > 0.0)
        {
            target_vec = glm::normalize(target_vec);
            glm::vec3 rot_axis = glm::cross(source_vec, target_vec);

            if (glm::length(rot_axis) == 0)
            {
                if (glm::dot(source_vec, target_vec) < 0)
                {
                    glm::vec3 temp = target_vec;
                    temp[0] = temp[0] + 1.432342f; temp[1] = temp[1] + 1.234235342f; temp[2] = temp[2] + 1.1244325f;
                    rot_axis = glm::cross(source_vec, temp);
                    M = glm::rotate(M, 3.1415f, rot_axis);
                }
            }
            else
            {
                float rot_angle = glm::acos(glm::dot(source_vec, target_vec));
                M = glm::rotate(M, rot_angle, rot_axis);                    // rotation matrix from (0,0,1) to velocity dir at this location
            }
        }

        // Loop through the arrow skeleton
        for (int loopVarGlyphPts = 0; loopVarGlyphPts < ArrowGlyphSize *3; loopVarGlyphPts += 3)
        {
            double velVectorScale;
            if (_autoScale)
            {
                if (max_velocity != 0)
                {
                    velVectorScale = exp(sqrt(velTemp[0]*velTemp[0] + velTemp[1]*velTemp[1] + velTemp[2]*velTemp[2])/max_velocity)/exp(1);
                }
                else
                {
                    velVectorScale = 1;
                }
            }
            else
            {
                double scaleFactor;
                if (_scaleFactorMin == _scaleFactorMax)
                {
                    scaleFactor = _scaleFactorMin;
                }
                else
                {
                    if (_colorParamField == "velocity")    /// HACK: We should not be checking the name of the field here
                    {
                        scaleFactor = glm::abs((glm::length(glm::vec3(velTemp[0], velTemp[1], velTemp[2])) - _scaleFactorMin) / (_scaleFactorMax - _scaleFactorMin));
                    }
                    else
                    {
                        scaleFactor = glm::abs(((color_scalarField->at(loopVarVertices))[0] - _scaleFactorMin) / (_scaleFactorMax - _scaleFactorMin));
                    }
                }
                velVectorScale = glm::mix(0.0, 1.0, scaleFactor);
            }

            // get coords for current vertex
            glm::vec4 glyphPointTemp = glm::vec4(
                            local_scaling*velVectorScale*g_arrow2d_vertex_buffer_data[loopVarGlyphPts+0],
                            local_scaling*velVectorScale*g_arrow2d_vertex_buffer_data[loopVarGlyphPts+1],
                            local_scaling*velVectorScale*g_arrow2d_vertex_buffer_data[loopVarGlyphPts+2],
                            1.0);
            // apply rotation & translation transforms
            glyphPointTemp = M * glyphPointTemp;

            // store (x,y,z) components of current vertex
            for (int loopVarComponents = 0; loopVarComponents < 3; loopVarComponents++)
            {
                _vertex_buffer_data[i] = glyphPointTemp[loopVarComponents];
                i++;
            }
        }
    }
    std::cout << i << std::endl;
    /** Copy velocity data **/

    _totalAttributes = 1; // TODO: don't hard-code this...
    _vertex_attrib_data = new GLfloat*[_totalAttributes];
    _vertex_attrib_data[0] = new GLfloat[_totalVertices * ArrowGlyphSize]; // 1 velocity magnitude per *vertex*
    i = 0;
    for (auto scalar_vector : *color_scalarField)
    {
        for (int v = 0; v < ArrowGlyphSize; v++)
        {
            switch(_scalarParamType)
            {
                case VECTOR_MAGNITUDE:
                {
                    glm::vec4 val(0);
                    int c = 0;
                    for (auto component : scalar_vector)
                    {
                        val[c] = (float)component;
                        c++;
                    }
                    _vertex_attrib_data[0][i] = glm::length(val);
                    break;
                }
                case VECTOR_X:
                {
                    _vertex_attrib_data[0][i] = (GLfloat)scalar_vector[0];
                    break;
                }
                case VECTOR_Y:
                {
                    _vertex_attrib_data[0][i] = (GLfloat)scalar_vector[1];
                    break;
                }
                case VECTOR_Z:
                {
                    _vertex_attrib_data[0][i] = (GLfloat)scalar_vector[2];
                    break;
                }
                default:
                {
                    std::cout << "ERROR: <GlyphRenderer::PrepareGeometry>: Unknown ScalarParamType '" << _scalarParamType << "'!" << std::endl;
                    _vertex_attrib_data[0][i] = 0;
                    break;
                }
            }
            i++;
        }
    }


    GLuint vao, vbo, velocity_buf;

    /** copy vertex data to GPU & save VAO and VBO handles **/
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * _totalVertices * 3 * ArrowGlyphSize, _vertex_buffer_data, GL_STATIC_DRAW);

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

    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * _totalVertices * ArrowGlyphSize, _vertex_attrib_data[0], GL_STATIC_DRAW);

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

    _VAO = vao;
    _VBO = vbo;


    std::cout << "GlyphRenderer: Max Scalar: " << _maxGradientValue << ", Min: " << _minGradientValue << std::endl;
}

void GlyphRenderer::Draw(glm::mat4 MVP)
{
    if (!_enabled) { return; }

    // if we have no shaders, vertices, etc., we can't render anything
    if (_shaderProgram == NULL || _VBO <= 0 || _VAO <= 0)
    {
        return; /// TODO: Log an error here!
    }

    // set shaders
    _shaderProgram->enable();

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
    glDrawArrays(GL_TRIANGLES, 0, (_totalVertices)*60);

    // unbind VAO
    glBindVertexArray(0);

    // unset shaders
    _shaderProgram->disable();
}

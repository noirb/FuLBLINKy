#include "PointRenderer.hpp"
#include "Compositor.hpp"
#include <iostream>

void PointRenderer::PrepareGeometry(DataProvider* provider)
{
    if (!provider) { return; } // do not try to generate geometry without a data provider!

    std::vector<std::vector<double> >* points;
    std::vector<std::vector<double> >* color_scalarField; // scalar values for determining shading of points

    if ( provider->GetField("points", &points) != 0)
    {
        std::cout << "ERROR<PointRenderer::PrepareGeometry>: Points Field Could not be retrieved!" << std::endl;
        return;
    }
    if ( provider->GetField(_colorParamField, &color_scalarField) != 0)
    {
        std::cout << "ERROR<PointRenderer::PrepareGeometry>: Scalar Field '" << _colorParamField << "' could not be retrieved!" << std::endl;
        return;
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
    // PointRenderer only needs ONE vertex per data point
    _totalVertices = (*points).size();
    _vertex_buffer_data = new GLfloat[3 * _totalVertices]; // 3 floats per vertex

    // we want to render points exactly at the locations specified by points, so just copy them
    int i = 0;
    for (auto point : *points)
    {
        // get coords for current point
        for (auto component : point)
        {
            _vertex_buffer_data[i] = (GLfloat)component; // copy each x,y,z component from each point
            i++;
        }
    }

    /** Copy scalar data **/

    _totalAttributes = 1; // TODO: don't hard-code this...
    _vertex_attrib_data = new GLfloat*[_totalAttributes];
    _vertex_attrib_data[0] = new GLfloat[_totalVertices]; // 1 scalar per vertex
    i = 0;
    for (auto scalar_vector : *color_scalarField)
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
                 std::cout << "ERROR: <PointRenderer::PrepareGeometry>: Unknown ScalarParamType '" << _scalarParamType << "'!" << std::endl;
                 _vertex_attrib_data[0][i] = 0;
                 break;
             }
         }
         i++;
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

    /** Buffer density data **/
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

    // save min/max values for rendering colored gradients/scaling/etc
    _maxGradientValue = provider->GetMaxValueFromField(_colorParamField);
    _minGradientValue = provider->GetMinValueFromField(_colorParamField);

    // save min/max values for scaling
    if (_autoScale)
    {
        _scaleFactorMin = _minGradientValue;
        _scaleFactorMax = _maxGradientValue;
    }
}

void PointRenderer::Draw(glm::mat4 MVP)
{
    if (!_enabled) { return; }

    // if we have no shaders, vertices, etc., we can't render anything
    if (_shaderProgram == NULL || _VBO <= 0 || _VAO <= 0)
    {
        return; /// TODO: Log an error here!
    }

    // set shaders
    _shaderProgram->enable();
    glUniformMatrix4fv(_shaderProgram->getUniform("MVP"), 1, GL_FALSE, &MVP[0][0]);
    glUniform1f(_shaderProgram->getUniform("max_scalar"), (GLfloat)_maxGradientValue);
    glUniform1f(_shaderProgram->getUniform("min_scalar"), (GLfloat)_minGradientValue);
    glUniform1f(_shaderProgram->getUniform("max_sizeScalar"), (GLfloat)_scaleFactorMax);
    glUniform1f(_shaderProgram->getUniform("min_sizeScalar"), (GLfloat)_scaleFactorMin);
    glUniform4fv(_shaderProgram->getUniform("hotColor"), 1, _maxColor);
    glUniform4fv(_shaderProgram->getUniform("coldColor"), 1, _minColor);
    glUniform1f(_shaderProgram->getUniform("bias"), (GLfloat)_bias);
    glUniform1i(_shaderProgram->getUniform("interpolator"), _interpolator);
    glBindVertexArray(_VAO);

    // DRAW!
    glDrawArrays(GL_POINTS, 0, _totalVertices);

    // unset shaders
    _shaderProgram->disable();

    // unbind VAO
    glBindVertexArray(0);
}


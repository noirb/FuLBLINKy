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
    if ( provider->GetField(this->colorParamField, &color_scalarField) != 0)
    {
        std::cout << "ERROR<PointRenderer::PrepareGeometry>: Scalar Field '" << this->colorParamField << "' could not be retrieved!" << std::endl;
        return;
    }

    std::cout << "<PointRenderer::PrepareGeometry>: processing " << (*points).size() << " points and " << (*color_scalarField).size() << " scalars" << std::endl;

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

    /** Copy scalar data **/

    this->totalAttributes = 1; // TODO: don't hard-code this...
    this->vertex_attrib_data = new GLfloat*[this->totalAttributes];
    this->vertex_attrib_data[0] = new GLfloat[this->totalVertices]; // 1 scalar per vertex
    i = 0;
    for (auto scalar_vector : *color_scalarField)
    {
         switch(this->scalarParamType)
         {
             case VECTOR_MAGNITUDE:
             {
                 glm::vec4 val(0);
                 int c = 0;
                 for (auto component : scalar_vector)
                 {
                     val[c] = component;
                     c++;
                 }
                 this->vertex_attrib_data[0][i] = glm::length(val);
                 break;
             }
             case VECTOR_X:
             {
                 this->vertex_attrib_data[0][i] = scalar_vector[0];
                 break;
             }
             case VECTOR_Y:
             {
                 this->vertex_attrib_data[0][i] = scalar_vector[1];
                 break;
             }
             case VECTOR_Z:
             {
                 this->vertex_attrib_data[0][i] = scalar_vector[2];
                 break;
             }
             default:
             {
                 std::cout << "ERROR: <PointRenderer::PrepareGeometry>: Unknown ScalarParamType '" << this->scalarParamType << "'!" << std::endl;
                 this->vertex_attrib_data[0][i] = 0;
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
    glGenBuffers(1, &scalar_buf);
    glBindBuffer(GL_ARRAY_BUFFER, scalar_buf);

    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * this->totalVertices, this->vertex_attrib_data[0], GL_STATIC_DRAW);

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

    this->VAO = vao;
    this->VBO = vbo;

    // save min/max values for rendering colored gradients/scaling/etc
    this->maxGradientValue = provider->GetMaxValueFromField(this->colorParamField);
    this->minGradientValue = provider->GetMinValueFromField(this->colorParamField);
    std::cout << "PointRenderer: Max Scalar Value: " << this->maxGradientValue << ", Min: " << this->minGradientValue << std::endl;
}

void PointRenderer::Draw(glm::mat4 MVP)
{
    if (!this->enabled) { return; }

    // if we have no shaders, vertices, etc., we can't render anything
    if (this->shaderProgram == NULL || this->VBO <= 0 || this->VAO <= 0)
    {
        return; /// TODO: Log an error here!
    }

    // set shaders
    shaderProgram->enable();
    glUniformMatrix4fv(shaderProgram->getUniform("MVP"), 1, GL_FALSE, &MVP[0][0]);
    glUniform1f(shaderProgram->getUniform("max_scalar"), this->maxGradientValue);
    glUniform1f(shaderProgram->getUniform("min_scalar"), this->minGradientValue);
    glUniform4fv(shaderProgram->getUniform("hotColor"), 1, this->maxColor);
    glUniform4fv(shaderProgram->getUniform("coldColor"), 1, this->minColor);
    glUniform1f(shaderProgram->getUniform("bias"), this->bias);
    glUniform1i(shaderProgram->getUniform("interpolator"), this->interpolator);
    glBindVertexArray(this->VAO);

    // DRAW!
    glDrawArrays(GL_POINTS, 0, this->totalVertices);

    // unset shaders
    shaderProgram->disable();

    // unbind VAO
    glBindVertexArray(0);
}


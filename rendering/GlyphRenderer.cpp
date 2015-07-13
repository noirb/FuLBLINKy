#include "GlyphRenderer.hpp"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
void GlyphRenderer::PrepareGeometry(DataProvider* provider)
{
    if (!provider) { return; } // do not attempt to generate geometry without a provider!

    static const int ArrowGlyphSize = 9;//sizeof(g_arrow2d_vertex_buffer_data)/sizeof(float);
    std::vector<std::vector<double> >* points;
    std::vector<std::vector<double> >* color_scalarField;
    std::vector<std::vector<double> >* velocities;

    if ( provider->GetField("points", &points) != 0)
    {
        std::cout << "ERROR<GlyphRenderer::PrepareGeometry>: Points Field Could not be retrieved!" << std::endl;
        return;
    }
    if ( provider->GetField(this->colorParamField, &color_scalarField) != 0)
    {
        std::cout << "ERROR<GlyphRenderer::PrepareGeometry>: " << this->colorParamField << " Field could not be retrieved!" << std::endl;
        return;
    }
    if ( provider->GetField("velocity", &velocities) != 0)
    {
        std::cout << "ERROR<GlyphRenderer::PrepareGeometry>: Velocity Field could not be retrieved!" << std::endl;
        return;
    }

    // save velocity max/min for rendering  /// TODO: These could be moved?
    this->maxGradientValue = provider->GetMaxValueFromField(this->colorParamField);
    this->minGradientValue = provider->GetMinValueFromField(this->colorParamField);

    if (this->autoScale)
    {
        this->scaleFactorMin = this->minGradientValue;
        this->scaleFactorMax = this->maxGradientValue;
    }

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
    // GlyphRenderer needs ArrowGlyphSize vertices per data point
    this->totalVertices = (*points).size();
    this->vertex_buffer_data = new GLfloat[ArrowGlyphSize * 3 * this->totalVertices];


    // we want to render points exactly at the locations specified by points, so just copy them
    int i = 0;
    double max_velocity = provider->GetMaxValueFromField("velocity");
    for (int loopVarVertices = 0; loopVarVertices < this->totalVertices; loopVarVertices++)
    {
        float velTemp[3];
        for (int loopVarComponents = 0; loopVarComponents < 3; loopVarComponents++)
        {
            velTemp[loopVarComponents] = (velocities->at(loopVarVertices))[loopVarComponents];
        }

        double local_scaling;
        if (max_velocity != 0)
        {
            local_scaling = exp(sqrt(velTemp[0]*velTemp[0] + velTemp[1]*velTemp[1] + velTemp[2]*velTemp[2])/max_velocity)/exp(1);
        }
        else
        {
            local_scaling = 0.1;
        }
	
        glm::mat4 M = glm::mat4(1.0f);
        M = glm::translate(M,  glm::vec3((points->at(loopVarVertices))[0],    // translation matrix to current location in dataset
                                         (points->at(loopVarVertices))[1],
                                         (points->at(loopVarVertices))[2]));
        glm::vec3 source_vec = glm::normalize(glm::vec3(0.0, 0.0, 1.0));                      // our current direction (all glyphs face +Z by default)
        glm::vec3 target_vec = glm::vec3(velTemp[0], velTemp[1], velTemp[2]); // vector facing direction we want to face
        if (glm::length(target_vec) > 0.0)
        {
            target_vec = glm::normalize(target_vec);
        	glm::vec3 rot_axis = glm::cross(source_vec, target_vec);
        	float rot_angle = glm::acos(glm::dot(source_vec, target_vec));
        	M = glm::rotate(M, rot_angle, rot_axis);                              // rotation matrix from (0,0,1) to velocity dir at this location
        }

        // Loop through the arrow skeleton
        for (int loopVarGlyphPts = 0; loopVarGlyphPts < ArrowGlyphSize *3; loopVarGlyphPts += 3)
        {
            double scaleFactor;
            if (this->scaleFactorMin == this->scaleFactorMax)
            {
                scaleFactor = this->scaleFactorMin;
            }
            else
            {
                if (this->colorParamField == "velocity")    /// HACK: We should not be checking the name of the field here
                {
                    scaleFactor = (glm::length(glm::vec3(velTemp[0], velTemp[1], velTemp[2])) - this->scaleFactorMin) / (this->scaleFactorMax - this->scaleFactorMin);
                }
                else
                {
                    scaleFactor = ((color_scalarField->at(loopVarVertices))[0] - this->scaleFactorMin) / (this->scaleFactorMax - this->scaleFactorMin);
                }
            }
            double velVectorScale = glm::mix(0.0, 1.0, scaleFactor);

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
                this->vertex_buffer_data[i] = glyphPointTemp[loopVarComponents];
                i++;
            }
        }
    }
    std::cout << i << std::endl;
    /** Copy velocity data **/

    this->totalAttributes = 1; // TODO: don't hard-code this...
    this->vertex_attrib_data = new GLfloat*[this->totalAttributes];
    this->vertex_attrib_data[0] = new GLfloat[this->totalVertices * ArrowGlyphSize]; // 1 velocity magnitude per *vertex*
    i = 0;
    for (auto scalar_vector : *color_scalarField)
    {
        for (int v = 0; v < ArrowGlyphSize; v++)
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
                    std::cout << "ERROR: <GlyphRenderer::PrepareGeometry>: Unknown ScalarParamType '" << this->scalarParamType << "'!" << std::endl;
                    this->vertex_attrib_data[0][i] = 0;
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

    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * this->totalVertices * 3 * ArrowGlyphSize, this->vertex_buffer_data, GL_STATIC_DRAW);

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

    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * this->totalVertices * ArrowGlyphSize, this->vertex_attrib_data[0], GL_STATIC_DRAW);

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
}

void GlyphRenderer::Draw(glm::mat4 MVP)
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
    glDrawArrays(GL_TRIANGLES, 0, (this->totalVertices)*9);

    // unbind VAO
    glBindVertexArray(0);

    // unset shaders
    this->shaderProgram->disable();
}


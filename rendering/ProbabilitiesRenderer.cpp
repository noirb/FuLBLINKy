#include "ProbabilitiesRenderer.hpp"
#include "Compositor.hpp"
#include <iostream>

#define COMPUTEINDEXOF(x, y, z) (int)( (x) * (ylength) * (zlength) + (y) * (zlength) + (z) )
std::vector<double> velocityMagnitudes;

std::vector<double> ProbabilitiesRenderer::GetStartPoint()
{
    return this->startPoint;
}

void ProbabilitiesRenderer::SetStartPoint(double newx, double newy, double newz)
{
    std::cout << "<ProbabilitiesRenderer::SetStartPoint> : " << newx << ", " << newy << ", " << newz << std::endl;
    this->startPoint[0] = newx < this->endPoint[0] ? newx : this->endPoint[0]; // do not allow start/end points to cross
    this->startPoint[1] = newy < this->endPoint[1] ? newy : this->endPoint[1];
    this->startPoint[2] = newz < this->endPoint[2] ? newz : this->endPoint[2];
}

std::vector<double> ProbabilitiesRenderer::GetEndPoint()
{
    return this->endPoint;
}

void ProbabilitiesRenderer::SetEndPoint(double newx, double newy, double newz)
{
    std::cout << "<ProbabilitiesRenderer::SetEndPoint> : " << newx << ", " << newy << ", " << newz << std::endl;
    this->endPoint[0] = newx > this->startPoint[0] ? newx : this->startPoint[0]; // do not allow start/end points to cross
    this->endPoint[1] = newy > this->startPoint[1] ? newy : this->startPoint[1];
    this->endPoint[2] = newz > this->startPoint[2] ? newz : this->startPoint[2];
}

void ProbabilitiesRenderer::PrepareGeometry(DataProvider* provider)
{
    if (!provider) { return; } // do not attempt to generate geometry without a provider!

    double maxProbability = -0.1;
    float VectorScale = 0.5;
    velocityMagnitudes.clear();

    // Number of points in the glyph
    static const int ArrowGlyphSize = 60;

    if ( provider->GetField("points", &_points) != 0)
    {
        std::cout << "ERROR<ProbabilitiesRenderer::PrepareGeometry>: Points Field Could not be retrieved!" << std::endl;
        return;
    }

    DomainParameters domainParameters;
    provider->getDomainParameters(&domainParameters);

    // Read Probabilities
    for (int i = 0; i < 19; i++)
    {
        std::stringstream sstm;
        sstm << "probability" << i;
        std::string probName = sstm.str();
        std::vector<std::vector<double> >* probDump = new std::vector<std::vector<double> >();
        // Push the ith probability field
        _probabilities.push_back(probDump);

        if ( provider->GetField(probName, &_probabilities[i]) != 0)
        {
            std::cout << "ERROR<ProbabilitiesRenderer::PrepareGeometry>: " << probName << " Field could not be retrieved!" << std::endl;
            return;
        }
    }

    std::cout << "ProbabilitiesRenderer::PrepareGeometry -- processing " << (*_points).size() << " points" << std::endl;
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

    // Get domain size
    double xlength = domainParameters.size[0];
    double ylength = domainParameters.size[1];
    double zlength = domainParameters.size[2];

    /** Copy point data **/

    // determine needed number of vertices & allocate space for them
    double minX = glm::max(0.0, startPoint[0]);
    double minY = glm::max(0.0, startPoint[1]);
    double minZ = glm::max(0.0, startPoint[2]);
    double maxX = glm::min(xlength, endPoint[0]);
    double maxY = glm::min(ylength, endPoint[1]);
    double maxZ = glm::min(zlength, endPoint[2]);
    _totalVertices = (unsigned int)((maxZ - minZ + 1) * (maxY - minY + 1) * (maxX - minX + 1));
    _vertex_buffer_data = new GLfloat[18 * ArrowGlyphSize * 3 * _totalVertices]; // 3 floats per vertex


    int globalCounter = 0;

    for (int i = (int)minX; i <= (int)maxX; i++)
    {
        for (int j = (int)minY; j <= (int)maxY; j++)
        {
            for (int k = (int)minZ; k <= (int)maxZ; k++)
            {
                for(int l = 0; l < 19; l++)
                {
                    if (l != 9)
                    {
                        glm::mat4 M = glm::mat4(1.0f);
                        M = glm::translate(M,  glm::vec3((_points->at(COMPUTEINDEXOF(i, j, k)))[0],    // translation matrix to current location in dataset
                                                         (_points->at(COMPUTEINDEXOF(i, j, k)))[1],
                                                         (_points->at(COMPUTEINDEXOF(i, j, k)))[2]));
                        glm::vec3 source_vec = glm::normalize(glm::vec3(0.0, 0.0, 1.0));                      // our current direction (all glyphs face +Z by default)
                        glm::vec3 target_vec = glm::vec3(LATTICEVELOCITIES[l][0], LATTICEVELOCITIES[l][1], LATTICEVELOCITIES[l][2]); // vector facing direction we want to face

                        if (glm::length(target_vec) > 0.0)
                        {
                            target_vec = glm::normalize(target_vec);
                            glm::vec3 rot_axis = glm::cross(source_vec, target_vec);

                            if ((rot_axis[0])*(rot_axis[0]) <= 0.0000001 && (rot_axis[1])*(rot_axis[1]) <= 0.0000001 && (rot_axis[2])*(rot_axis[2]) <= 0.0000001)
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
                                M = glm::rotate(M, rot_angle, rot_axis);                              // rotation matrix from (0,0,1) to velocity dir at this location
                            }
                        }

                        // Loop through the arrow skeleton
                        for (int loopVarGlyphPts = 0; loopVarGlyphPts < ArrowGlyphSize * 3; loopVarGlyphPts += 3)
                        {
                            glm::vec4 glyphPointTemp;
                            // get coords for current vertex
                            if (l == 2 || l == 6 || l == 9 || l == 10 || l == 12 || l == 16)
                            {
                                glyphPointTemp = glm::vec4(
                                            ((_probabilities[l])->at(COMPUTEINDEXOF(i, j, k)))[0]*VectorScale*g_arrow2d_vertex_buffer_data[loopVarGlyphPts+0],
                                            ((_probabilities[l])->at(COMPUTEINDEXOF(i, j, k)))[0]*VectorScale*g_arrow2d_vertex_buffer_data[loopVarGlyphPts+1],
                                            ((_probabilities[l])->at(COMPUTEINDEXOF(i, j, k)))[0]*VectorScale*g_arrow2d_vertex_buffer_data[loopVarGlyphPts+2],
                                            1.0);
                            }
                            else
                            {
                                glyphPointTemp = glm::vec4(
                                            1.414*((_probabilities[l])->at(COMPUTEINDEXOF(i, j, k)))[0]*VectorScale*g_arrow2d_vertex_buffer_data[loopVarGlyphPts+0],
                                            1.414*((_probabilities[l])->at(COMPUTEINDEXOF(i, j, k)))[0]*VectorScale*g_arrow2d_vertex_buffer_data[loopVarGlyphPts+1],
                                            1.414*((_probabilities[l])->at(COMPUTEINDEXOF(i, j, k)))[0]*VectorScale*g_arrow2d_vertex_buffer_data[loopVarGlyphPts+2],
                                            1.0);
                            }
                            // apply rotation & translation transforms
                            glyphPointTemp = M * glyphPointTemp;

                            //current probability
                            double probability = (_probabilities[l])->at(COMPUTEINDEXOF(i, j, k))[0];
                            if (probability > maxProbability)
                            {
                                maxProbability = probability;
                            }

                            // store (x,y,z) components of current vertex*/
                            velocityMagnitudes.push_back(probability);
                            for (int loopVarComponents = 0; loopVarComponents < 3; loopVarComponents++)
                            {
                                _vertex_buffer_data[globalCounter] = glyphPointTemp[loopVarComponents];
                                globalCounter++;
                            }
                        }
                    }
                }
            }
        }
    }



    /** Copy density data **/

    _totalAttributes = 1; // TODO: don't hard-code this...
    _vertex_attrib_data = new GLfloat*[_totalAttributes];
    int num_of_vertices = 18 * _totalVertices * ArrowGlyphSize;
    _vertex_attrib_data[0] = new GLfloat[num_of_vertices]; // 1 velocity magnitude per *vertex*
    for (int i = 0; i < num_of_vertices; i++)
    {
        _vertex_attrib_data[0][i] = (GLfloat)(velocityMagnitudes[i]/maxProbability);
    }

    GLuint vao, vbo, velocity_buf;

    /** copy vertex data to GPU & save VAO and VBO handles **/
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(GLfloat) * _totalVertices * 3 * ArrowGlyphSize, _vertex_buffer_data, GL_STATIC_DRAW);

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

    glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(GLfloat) * _totalVertices * ArrowGlyphSize, _vertex_attrib_data[0], GL_STATIC_DRAW);

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
    _minGradientValue = 0;//minProbability;
    _maxGradientValue = 1.0;// maxProbability;
}

void ProbabilitiesRenderer::Draw(glm::mat4 MVP)
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
    glUniform4fv(_shaderProgram->getUniform("hotColor"), 1, _maxColor);
    glUniform4fv(_shaderProgram->getUniform("coldColor"), 1, _minColor);
    glUniform1f(_shaderProgram->getUniform("bias"), (GLfloat)_bias);
    glUniform1i(_shaderProgram->getUniform("interpolator"), _interpolator);
    glBindVertexArray(_VAO);

    // DRAW!
    glDrawArrays(GL_TRIANGLES, 0, 18 * 60 * _totalVertices);

    // unset shaders
    _shaderProgram->disable();

    // unbind VAO
    glBindVertexArray(0);
}


#include "ProbabilitiesRenderer.hpp"
#include "Compositor.hpp"
#include <iostream>

#define COMPUTEINDEXOF(x, y, z) ( (x) * (ylength) * (zlength) + (y) * (zlength) + (z) )
std::vector<double> velocityMagnitudes;

std::vector<double> ProbabilitiesRenderer::GetStartPoint()
{
    return this->startPoint;
}

void ProbabilitiesRenderer::SetStartPoint(double newx, double newy, double newz)
{
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
    this->endPoint[0] = newx > this->startPoint[0] ? newx : this->startPoint[0]; // do not allow start/end points to cross
    this->endPoint[1] = newy > this->startPoint[1] ? newy : this->startPoint[1];
    this->endPoint[2] = newz > this->startPoint[2] ? newz : this->startPoint[2];
}

void ProbabilitiesRenderer::PrepareGeometry(DataProvider* provider)
{    
    if (!provider) { return; } // do not attempt to generate geometry without a provider!

    double maxProbability = -100;
    double minProbability =  100;
    float VectorScale = 0.5;	
    velocityMagnitudes.clear();

    // Number of points in the glyph
    static const int ArrowGlyphSize = 9;

    if ( provider->GetField("points", &points) != 0)
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
        probabilities.push_back(probDump);
    
        if ( provider->GetField(probName, &probabilities[i]) != 0)
    	{
            std::cout << "ERROR<ProbabilitiesRenderer::PrepareGeometry>: " << probName << " Field could not be retrieved!" << std::endl;
            return;
    	}
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

    // Get domain size
    double xlength = domainParameters.size[0];
    double ylength = domainParameters.size[1];
    double zlength = domainParameters.size[2];

    // determine needed number of vertices & allocate space for them
    double minX = glm::max(0.0, startPoint[0]);
    double minY = glm::max(0.0, startPoint[1]);
    double minZ = glm::max(0.0, startPoint[2]);
    double maxX = glm::min(xlength-1, endPoint[0]);
    double maxY = glm::min(ylength-1, endPoint[1]);
    double maxZ = glm::min(zlength-1, endPoint[2]);
    this->totalVertices = (maxZ - minZ + 1) * (maxY - minY + 1) * (maxX - minX + 1);
    this->vertex_buffer_data = new GLfloat[18 * ArrowGlyphSize * 3 * this->totalVertices]; // 3 floats per vertex


    int globalCounter = 0;

    for (int i = minX; i <= maxX; i++)
    {
    	for (int j = minY; j <= maxY; j++)
        {
            for (int k = minZ; k <= maxZ; k++)
            {
                for(int l = 0; l < 19; l++)
                {
                    if (l != 9)
                    {
			            glm::mat4 M = glm::mat4(1.0f);
			            M = glm::translate(M,  glm::vec3((points->at(COMPUTEINDEXOF(i, j, k)))[0],    // translation matrix to current location in dataset
                                         (points->at(COMPUTEINDEXOF(i, j, k)))[1],
                                         (points->at(COMPUTEINDEXOF(i, j, k)))[2]));
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
            						temp[0] = temp[0] + 1.432342; temp[1] = temp[1] + 1.234235342; temp[2] = temp[2] + 1.1244325;
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
                                            ((probabilities[l])->at(COMPUTEINDEXOF(i, j, k)))[0]*VectorScale*g_arrow2d_vertex_buffer_data[loopVarGlyphPts+0],
                                            ((probabilities[l])->at(COMPUTEINDEXOF(i, j, k)))[0]*VectorScale*g_arrow2d_vertex_buffer_data[loopVarGlyphPts+1],
                                            ((probabilities[l])->at(COMPUTEINDEXOF(i, j, k)))[0]*VectorScale*g_arrow2d_vertex_buffer_data[loopVarGlyphPts+2],
                                            1.0);
        					}
                            else
                            {
                                glyphPointTemp = glm::vec4(
                                                1.414*((probabilities[l])->at(COMPUTEINDEXOF(i, j, k)))[0]*VectorScale*g_arrow2d_vertex_buffer_data[loopVarGlyphPts+0],
                                                1.414*((probabilities[l])->at(COMPUTEINDEXOF(i, j, k)))[0]*VectorScale*g_arrow2d_vertex_buffer_data[loopVarGlyphPts+1],
                                                1.414*((probabilities[l])->at(COMPUTEINDEXOF(i, j, k)))[0]*VectorScale*g_arrow2d_vertex_buffer_data[loopVarGlyphPts+2],
                                                1.0);
                            }
                            // apply rotation & translation transforms
                            glyphPointTemp = M * glyphPointTemp;

                            //current probability
                            double probability = (probabilities[l])->at(COMPUTEINDEXOF(i, j, k))[0];
                            if (probability > maxProbability)
                                maxProbability = probability;
                            if (probability < minProbability)
                                minProbability = probability;

			                // store (x,y,z) components of current vertex*/
                            velocityMagnitudes.push_back(probability);
			                for (int loopVarComponents = 0; loopVarComponents < 3; loopVarComponents++)
			                {
                                this->vertex_buffer_data[globalCounter] = glyphPointTemp[loopVarComponents];
                                globalCounter++;
                            }
                        }
                    }
                }
            }
        }
    }



    /** Copy velocity data **/

    this->totalAttributes = 1; // TODO: don't hard-code this...
    this->vertex_attrib_data = new GLfloat*[this->totalAttributes];
    int num_of_vertices = 18 * this->totalVertices * ArrowGlyphSize;
    this->vertex_attrib_data[0] = new GLfloat[num_of_vertices]; // 1 velocity magnitude per *vertex*
    for (int i = 0; i < num_of_vertices; i++)
    {
        this->vertex_attrib_data[0][i] = velocityMagnitudes[i];
    }

    GLuint vao, vbo, velocity_buf;

    /** copy vertex data to GPU & save VAO and VBO handles **/
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(GLfloat) * this->totalVertices * 3 * ArrowGlyphSize, this->vertex_buffer_data, GL_STATIC_DRAW);

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

    glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(GLfloat) * this->totalVertices * ArrowGlyphSize, this->vertex_attrib_data[0], GL_STATIC_DRAW);

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
    this->minGradientValue = minProbability;
    this->maxGradientValue = maxProbability;
}

void ProbabilitiesRenderer::Draw(glm::mat4 MVP)
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
    glDrawArrays(GL_TRIANGLES, 0, 18 * 9 * this->totalVertices);

    // unset shaders
    shaderProgram->disable();

    // unbind VAO
    glBindVertexArray(0);
}


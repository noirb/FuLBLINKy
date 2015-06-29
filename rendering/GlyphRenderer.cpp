#include "GlyphRenderer.hpp"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
void GlyphRenderer::PrepareGeometry(DataProvider* provider)
{
    float velVectorScale = 0.1;
    static const int ArrowGlyphSize = 9;//sizeof(g_arrow2d_vertex_buffer_data)/sizeof(float);
    std::vector<std::vector<double> >* points;
    std::vector<std::vector<double> >* velocities;

    if ( provider->GetField("points", &points) != 0)
    {
        std::cout << "ERROR<GlyphRenderer::PrepareGeometry>: Points Field Could not be retrieved!" << std::endl;
        return;
    }
    if ( provider->GetField("velocity", &velocities) != 0)
    {
        std::cout << "ERROR<GlyphRenderer::PrepareGeometry>: Velocity Field could not be retrieved!" << std::endl;
    }

    std::cout << "GlyphRenderer::PrepareGeometry -- processing " << (*points).size() << " points and " << (*velocities).size() << " velocities" << std::endl;
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
    // GlyphRenderer only needs SEVEN vertex per data point
    this->totalVertices = (*points).size();
    this->vertex_buffer_data = new GLfloat[ArrowGlyphSize * 3 * this->totalVertices]; // 3 floats per triangle vertex (9 vertices per point)


    // we want to render points exactly at the locations specified by points, so just copy them
    int i = 0;
    //for (auto point : *points)
    for (int loopVarVertices = 0; loopVarVertices < this->totalVertices; loopVarVertices++)
    {
        float velTemp[3];
        for (int loopVarComponents = 0; loopVarComponents < 3; loopVarComponents++)
        {
            velTemp[loopVarComponents] = (velocities->at(loopVarVertices))[loopVarComponents];
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
            // get coords for current vertex
            glm::vec4 glyphPointTemp = glm::vec4(
                            velVectorScale*g_arrow2d_vertex_buffer_data[loopVarGlyphPts+0],
                            velVectorScale*g_arrow2d_vertex_buffer_data[loopVarGlyphPts+1],
                            velVectorScale*g_arrow2d_vertex_buffer_data[loopVarGlyphPts+2],
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
    for (auto velocity_vector : *velocities)
    {
        // compute velocity magnitude at this point
        glm::vec3 vel = glm::vec3(velocity_vector[0], velocity_vector[1], velocity_vector[2]);
        double mag = glm::length(vel);

        // for each vertex in the glyph at this piont, store the velocity magnitude
        for (int j = 0; j < ArrowGlyphSize; j++)
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

void GlyphRenderer::Draw(glm::mat4 MVP, GLuint MVP_ID)
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

    glBindVertexArray(this->VAO);

    // DRAW!
    glDrawArrays(GL_TRIANGLES, 0, (this->totalVertices)*9);

    // unset shaders
    glUseProgram(0);

    // unbind VAO
    glBindVertexArray(0);
}


#include "StreamLineRenderer.hpp"
#include "Compositor.hpp"
#include <iostream>

#define COMPUTEINDEXOF(x, y, z) ( (x) * (ylength) * (zlength) + (y) * (zlength) + (z) )
int streamLinePointCounter;

std::vector<double> StreamLineRenderer::trilinearVelocityInterpolator(double deltaX, 
                                                          double deltaY, 
							  double deltaZ,
							  double xlength,
							  double ylength,
							  double zlength,
							  std::vector<double> currPoint,
							  std::vector<std::vector<double> > localVelocities)
{
	    double x0 = ((int)(currPoint[0]/deltaX)) * deltaX;
	    double y0 = ((int)(currPoint[1]/deltaY)) * deltaY;
	    double z0 = ((int)(currPoint[2]/deltaZ)) * deltaZ;
	    double x1 = x0 + deltaX;
	    double y1 = y0 + deltaY;
	    double z1 = z0 + deltaZ;
	
	    // Determine local coordinates of THE point
	    double dx = currPoint[0] - x0;
	    double dy = currPoint[1] - y0;
	    double dz = currPoint[2] - z0;
	    
    	    // Get local velocities
	    // For POINT-0
	    {
	        std::vector<double> pointVelocity;	// Create temporary dump for local velocity
	        pointVelocity.push_back((velocities->at(COMPUTEINDEXOF(x0, y0, z0)))[0]);        // Find x-velocity
	        pointVelocity.push_back((velocities->at(COMPUTEINDEXOF(x0, y0, z0)))[1]);        // Find y-velocity
	        pointVelocity.push_back((velocities->at(COMPUTEINDEXOF(x0, y0, z0)))[2]);        // Find z-velocity
	        localVelocities.push_back(pointVelocity);	// Push velocity for this point into the localVelocities vector
	    }
	    // For POINT-1
	    {
	        std::vector<double> pointVelocity;	// Create temporary dump for local velocity
	        pointVelocity.push_back((velocities->at(COMPUTEINDEXOF(x1, y0, z0)))[0]);        // Find x-velocity
	        pointVelocity.push_back((velocities->at(COMPUTEINDEXOF(x1, y0, z0)))[1]);        // Find y-velocity
	        pointVelocity.push_back((velocities->at(COMPUTEINDEXOF(x1, y0, z0)))[2]);        // Find z-velocity
	        localVelocities.push_back(pointVelocity);	// Push velocity for this point into the localVelocities vector
	    }
	    // For POINT-2
	    {
	        std::vector<double> pointVelocity;	// Create temporary dump for local velocity
	        pointVelocity.push_back((velocities->at(COMPUTEINDEXOF(x1, y0, z1)))[0]);        // Find x-velocity
        	pointVelocity.push_back((velocities->at(COMPUTEINDEXOF(x1, y0, z1)))[1]);        // Find y-velocity
	        pointVelocity.push_back((velocities->at(COMPUTEINDEXOF(x1, y0, z1)))[2]);        // Find z-velocity
	        localVelocities.push_back(pointVelocity);	// Push velocity for this point into the localVelocities vector
	    }
	    // For POINT-3
	    {
	        std::vector<double> pointVelocity;	// Create temporary dump for local velocity
	        pointVelocity.push_back((velocities->at(COMPUTEINDEXOF(x0, y0, z1)))[0]);        // Find x-velocity
	        pointVelocity.push_back((velocities->at(COMPUTEINDEXOF(x0, y0, z1)))[1]);        // Find y-velocity
	        pointVelocity.push_back((velocities->at(COMPUTEINDEXOF(x0, y0, z1)))[2]);        // Find z-velocity
        	localVelocities.push_back(pointVelocity);	// Push velocity for this point into the localVelocities vector
	    }
    	    // For POINT-4
	    {
	        std::vector<double> pointVelocity;	// Create temporary dump for local velocity
	        pointVelocity.push_back((velocities->at(COMPUTEINDEXOF(x0, y1, z0)))[0]);        // Find x-velocity
	        pointVelocity.push_back((velocities->at(COMPUTEINDEXOF(x0, y1, z0)))[1]);        // Find y-velocity
	        pointVelocity.push_back((velocities->at(COMPUTEINDEXOF(x0, y1, z0)))[2]);        // Find z-velocity
	        localVelocities.push_back(pointVelocity);	// Push velocity for this point into the localVelocities vector
	    }
	    // For POINT-5
	    {
	        std::vector<double> pointVelocity;	// Create temporary dump for local velocity
	        pointVelocity.push_back((velocities->at(COMPUTEINDEXOF(x1, y1, z0)))[0]);        // Find x-velocity
	        pointVelocity.push_back((velocities->at(COMPUTEINDEXOF(x1, y1, z0)))[1]);        // Find y-velocity
	        pointVelocity.push_back((velocities->at(COMPUTEINDEXOF(x1, y1, z0)))[2]);        // Find z-velocity
	        localVelocities.push_back(pointVelocity);	// Push velocity for this point into the localVelocities vector
	    }
	    // For POINT-6
	    {
	        std::vector<double> pointVelocity;	// Create temporary dump for local velocity
	        pointVelocity.push_back((velocities->at(COMPUTEINDEXOF(x1, y1, z1)))[0]);        // Find x-velocity
	        pointVelocity.push_back((velocities->at(COMPUTEINDEXOF(x1, y1, z1)))[1]);        // Find y-velocity
	        pointVelocity.push_back((velocities->at(COMPUTEINDEXOF(x1, y1, z1)))[2]);        // Find z-velocity
	        localVelocities.push_back(pointVelocity);	// Push velocity for this point into the localVelocities vector
	    }
	    // For POINT-7
	    {
	        std::vector<double> pointVelocity;	// Create temporary dump for local velocity
	        pointVelocity.push_back((velocities->at(COMPUTEINDEXOF(x0, y1, z1)))[0]);        // Find x-velocity
	        pointVelocity.push_back((velocities->at(COMPUTEINDEXOF(x0, y1, z1)))[1]);        // Find y-velocity
	        pointVelocity.push_back((velocities->at(COMPUTEINDEXOF(x0, y1, z1)))[2]);        // Find z-velocity
	        localVelocities.push_back(pointVelocity);	// Push velocity for this point into the localVelocities vector
	    }

    std::vector<double> interpolatedVelocity;

    // Compute the interpolated velocity component-wise
    for (int i = 0; i < 3; i++){
	    double v_dx_00_00 = (deltaX - dx)*localVelocities[0][i]/deltaX + (dx)*localVelocities[1][i]/deltaX;
	    double v_dx_01_00 = (deltaX - dx)*localVelocities[4][i]/deltaX + (dx)*localVelocities[5][i]/deltaX;
	    double v_dx_00_01 = (deltaX - dx)*localVelocities[3][i]/deltaX + (dx)*localVelocities[2][i]/deltaX;
	    double v_dx_01_01 = (deltaX - dx)*localVelocities[7][i]/deltaX + (dx)*localVelocities[6][i]/deltaX;
	
	    double v_dx_00_dz = (deltaZ - dz)*v_dx_00_00/deltaZ + (dz)*v_dx_00_01/deltaZ;
	    double v_dx_01_dz = (deltaZ - dz)*v_dx_01_00/deltaZ + (dz)*v_dx_01_01/deltaZ;
	
	    double v_dx_dy_dz = (deltaY - dy)*v_dx_00_dz/deltaY + (dy)*v_dx_01_dz/deltaY;
	    interpolatedVelocity.push_back(v_dx_dy_dz);
    }
    
    return interpolatedVelocity;
}

void StreamLineRenderer::RK45(double deltaX, 
                                                          double deltaY, 
							  double deltaZ, 
							  double xlength,
							  double ylength,
							  double zlength,
							  double dt,
							  std::vector<double> &currPoint)
{
        std::vector<std::vector<double> > localVelocities;
	std::vector<double> k1 = trilinearVelocityInterpolator(deltaX, deltaY, deltaZ, xlength, ylength, zlength, currPoint, localVelocities);
	std::vector<double> k2;
	std::vector<double> k3;
	std::vector<double> k4;
	std::vector<double> tempCoordinates; 

	tempCoordinates.push_back(currPoint[0] + dt/2.0*k1[0]);
	tempCoordinates.push_back(currPoint[1] + dt/2.0*k1[1]);
	tempCoordinates.push_back(currPoint[2] + dt/2.0*k1[2]);	
	k2 = trilinearVelocityInterpolator(deltaX, deltaY, deltaZ, xlength, ylength, zlength, tempCoordinates, localVelocities);

	tempCoordinates.clear();
	tempCoordinates.push_back(currPoint[0] + dt/2.0*k2[0]);
	tempCoordinates.push_back(currPoint[1] + dt/2.0*k2[1]);
	tempCoordinates.push_back(currPoint[2] + dt/2.0*k2[2]);	
	k3 = trilinearVelocityInterpolator(deltaX, deltaY, deltaZ, xlength, ylength, zlength, tempCoordinates, localVelocities);

	tempCoordinates.clear();
	tempCoordinates.push_back(currPoint[0] + dt*k3[0]);
	tempCoordinates.push_back(currPoint[1] + dt*k3[1]);
	tempCoordinates.push_back(currPoint[2] + dt*k3[2]);	
	k4 = trilinearVelocityInterpolator(deltaX, deltaY, deltaZ, xlength, ylength, zlength, tempCoordinates, localVelocities);

	currPoint[0] += (dt/6.0) * (k1[0] + 2.0*(k2[0] + k3[0]) + k4[0]);
	currPoint[1] += (dt/6.0) * (k1[1] + 2.0*(k2[1] + k3[1]) + k4[1]);
	currPoint[2] += (dt/6.0) * (k1[2] + 2.0*(k2[2] + k3[2]) + k4[2]);
}

void StreamLineRenderer::PrepareGeometry(DataProvider* provider)
{
    if ( provider->GetField("points", &points) != 0)
    {
        std::cout << "ERROR<StreamLineRenderer::PrepareGeometry>: Points Field Could not be retrieved!" << std::endl;
        return;
    }
    if ( provider->GetField("velocity", &velocities) != 0)
    {
        std::cout << "ERROR<StreamLineRenderer::PrepareGeometry>: Velocity Field could not be retrieved!" << std::endl;
        return;
    }
    if ( provider->GetField("dimensions", &domainSize) != 0)
    {
        std::cout << "ERROR<StreamLineRenderer::PrepareGeometry>: Dimensions Field could not be retrieved!" << std::endl;
        return;
    }

    std::cout << "StreamLineRenderer::PrepareGeometry -- processing " << (*points).size() << " points and " << (*velocities).size() << " velocities" << std::endl;
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
    // StreamLineRenderer only needs ONE vertex per data point
    this->totalVertices = (*points).size();

    // for now, create local deltaX/Y/Z, streamlineSource, maxStreamLineLength, dt
    double dt = 100;
    double maxStreamLineLength = 200.0;
    double deltaX = 1.0;
    double deltaY = 1.0;
    double deltaZ = 1.0;
    double streamLineSource[3] = {9.0, 9.0, 9.0};
    
    // Get domain size
    double xlength = (domainSize->at(0))[0];
    double ylength = (domainSize->at(0))[1];
    double zlength = (domainSize->at(0))[2];

    // Create a holder vector for all points on the streamline. We read from this vector and write to the vertex_buffer_array AFTER the while loop 
    std::vector<double> streamLinePoints;
    // First add the source point
    streamLinePoints.push_back(streamLineSource[0]);
    streamLinePoints.push_back(streamLineSource[1]);
    streamLinePoints.push_back(streamLineSource[2]);
    // Create a counter for how far we are on the streamLine
    streamLinePointCounter = 3;
    // Creater a length tape
    double streamLineLength = 0.0;

// While loop starts here..
    while( streamLinePoints[streamLinePointCounter - 3] < xlength-1 && streamLinePoints[streamLinePointCounter - 3] > 1 &&
	   streamLinePoints[streamLinePointCounter - 2] < ylength-1 && streamLinePoints[streamLinePointCounter - 2] > 1 &&
	   streamLinePoints[streamLinePointCounter - 1] < zlength-1 && streamLinePoints[streamLinePointCounter - 1] > 1 &&

	   streamLineLength < maxStreamLineLength &&

           streamLinePointCounter < 5000000)
    {
	   // Create a copy of the current point
	   std::vector<double> currPoint;
           currPoint.push_back(streamLinePoints[streamLinePointCounter - 3]);
           currPoint.push_back(streamLinePoints[streamLinePointCounter - 2]);
           currPoint.push_back(streamLinePoints[streamLinePointCounter - 1]);
	   // Call func! 		   
	   RK45(deltaX, deltaY, deltaZ, xlength, ylength, zlength, dt, currPoint);
	   streamLinePoints.push_back(currPoint[0]);
	   streamLinePoints.push_back(currPoint[1]);
	   streamLinePoints.push_back(currPoint[2]);
           streamLinePointCounter += 3;

           streamLineLength += sqrt((streamLinePoints[streamLinePointCounter - 3] - streamLinePoints[streamLinePointCounter - 6]) * (streamLinePoints[streamLinePointCounter - 3] - streamLinePoints[streamLinePointCounter - 6]) +
				     (streamLinePoints[streamLinePointCounter - 2] - streamLinePoints[streamLinePointCounter - 5]) * (streamLinePoints[streamLinePointCounter - 2] - streamLinePoints[streamLinePointCounter - 5]) +
                                     (streamLinePoints[streamLinePointCounter - 1] - streamLinePoints[streamLinePointCounter - 4]) * (streamLinePoints[streamLinePointCounter - 1] - streamLinePoints[streamLinePointCounter - 4]));
    }
    
    // we want to render points exactly at the locations specified by points, so just copy them
    int temp = streamLinePointCounter-3;
    this->vertex_buffer_data = new GLfloat[temp];

    for(int i = 0; i < streamLinePointCounter - 3; i++)
    {
            this->vertex_buffer_data[i] = streamLinePoints[i]; // copy each x,y,z component from each point
    }

    /** Copy velocity data **/

    this->totalAttributes = 1; // TODO: don't hard-code this...
    this->vertex_attrib_data = new GLfloat*[this->totalAttributes];
    int num_of_vertices = (streamLinePointCounter-3)/3;
    this->vertex_attrib_data[0] = new GLfloat[num_of_vertices]; // 1 velocity magnitude per *vertex*
    int i = 0;
    for (i = 0; i < num_of_vertices; i++)
    {
        this->vertex_attrib_data[0][i] = provider->GetMaxValueFromField("velocity");
    }
   /* for (auto velocity_vector : *velocities)
    {
        // compute velocity magnitude at this point
        glm::vec3 vel = glm::vec3(velocity_vector[0], velocity_vector[1], velocity_vector[2]);
        double mag = glm::length(vel);

        // for each vertex in the glyph at this piont, store the velocity magnitude
        this->vertex_attrib_data[0][i] = mag;
        i++;
    }*/

    GLuint vao, vbo, velocity_buf; vao = vbo = velocity_buf = 0;

    /** copy vertex data to GPU & save VAO and VBO handles **/
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * (streamLinePointCounter-3), this->vertex_buffer_data, GL_STATIC_DRAW);

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

    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * (streamLinePointCounter-3)/3, this->vertex_attrib_data[0], GL_STATIC_DRAW);

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

    // save min/max values for rendering colored gradients/scaling/etc
    this->maxGradientValue = provider->GetMaxValueFromField("velocity");
    this->minGradientValue = provider->GetMinValueFromField("velocity");
    std::cout << "StreamLineRenderer: Max Velocity: " << this->maxGradientValue << ", Min: " << this->minGradientValue << std::endl;
}

void StreamLineRenderer::Draw(glm::mat4 MVP)
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
    glDrawArrays(GL_LINE_STRIP, 0, (streamLinePointCounter-3)/3);

    // unset shaders
    glUseProgram(0);

    // unbind VAO
    glBindVertexArray(0);
}


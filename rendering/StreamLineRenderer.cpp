#include "StreamLineRenderer.hpp"
#include "Compositor.hpp"
#include <iostream>

// Indexing Macro for velocities
#define INDEXOF(x, y, z) ( (x) * (ylength) * (zlength) + (y) * (zlength) + (z) )

// Create a counter for how far we are on the streamLine
std::vector<int> streamLinePointCounter;
std::vector<double> velocity_magnitudes;
const int lineSourceSize = 15;

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
	        pointVelocity.push_back((velocities->at(INDEXOF(x0, y0, z0)))[0]);        // Find x-velocity
	        pointVelocity.push_back((velocities->at(INDEXOF(x0, y0, z0)))[1]);        // Find y-velocity
	        pointVelocity.push_back((velocities->at(INDEXOF(x0, y0, z0)))[2]);        // Find z-velocity
	        localVelocities.push_back(pointVelocity);	// Push velocity for this point into the localVelocities vector
	    }
	    // For POINT-1
	    {
	        std::vector<double> pointVelocity;	// Create temporary dump for local velocity
	        pointVelocity.push_back((velocities->at(INDEXOF(x1, y0, z0)))[0]);        // Find x-velocity
	        pointVelocity.push_back((velocities->at(INDEXOF(x1, y0, z0)))[1]);        // Find y-velocity
	        pointVelocity.push_back((velocities->at(INDEXOF(x1, y0, z0)))[2]);        // Find z-velocity
	        localVelocities.push_back(pointVelocity);	// Push velocity for this point into the localVelocities vector
	    }
	    // For POINT-2
	    {
	        std::vector<double> pointVelocity;	// Create temporary dump for local velocity
	        pointVelocity.push_back((velocities->at(INDEXOF(x1, y0, z1)))[0]);        // Find x-velocity
        	pointVelocity.push_back((velocities->at(INDEXOF(x1, y0, z1)))[1]);        // Find y-velocity
	        pointVelocity.push_back((velocities->at(INDEXOF(x1, y0, z1)))[2]);        // Find z-velocity
	        localVelocities.push_back(pointVelocity);	// Push velocity for this point into the localVelocities vector
	    }
	    // For POINT-3
	    {
	        std::vector<double> pointVelocity;	// Create temporary dump for local velocity
	        pointVelocity.push_back((velocities->at(INDEXOF(x0, y0, z1)))[0]);        // Find x-velocity
	        pointVelocity.push_back((velocities->at(INDEXOF(x0, y0, z1)))[1]);        // Find y-velocity
	        pointVelocity.push_back((velocities->at(INDEXOF(x0, y0, z1)))[2]);        // Find z-velocity
        	localVelocities.push_back(pointVelocity);	// Push velocity for this point into the localVelocities vector
	    }
    	    // For POINT-4
	    {
	        std::vector<double> pointVelocity;	// Create temporary dump for local velocity
	        pointVelocity.push_back((velocities->at(INDEXOF(x0, y1, z0)))[0]);        // Find x-velocity
	        pointVelocity.push_back((velocities->at(INDEXOF(x0, y1, z0)))[1]);        // Find y-velocity
	        pointVelocity.push_back((velocities->at(INDEXOF(x0, y1, z0)))[2]);        // Find z-velocity
	        localVelocities.push_back(pointVelocity);	// Push velocity for this point into the localVelocities vector
	    }
	    // For POINT-5
	    {
	        std::vector<double> pointVelocity;	// Create temporary dump for local velocity
	        pointVelocity.push_back((velocities->at(INDEXOF(x1, y1, z0)))[0]);        // Find x-velocity
	        pointVelocity.push_back((velocities->at(INDEXOF(x1, y1, z0)))[1]);        // Find y-velocity
	        pointVelocity.push_back((velocities->at(INDEXOF(x1, y1, z0)))[2]);        // Find z-velocity
	        localVelocities.push_back(pointVelocity);	// Push velocity for this point into the localVelocities vector
	    }
	    // For POINT-6
	    {
	        std::vector<double> pointVelocity;	// Create temporary dump for local velocity
	        pointVelocity.push_back((velocities->at(INDEXOF(x1, y1, z1)))[0]);        // Find x-velocity
	        pointVelocity.push_back((velocities->at(INDEXOF(x1, y1, z1)))[1]);        // Find y-velocity
	        pointVelocity.push_back((velocities->at(INDEXOF(x1, y1, z1)))[2]);        // Find z-velocity
	        localVelocities.push_back(pointVelocity);	// Push velocity for this point into the localVelocities vector
	    }
	    // For POINT-7
	    {
	        std::vector<double> pointVelocity;	// Create temporary dump for local velocity
	        pointVelocity.push_back((velocities->at(INDEXOF(x0, y1, z1)))[0]);        // Find x-velocity
	        pointVelocity.push_back((velocities->at(INDEXOF(x0, y1, z1)))[1]);        // Find y-velocity
	        pointVelocity.push_back((velocities->at(INDEXOF(x0, y1, z1)))[2]);        // Find z-velocity
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
	velocity_magnitudes.push_back(sqrt(k1[0]*k1[0] + k1[1]*k1[1] + k1[2]*k1[2]));
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
    }
    if ( provider->GetField("dimensions", &domainSize) != 0)
    {
        std::cout << "ERROR<StreamLineRenderer::PrepareGeometry>: Dimensions Field could not be retrieved!" << std::endl;
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
    // create the end-points of the lines
    std::vector<double> lineSourcePoint1;
	lineSourcePoint1.push_back(3.0);
	lineSourcePoint1.push_back(3.0);
	lineSourcePoint1.push_back(3.0);
    std::vector<double> lineSourcePoint2;
	lineSourcePoint2.push_back(10.0);
	lineSourcePoint2.push_back(10.0);
	lineSourcePoint2.push_back(10.5);
    std::vector<double> steps;
	steps.push_back( (lineSourcePoint2[0] - lineSourcePoint1[0])/(lineSourceSize-1) );
	steps.push_back( (lineSourcePoint2[1] - lineSourcePoint1[1])/(lineSourceSize-1) );
	steps.push_back( (lineSourcePoint2[2] - lineSourcePoint1[2])/(lineSourceSize-1) );
    
    // create a global counter
    int k;

    double dt = 100;
    double maxStreamLineLength = 200.0;
    double deltaX = 1.0;
    double deltaY = 1.0;
    double deltaZ = 1.0;
    std::vector<std::vector<double> > streamLineSource;
    // Add points to the stream line source (equidistant distribution)    
    for (int i = 0; i < lineSourceSize; i++)
    {
	std::vector<double> temp;
	temp.push_back( lineSourcePoint1[0] + i*steps[0] );
	temp.push_back( lineSourcePoint1[1] + i*steps[1] );
	temp.push_back( lineSourcePoint1[2] + i*steps[2] );
	streamLineSource.push_back(temp);
    }

    // Get domain size
    double xlength = (domainSize->at(0))[0];
    double ylength = (domainSize->at(0))[1];
    double zlength = (domainSize->at(0))[2];

    // Create a holder vector for all points on the streamline. We read from this vector and write to the vertex_buffer_array AFTER the while loop 
    std::vector<double> streamLinePoints;

    // Creater a length tape
    std::vector<double> streamLineLength;

    // Loop through all points
    for (int i = 0; i < lineSourceSize; i++)
    {
	// Initialize ith counter and length to zero
	streamLinePointCounter.push_back(3);
	k += 3;
        streamLineLength.push_back(0.0);
	
	// First add the ith source point
	streamLinePoints.push_back(streamLineSource[i][0]);
	streamLinePoints.push_back(streamLineSource[i][1]);
	streamLinePoints.push_back(streamLineSource[i][2]);

	// ith while loop starts here..
	while( streamLinePoints[k - 3] < xlength-1 && streamLinePoints[k - 3] > 1 &&
	       streamLinePoints[k - 2] < ylength-1 && streamLinePoints[k - 2] > 1 &&
	       streamLinePoints[k - 1] < zlength-1 && streamLinePoints[k - 1] > 1 &&
	       streamLineLength[i] < maxStreamLineLength &&

	       streamLinePointCounter[i] < 5000)
	{
		// Create a copy of the current point
		std::vector<double> currPoint;
	        currPoint.push_back(streamLinePoints[k - 3]);
	        currPoint.push_back(streamLinePoints[k - 2]);
	        currPoint.push_back(streamLinePoints[k - 1]);
		// Call func! 		   
		RK45(deltaX, deltaY, deltaZ, xlength, ylength, zlength, dt, currPoint);
		streamLinePoints.push_back(currPoint[0]);
		streamLinePoints.push_back(currPoint[1]);
		streamLinePoints.push_back(currPoint[2]);
		streamLinePointCounter[i] += 3;
		k += 3;

		streamLineLength[i] += sqrt((streamLinePoints[k - 3] - streamLinePoints[k - 6]) * (streamLinePoints[k - 3] - streamLinePoints[k - 6]) +
					    (streamLinePoints[k - 2] - streamLinePoints[k - 5]) * (streamLinePoints[k - 2] - streamLinePoints[k - 5]) +
                                     	    (streamLinePoints[k - 1] - streamLinePoints[k - 4]) * (streamLinePoints[k - 1] - streamLinePoints[k - 4]));
    	}
    }

    // we want to render points exactly at the locations specified by points, so just copy them
    int totalStoredCoords = k;// k-3*lineSourceSize;
    this->vertex_buffer_data = new GLfloat[totalStoredCoords];

    for(int i = 0; i < totalStoredCoords; i++)
    {
            this->vertex_buffer_data[i] = streamLinePoints[i]; // copy each x,y,z component from each point
    }

    /** Copy velocity data **/		

    this->totalAttributes = 1; // TODO: don't hard-code this...
    this->vertex_attrib_data = new GLfloat*[this->totalAttributes];
    int num_of_vertices = totalStoredCoords/3;
    this->vertex_attrib_data[0] = new GLfloat[num_of_vertices]; // 1 velocity magnitude per *vertex*
    int i = 0;
    for (i = 0; i < num_of_vertices; i++)
    {
        this->vertex_attrib_data[0][i] = velocity_magnitudes[i];//provider->GetMaxValueFromField("velocity");
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

    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * totalStoredCoords, this->vertex_buffer_data, GL_STATIC_DRAW);

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

    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * totalStoredCoords/3, this->vertex_attrib_data[0], GL_STATIC_DRAW);

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
    int startIndex = 0;
    glDrawArrays(GL_LINE_STRIP, startIndex, (streamLinePointCounter[0]-3)/3);
    for (int i = 1; i < lineSourceSize; i++)
    {
	startIndex += streamLinePointCounter[i-1]/3 ;
	glDrawArrays(GL_LINE_STRIP, startIndex, (streamLinePointCounter[i]-3)/3);
    }

    // unset shaders
    glUseProgram(0);

    // unbind VAO
    glBindVertexArray(0);
}


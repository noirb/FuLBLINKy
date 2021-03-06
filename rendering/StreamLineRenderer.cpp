#include "StreamLineRenderer.hpp"
#include "Compositor.hpp"
#include <iostream>

#define COMPUTEINDEXOF(x, y, z) (int)( (x) * (ylength) * (zlength) + (y) * (zlength) + (z) )

// Create a counter for how far we are on the streamLine
std::vector<int> streamLinePointCounter;
std::vector<double> velocity_magnitudes;

const int MAX_STREAMLINE_ITERATIONS = 512; // maximum number of loop iterations while trying to follow a streamline

double* StreamLineRenderer::GetStartPoint()
{
    return this->startPoint;
}

void StreamLineRenderer::SetStartPoint(double newx, double newy, double newz)
{
    this->startPoint[0] = newx < this->endPoint[0] ? newx : this->endPoint[0];
    this->startPoint[1] = newy < this->endPoint[1] ? newy : this->endPoint[1];
    this->startPoint[2] = newz < this->endPoint[2] ? newz : this->endPoint[2];
}

double* StreamLineRenderer::GetEndPoint()
{
    return this->endPoint;
}

void StreamLineRenderer::SetEndPoint(double newx, double newy, double newz)
{
    this->endPoint[0] = newx > this->startPoint[0] ? newx : this->startPoint[0];
    this->endPoint[1] = newy > this->startPoint[1] ? newy : this->startPoint[1];
    this->endPoint[2] = newz > this->startPoint[2] ? newz : this->startPoint[2];
}

int StreamLineRenderer::GetLineSize()
{
    return this->lineSourceSize;
}

void StreamLineRenderer::SetLineSize(int newSize)
{
    this->lineSourceSize = newSize > 0 ? newSize : 0; // disallow negative sizes
}

double StreamLineRenderer::GetLineLength()
{
    return this->maxStreamlineLength;
}

void StreamLineRenderer::SetLineLength(double newLen)
{
    this->maxStreamlineLength = newLen > 0.0 ? newLen : 0.0; // disallow negative lengths
}

std::vector<double> StreamLineRenderer::trilinearVelocityInterpolator(
                              double deltaX, 
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
    double x1 = glm::min(x0 + deltaX, xlength-1);
    double y1 = glm::min(y0 + deltaY, ylength-1);
    double z1 = glm::min(z0 + deltaZ, zlength-1);

    // Determine local coordinates of THE point
    double dx = currPoint[0] - x0;
    double dy = currPoint[1] - y0;
    double dz = currPoint[2] - z0;

    // Get local velocities
    // For POINT-0
    {
        std::vector<double> pointVelocity; // Create temporary dump for local velocity
        pointVelocity.push_back((_velocities->at(COMPUTEINDEXOF(x0, y0, z0)))[0]);        // Find x-velocity
        pointVelocity.push_back((_velocities->at(COMPUTEINDEXOF(x0, y0, z0)))[1]);        // Find y-velocity
        pointVelocity.push_back((_velocities->at(COMPUTEINDEXOF(x0, y0, z0)))[2]);        // Find z-velocity
        localVelocities.push_back(pointVelocity); // Push velocity for this point into the localVelocities vector
    }
    // For POINT-1
    {
        std::vector<double> pointVelocity; // Create temporary dump for local velocity
        pointVelocity.push_back((_velocities->at(COMPUTEINDEXOF(x1, y0, z0)))[0]);        // Find x-velocity
        pointVelocity.push_back((_velocities->at(COMPUTEINDEXOF(x1, y0, z0)))[1]);        // Find y-velocity
        pointVelocity.push_back((_velocities->at(COMPUTEINDEXOF(x1, y0, z0)))[2]);        // Find z-velocity
        localVelocities.push_back(pointVelocity); // Push velocity for this point into the localVelocities vector
    }
    // For POINT-2
    {
        std::vector<double> pointVelocity; // Create temporary dump for local velocity
        pointVelocity.push_back((_velocities->at(COMPUTEINDEXOF(x1, y0, z1)))[0]);        // Find x-velocity
        pointVelocity.push_back((_velocities->at(COMPUTEINDEXOF(x1, y0, z1)))[1]);        // Find y-velocity
        pointVelocity.push_back((_velocities->at(COMPUTEINDEXOF(x1, y0, z1)))[2]);        // Find z-velocity
        localVelocities.push_back(pointVelocity); // Push velocity for this point into the localVelocities vector
    }
    // For POINT-3
    {
        std::vector<double> pointVelocity; // Create temporary dump for local velocity
        pointVelocity.push_back((_velocities->at(COMPUTEINDEXOF(x0, y0, z1)))[0]);        // Find x-velocity
        pointVelocity.push_back((_velocities->at(COMPUTEINDEXOF(x0, y0, z1)))[1]);        // Find y-velocity
        pointVelocity.push_back((_velocities->at(COMPUTEINDEXOF(x0, y0, z1)))[2]);        // Find z-velocity
        localVelocities.push_back(pointVelocity); // Push velocity for this point into the localVelocities vector
    }
    // For POINT-4
    {
        std::vector<double> pointVelocity; // Create temporary dump for local velocity
        pointVelocity.push_back((_velocities->at(COMPUTEINDEXOF(x0, y1, z0)))[0]);        // Find x-velocity
        pointVelocity.push_back((_velocities->at(COMPUTEINDEXOF(x0, y1, z0)))[1]);        // Find y-velocity
        pointVelocity.push_back((_velocities->at(COMPUTEINDEXOF(x0, y1, z0)))[2]);        // Find z-velocity
        localVelocities.push_back(pointVelocity); // Push velocity for this point into the localVelocities vector
    }
    // For POINT-5
    {
        std::vector<double> pointVelocity; // Create temporary dump for local velocity
        pointVelocity.push_back((_velocities->at(COMPUTEINDEXOF(x1, y1, z0)))[0]);        // Find x-velocity
        pointVelocity.push_back((_velocities->at(COMPUTEINDEXOF(x1, y1, z0)))[1]);        // Find y-velocity
        pointVelocity.push_back((_velocities->at(COMPUTEINDEXOF(x1, y1, z0)))[2]);        // Find z-velocity
        localVelocities.push_back(pointVelocity); // Push velocity for this point into the localVelocities vector
    }
    // For POINT-6
    {
        std::vector<double> pointVelocity; // Create temporary dump for local velocity
        pointVelocity.push_back((_velocities->at(COMPUTEINDEXOF(x1, y1, z1)))[0]);        // Find x-velocity
        pointVelocity.push_back((_velocities->at(COMPUTEINDEXOF(x1, y1, z1)))[1]);        // Find y-velocity
        pointVelocity.push_back((_velocities->at(COMPUTEINDEXOF(x1, y1, z1)))[2]);        // Find z-velocity
        localVelocities.push_back(pointVelocity); // Push velocity for this point into the localVelocities vector
    }
    // For POINT-7
    {
        std::vector<double> pointVelocity; // Create temporary dump for local velocity
        pointVelocity.push_back((_velocities->at(COMPUTEINDEXOF(x0, y1, z1)))[0]);        // Find x-velocity
        pointVelocity.push_back((_velocities->at(COMPUTEINDEXOF(x0, y1, z1)))[1]);        // Find y-velocity
        pointVelocity.push_back((_velocities->at(COMPUTEINDEXOF(x0, y1, z1)))[2]);        // Find z-velocity
        localVelocities.push_back(pointVelocity); // Push velocity for this point into the localVelocities vector
    }

    std::vector<double> interpolatedVelocity;

    // Compute the interpolated velocity component-wise
    for (int i = 0; i < 3; i++)
    {
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

//                                                boxBounds is assumed to contain: [-X, +X, -Y, +Y, -Z, +Z]
int StreamLineRenderer::lineBoxIntersect(double * boxBounds, double * lineStart, double * lineEnd, double * intersections)
{
    int nIntersections = 0;
    double tmin = -INFINITY, tmax = INFINITY;
    glm::vec3 start = glm::vec3(lineStart[0], lineStart[1], lineStart[2]);
    glm::vec3 end = glm::vec3(lineEnd[0], lineEnd[1], lineEnd[2]);
    glm::vec3 dir = glm::normalize(start + end);

    // for each dimension...
    for (int i = 0; i < 3; ++i)
    {
        if (dir[i] != 0.0) {
            double t1 = (boxBounds[i*2]     - start[i]) / dir[i];
            double t2 = (boxBounds[i*2 + 1] - start[i]) / dir[i];

            tmin = glm::max(tmin, glm::min(t1, t2));
            tmax = glm::min(tmax, glm::max(t1, t2));
        }
        else if (start[i] <= boxBounds[i*2] || start[i] >= boxBounds[i*2 + 1])
        {
            return 0;
        }
    }

    if (tmax > tmin && tmax > 0.0)
    {
        if (tmin > 0.0)
        {
            glm::vec3 intersection1 = start + dir * (float)tmin;
            intersections[0] = intersection1[0];
            intersections[1] = intersection1[1];
            intersections[2] = intersection1[2];
            nIntersections++;
        }
        glm::vec3 intersection2 = start + dir * (float)tmax;
        intersections[nIntersections * 3 + 0] = intersection2[0];
        intersections[nIntersections * 3 + 1] = intersection2[1];
        intersections[nIntersections * 3 + 2] = intersection2[2];
        nIntersections++;
    }

    return nIntersections;
}

bool StreamLineRenderer::isInBox(double * boxBounds, double * point)
{
    // xmin, xmax
    if (point[0] < boxBounds[0]) return false;
    if (point[0] > boxBounds[1]) return false;

    // ymin, ymax
    if (point[1] < boxBounds[2]) return false;
    if (point[1] > boxBounds[3]) return false;

    // zmin, zmax
    if (point[2] < boxBounds[4]) return false;
    if (point[2] > boxBounds[5]) return false;

    return true;
}

void StreamLineRenderer::PrepareGeometry(DataProvider* provider)
{
    if (!provider) { return; } // do not attempt to generate geometry without a provider!

    std::vector<std::vector<double> >* color_scalarField;
    std::vector<double> scalar_mags;


    if ( provider->GetField("points", &_points) != 0)
    {
        std::cout << "ERROR<StreamLineRenderer::PrepareGeometry>: Points Field Could not be retrieved!" << std::endl;
        return;
    }
    if ( provider->GetField("velocity", &_velocities) != 0)
    {
        std::cout << "ERROR<StreamLineRenderer::PrepareGeometry>: Velocity Field could not be retrieved!" << std::endl;
        return;
    }
    if ( provider->GetField(_colorParamField, &color_scalarField) != 0)
    {
        std::cout << "ERROR<StreamLineRenderer::PrepareGeometry>: " << _colorParamField << " Field could not be retrieved!" << std::endl;
        return;
    }

    // get domain parameters (domain size, etc.)
    DomainParameters domainParameters;
    provider->getDomainParameters(&domainParameters);

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

    streamLinePointCounter.clear();
    velocity_magnitudes.clear();
    
    // determine needed number of vertices & allocate space for them
    _totalVertices = (*_points).size();

    // for now, create local deltaX/Y/Z, streamlineSource, maxStreamLineLength, dt
    // create the end-points of the lines
    std::vector<double> lineSourcePoint1;
    std::vector<double> lineSourcePoint2;

    // our solver will fail outside the range 2..length-1, so pick endpoints which are inside this range
    double* domain = provider->GetExtents();
    double intersectionPoints[6] = { 0, 0, 0, 0, 0, 0 };
    double lineSourceDomain[6];
    lineSourceDomain[0] = domain[0] + 2;    lineSourceDomain[1] = domain[1] - 1; // xmin, xmax
    lineSourceDomain[2] = domain[2] + 2;    lineSourceDomain[3] = domain[3] - 1; // ymin, ymax
    lineSourceDomain[4] = domain[4] + 2;    lineSourceDomain[5] = domain[5] - 1; // zmin, zmax

    int intersections = lineBoxIntersect(lineSourceDomain, startPoint, endPoint, intersectionPoints);
    if (intersections == 1) // one of start or end is outside the box
    {
        if (isInBox(lineSourceDomain, startPoint)) // linesource should be startPoint-->intersection
        {
            lineSourcePoint1.push_back(startPoint[0]);
            lineSourcePoint1.push_back(startPoint[1]);
            lineSourcePoint1.push_back(startPoint[2]);

            lineSourcePoint2.push_back(intersectionPoints[0]);
            lineSourcePoint2.push_back(intersectionPoints[1]);
            lineSourcePoint2.push_back(intersectionPoints[2]);
        }
        else // linesource should be intersection-->endPoint
        {
            lineSourcePoint1.push_back(intersectionPoints[0]);
            lineSourcePoint1.push_back(intersectionPoints[1]);
            lineSourcePoint1.push_back(intersectionPoints[2]);

            lineSourcePoint2.push_back(endPoint[0]);
            lineSourcePoint2.push_back(endPoint[1]);
            lineSourcePoint2.push_back(endPoint[2]);
        }
    }
    else if (intersections == 2) // both start and end are outside the box
    {
        lineSourcePoint1.push_back(intersectionPoints[0]);
        lineSourcePoint1.push_back(intersectionPoints[1]);
        lineSourcePoint1.push_back(intersectionPoints[2]);

        lineSourcePoint2.push_back(intersectionPoints[3]);
        lineSourcePoint2.push_back(intersectionPoints[4]);
        lineSourcePoint2.push_back(intersectionPoints[5]);
    }
    else // both start and end are either inside the box or outside the box
    {
        lineSourcePoint1.push_back(startPoint[0]);
        lineSourcePoint1.push_back(startPoint[1]);
        lineSourcePoint1.push_back(startPoint[2]);

        lineSourcePoint2.push_back(endPoint[0]);
        lineSourcePoint2.push_back(endPoint[1]);
        lineSourcePoint2.push_back(endPoint[2]);
    }

    std::vector<double> steps;
    steps.push_back( (lineSourcePoint2[0] - lineSourcePoint1[0])/(lineSourceSize-1) );
    steps.push_back( (lineSourcePoint2[1] - lineSourcePoint1[1])/(lineSourceSize-1) );
    steps.push_back( (lineSourcePoint2[2] - lineSourcePoint1[2])/(lineSourceSize-1) );
    
    // create a global counter
    int k = 0;

    double dt = 100;
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
    double xlength = domainParameters.size[0];
    double ylength = domainParameters.size[1];
    double zlength = domainParameters.size[2];

    // Create a holder vector for all points on the streamline. We read from this vector and write to the vertex_buffer_array AFTER the while loop 
    std::vector<double> streamLinePoints;

    // Create a length tape
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

        // Create a copy of the current point
        std::vector<double> currPoint;
        int iter_count = 0;
        // ith while loop starts here..
        while( streamLinePoints[k - 3] < xlength-1 && streamLinePoints[k - 3] > 1 &&
               streamLinePoints[k - 2] < ylength-1 && streamLinePoints[k - 2] > 1 &&
               streamLinePoints[k - 1] < zlength-1 && streamLinePoints[k - 1] > 1 &&
               streamLineLength[i] < maxStreamlineLength &&
               iter_count < MAX_STREAMLINE_ITERATIONS
            )
        {
            iter_count++;

            currPoint.clear();
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

            // get scalar value nearest to currPoint
            scalar_mags.push_back(color_scalarField->at(COMPUTEINDEXOF(glm::floor(currPoint[0]), glm::floor(currPoint[1]), glm::floor(currPoint[2])))[0]); // HACK: just take first component--great if it's just a scalar but we're losing data from vectors :(
        }

        if (currPoint.size() > 0) // if we generated points in the loop above...
        {
            std::vector<std::vector<double> > localVelocities;
            std::vector<double> k1 = trilinearVelocityInterpolator(deltaX, deltaY, deltaZ, xlength, ylength, zlength, currPoint, localVelocities);
            velocity_magnitudes.push_back(sqrt(k1[0]*k1[0] + k1[1]*k1[1] + k1[2]*k1[2]));
            scalar_mags.push_back(color_scalarField->at(COMPUTEINDEXOF(glm::floor(currPoint[0]), glm::floor(currPoint[1]), glm::floor(currPoint[2])))[0]); // HACK: Same as above...
        }
        else
        {
            k -= 3;
        }
    }

    // we want to render points exactly at the locations specified by points, so just copy them
    int totalStoredCoords = k;
    _vertex_buffer_data = new GLfloat[totalStoredCoords];

    for(int i = 0; i < totalStoredCoords; i++)
    {
        _vertex_buffer_data[i] = (GLfloat)streamLinePoints[i]; // copy each x,y,z component from each point
    }

    /** Copy velocity data **/

    _totalAttributes = 1; // TODO: don't hard-code this...
    _vertex_attrib_data = new GLfloat*[_totalAttributes];
    int num_of_vertices = totalStoredCoords/3;
    _vertex_attrib_data[0] = new GLfloat[num_of_vertices]; // 1 velocity magnitude per *vertex*
    int i = 0;
    for (i = 0; i < num_of_vertices; i++)
    {
        if (_colorParamField == "velocity") // HACK: We should not need to perform this test
            _vertex_attrib_data[0][i] = (GLfloat)velocity_magnitudes[i];
        else
            _vertex_attrib_data[0][i] = (GLfloat)scalar_mags[i];
    }

    GLuint vao, vbo, velocity_buf; vao = vbo = velocity_buf = 0;

    /** copy vertex data to GPU & save VAO and VBO handles **/
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * totalStoredCoords, _vertex_buffer_data, GL_STATIC_DRAW);

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

    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * totalStoredCoords/3, _vertex_attrib_data[0], GL_STATIC_DRAW);

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

    // save min/max values for rendering colored gradients/scaling/etc
    _maxGradientValue = provider->GetMaxValueFromField(_colorParamField);
    _minGradientValue = provider->GetMinValueFromField(_colorParamField);
}

void StreamLineRenderer::Draw(glm::mat4 MVP)
{
    if (!_enabled) { return; }

    // if we have no shaders, vertices, etc., we can't render anything
    if (_shaderProgram == NULL || _VBO <= 0 || _VAO <= 0)
    {
        return;
    }

    // set shaders
    _shaderProgram->enable();

    // set line width
    glLineWidth((GLfloat)_scaleFactorMin);

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
    int startIndex = 0;
    glDrawArrays(GL_LINE_STRIP, startIndex, (streamLinePointCounter[0]-3)/3);
    for (int i = 1; i < lineSourceSize; i++)
    {
        startIndex += streamLinePointCounter[i-1]/3 ;
        glDrawArrays(GL_LINE_STRIP, startIndex, (streamLinePointCounter[i]-3)/3);
    }

    // unset shaders
    _shaderProgram->disable();

    // unbind VAO
    glBindVertexArray(0);
}

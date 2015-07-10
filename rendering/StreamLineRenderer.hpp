#ifndef _STREAMLINE_RENDERER_H
#define _STREAMLINE_RENDERER_H

#include <vector>
#include "RenderableComponent.hpp"
#include "../dataProviders/DataProvider.hpp"

class StreamLineRenderer: public RenderableComponent
{
    public:
        virtual void PrepareGeometry(DataProvider* provider);

        virtual void Draw(glm::mat4 MVP);

    private:
        double maxGradientValue;
        double minGradientValue;
	std::vector<double> trilinearVelocityInterpolator(double deltaX, 
                                                          double deltaY, 
							  double deltaZ, 
							  double xlength,
							  double ylength,
							  double zlength,
							  std::vector<double> currPoint,
							  std::vector<std::vector<double> > localVelocities);
	void RK45(double deltaX, 
                                                          double deltaY, 
							  double deltaZ, 
							  double xlength,
							  double ylength,
							  double zlength,
							  double dt,
							  std::vector<double> &currPoint);
	std::vector<std::vector<double> >* points;
        std::vector<std::vector<double> >* velocities;
        std::vector<std::vector<double> >* domainSize;
};

#endif

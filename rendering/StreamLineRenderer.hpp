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

        double* GetStartPoint();
        void SetStartPoint(double, double, double);

        double* GetEndPoint();
        void SetEndPoint(double, double, double);

        int GetLineSize();
        void SetLineSize(int);

        double GetLineLength();
        void SetLineLength(double);

        double startPoint[3]  = {3.0, 3.0, 3.0};
        double endPoint[3]    = {5.0, 5.0, 5.0};
        int    lineSourceSize = 15;
        double maxStreamlineLength = 500.0;

    private:
        double _maxGradientValue;
        double _minGradientValue;
        std::vector<double> trilinearVelocityInterpolator(
                  double deltaX, 
                  double deltaY, 
                  double deltaZ, 
                  double xlength,
                  double ylength,
                  double zlength,
                  std::vector<double> currPoint,
                  std::vector<std::vector<double> > localVelocities
                  );

        void RK45(
                  double deltaX, 
                  double deltaY, 
                  double deltaZ, 
                  double xlength,
                  double ylength,
                  double zlength,
                  double dt,
                  std::vector<double> &currPoint
                  );

        /// Computes the intersection points between the line defined by lineStart and lineEnd and the box
        /// defined by boxBounds (assumed to be arranged like the extents from DataProvider).
        ///
        /// Returns: Number of intersection points
        ///	         If the number of intersection points is greater than 0, they will be stored in intersections.
        int lineBoxIntersect(double* boxBounds, double* lineStart, double* lineEnd, double* intersections);

        /// Tests to see if the given point resides within the given box bounds
        bool isInBox(double* boxBounds, double* point);

        std::vector<std::vector<double> >* _points;
        std::vector<std::vector<double> >* _velocities;
};

#endif

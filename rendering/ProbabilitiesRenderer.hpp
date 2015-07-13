#ifndef _PROBABILITIES_RENDERER_H
#define _PROBABILITIES_RENDERER_H

#include <vector>
#include <string>
#include <sstream>
#include "RenderableComponent.hpp"
#include "../dataProviders/DataProvider.hpp"

class ProbabilitiesRenderer: public RenderableComponent
{
    public:
        virtual void PrepareGeometry(DataProvider* provider);

        virtual void Draw(glm::mat4 MVP);

        std::vector<double> GetStartPoint();
        void SetStartPoint(double, double, double);
        std::vector<double> GetEndPoint();
        void SetEndPoint(double, double, double);
    private:
        double maxGradientValue;
        double minGradientValue;
        std::vector<std::vector<double> >* points;
        std::vector<std::vector<std::vector<double> >* > probabilities;

        std::vector<double> startPoint = {1.0, 1.0, 1.0};
        std::vector<double> endPoint   = {5.0, 5.0, 5.0};
};

#endif

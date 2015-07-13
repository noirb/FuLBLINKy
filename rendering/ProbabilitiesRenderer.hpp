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

    private:
        double maxGradientValue;
        double minGradientValue;
        std::vector<std::vector<double> >* points;
        std::vector<std::vector<std::vector<double> >* > probabilities;
};

#endif

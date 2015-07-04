#ifndef _POINT_RENDERER_H
#define _POINT_RENDERER_H

#include <vector>
#include "RenderableComponent.hpp"
#include "../dataProviders/DataProvider.hpp"

class PointRenderer: public RenderableComponent
{
    public:
        virtual void PrepareGeometry(DataProvider* provider);

        virtual void Draw(glm::mat4 MVP);

    private:
        double maxGradientValue;
        double minGradientValue;
};

#endif

#ifndef _LINE_RENDERER_H
#define _LINE_RENDERER_H

#include <vector>
#include "RenderableComponent.hpp"
#include "Compositor.hpp"
#include "../dataProviders/DataProvider.hpp"

class LineRenderer: public RenderableComponent
{
    public:
        virtual void PrepareGeometry(DataProvider* provider);

        virtual void Draw(glm::mat4 MVP);

    private:
        double maxGradientValue;
        double minGradientValue;
};

#endif

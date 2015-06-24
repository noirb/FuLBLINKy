#ifndef _POINT_RENDERER_H
#define _POINT_RENDERER_H

#include <vector>
#include "RenderableComponent.hpp"

class PointRenderer: public RenderableComponent
{
    public:
        virtual void PrepareGeometry();
        virtual void PrepareGeometry(std::vector<std::vector<double> >* points);

        virtual void Draw();
        virtual void Draw(glm::mat4 MVP, GLuint MVP_ID);

    private:

};

#endif

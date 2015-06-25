#ifndef _POINT_RENDERER_H
#define _POINT_RENDERER_H

#include <vector>
#include "RenderableComponent.hpp"
#include "../dataProviders/DataProvider.hpp"

class PointRenderer: public RenderableComponent
{
    public:
        virtual void PrepareGeometry();
        virtual void PrepareGeometry(DataProvider* provider);

        virtual void Draw();
        virtual void Draw(glm::mat4 MVP, GLuint MVP_ID, double mouseX, double mouseY, GLuint mouseID);
                            /// FIXME: Do NOT take all these parameters...

    private:

};

#endif

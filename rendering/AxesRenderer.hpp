#ifndef _AXES_RENDERER_H
#define _AXES_RENDERER_H

#include "RenderableComponent.hpp"

/* ------------------------------------------- */
/* Draws coordinate axes at the origin (0,0,0) */
/* ------------------------------------------- */

class AxesRenderer : public RenderableComponent
{
    public:
        virtual void PrepareGeometry(DataProvider*);

        virtual void Draw(glm::mat4 MVP, GLuint MVP_ID);

    private:

};

#endif

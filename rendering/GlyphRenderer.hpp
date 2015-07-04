#ifndef _GLYPH_RENDERER_H
#define _GLYPH_RENDERER_H

#include <vector>
#include "RenderableComponent.hpp"
#include "Compositor.hpp"
#include "../dataProviders/DataProvider.hpp"

class GlyphRenderer: public RenderableComponent
{
    public:
        virtual void PrepareGeometry(DataProvider* provider);

        virtual void Draw(glm::mat4 MVP);

    private:
        double maxGradientValue;
        double minGradientValue;
};

static const GLfloat g_arrow2d_vertex_buffer_data[] = {
     1.0f,  1.0f,  0.0f,  // triangle 1 : 3
     1.0f,  0.0f,  0.0f,  // 4
     1.0f,  1.0f,  10.0f, // 7

     1.0f,  0.0f,  0.0f,  // 4
     1.0f,  1.0f,  10.0f, // 7
     1.0f,  0.0f,  10.0f, // 8

     1.0f,  2.0f, 10.0f,  //10
     1.0f, -1.0f, 10.0f,  // 11
     1.0f,  0.5f, 15.0f,  // 14
  
};

#endif

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
     // One face of the 3D arrow (2D arrow reversed along the xaxis
     1.0f,  1.0f,  0.0f,  //
     1.0f,  0.0f,  0.0f,  //
     1.0f,  1.0f,  10.0f, //

     1.0f,  0.0f,  0.0f,  //
     1.0f,  1.0f,  10.0f, //
     1.0f,  0.0f,  10.0f, //

     // ------------------//

     1.0f,  2.0f, 10.0f,  //
     1.0f, -1.0f, 10.0f,  //
     1.0f,  0.5f, 15.0f,  //

     // ------------------//

     // Other face of the 3D arrow (2D arrow reversed along the xaxis
     0.0f,  1.0f,  0.0f,
     0.0f,  0.0f,  0.0f,
     0.0f,  1.0f,  10.0f,

     0.0f,  0.0f,  0.0f,  //
     0.0f,  1.0f,  10.0f, //
     0.0f,  0.0f,  10.0f, //

     // ------------------//

     0.0f,  2.0f, 10.0f,  //
     0.0f, -1.0f, 10.0f,  //
     0.0f,  0.5f, 15.0f,  //

     // ------------------//

     // The three upper faces
     0.0f,  1.0f, 0.0f,
     1.0f,  1.0f, 0.0f,
     0.0f,  1.0f, 10.0f,

     0.0f,  1.0f, 10.0f,
     1.0f,  1.0f, 10.0f,
     1.0f,  1.0f, 0.0f,

     // ------------------//

     0.0f,  1.0f, 10.0f,
     1.0f,  1.0f, 10.0f,
     0.0f,  2.0f, 10.0f,

     1.0f,  1.0f, 10.0f,
     0.0f,  2.0f, 10.0f,
     1.0f,  2.0f, 10.0f,

     // ------------------//

     0.0f,  2.0f, 10.0f,
     1.0f,  2.0f, 10.0f,
     0.0f,  0.5f, 15.0f,

     0.0f,  0.5f, 15.0f,
     1.0f,  0.5f, 15.0f,
     1.0f,  2.0f, 10.0f,

     // ------------------//

     // The corresponding three lower faces
     0.0f,  0.0f, 0.0f,
     1.0f,  0.0f, 0.0f,
     0.0f,  0.0f, 10.0f,

     0.0f,  0.0f, 10.0f,
     1.0f,  0.0f, 10.0f,
     1.0f,  0.0f, 0.0f,

     // ------------------//

     0.0f,  0.0f, 10.0f,
     1.0f,  0.0f, 10.0f,
     0.0f, -1.0f, 10.0f,

     1.0f,  0.0f, 10.0f,
     0.0f, -1.0f, 10.0f,
     1.0f, -1.0f, 10.0f,

     // ------------------//

     0.0f, -1.0f, 10.0f,
     1.0f, -1.0f, 10.0f,
     0.0f,  0.5f, 15.0f,

     0.0f,  0.5f, 15.0f,
     1.0f,  0.5f, 15.0f,
     1.0f, -1.0f, 10.0f,

     // ------------------//

     // Finally, the base of the arrow

     0.0f,  0.0f, 0.0f,
     1.0f,  0.0f, 0.0f,
     1.0f,  1.0f, 0.0f,

     0.0f,  0.0f, 0.0f,
     0.0f,  1.0f, 0.0f,
     1.0f,  1.0f, 0.0f,

     // ------------------//

};

#endif

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
        double _maxGradientValue;
        double _minGradientValue;
};

/// See 3DGlyphDiag.jpg for details
static const GLfloat g_arrow2d_vertex_buffer_data[] = {
     // One face of the 3D arrow
     1.0f,  1.0f,  0.0f,  // B
     1.0f,  0.0f,  0.0f,  // A
     1.0f,  1.0f,  10.0f, // C

     1.0f,  0.0f,  0.0f,  // A
     1.0f,  1.0f,  10.0f, // C
     1.0f,  0.0f,  10.0f, // D

     // ------------------//

     1.0f,  2.0f, 10.0f,  // F
     1.0f, -1.0f, 10.0f,  // E
     1.0f,  0.5f, 15.0f,  // G

     // ------------------//

     // Other face of the 3D arrow (2D arrow reversed along the xaxis
     0.0f,  1.0f,  0.0f,  // I
     0.0f,  0.0f,  0.0f,  // H
     0.0f,  1.0f,  10.0f, // J

     0.0f,  0.0f,  0.0f,  // H
     0.0f,  1.0f,  10.0f, // J
     0.0f,  0.0f,  10.0f, // K

     // ------------------//

     0.0f,  2.0f, 10.0f,  // M
     0.0f, -1.0f, 10.0f,  // L
     0.0f,  0.5f, 15.0f,  // N

     // ------------------//

     // The three upper faces
     0.0f,  1.0f, 0.0f,   // I
     1.0f,  1.0f, 0.0f,   // B
     0.0f,  1.0f, 10.0f,  // J

     0.0f,  1.0f, 10.0f,  // J
     1.0f,  1.0f, 10.0f,  // C
     1.0f,  1.0f, 0.0f,   // B

     // ------------------//

     0.0f,  1.0f, 10.0f,  // J
     1.0f,  1.0f, 10.0f,  // C
     0.0f,  2.0f, 10.0f,  // M

     1.0f,  1.0f, 10.0f,  // C
     0.0f,  2.0f, 10.0f,  // M
     1.0f,  2.0f, 10.0f,  // F

     // ------------------//

     0.0f,  2.0f, 10.0f,  // M
     1.0f,  2.0f, 10.0f,  // F
     0.0f,  0.5f, 15.0f,  // N

     0.0f,  0.5f, 15.0f,  // N
     1.0f,  0.5f, 15.0f,  // G
     1.0f,  2.0f, 10.0f,  // F

     // ------------------//

     // The corresponding three lower faces
     0.0f,  0.0f, 0.0f,   // H
     1.0f,  0.0f, 0.0f,   // A
     0.0f,  0.0f, 10.0f,  // K

     0.0f,  0.0f, 10.0f,  // K
     1.0f,  0.0f, 10.0f,  // D
     1.0f,  0.0f, 0.0f,   // A

     // ------------------//

     0.0f,  0.0f, 10.0f,  // K
     1.0f,  0.0f, 10.0f,  // D
     0.0f, -1.0f, 10.0f,  // L

     1.0f,  0.0f, 10.0f,  // D
     0.0f, -1.0f, 10.0f,  // L
     1.0f, -1.0f, 10.0f,  // E

     // ------------------//

     0.0f, -1.0f, 10.0f,  // L
     1.0f, -1.0f, 10.0f,  // E
     0.0f,  0.5f, 15.0f,  // N

     0.0f,  0.5f, 15.0f,  // N
     1.0f,  0.5f, 15.0f,  // G
     1.0f, -1.0f, 10.0f,  // E

     // ------------------//

     // Finally, the base of the arrow

     0.0f,  0.0f, 0.0f,   // H
     1.0f,  0.0f, 0.0f,   // A
     1.0f,  1.0f, 0.0f,   // B

     0.0f,  0.0f, 0.0f,   // H
     0.0f,  1.0f, 0.0f,   // I
     1.0f,  1.0f, 0.0f,   // B

     // ------------------//

};

#endif

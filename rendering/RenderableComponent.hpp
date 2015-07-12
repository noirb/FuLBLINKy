#ifndef _RENDERABLE_COMPONENT_H
#define _RENDERABLE_COMPONENT_H

#include "ShaderProgram.hpp"
#include "../common.hpp"
#include "../dataProviders/DataProvider.hpp"

/********************************************************************************/
/* Base class for all Renderable Component Objects                              */
/* Anything which contributes to the frame buffer must be a RenderableComponent */
/********************************************************************************/

class RenderableComponent
{
    public:
        virtual ~RenderableComponent();

        // must be called before Draw(). Used to generate any needed vertex buffers, etc.
        virtual void PrepareGeometry(DataProvider*) = 0;

        virtual void Draw(glm::mat4) = 0;

        void SetShader(ShaderProgram*);

        void SetMaxColor(float, float, float, float);
        void SetMinColor(float, float, float, float);
        void SetInterpolator(Interpolation);
        void SetInterpolationBias(float);

        void SetColorField(std::string);
        void SetScaleField(std::string);
        void SetScale(double);

        void Enable();

        void Disable();

        enum ScalarParamType {
            VECTOR_MAGNITUDE,
            VECTOR_X,
            VECTOR_Y,
            VECTOR_Z
        };

    protected:
        GLfloat*  vertex_buffer_data;  // vertex data specific to this renderer
        GLfloat** vertex_attrib_data;  // collection of vertex attributes (e.g. color, density, etc)
        ShaderProgram* shaderProgram;  // shaders to use when we render
        GLuint VBO;                   // Vertex Buffer Object
        GLuint VAO;                   // Vertex Array Object
        int totalVertices = 0;        // Total # of vertices this object will draw each frame
        int totalAttributes = 0;
        bool enabled;

        std::string colorParamField;  // used to select the scalar field coloring & scaling is defined by
        std::string scaleParamField;

        GLuint maxColorID;
        GLuint minColorID;
        float  maxColor[4] = {1.0f, 0.0f, 0.0f, 1.0f};   /// FIXME: This is getting out of hand...
        float  minColor[4] = {0.0f, 0.0f, 1.0f, 1.0f};   ///        We need a more generic method of storing this stuff
        double scaleFactor = 0.1;
        Interpolation interpolator = Interpolation::LINEAR;
        ScalarParamType scalarParamType = VECTOR_MAGNITUDE;
        float bias = 2.5f;
};

#endif

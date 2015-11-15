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
        void SetInterpolationBias(double);

        void SetColorField(std::string);
        void SetScaleField(std::string);
        void SetAutoScale(bool);
        void SetScale(double, double);

        void Enable();

        void Disable();

        enum ScalarParamType {
            VECTOR_MAGNITUDE,
            VECTOR_X,
            VECTOR_Y,
            VECTOR_Z
        };

    protected:
        GLfloat*  _vertex_buffer_data;  // vertex data specific to this renderer
        GLfloat** _vertex_attrib_data;  // collection of vertex attributes (e.g. color, density, etc)
        ShaderProgram* _shaderProgram;  // shaders to use when we render
        GLuint _VBO;                   // Vertex Buffer Object
        GLuint _VAO;                   // Vertex Array Object
        unsigned int _totalVertices = 0;        // Total # of vertices this object will draw each frame
        unsigned int _totalAttributes = 0;
        bool _enabled;

        std::string _colorParamField;  // used to select the scalar field coloring & scaling is defined by
        std::string _scaleParamField;

        float  _maxColor[4] = {1.0f, 0.0f, 0.0f, 1.0f};   /// FIXME: This is getting out of hand...
        float  _minColor[4] = {0.0f, 0.0f, 1.0f, 1.0f};   ///        We need a more generic method of storing this stuff
        bool   _autoScale = true;
        double _scaleFactorMin = 0.1;
        double _scaleFactorMax = 0.2;
        Interpolation _interpolator = Interpolation::LINEAR;
        ScalarParamType _scalarParamType = VECTOR_MAGNITUDE;
        double _bias = 2.5;
};

#endif

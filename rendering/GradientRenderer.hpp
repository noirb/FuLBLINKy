#ifndef _GRADIENT_RENDERER_H
#define _GRADIENT_RENDERER_H

#include "RenderableComponent.hpp"
class GradientRenderer : public RenderableComponent
{
    public:
        virtual void PrepareGeometry(DataProvider*);

        virtual void Draw(glm::mat4 MVP);

        void SetStartColor(float* rgba);
        void SetEndColor(float* rgba);

        void SetColors(float* start, float* end);

    private:
        float _startColor[4] = { 0.0f, 0.1f, 0.15f, 1.0f };
        float _endColor[4] =   { 0.4f, 0.4f, 0.35f, 1.0f };
};

#endif
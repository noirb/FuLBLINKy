#ifndef _COMPOSITOR_H
#define _COMPOSITOR_H

#include <iostream>
#include <vector>
#include "../nativefiledialog/include/nfd.h"
#include "../dataProviders/vtkLegacyReader.hpp"
#include "../common.hpp"
#include "../loadShaders.hpp"
#include "RenderableComponent.hpp"
#include "AxesRenderer.hpp"
#include "PointRenderer.hpp"

/* ----------------------------------------------------------------- */
/* A lazy singleton used to manage the state of the rendering system */
/* and organize drawing.                                             */
/* ----------------------------------------------------------------- */

class Compositor
{
    public:
        static Compositor& Instance()
        {
            static Compositor _compositor;
            return _compositor;
        }

        // Begins render loop
        void Start();

        // Stops rendering
        void ShutDown();

        void Render(glm::mat4);

        // gets time since the last frame was drawn
        double DeltaTime();

        void UpdateCamera();

        void AddRenderer(RenderableComponent*);

    private:
        Compositor();
        ~Compositor();
        Compositor(Compositor const&);
        Compositor& operator=(Compositor const&);

        CEGUI::OpenGL3Renderer* guiRenderer;
        CEGUI::Window* guiRoot;

        glm::mat4 MVP;
        GLuint mvpID;
        GLuint _axesShader;
        GLuint _scalarMapShader;

        std::vector<RenderableComponent* > _renderers;

        double lastFrameTime; // time when the last frame was drawn
        bool running = false;

        void InitGUI(CEGUI::Window*);
        void InitShaders();
        void UpdateRenderers(DataProvider*);
};

#endif

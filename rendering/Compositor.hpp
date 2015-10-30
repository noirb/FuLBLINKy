#ifndef _COMPOSITOR_H
#define _COMPOSITOR_H

#ifdef _WIN32
#include <functional>
#endif
#include <iostream>
#include <vector>
#include "../nativefiledialog/include/nfd.h"
#include "../dataProviders/vtkLegacyReader.hpp"
#include "../dataProviders/lbsimWrapper.hpp"
#include "../common.hpp"
#include "../loadShaders.hpp"
#include "ShaderProgram.hpp"
#include "RenderableComponent.hpp"
#include "AxesRenderer.hpp"
#include "GradientRenderer.hpp"
#include "PointRenderer.hpp"
#include "GlyphRenderer.hpp"
#include "LineRenderer.hpp"
#include "StreamLineRenderer.hpp"
#include "ProbabilitiesRenderer.hpp"

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

        enum Renderers {
            RENDERER_AXES,
            RENDERER_GRADIENT,
            RENDERER_POINTS,
            RENDERER_GLYPHS,
            RENDERER_LINES,
            RENDERER_STREAMLINES,
            RENDERER_PROBABILITIES,
        };

        // Begins render loop
        void Start();

        // Stops rendering
        void ShutDown();

        // Updates renderable data
        void Update();

        // Renders one frame
        void Render(glm::mat4);

        // gets time since the last frame was drawn
        double DeltaTime();

        // Updates camera matrix based on mouse movement
        void UpdateCamera(double dx, double dy);

        // Updates camera position relative to the screen
        void PanCamera(double dx, double dy);

        // Updates camera's orbit radius
        void ZoomCamera(double dz);

        // Updates FOV, etc., as necessary
        void UpdateAspectRatio(int width, int height);

        // used to notify compositor that the rendering area has changed somehow (window resized, etc)
        void DisplayChanged(int width, int height);

        // Adds the specified renderer to the scene & constructs the relevant GUI for it
        void AddRenderer(RenderableComponent*, bool);

        void AddRenderer(Renderers, bool);

        glm::mat4 GetProjectionMatrix();
        glm::mat4 GetViewMatrix();

    private:
        Compositor();
        ~Compositor();
        Compositor(Compositor const&);
        Compositor& operator=(Compositor const&);

        CEGUI::OpenGL3Renderer* guiRenderer;
        CEGUI::Window* guiRoot;

        glm::mat4 MVP;
        ShaderProgram _axesShader;
        ShaderProgram _backgroundShader;
        ShaderProgram _scalarMapShader;

        std::vector<RenderableComponent* > _renderers;
        
        DataProvider* _dataProvider;

        double lastFrameTime; // time when the last frame was drawn
        bool running = false;
        bool waitingForProvider = false; // True if we are expecting a dataprovider to asynchronously retrieve data for us
        bool autoplay = false; // if true, we load the next timestep automatically
        double autoplay_interval = 0.01f; // time, in seconds, before we load the next timestep in autoplay mode

        int windowSize[2];

        void InitGUI(CEGUI::Window*);
        void InitCamera();
        void InitShaders();
        void UpdateRenderers(DataProvider*);
        void LoadVTK(std::string, CEGUI::Window*);
        void LoadLBM(std::string, CEGUI::Window*);
        CEGUI::Window* AddRendererPopup();
        void UpdateDataGUI(CEGUI::Window*);
        void CenterCameraOnExtents(double*);

        // must correspond with the Renderers enum above
        std::vector<std::string> RendererStrs = {
            "Axes",
            "Background Gradient",
            "Points",
            "Arrow Glyphs",
            "Lines",
            "Stream Lines",
            "Probabilities",
        };

        glm::mat4 _projectionMatrix;
        glm::mat4 _viewMatrix;

        struct camera_parms {
            glm::vec3 cameraPos;    // where the camera is
            glm::vec3 cameraTarget; // where the camera is looking
            float horizontalAngle;
            float verticalAngle;
            float orbitRadius;      // distance from cameraTarget
            float initialFoV;
            float Near;
            float Far;
            float speed;
            float mouseSpeed;
            float panSpeed;
        } camera;
};

#endif

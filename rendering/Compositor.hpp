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
#include "GlyphRenderer.hpp"



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
            RENDERER_POINTS,
            RENDERER_GLYPHS
        };

        // Begins render loop
        void Start();

        // Stops rendering
        void ShutDown();

        void Render(glm::mat4);

        // gets time since the last frame was drawn
        double DeltaTime();

        void UpdateCamera(double dx, double dy);

        void ZoomCamera(double dz);

        void UpdateAspectRatio(int width, int height);

        // used to notify compositor that the rendering area has changed somehow (window resized, etc)
        void DisplayChanged(int width, int height);

        void AddRenderer(RenderableComponent*);

        void AddRenderer(Renderers);

        glm::mat4 GetProjectionMatrix();
        glm::mat4 GetViewMatrix();

        GLuint scalarMinID;     /// TODO: Abstract this away in a Shader wrapper class
        GLuint scalarMaxID;
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
        
        DataProvider* _dataProvider;

        double lastFrameTime; // time when the last frame was drawn
        bool running = false;
        bool autoplay = false; // if true, we load the next timestep automatically
        double autoplay_interval = 0.01f; // time, in seconds, before we load the next timestep in autoplay mode

        void InitGUI(CEGUI::Window*);
        void InitCamera();
        void InitShaders();
        void UpdateRenderers(DataProvider*);
        void LoadVTK(std::string, CEGUI::Window*);
        void UpdateDataGUI(CEGUI::Window*);
        void CenterCameraOnExtents(double*);

        // must correspond with the Renderers enum above
        std::vector<std::string> RendererStrs = {
            "Axes",
            "Points",
            "Arrow Glyphs"
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
            float near;
            float far;
            float speed;
            float mouseSpeed;
        } camera;
};

#endif

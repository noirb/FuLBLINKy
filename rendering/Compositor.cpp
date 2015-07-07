#include "Compositor.hpp"
#include "CEGUI/CommonDialogs/Module.h"
#include "CEGUI/CommonDialogs/ColourPicker/Controls.h"
#include "CEGUI/CommonDialogs/ColourPicker/ColourPicker.h"

Compositor::Compositor()
{
}

Compositor::~Compositor()
{
//    this->guiRenderer->destroySystem();
}

void Compositor::Start()
{
    // set up GUI
    initialiseCEGUICommonDialogs();
    this->guiRenderer = &CEGUI::OpenGL3Renderer::bootstrapSystem();
    this->guiRoot = CEGUI::WindowManager::getSingleton().createWindow("DefaultWindow", "_MasterRoot");
    this->InitShaders();
    this->InitCamera();
    this->InitGUI(guiRoot);

    this->running = true;

    // set a default background color for any pixels we don't draw to
    glClearColor(0.1f, 0.1f, 0.15f, 0.0f);

/*    while (this->running)
    {
        this->Render();
        this->lastFrameTime = glfwGetTime();
    }
*/
    // add default axes renderer
    this->AddRenderer(RENDERER_AXES);
    // add point renderer
    this->AddRenderer(RENDERER_POINTS);
    // add glyph renderer
    this->AddRenderer(RENDERER_GLYPHS);
    // add line renderer
    this->AddRenderer(RENDERER_LINES);
    // add streamline renderer
    this->AddRenderer(RENDERER_STREAMLINES);
}

void Compositor::ShutDown()
{
    this->running = false;
}

double Compositor::DeltaTime()
{
    return glfwGetTime() - this->lastFrameTime;
}

void Compositor::InitCamera()
{
    this->camera.cameraPos = glm::vec3(-12, 50, -8);
    this->camera.cameraTarget = glm::vec3(0, 0, 0);
    this->camera.orbitRadius = 40.0f;
    this->camera.horizontalAngle = 3.0 * 3.14f/2.0f;
    this->camera.verticalAngle = 0.0f;
    this->camera.initialFoV = 45.0f;
    this->camera.near = 0.1f;
    this->camera.far  = 1000.0f;
    this->camera.speed = 3.0f;
    this->camera.mouseSpeed = 0.005f;
    this->camera.panSpeed = 0.015f;

    this->_projectionMatrix = glm::perspective(this->camera.initialFoV, 4.0f / 3.0f, this->camera.near, this->camera.far);
    this->_viewMatrix = glm::lookAt(
        this->camera.cameraPos,     // camera's default location in space
        this->camera.cameraTarget,  // location camera is pointing at
        glm::vec3(0, 1, 0)          // "up" relative to camera
    );

}

// update camera pose & projection matrices based on input
void Compositor::UpdateCamera(double dx, double dy)
{
    // compute new camera orientation
    this->camera.horizontalAngle += this->camera.mouseSpeed * dx; /// TODO: Should include a reference to DeltaTime() here for stable movement
    this->camera.verticalAngle   -= this->camera.mouseSpeed * dy;

    // compute new camera position
    this->camera.cameraPos = this->camera.orbitRadius * glm::vec3(
        cos(this->camera.verticalAngle) * sin(this->camera.horizontalAngle),
        sin(this->camera.verticalAngle),
        cos(this->camera.horizontalAngle)
    );
    this->camera.cameraPos = this->camera.cameraPos + this->camera.cameraTarget;

    this->_projectionMatrix = glm::perspective(this->camera.initialFoV, 4.0f / 3.0f, this->camera.near, this->camera.far);
    this->_viewMatrix = glm::lookAt(
        this->camera.cameraPos,
        this->camera.cameraTarget,
        glm::vec3(0, 1, 0)
    );
}

// update camera pose & projection matrices based on panning input
void Compositor::PanCamera(double dx, double dy)
{
    glm::vec3 lookDir = glm::normalize(this->camera.cameraPos - this->camera.cameraTarget); // direction camera is looking in
    glm::vec3 cameraRight = glm::cross(lookDir, glm::vec3(0, 1, 0));                        // "right" w.r.t. the camera
    glm::vec3 cameraUp = glm::cross(lookDir, cameraRight);                                  // "up" w.r.t. the camera
    glm::vec3 translation = this->camera.panSpeed * ((float)dx * cameraRight - (float)dy * cameraUp);

    // need to update cameraPos & cameraTarget together or else we'd get a rotation after performing UpdateCamera!
    this->camera.cameraPos += translation;
    this->camera.cameraTarget += translation;

    this->UpdateCamera(0, 0);
}

// update camera pose & preojection matrices based on zoom input
void Compositor::ZoomCamera(double dz)
{
    this->camera.orbitRadius += dz;

    this->UpdateCamera(0, 0);
}

void Compositor::CenterCameraOnExtents(double* extents)
{
    // compute center of bounds
    glm::vec3 center = glm::vec3(
                      extents[0] + (extents[1] - extents[0]) / 2.0f, // (xmax - xmin) / 2
                      extents[2] + (extents[3] - extents[2]) / 2.0f, // (ymax - ymin) / 2
                      extents[4] + (extents[5] - extents[4]) / 2.0f);// (zmax - zmin) / 2
    this->camera.cameraTarget = center;
    std::cout << "New camera orbit point: " << center[0] << ", " << center[1] << ", " << center[2] << std::endl;
    // compute widest radius necessary to enclose bounds
    this->camera.orbitRadius = glm::max( center[0] + extents[1],
                                         glm::max( center[1] + extents[3], center[2] + extents[5])
                                       );
    this->UpdateCamera(0, 0);
}

// update rendering parameters based on new window aspect ratio
void Compositor::UpdateAspectRatio(int width, int height)
{
    this->_projectionMatrix = glm::perspective(this->camera.initialFoV, (float)width / (float)height, this->camera.near, this->camera.far);
    this->DisplayChanged(width, height);
}

glm::mat4 Compositor::GetProjectionMatrix()
{
    return this->_projectionMatrix;
}

glm::mat4 Compositor::GetViewMatrix()
{
    return this->_viewMatrix;
}

void Compositor::DisplayChanged(int width, int height)
{
    CEGUI::System::getSingleton().notifyDisplaySizeChanged(CEGUI::Sizef(width, height));
}

void Compositor::AddRenderer(RenderableComponent* renderer)
{
    this->_renderers.push_back(renderer);
}

void Compositor::AddRenderer(Renderers rendererType)
{
    std::cout << "Adding GUI for new Renderer to scene (" << rendererType << ")" << std::endl;

    RenderableComponent* newRenderer;
    std::string rendererName;

    switch (rendererType)
    {
        case RENDERER_AXES:
            newRenderer = new AxesRenderer();
            newRenderer->SetShader(&(this->_axesShader));
            newRenderer->PrepareGeometry(NULL);
            break;
        case RENDERER_POINTS:
            newRenderer = new PointRenderer();
            newRenderer->SetShader(&(this->_scalarMapShader));
            break;
        case RENDERER_GLYPHS:
            newRenderer = new GlyphRenderer();
            newRenderer->SetShader(&(this->_scalarMapShader));
            break;
        case RENDERER_LINES:
            newRenderer = new LineRenderer();
            newRenderer->SetShader(&(this->_scalarMapShader));
            break;
        case RENDERER_STREAMLINES:
            newRenderer = new StreamLineRenderer();
            newRenderer->SetShader(&(this->_scalarMapShader));
            break;
        default:
            std::cout << "ERROR <Compositor::AddRenderer> : Invalid Renderer Type " << rendererType << std::endl;
            return;
    }

    rendererName = this->RendererStrs[rendererType] + std::to_string(this->_renderers.size()); // give new renderer a unique name

    std::cout << "\tAdding ToggleButton...";

    // Add UI controls for the new renderer
    CEGUI::VerticalLayoutContainer* entries_container = static_cast<CEGUI::VerticalLayoutContainer*>(this->guiRoot->getChildRecursive("renderers_container"));
    CEGUI::ToggleButton* rWnd = static_cast<CEGUI::ToggleButton*>(CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/Checkbox", rendererName));
    entries_container->addChild(rWnd);
    rWnd->setText(this->RendererStrs[rendererType]);
    rWnd->setSize(CEGUI::USize(CEGUI::UDim(1, 0), CEGUI::UDim(0, 50)));
    rWnd->setSelected(false);


    // subscribe to CheckStateChanged so we know when the renderer is enabled/disabled
    rWnd->subscribeEvent(CEGUI::ToggleButton::EventSelectStateChanged, 
                    [rWnd, newRenderer] (const CEGUI::EventArgs &e)->bool
                        {
                            if (rWnd->isSelected())
                                newRenderer->Enable();
                            else
                                newRenderer->Disable();
                            return true;
                        }
    );

    std::cout << "Done!" << std::endl;

    // if new renderer is not an AxesRenderer, add color pickers for hot/cold colors, combobox for interpolation, etc.
    if (rendererType != RENDERER_AXES)
    {
        std::cout << "\tAdding color interpolation combobox...";

        /*  Interpolation ComboBox */
        CEGUI::Combobox* combobox = static_cast<CEGUI::Combobox*>(CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/Combobox"));
        entries_container->addChild(combobox);
        combobox->setAutoSizeListHeightToContent(true);
        combobox->setSize(CEGUI::USize(CEGUI::UDim(0, 160), CEGUI::UDim(0, 200)));
        combobox->setMargin(CEGUI::UBox( CEGUI::UDim(0, 0),
                                         CEGUI::UDim(0, 0),
                                         CEGUI::UDim(0, -175),  // fix bottom margin of combobox to avoid breaking layout
                                         CEGUI::UDim(0, 0) ));
        CEGUI::ListboxTextItem* comboEntry1 = new CEGUI::ListboxTextItem("Linear", Interpolation::LINEAR);
        CEGUI::ListboxTextItem* comboEntry2 = new CEGUI::ListboxTextItem("Smooth", Interpolation::SMOOTH);
        CEGUI::ListboxTextItem* comboEntry3 = new CEGUI::ListboxTextItem("Exponential", Interpolation::EXPONENTIAL);

        combobox->addItem(comboEntry1);
        combobox->addItem(comboEntry2);
        combobox->addItem(comboEntry3);
        combobox->setItemSelectState(comboEntry1, true);

        // register for selection changed event
        combobox->subscribeEvent(CEGUI::Combobox::EventListSelectionAccepted,
                    [newRenderer] (const CEGUI::EventArgs &e)->bool
                        {
                            const CEGUI::WindowEventArgs &wargs = static_cast<const CEGUI::WindowEventArgs&>(e);
                            CEGUI::Combobox* combobox = static_cast<CEGUI::Combobox*>(wargs.window);
                            CEGUI::ListboxItem* selected = combobox->getSelectedItem();
                            newRenderer->SetInterpolator(Interpolation(selected->getID()));
                            return true;
                        }
        );        

        std::cout << "Done!" << std::endl;
        std::cout << "\tAdding bias spinner...";

        /*  Interpolation Bias setter */
        CEGUI::Spinner* spinner = static_cast<CEGUI::Spinner*>(CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/Spinner"));
        entries_container->addChild(spinner);
        spinner->setMinimumValue(-4.0);
        spinner->setMaximumValue(4.0);
        spinner->setStepSize(0.1);
        spinner->setTextInputMode(CEGUI::Spinner::TextInputMode::FloatingPoint);
        spinner->setCurrentValue(0.5);
        spinner->subscribeEvent(CEGUI::Spinner::EventValueChanged,
                    [newRenderer] (const CEGUI::EventArgs &e)->bool
                        {
                            const CEGUI::WindowEventArgs &wargs = static_cast<const CEGUI::WindowEventArgs&>(e);
                            CEGUI::Spinner* spinner = static_cast<CEGUI::Spinner*>(wargs.window);
                            newRenderer->SetInterpolationBias(spinner->getCurrentValue());
                            return true;
                        }
        );

        std::cout << "Done!" << std::endl;
        std::cout << "\tAdding color pickers...";

        /*  COLOR PICKERS */
        CEGUI::ColourPicker* colourPicker_max = static_cast<CEGUI::ColourPicker*>(CEGUI::WindowManager::getSingleton().createWindow("Vanilla/ColourPicker"));
        entries_container->addChild(colourPicker_max);
        colourPicker_max->setPosition(CEGUI::UVector2(CEGUI::UDim(0, 20), CEGUI::UDim(0, 40)));
        colourPicker_max->setSize(CEGUI::USize(CEGUI::UDim(0, 50), CEGUI::UDim(0, 30)));
        colourPicker_max->setColour(CEGUI::Colour(1.0f, 0.0f, 0.0f, 1.0f));
        colourPicker_max->subscribeEvent(CEGUI::ColourPicker::EventAcceptedColour,
                    [newRenderer] (const CEGUI::EventArgs &e)->bool
                        {
                            const CEGUI::WindowEventArgs &wargs = static_cast<const CEGUI::WindowEventArgs&>(e);
                            CEGUI::ColourPicker* picker = static_cast<CEGUI::ColourPicker*>(wargs.window);
                            CEGUI::Colour c = picker->getColour();
                            newRenderer->SetMaxColor(c.getRed(), c.getGreen(), c.getBlue(), c.getAlpha());
                            return true;
                        }
        );

        // label for the colourpicker
        CEGUI::Window* colourPickerLabel_max = CEGUI::WindowManager::getSingleton().createWindow("Vanilla/Label");
        colourPicker_max->addChild(colourPickerLabel_max);
        colourPickerLabel_max->setSize(CEGUI::USize(CEGUI::UDim(1.0f, 0.0f), CEGUI::UDim(0.0f, 30.0f)));
        colourPickerLabel_max->setText("Max");
        colourPickerLabel_max->setMousePassThroughEnabled(true);
        colourPickerLabel_max->setAlwaysOnTop(true);

         CEGUI::ColourPicker* colourPicker_min = static_cast<CEGUI::ColourPicker*>(CEGUI::WindowManager::getSingleton().createWindow("Vanilla/ColourPicker"));
         entries_container->addChild(colourPicker_min);
         colourPicker_min->setPosition(CEGUI::UVector2(CEGUI::UDim(0, 20), CEGUI::UDim(0, 40)));
         colourPicker_min->setSize(CEGUI::USize(CEGUI::UDim(0, 50), CEGUI::UDim(0, 30))); 
         colourPicker_min->setColour(CEGUI::Colour(0.0f, 0.0f, 0.8f, 1.0f));
         colourPicker_min->subscribeEvent(CEGUI::ColourPicker::EventAcceptedColour,
                    [newRenderer] (const CEGUI::EventArgs &e)->bool
                        {
                            const CEGUI::WindowEventArgs &wargs = static_cast<const CEGUI::WindowEventArgs&>(e);
                            CEGUI::ColourPicker* picker = static_cast<CEGUI::ColourPicker*>(wargs.window);
                            CEGUI::Colour c = picker->getColour();
                            newRenderer->SetMinColor(c.getRed(), c.getGreen(), c.getBlue(), c.getAlpha());
                            return true;
                        }
        );
     
         // label for the colourpicker
         CEGUI::Window* colourPickerLabel_min = CEGUI::WindowManager::getSingleton().createWindow("Vanilla/Label");
         colourPicker_min->addChild(colourPickerLabel_min);
         colourPickerLabel_min->setSize(CEGUI::USize(CEGUI::UDim(1.0f, 0.0f), CEGUI::UDim(0.0f, 30.0f)));
         colourPickerLabel_min->setText("Min");
         colourPickerLabel_min->setMousePassThroughEnabled(true);
         colourPickerLabel_min->setAlwaysOnTop(true);

        std::cout << "Done!" << std::endl;

    }


    // add new renderer to compositor
    newRenderer->Disable(); // renderer OFF by default//Enable();
    this->AddRenderer(newRenderer);

    std::cout << "Done setting things up for the new renderer :D (" << newRenderer << ")" << std::endl;
}

void Compositor::InitGUI(CEGUI::Window* guiRoot)
{
    // set default resource paths
    CEGUI::DefaultResourceProvider* rp = static_cast<CEGUI::DefaultResourceProvider*>(CEGUI::System::getSingleton().getResourceProvider());
    rp->setResourceGroupDirectory("schemes", "./cegui_layout/schemes/");
    rp->setResourceGroupDirectory("imagesets", "./cegui_layout/imagesets/");
    rp->setResourceGroupDirectory("fonts", "./cegui_layout/fonts/");
    rp->setResourceGroupDirectory("layouts", "./cegui_layout/layouts/");
    rp->setResourceGroupDirectory("looknfeels", "./cegui_layout/looknfeel");
    rp->setResourceGroupDirectory("lua_scripts", "./cegui_layout/lua_scripts");
    CEGUI::ImageManager::setImagesetDefaultResourceGroup("imagesets");
    CEGUI::Font::setDefaultResourceGroup("fonts");
    CEGUI::WidgetLookManager::setDefaultResourceGroup("looknfeels");
    CEGUI::WindowManager::setDefaultResourceGroup("layouts");
    CEGUI::ScriptModule::setDefaultResourceGroup("lua_scripts");
    CEGUI::Scheme::setDefaultResourceGroup("schemes");
    CEGUI::SchemeManager::getSingleton().createFromFile("TaharezLook.scheme");
    CEGUI::SchemeManager::getSingleton().createFromFile("VanillaSkin.scheme");
    CEGUI::SchemeManager::getSingleton().createFromFile("VanillaCommonDialogs.scheme");
    CEGUI::System::getSingleton().getDefaultGUIContext().setDefaultFont("DejaVuSans-12");
    CEGUI::System::getSingleton().getDefaultGUIContext().setDefaultTooltipType("TaharezLook/Tooltip");
    // force CEGUI's mouse position to (0,0)     /// TODO: do this in InputManager
    CEGUI::Vector2<float> mousePos = CEGUI::System::getSingleton().getDefaultGUIContext().getMouseCursor().getPosition();
    CEGUI::System::getSingleton().getDefaultGUIContext().injectMouseMove(-mousePos.d_x, -mousePos.d_y);

    // set root window
    CEGUI::System::getSingleton().getDefaultGUIContext().setRootWindow(guiRoot);
    guiRoot->setMousePassThroughEnabled(true);
    // load default window layout
    CEGUI::Window* fWnd = CEGUI::WindowManager::getSingleton().loadLayoutFromFile("default.layout");
    guiRoot->addChild(fWnd);
    CEGUI::Window* data_window = fWnd->getChildRecursive("data_window"); // main window holding timestep controls, etc.

    // Configure the Load VTK button
    fWnd->getChildRecursive("LoadVTKbtn")->subscribeEvent(CEGUI::PushButton::EventClicked, 
                         [this, data_window](const CEGUI::EventArgs &e)->bool {
                            nfdchar_t* outPath = NULL;
                            nfdresult_t result = NFD_OpenDialog("vtk", NULL, &outPath);

                            if (result == NFD_OKAY)
                            {
                                std::cout << "Opening file: '" << outPath << "'" << std::endl;
                                this->LoadVTK(outPath, data_window);
                            }
                            else if (result == NFD_CANCEL)
                            {
                                std::cout << "User pressed Cancel..." << std::endl;
                            }
                            else
                            {
                                std::cout << "ERROR: " << NFD_GetError() << std::endl;
                            }
                            return true;
                        }
    );

    // Configure the Load LBD button
    fWnd->getChildRecursive("LoadLBMbtn")->subscribeEvent(CEGUI::PushButton::EventClicked,
                        [this, data_window](const CEGUI::EventArgs &e)->bool {
                            nfdchar_t* outPath = NULL;
                            nfdresult_t result = NFD_OpenDialog("dat", NULL, &outPath);

                            if (result == NFD_OKAY)
                            {
                                std::cout << "Opening file: '" << outPath << "'" << std::endl;
                                this->LoadLBM(outPath, data_window);
                            }
                            else if (result != NFD_CANCEL)
                            {
                                std::cout << "ERROR: " << NFD_GetError() << std::endl;
                            }
                            return true;
                        }
    );

    // Configure the timestep control buttons
    fWnd->getChildRecursive("btnNextTimeStep")->subscribeEvent(CEGUI::PushButton::EventClicked,
                        [this, data_window](const CEGUI::EventArgs &e)->bool {
                            this->_dataProvider->NextTimeStep();
                            this->UpdateDataGUI(data_window);
                            return true;
                        }
    );
    fWnd->getChildRecursive("btnPrevTimeStep")->subscribeEvent(CEGUI::PushButton::EventClicked,
                        [this, data_window](const CEGUI::EventArgs &e)->bool {
                            this->_dataProvider->PrevTimeStep();
                            this->UpdateDataGUI(data_window);
                            return true;
                        }
    );
    fWnd->getChildRecursive("btnPlay")->subscribeEvent(CEGUI::PushButton::EventClicked,
                        [this, data_window](const CEGUI::EventArgs &e)->bool {
                            const CEGUI::WindowEventArgs &wargs = static_cast<const CEGUI::WindowEventArgs&>(e);
                            if (this->autoplay)
                            {
                                wargs.window->setText(">>");
                            }
                            else
                            {
                                wargs.window->setText("||");
                            }
                            this->autoplay = !this->autoplay;
                            return true;
                        }
    );

    // set mouse cursor
    CEGUI::System::getSingleton().getDefaultGUIContext().getMouseCursor().setDefaultImage("TaharezLook/MouseArrow");
    guiRoot->setMouseCursor("TaharezLook/MouseArrow");
    fWnd->setMouseCursor("TaharezLook/MouseArrow");

}

void Compositor::InitShaders()
{
    // load our vertex & fragment shaders so they're ready & compiled when we need them
    this->_axesShader.loadAndLink("shaders/_coordinateAxes.vertex", "shaders/_coordinateAxes.fragment");
    this->_axesShader.addUniform("MVP");

    this->_scalarMapShader.loadAndLink("shaders/scalarGradientMap1D.vertex", "shaders/scalarGradientMap1D.fragment");
    this->_scalarMapShader.addUniform("MVP");
    this->_scalarMapShader.addUniform("min_scalar");
    this->_scalarMapShader.addUniform("max_scalar");
    this->_scalarMapShader.addUniform("hotColor");
    this->_scalarMapShader.addUniform("coldColor");
    this->_scalarMapShader.addUniform("bias");         // used for exponential interpolation
    this->_scalarMapShader.addUniform("interpolator"); // used to select an interpolation mode
}

void Compositor::LoadVTK(std::string filename, CEGUI::Window* vtkWindowRoot)
{
    if (this->_dataProvider)
    {
        delete this->_dataProvider;
    }

    this->_dataProvider = new vtkLegacyReader(filename);
    vtkWindowRoot->setText(filename.substr(filename.find_last_of("/")+1));

    CEGUI::Window* timestep_label = vtkWindowRoot->getChildRecursive("lblTimestep");
    CEGUI::Window* maxTimestep_label = vtkWindowRoot->getChildRecursive("lblMaxTimestep");

    if (this->_dataProvider->GetTimeStep() >= 0)
        timestep_label->setText(std::to_string(this->_dataProvider->GetTimeStep()));
    else
        timestep_label->setText("N/A");

    if (this->_dataProvider->GetMaxTimeStep() >= 0)
        maxTimestep_label->setText(std::to_string(this->_dataProvider->GetMaxTimeStep()-1));
    else
        timestep_label->setText("--");

    this->CenterCameraOnExtents(this->_dataProvider->GetExtents());
    this->UpdateRenderers(this->_dataProvider);
}

void Compositor::LoadLBM(std::string filename, CEGUI::Window* dataWindowRoot)
{
    if (this->_dataProvider)
    {
        delete this->_dataProvider;
    }

    this->_dataProvider = new lbsimWrapper(filename);
    dataWindowRoot->setText(filename.substr(filename.find_last_of("/")+1));

    CEGUI::Window* timestep_label = dataWindowRoot->getChildRecursive("lblTimestep");
    CEGUI::Window* maxTimestep_label = dataWindowRoot->getChildRecursive("lblMaxTimestep");

    if (this->_dataProvider->GetTimeStep() >= 0)
        timestep_label->setText(std::to_string(this->_dataProvider->GetTimeStep()));
    else
        timestep_label->setText("N/A");

    maxTimestep_label->setText("--"); // LBM always has infinite timesteps

    this->CenterCameraOnExtents(this->_dataProvider->GetExtents());
    this->UpdateRenderers(this->_dataProvider);
}

void Compositor::UpdateDataGUI(CEGUI::Window* dataWindowRoot)
{
    // check to see if our dataProvider is a vtkLegacyReader
    vtkLegacyReader* legacyReader = dynamic_cast<vtkLegacyReader*>(this->_dataProvider);
    if (legacyReader)
    {
        dataWindowRoot->setText(legacyReader->GetFileName());
    }

    CEGUI::Window* timestep_label = dataWindowRoot->getChildRecursive("lblTimestep");

    if (this->_dataProvider->GetTimeStep() >= 0)
        timestep_label->setText(std::to_string(this->_dataProvider->GetTimeStep()));
    else
        timestep_label->setText("N/A");

    this->UpdateRenderers(this->_dataProvider);
}

void Compositor::UpdateRenderers(DataProvider* provider)
{
    for (auto r : this->_renderers)
    {
        r->PrepareGeometry(provider);
    }
}

void Compositor::Render(glm::mat4 MVP)
{
    static double timer = 0.0;

    // tell CEGUI how long its been since the last frame
    double dt = this->DeltaTime();
    CEGUI::System::getSingleton().injectTimePulse(dt);
    CEGUI::System::getSingleton().getDefaultGUIContext().injectTimePulse(dt);
    this->lastFrameTime = glfwGetTime();

    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (auto r : this->_renderers)
    {
        r->Draw(MVP);
    }

    glDisable(GL_DEPTH_TEST); // no depth testing for GUIs
    // render GUI -- must be the LAST drawing call we make!
    CEGUI::System::getSingleton().renderAllGUIContexts();

    timer += dt;
    if (this->autoplay && timer >= this->autoplay_interval)
    {
        timer = 0;
        this->_dataProvider->NextTimeStep();
        this->UpdateDataGUI(this->guiRoot->getChildRecursive("data_window"));
        // if we've reached the maximal timestep, stop autoplaying
        if (this->_dataProvider->GetMaxTimeStep()-1 == this->_dataProvider->GetTimeStep())
        {
            this->autoplay = false;
            this->guiRoot->getChildRecursive("btnPlay")->setText(">>"); /// HACK: Should probably have start/stop autoplaying functions to avoid so many hardcoded strings...
        }
    }
}

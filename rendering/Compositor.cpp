#include "Compositor.hpp"

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
    this->guiRenderer = &CEGUI::OpenGL3Renderer::bootstrapSystem();
    this->guiRoot = CEGUI::WindowManager::getSingleton().createWindow("DefaultWindow", "_MasterRoot");
    this->InitShaders();
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
}

void Compositor::ShutDown()
{
    this->running = false;
}

double Compositor::DeltaTime()
{
    return glfwGetTime() - this->lastFrameTime;
}

void Compositor::UpdateCamera()
{

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
    RenderableComponent* newRenderer;
    std::string rendererName;

    switch (rendererType)
    {
        case RENDERER_AXES:
            newRenderer = new AxesRenderer();
            newRenderer->SetShader(this->_axesShader);
            newRenderer->PrepareGeometry(NULL);
            break;
        case RENDERER_POINTS:
            newRenderer = new PointRenderer();
            newRenderer->SetShader(this->_scalarMapShader);
            break;
        default:
            std::cout << "ERROR <Compositor::AddRenderer> : Invalid Renderer Type " << rendererType << std::endl;
            return;
    }

    rendererName = this->RendererStrs[rendererType] + std::to_string(this->_renderers.size()); // give new renderer a unique name

    // Add UI controls for the new renderer
    CEGUI::VerticalLayoutContainer* entries_container = static_cast<CEGUI::VerticalLayoutContainer*>(this->guiRoot->getChildRecursive("renderers_container"));
    CEGUI::ToggleButton* rWnd = static_cast<CEGUI::ToggleButton*>(CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/Checkbox", rendererName));
    entries_container->addChild(rWnd);
    rWnd->setText(rendererName);
    rWnd->setSize(CEGUI::USize(CEGUI::UDim(1, 0), CEGUI::UDim(0, 50)));
    rWnd->setSelected(true);

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

    // add new renderer to compositor
    newRenderer->Enable();
    this->AddRenderer(newRenderer);
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
    CEGUI::Window* timestep_label = fWnd->getChildRecursive("lblTimestep");
    fWnd->getChildRecursive("LoadVTKbtn")->subscribeEvent(CEGUI::PushButton::EventClicked, 
                         [this, timestep_label](const CEGUI::EventArgs &e)->bool {
                            nfdchar_t* outPath = NULL;
                            nfdresult_t result = NFD_OpenDialog("vtk", NULL, &outPath);

                            if (result == NFD_OKAY)
                            {
                                std::cout << "Opening file: '" << outPath << "'" << std::endl;

                                vtkLegacyReader vtkReader = vtkLegacyReader(outPath);
                                if (vtkReader.GetTimeStep() >= 0)
                                    timestep_label->setText(std::to_string(vtkReader.GetTimeStep()));
                                else
                                    timestep_label->setText("N/A");

                                this->UpdateRenderers(&vtkReader);
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

    // set mouse cursor
    CEGUI::System::getSingleton().getDefaultGUIContext().getMouseCursor().setDefaultImage("TaharezLook/MouseArrow");
    guiRoot->setMouseCursor("TaharezLook/MouseArrow");
    fWnd->setMouseCursor("TaharezLook/MouseArrow");

}

void Compositor::InitShaders()
{
    // load our vertex & fragment shaders so they're ready & compiled when we need them
    this->_scalarMapShader = LoadShaders("shaders/scalarGradientMap1D.vertex", "shaders/scalarGradientMap1D.fragment");
    this->_axesShader = LoadShaders("shaders/_coordinateAxes.vertex", "shaders/_coordinateAxes.fragment");
    this->mvpID = glGetUniformLocation(_scalarMapShader, "MVP");
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
    // tell CEGUI how long its been since the last frame
    double dt = this->DeltaTime();
    CEGUI::System::getSingleton().injectTimePulse(dt);
    CEGUI::System::getSingleton().getDefaultGUIContext().injectTimePulse(dt);
    this->lastFrameTime = glfwGetTime();

    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (auto r : this->_renderers)
    {
        r->Draw(MVP, this->mvpID);
    }

    glDisable(GL_DEPTH_TEST); // no depth testing for GUIs
    // render GUI -- must be the LAST drawing call we make!
    CEGUI::System::getSingleton().renderAllGUIContexts();
}

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
    this->InitGUI(guiRoot);
    this->InitShaders();

    /// FIXME: These are manually added here just for testing. Remove them someday.
    this->_renderers.push_back(new AxesRenderer());
    this->_renderers.back()->SetShader(this->_axesShader);
    this->_renderers.back()->PrepareGeometry(NULL);
    this->_renderers.push_back(new PointRenderer());
    this->_renderers.back()->SetShader(this->_scalarMapShader);

    this->running = true;

    // set a default background color for any pixels we don't draw to
    glClearColor(0.1f, 0.1f, 0.15f, 0.0f);

/*    while (this->running)
    {
        this->Render();
        this->lastFrameTime = glfwGetTime();
    }
*/
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

    fWnd->getChildRecursive("LoadVTKbtn")->subscribeEvent(CEGUI::PushButton::EventClicked, 
                         [this](const CEGUI::EventArgs &e)->bool {
                            nfdchar_t* outPath = NULL;
                            nfdresult_t result = NFD_OpenDialog("vtk", NULL, &outPath);

                            if (result == NFD_OKAY)
                            {
                                std::cout << "Opening file: '" << outPath << "'" << std::endl;
                                vtkLegacyReader vtkReader = vtkLegacyReader(outPath);
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

    CEGUI::VerticalLayoutContainer* entries_container = static_cast<CEGUI::VerticalLayoutContainer*>(fWnd->getChildRecursive("renderers_container"));

    CEGUI::ToggleButton* rWnd = static_cast<CEGUI::ToggleButton*>(CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/Checkbox", "renderer_1"));
    entries_container->addChild(rWnd);
    rWnd->setText("AxesRenderer");
    rWnd->setSize(CEGUI::USize(CEGUI::UDim(1, 0), CEGUI::UDim(0, 50)));

    CEGUI::ToggleButton* rWnd2 = static_cast<CEGUI::ToggleButton*>(CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/Checkbox", "renderer_2"));
    entries_container->addChild(rWnd2);
    rWnd2->setText("PointRenderer");
    rWnd2->setSize(CEGUI::USize(CEGUI::UDim(1, 0), CEGUI::UDim(0, 50)));

    entries_container->update(0.5f);
    // set mouse cursor
    CEGUI::System::getSingleton().getDefaultGUIContext().getMouseCursor().setDefaultImage("TaharezLook/MouseArrow");
    guiRoot->setMouseCursor("TaharezLook/MouseArrow");
    fWnd->setMouseCursor("TaharezLook/MouseArrow");

    CEGUI::System::getSingleton().notifyDisplaySizeChanged(CEGUI::Sizef(1024, 768));
    guiRoot->invalidate(true);
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

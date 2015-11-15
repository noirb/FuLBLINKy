#include "Compositor.hpp"
#include "CEGUI/CommonDialogs/Module.h"
#include "CEGUI/CommonDialogs/ColourPicker/Controls.h"
#include <iostream>
#include "CEGUI/CommonDialogs/ColourPicker/ColourPicker.h"
//#include "CEGUI.h"

Compositor::Compositor()
{
}

Compositor::~Compositor()
{
//    _guiRenderer->destroySystem();
}

void Compositor::Start()
{
    // set up GUI
    initialiseCEGUICommonDialogs();
    _guiRenderer = &CEGUI::OpenGL3Renderer::bootstrapSystem();
    _guiRoot = CEGUI::WindowManager::getSingleton().createWindow("DefaultWindow", "_MasterRoot");
    this->InitShaders();
    this->InitCamera();
    this->InitGUI(_guiRoot);

    _running = true;

    // set a default background color for any pixels we don't draw to
    glClearColor(0.1f, 0.1f, 0.15f, 0.0f);

    // add background gradient renderer
    this->AddRenderer(RENDERER_GRADIENT, true);
    // add default axes renderer
    this->AddRenderer(RENDERER_AXES, true);
}

void Compositor::ShutDown()
{
    _running = false;
}

double Compositor::DeltaTime()
{
    return glfwGetTime() - _lastFrameTime;
}

void Compositor::InitCamera()
{
    this->camera.cameraPos = glm::vec3(-12, 50, -8);
    this->camera.cameraTarget = glm::vec3(0, 0, 0);
    this->camera.orbitRadius = 40.0f;
    this->camera.horizontalAngle = 3.0f * 3.14f/2.0f;
    this->camera.verticalAngle = 0.0f;
    this->camera.initialFoV = 45.0f;
    this->camera.Near = 0.1f;
    this->camera.Far  = 1000.0f;
    this->camera.speed = 3.0f;
    this->camera.mouseSpeed = 0.005f;
    this->camera.panSpeed = 0.015f;

    _projectionMatrix = glm::perspective(this->camera.initialFoV, 4.0f / 3.0f, this->camera.Near, this->camera.Far);
    _viewMatrix = glm::lookAt(
        this->camera.cameraPos,     // camera's default location in space
        this->camera.cameraTarget,  // location camera is pointing at
        glm::vec3(0, 1, 0)          // "up" relative to camera
    );

}

// update camera pose & projection matrices based on input
void Compositor::UpdateCamera(double dx, double dy)
{
    // compute new camera orientation
    this->camera.horizontalAngle += this->camera.mouseSpeed * (float)dx; /// TODO: Should include a reference to DeltaTime() here for stable movement
    this->camera.verticalAngle   -= this->camera.mouseSpeed * (float)dy;

    // compute new camera position
    this->camera.cameraPos = this->camera.orbitRadius * glm::vec3(
        cos(this->camera.verticalAngle) * sin(this->camera.horizontalAngle),
        sin(this->camera.verticalAngle),
        cos(this->camera.horizontalAngle)
    );
    this->camera.cameraPos = this->camera.cameraPos + this->camera.cameraTarget;

    _projectionMatrix = glm::perspective(this->camera.initialFoV, 4.0f / 3.0f, this->camera.Near, this->camera.Far);
    _viewMatrix = glm::lookAt(
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
    this->camera.orbitRadius += (float)dz;

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
    this->camera.orbitRadius = (float)glm::max( center[0] + extents[1],
                                      glm::max( center[1] + extents[3], center[2] + extents[5])
                                     );
    this->UpdateCamera(0, 0);
}

// update rendering parameters based on new window aspect ratio
void Compositor::UpdateAspectRatio(int width, int height)
{
    _projectionMatrix = glm::perspective(this->camera.initialFoV, (float)width / (float)height, this->camera.Near, this->camera.Far);
    this->DisplayChanged(width, height);
}

glm::mat4 Compositor::GetProjectionMatrix()
{
    return _projectionMatrix;
}

glm::mat4 Compositor::GetViewMatrix()
{
    return _viewMatrix;
}

void Compositor::DisplayChanged(int width, int height)
{
    _windowSize[0] = width;
    _windowSize[1] = height;
    CEGUI::System::getSingleton().notifyDisplaySizeChanged(CEGUI::Sizef((float)width, (float)height));
}

void Compositor::AddRenderer(RenderableComponent* renderer, bool onByDefault)
{
    _renderers.push_back(renderer);

    // if we already have data to visualize, send it to the new renderer
    if (_dataProvider)
    {
        renderer->PrepareGeometry(_dataProvider);
    }

    if (onByDefault)
        renderer->Enable();
    else
        renderer->Disable();
}

void Compositor::AddSystemRenderer(RenderableComponent* renderer, bool onByDefault)
{
    _systemRenderers.push_back(renderer);

    if (onByDefault)
        renderer->Enable();
    else
        renderer->Disable();
}

void Compositor::AddRenderer(Renderers rendererType, bool onByDefault)
{
    RenderableComponent* newRenderer;
    std::string rendererName;

    switch (rendererType)
    {
        case RENDERER_AXES:
            newRenderer = new AxesRenderer();
            newRenderer->SetShader(&(_axesShader));
            newRenderer->PrepareGeometry(NULL);
            AddSystemRenderer(newRenderer, true);
            return;
            break;
        case RENDERER_GRADIENT:
            newRenderer = new GradientRenderer();
            newRenderer->SetShader(&(_backgroundShader));
            newRenderer->PrepareGeometry(NULL);
            AddSystemRenderer(newRenderer, true);
            return;
            break;
        case RENDERER_POINTS:
            newRenderer = new PointRenderer();
            newRenderer->SetShader(&(_scalarMapShader));
            break;
        case RENDERER_GLYPHS:
            newRenderer = new GlyphRenderer();
            newRenderer->SetShader(&(_scalarMapShader));
            break;
        case RENDERER_LINES:
            newRenderer = new LineRenderer();
            newRenderer->SetShader(&(_scalarMapShader));
            break;
        case RENDERER_STREAMLINES:
            newRenderer = new StreamLineRenderer();
            newRenderer->SetShader(&(_scalarMapShader));
            break;
        case RENDERER_PROBABILITIES:
            newRenderer = new ProbabilitiesRenderer();
            newRenderer->SetShader(&(_scalarMapShader));
            break;
        default:
            std::cout << "ERROR <Compositor::AddRenderer> : Invalid Renderer Type " << rendererType << std::endl;
            return;
    }

    std::cout << "Adding GUI for new Renderer to scene (" << this->RendererStrs[rendererType] << ")" << std::endl;

    rendererName = this->RendererStrs[rendererType] + std::to_string(_renderers.size()); // give new renderer a unique name

    // Add basic UI controls for the new renderer
    CEGUI::VerticalLayoutContainer* entries_container = static_cast<CEGUI::VerticalLayoutContainer*>(_guiRoot->getChildRecursive("renderers_container"));
    CEGUI::VerticalLayoutContainer* params_root = static_cast<CEGUI::VerticalLayoutContainer*>(CEGUI::WindowManager::getSingleton().createWindow("VerticalLayoutContainer"));
    entries_container->addChild(params_root);

    CEGUI::HorizontalLayoutContainer* title_container = static_cast<CEGUI::HorizontalLayoutContainer*>(CEGUI::WindowManager::getSingleton().createWindow("HorizontalLayoutContainer", rendererName));
    params_root->addChild(title_container);

    CEGUI::ToggleButton* rWnd = static_cast<CEGUI::ToggleButton*>(CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/Checkbox", rendererName));
    title_container->addChild(rWnd);
    rWnd->setText(this->RendererStrs[rendererType]);
    rWnd->setSize(CEGUI::USize(CEGUI::UDim(0.75, 0), CEGUI::UDim(0, 50)));
    rWnd->setSelected(onByDefault);

    CEGUI::PushButton* btnClose = static_cast<CEGUI::PushButton*>(CEGUI::WindowManager::getSingleton().loadLayoutFromFile("closeButton.layout"));
    title_container->addChild(btnClose);
    btnClose->subscribeEvent(CEGUI::PushButton::EventClicked,
                    [this, params_root, newRenderer] (const CEGUI::EventArgs &e)->bool
                        {
                            this->_renderers.erase(std::remove(this->_renderers.begin(), this->_renderers.end(), newRenderer ), this->_renderers.end());
                            CEGUI::WindowManager::getSingleton().destroyWindow(params_root);
                            delete(newRenderer);
                            return true;
                        }
    );

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

    // if new renderer is not an AxesRenderer or GradientRenderer, add color pickers for hot/cold colors, combobox for interpolation, etc.
    if (rendererType != RENDERER_AXES && rendererType != RENDERER_GRADIENT)
    {
        // container to hold all the parameter controls
        CEGUI::FrameWindow* paramBox_parent = static_cast<CEGUI::FrameWindow*>(CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/FrameWindow_Auto"));
        CEGUI::Window* paramBox = paramBox_parent->getChild("__auto_clientarea__");
        paramBox_parent->setFrameEnabled(false);
        paramBox_parent->setTitleBarEnabled(false);
        paramBox_parent->setSizingEnabled(false);
        paramBox_parent->setCloseButtonEnabled(false);
        paramBox_parent->setDragMovingEnabled(false);
        paramBox_parent->setRollupEnabled(true);
        paramBox_parent->setSize(CEGUI::USize(CEGUI::UDim(1, 0), CEGUI::UDim(0,200)));
        paramBox_parent->setRolledup(!onByDefault);
        params_root->addChild(paramBox_parent);

        /* --------------------------------
            Stream Line-Specific Controls
           -------------------------------- */
        if (rendererType == RENDERER_STREAMLINES)
        {
            StreamLineRenderer* newStreamRenderer = static_cast<StreamLineRenderer*>(newRenderer);
            AddStreamlineRendererPropertySheet(paramBox, paramBox_parent, rWnd, newStreamRenderer);
            
        } // end stream-line specific controls

        /* --------------------------------
            Probability-Specific Controls
           -------------------------------- */
        else if (rendererType == RENDERER_PROBABILITIES)
        {
            ProbabilitiesRenderer* newProbRenderer = static_cast<ProbabilitiesRenderer*>(newRenderer);
            AddProbabilitiesRendererPropertySheet(paramBox, paramBox_parent, rWnd, newProbRenderer);
        }

        /* --------------------------------
            All other renderer's controls
           -------------------------------- */
        else
        {
            AddStandardRendererPropertySheet(paramBox, paramBox_parent, rWnd, newRenderer);
        }

    }

    // add new renderer to compositor
    this->AddRenderer(newRenderer, onByDefault);

    std::cout << "Done setting things up for the new renderer :D (" << this->RendererStrs[rendererType] << ")" << std::endl;
}

/// Adds comboboxes for selecting a scalar field to color by and selecting an interpolation mode for the coloring
void Compositor::RendererAddFieldSelectionCombobox(CEGUI::Window* root, RenderableComponent* newRenderer)
{
    /* Scalar Field selection combobox for colors */
    CEGUI::Combobox* colorField_combobox = static_cast<CEGUI::Combobox*>(CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/Combobox"));
    root->addChild(colorField_combobox);
    colorField_combobox->setAutoSizeListHeightToContent(true);
    colorField_combobox->setSize(CEGUI::USize(CEGUI::UDim(0, 160), CEGUI::UDim(0, 100)));
    colorField_combobox->setMargin(CEGUI::UBox(CEGUI::UDim(0, 0),
        CEGUI::UDim(0, 0),
        CEGUI::UDim(0, -60), // fix bottom margin to avoid breaking layout
        CEGUI::UDim(0, 0)));
    colorField_combobox->subscribeEvent(CEGUI::Combobox::EventListSelectionAccepted,
        [this, newRenderer](const CEGUI::EventArgs &e)->bool
        {
            const CEGUI::WindowEventArgs &wargs = static_cast<const CEGUI::WindowEventArgs&>(e);
            CEGUI::Combobox* combobox = static_cast<CEGUI::Combobox*>(wargs.window);
            CEGUI::ListboxItem* selected = combobox->getSelectedItem();
            newRenderer->SetColorField(selected->getText().c_str());
            newRenderer->PrepareGeometry(this->_dataProvider);
            return true;
        }
    );

    if (_dataProvider)
    {
        for (auto field : _dataProvider->GetFieldNames())
        {
            colorField_combobox->addItem(new CEGUI::ListboxTextItem(field, RenderableComponent::ScalarParamType::VECTOR_MAGNITUDE));
        }

        // if we have at least 1 field, select the first one by default
        if (colorField_combobox->getItemCount() > 0)
        {
            colorField_combobox->setItemSelectState((size_t)0, true);
            newRenderer->SetColorField(colorField_combobox->getSelectedItem()->getText().c_str());
        }
    }
}

void Compositor::RendererAddInterpolationCombobox(CEGUI::Window* root, RenderableComponent* newRenderer)
{
    /*  Interpolation ComboBox */
    CEGUI::Combobox* combobox = static_cast<CEGUI::Combobox*>(CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/Combobox"));
    root->addChild(combobox);
    combobox->setAutoSizeListHeightToContent(true);
    combobox->setSize(CEGUI::USize(CEGUI::UDim(0, 160), CEGUI::UDim(0, 200)));
    combobox->setMargin(CEGUI::UBox(CEGUI::UDim(0, 0),
        CEGUI::UDim(0, 0),
        CEGUI::UDim(0, -165),  // fix bottom margin of combobox to avoid breaking layout
        CEGUI::UDim(0, 0)));
    CEGUI::ListboxTextItem* comboEntry1 = new CEGUI::ListboxTextItem("Linear", Interpolation::LINEAR);
    CEGUI::ListboxTextItem* comboEntry2 = new CEGUI::ListboxTextItem("Smooth", Interpolation::SMOOTH);
    CEGUI::ListboxTextItem* comboEntry3 = new CEGUI::ListboxTextItem("Exponential", Interpolation::EXPONENTIAL);

    combobox->addItem(comboEntry1);
    combobox->addItem(comboEntry2);
    combobox->addItem(comboEntry3);
    combobox->setItemSelectState(comboEntry1, true);

    // register for selection changed event
    combobox->subscribeEvent(CEGUI::Combobox::EventListSelectionAccepted,
        [newRenderer](const CEGUI::EventArgs &e)->bool
        {
            const CEGUI::WindowEventArgs &wargs = static_cast<const CEGUI::WindowEventArgs&>(e);
            CEGUI::Combobox* combobox = static_cast<CEGUI::Combobox*>(wargs.window);
            CEGUI::ListboxItem* selected = combobox->getSelectedItem();
            newRenderer->SetInterpolator(Interpolation(selected->getID()));
            return true;
        }
    );
}

/// Adds a Spinner to the given root element
CEGUI::Spinner* Compositor::RendererAddSpinner(double min, double max, double current, double stepSize, std::string tooltip, bool enabled, CEGUI::Window* root, RenderableComponent* newRenderer, CEGUI::Event::Subscriber valueChangedEvent)
{
    CEGUI::Spinner* spinner = static_cast<CEGUI::Spinner*>(CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/Spinner"));
    root->addChild(spinner);

    spinner->setMinimumValue(min);
    spinner->setMaximumValue(max);
    spinner->setStepSize(stepSize);
    spinner->setTextInputMode(CEGUI::Spinner::TextInputMode::FloatingPoint);
    spinner->setTooltipText(tooltip);
    spinner->setCurrentValue(current);
    spinner->setDisabled(!enabled);
    spinner->setSize(CEGUI::USize(CEGUI::UDim(0.3f, 0.0f), CEGUI::UDim(0.0f, 30.0f)));
    spinner->subscribeEvent(CEGUI::Spinner::EventValueChanged, valueChangedEvent);

    return spinner;
}

/// Adds a Checkbox (ToggleButton) to the given root element
CEGUI::Window* Compositor::RendererAddCheckbox(CEGUI::Window* root, bool selected, std::string label, std::string tooltip, CEGUI::Event::Subscriber valueChangedEvent)
{
    CEGUI::ToggleButton* checkbox = static_cast<CEGUI::ToggleButton*>(CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/Checkbox"));
    root->addChild(checkbox);
    checkbox->setText(label);
    checkbox->setTooltipText(tooltip); // "Automatically detect min/max values from dataset");
    checkbox->setSelected(selected);
    checkbox->subscribeEvent(CEGUI::ToggleButton::EventSelectStateChanged, valueChangedEvent);

    return checkbox;
}

/// Adds a Label to the given root element
CEGUI::Window* Compositor::RendererAddLabel(CEGUI::Window* root, CEGUI::USize size, std::string text, std::string tooltip)
{
    CEGUI::Window* label = static_cast<CEGUI::Window*>(CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/Label"));
    label->setText(text);
    label->setProperty("HorzFormatting", "Left");
    label->setProperty("VertFormatting", "WordWrapLeftAligned");
    label->setTooltipText(tooltip);
    label->setSize(size);
    root->addChild(label);

    return label;
}

/// Adds an Editbox to the given root element
CEGUI::Editbox* Compositor::RendererAddEditbox(CEGUI::Window* root, std::string text, std::string tooltip)
{
    CEGUI::Editbox* editbox = static_cast<CEGUI::Editbox*>(CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/Editbox"));
    root->addChild(editbox);
    editbox->setText(text);
    editbox->setSize(CEGUI::USize(CEGUI::UDim(0.3f, 0.0f), CEGUI::UDim(0.0f, 30.0f)));

    return editbox;
}

/// Adds an Editbox to the given root element, and subscribes to the TextAccepted event
CEGUI::Editbox* Compositor::RendererAddEditbox(CEGUI::Window* root, std::string text, std::string tooltip, CEGUI::Event::Subscriber valueChangedEvent)
{
    CEGUI::Editbox* editbox = RendererAddEditbox(root, text, tooltip);

    editbox->subscribeEvent(CEGUI::Editbox::EventTextAccepted, valueChangedEvent);

    return editbox;
}

/// Adds a ColourPicker to the given root element, subscribes to the AcceptedColour event, and adds a text label to the picker if the label string is set
void Compositor::RendererAddColorPicker(CEGUI::Window* root, std::string label, CEGUI::Colour defaultColor, CEGUI::Event::Subscriber valueChangedEvent)
{
    CEGUI::ColourPicker* colourPicker = static_cast<CEGUI::ColourPicker*>(CEGUI::WindowManager::getSingleton().createWindow("Vanilla/ColourPicker"));
    root->addChild(colourPicker);
    colourPicker->setPosition(CEGUI::UVector2(CEGUI::UDim(0, 20), CEGUI::UDim(0, 40)));
    colourPicker->setSize(CEGUI::USize(CEGUI::UDim(0, 50), CEGUI::UDim(0, 30)));
    colourPicker->setColour(defaultColor);
    colourPicker->subscribeEvent(CEGUI::ColourPicker::EventAcceptedColour, valueChangedEvent);

    // label for the colourpicker
    if (label.length() > 0)
    {
        CEGUI::Window* colourPickerLabel = CEGUI::WindowManager::getSingleton().createWindow("Vanilla/Label");
        colourPicker->addChild(colourPickerLabel);
        colourPickerLabel->setSize(CEGUI::USize(CEGUI::UDim(1.0f, 0.0f), CEGUI::UDim(0.0f, 30.0f)));
        colourPickerLabel->setText(label);
        colourPickerLabel->setMousePassThroughEnabled(true);
        colourPickerLabel->setAlwaysOnTop(true);
    }
}

/// Creates all the standard parameter controls used for the glyph-based renderers (points, lines, arrows)
void Compositor::AddStandardRendererPropertySheet(CEGUI::Window* root, CEGUI::Window* container, CEGUI::ToggleButton* visibleToggle, RenderableComponent* newRenderer)
{
    // add an additional subscriber to CheckStateChanged to shade/unshade parameter lists
    visibleToggle->subscribeEvent(CEGUI::ToggleButton::EventSelectStateChanged,
        [visibleToggle, container](const CEGUI::EventArgs &e)->bool
        {
            if (visibleToggle->isSelected())
            {
                static_cast<CEGUI::FrameWindow*>(container)->setRolledup(false);
                container->setMargin(CEGUI::UBox(CEGUI::UDim(0, 0)));
            }
            else
            {
                static_cast<CEGUI::FrameWindow*>(container)->setRolledup(true);
                container->setMargin(CEGUI::UBox(CEGUI::UDim(0, 0), CEGUI::UDim(0, 0), CEGUI::UDim(0, -200), CEGUI::UDim(0, 0)));
            }
            return true;
        }
    );

    // scalar field selection
    RendererAddFieldSelectionCombobox(root, newRenderer);
    RendererAddInterpolationCombobox(root, newRenderer);

    /*  Interpolation Bias setter */
    RendererAddSpinner(-4.0, 4.0, 0.5, 0.1, "", true, root, newRenderer,
        [newRenderer](const CEGUI::EventArgs &e)->bool
        {
            const CEGUI::WindowEventArgs &wargs = static_cast<const CEGUI::WindowEventArgs&>(e);
            CEGUI::Spinner* spinner = static_cast<CEGUI::Spinner*>(wargs.window);
            newRenderer->SetInterpolationBias(spinner->getCurrentValue());
            return true;
        }
    );

    /*  COLOR PICKERS */
    CEGUI::HorizontalLayoutContainer* picker_container = static_cast<CEGUI::HorizontalLayoutContainer*>(CEGUI::WindowManager::getSingleton().createWindow("HorizontalLayoutContainer"));
    root->addChild(picker_container);

    // maximum color
    RendererAddColorPicker(picker_container, "Max", CEGUI::Colour(1.0f, 0.0f, 0.0f, 1.0f),
        [newRenderer](const CEGUI::EventArgs &e)->bool
        {
            const CEGUI::WindowEventArgs &wargs = static_cast<const CEGUI::WindowEventArgs&>(e);
            CEGUI::ColourPicker* picker = static_cast<CEGUI::ColourPicker*>(wargs.window);
            CEGUI::Colour c = picker->getColour();
            newRenderer->SetMaxColor(c.getRed(), c.getGreen(), c.getBlue(), c.getAlpha());
            return true;
        }
    );

    // minimum color
    RendererAddColorPicker(picker_container, "Min", CEGUI::Colour(0.0f, 0.0f, 0.8f, 1.0f),
        [newRenderer](const CEGUI::EventArgs &e)->bool
        {
            const CEGUI::WindowEventArgs &wargs = static_cast<const CEGUI::WindowEventArgs&>(e);
            CEGUI::ColourPicker* picker = static_cast<CEGUI::ColourPicker*>(wargs.window);
            CEGUI::Colour c = picker->getColour();
            newRenderer->SetMinColor(c.getRed(), c.getGreen(), c.getBlue(), c.getAlpha());
            return true;
        }
    );

    /*  Scale Parameters */
    RendererAddLabel(root, CEGUI::USize(CEGUI::UDim(1, 0), CEGUI::UDim(0, 30)), "Scale:", "Absolute scale of points");

    // layout container to hold min/max scale controls
    CEGUI::HorizontalLayoutContainer* scale_container = static_cast<CEGUI::HorizontalLayoutContainer*>(CEGUI::WindowManager::getSingleton().createWindow("HorizontalLayoutContainer"));
    root->addChild(scale_container);

    // minimum scale selection box
    CEGUI::Spinner* scaleBox_min = RendererAddSpinner(0.0, 10.0, 1.0, 0.001, "Minimum interpolation value", false, scale_container, newRenderer,
        [this, newRenderer](const CEGUI::EventArgs &e)->bool
        {
            const CEGUI::WindowEventArgs &wargs = static_cast<const CEGUI::WindowEventArgs&>(e);
            CEGUI::Spinner* box = static_cast<CEGUI::Spinner*>(wargs.window);
            newRenderer->SetScale(box->getCurrentValue(), -1);
            newRenderer->PrepareGeometry(this->_dataProvider);
            return true;
        }
    );

    // maximum scale selection box
    CEGUI::Spinner* scaleBox_max = RendererAddSpinner(0.0, 10.0, 1.0, 0.001, "Maximum interpolation value", false, scale_container, newRenderer,
        [this, newRenderer](const CEGUI::EventArgs &e)->bool
        {
            const CEGUI::WindowEventArgs &wargs = static_cast<const CEGUI::WindowEventArgs&>(e);
            CEGUI::Spinner* box = static_cast<CEGUI::Spinner*>(wargs.window);
            newRenderer->SetScale(-1, box->getCurrentValue());
            newRenderer->PrepareGeometry(this->_dataProvider);
            return true;
        }
    );

    // toggle for auto-scaling
    RendererAddCheckbox(scale_container, true, "Auto", "Automatically detect min/max values from dataset",
        [this, newRenderer, scaleBox_min, scaleBox_max](const CEGUI::EventArgs &e)->bool
        {
            const CEGUI::WindowEventArgs &wargs = static_cast<const CEGUI::WindowEventArgs&>(e);
            CEGUI::ToggleButton* chkBox = static_cast<CEGUI::ToggleButton*>(wargs.window);
            if (chkBox->isSelected())
            {
                scaleBox_min->setDisabled(true);
                scaleBox_max->setDisabled(true);
                newRenderer->SetAutoScale(true);
                newRenderer->PrepareGeometry(this->_dataProvider);
            }
            else
            {
                scaleBox_min->setDisabled(false);
                scaleBox_max->setDisabled(false);
                newRenderer->SetAutoScale(false);
                newRenderer->SetScale(scaleBox_min->getCurrentValue(), scaleBox_max->getCurrentValue());
            }
            return true;
        }
    );
}

/// Creates the parameter controls used by the StreamLineRenderer
void Compositor::AddStreamlineRendererPropertySheet(CEGUI::Window* root, CEGUI::Window* container, CEGUI::ToggleButton* visibleToggle, StreamLineRenderer* newRenderer)
{
    // add an additional subscriber to CheckStateChanged to shade/unshade parameter lists
    visibleToggle->subscribeEvent(CEGUI::ToggleButton::EventSelectStateChanged,
        [visibleToggle, container](const CEGUI::EventArgs &e)->bool
        {
            if (visibleToggle->isSelected())
            {
                static_cast<CEGUI::FrameWindow*>(container)->setRolledup(false);
                container->setMargin(CEGUI::UBox(CEGUI::UDim(0, 0)));
            }
            else
            {
                static_cast<CEGUI::FrameWindow*>(container)->setRolledup(true);
                container->setMargin(CEGUI::UBox(CEGUI::UDim(0, 0), CEGUI::UDim(0, 0), CEGUI::UDim(0, -450), CEGUI::UDim(0, 0)));
            }
            return true;
        }
    );

    // scalar field selection
    RendererAddFieldSelectionCombobox(root, newRenderer);
    RendererAddInterpolationCombobox(root, newRenderer);

    /*  Interpolation Bias setter */
    RendererAddSpinner(-4.0, 4.0, 0.5, 0.1, "", true, root, newRenderer,
        [newRenderer](const CEGUI::EventArgs &e)->bool
        {
            const CEGUI::WindowEventArgs &wargs = static_cast<const CEGUI::WindowEventArgs&>(e);
            CEGUI::Spinner* spinner = static_cast<CEGUI::Spinner*>(wargs.window);
            newRenderer->SetInterpolationBias(spinner->getCurrentValue());
            return true;
        }
    );

    /*  COLOR PICKERS */
    CEGUI::HorizontalLayoutContainer* picker_container = static_cast<CEGUI::HorizontalLayoutContainer*>(CEGUI::WindowManager::getSingleton().createWindow("HorizontalLayoutContainer"));
    root->addChild(picker_container);

    // maximum color
    RendererAddColorPicker(picker_container, "Max", CEGUI::Colour(1.0f, 0.0f, 0.0f, 1.0f),
        [newRenderer](const CEGUI::EventArgs &e)->bool
        {
            const CEGUI::WindowEventArgs &wargs = static_cast<const CEGUI::WindowEventArgs&>(e);
            CEGUI::ColourPicker* picker = static_cast<CEGUI::ColourPicker*>(wargs.window);
            CEGUI::Colour c = picker->getColour();
            newRenderer->SetMaxColor(c.getRed(), c.getGreen(), c.getBlue(), c.getAlpha());
            return true;
        }
    );

    // minimum color
    RendererAddColorPicker(picker_container, "Min", CEGUI::Colour(0.0f, 0.0f, 0.8f, 1.0f),
        [newRenderer](const CEGUI::EventArgs &e)->bool
        {
            const CEGUI::WindowEventArgs &wargs = static_cast<const CEGUI::WindowEventArgs&>(e);
            CEGUI::ColourPicker* picker = static_cast<CEGUI::ColourPicker*>(wargs.window);
            CEGUI::Colour c = picker->getColour();
            newRenderer->SetMinColor(c.getRed(), c.getGreen(), c.getBlue(), c.getAlpha());
            return true;
        }
    );

    /*  Scale Parameters */
    RendererAddLabel(root, CEGUI::USize(CEGUI::UDim(1, 0), CEGUI::UDim(0, 30)), "Scale:", "Line Thickness");

    // layout container to hold min/max scale controls
    CEGUI::HorizontalLayoutContainer* scale_container = static_cast<CEGUI::HorizontalLayoutContainer*>(CEGUI::WindowManager::getSingleton().createWindow("HorizontalLayoutContainer"));
    root->addChild(scale_container);

    // scale selection box
    RendererAddSpinner(0.0, 10.0, 1.0, 0.001, "Width value", true, scale_container, newRenderer,
        [this, newRenderer](const CEGUI::EventArgs &e)->bool
        {
            const CEGUI::WindowEventArgs &wargs = static_cast<const CEGUI::WindowEventArgs&>(e);
            CEGUI::Spinner* box = static_cast<CEGUI::Spinner*>(wargs.window);
            newRenderer->SetScale(box->getCurrentValue(), -1);
            newRenderer->PrepareGeometry(this->_dataProvider);
            return true;
        }
    );


    /* -------------------------------
        Stream-line-specific controls
       ------------------------------- */

    // due to extra widgets, the paramBox container needs to be taller than default
    container->setSize(CEGUI::USize(CEGUI::UDim(1, 0), CEGUI::UDim(0, 450)));

    //Label for the start point
    RendererAddLabel(root, CEGUI::USize(CEGUI::UDim(0, 100), CEGUI::UDim(0, 30)), "Start Point:", "Starting point of the line streamline source");

    //Coordinates of start point
    CEGUI::HorizontalLayoutContainer* startpointContainer = static_cast<CEGUI::HorizontalLayoutContainer*>(CEGUI::WindowManager::getSingleton().createWindow("HorizontalLayoutContainer"));
    root->addChild(startpointContainer);

    CEGUI::Editbox* editbox_X_start = RendererAddEditbox(startpointContainer, std::to_string(newRenderer->startPoint[0]), "Start X Coord");
    CEGUI::Editbox* editbox_Y_start = RendererAddEditbox(startpointContainer, std::to_string(newRenderer->startPoint[1]), "Start Y Coord");
    CEGUI::Editbox* editbox_Z_start = RendererAddEditbox(startpointContainer, std::to_string(newRenderer->startPoint[2]), "Start Z Coord");

    std::function<bool(const CEGUI::EventArgs&)> updateStartPoint = [this, newRenderer, editbox_X_start, editbox_Y_start, editbox_Z_start](const CEGUI::EventArgs &e)->bool
    {
        double newX, newY, newZ;
        std::stringstream sstm;
        sstm << editbox_X_start->getText() << " " << editbox_Y_start->getText() << " " << editbox_Z_start->getText();
        sstm >> newX;
        sstm >> newY;
        sstm >> newZ;
        (static_cast<StreamLineRenderer*>(newRenderer))->SetStartPoint(newX, newY, newZ);
        // in case new changes were rejected, get values again so they can be displayed in the editboxes
        double* newPoint = (static_cast<StreamLineRenderer*>(newRenderer))->GetStartPoint();
        editbox_X_start->setText(std::to_string(newPoint[0]));
        editbox_Y_start->setText(std::to_string(newPoint[1]));
        editbox_Z_start->setText(std::to_string(newPoint[2]));
        newRenderer->PrepareGeometry(this->_dataProvider);
        return true;

    };
    editbox_X_start->subscribeEvent(CEGUI::Editbox::EventTextAccepted, updateStartPoint);
    editbox_Y_start->subscribeEvent(CEGUI::Editbox::EventTextAccepted, updateStartPoint);
    editbox_Z_start->subscribeEvent(CEGUI::Editbox::EventTextAccepted, updateStartPoint);

    //Label for the start point
    RendererAddLabel(root, CEGUI::USize(CEGUI::UDim(0, 100), CEGUI::UDim(0, 30)), "End Point:", "End point of the line streamline source");

    //Coordinates of end point
    CEGUI::HorizontalLayoutContainer* endpointContainer = static_cast<CEGUI::HorizontalLayoutContainer*>(CEGUI::WindowManager::getSingleton().createWindow("HorizontalLayoutContainer"));
    root->addChild(endpointContainer);

    CEGUI::Editbox* editbox_X_end = RendererAddEditbox(endpointContainer, std::to_string(newRenderer->endPoint[0]), "End X Coord");
    CEGUI::Editbox* editbox_Y_end = RendererAddEditbox(endpointContainer, std::to_string(newRenderer->endPoint[1]), "End Y Coord");
    CEGUI::Editbox* editbox_Z_end = RendererAddEditbox(endpointContainer, std::to_string(newRenderer->endPoint[2]), "End Z Coord");

    std::function<bool(const CEGUI::EventArgs&)> updateEndPoint = [this, newRenderer, editbox_X_end, editbox_Y_end, editbox_Z_end](const CEGUI::EventArgs &e)->bool
    {
        double newX, newY, newZ;
        std::stringstream sstm;
        sstm << editbox_X_end->getText() << " " << editbox_Y_end->getText() << " " << editbox_Z_end->getText();
        sstm >> newX;
        sstm >> newY;
        sstm >> newZ;
        (static_cast<StreamLineRenderer*>(newRenderer))->SetEndPoint(newX, newY, newZ);
        // in case new changes were rejected, get values again so they can be displayed in the editboxes
        double* newPoint = (static_cast<StreamLineRenderer*>(newRenderer))->GetEndPoint();
        editbox_X_end->setText(std::to_string(newPoint[0]));
        editbox_Y_end->setText(std::to_string(newPoint[1]));
        editbox_Z_end->setText(std::to_string(newPoint[2]));
        newRenderer->PrepareGeometry(this->_dataProvider);
        return true;

    };
    editbox_X_end->subscribeEvent(CEGUI::Editbox::EventTextAccepted, updateEndPoint);
    editbox_Y_end->subscribeEvent(CEGUI::Editbox::EventTextAccepted, updateEndPoint);
    editbox_Z_end->subscribeEvent(CEGUI::Editbox::EventTextAccepted, updateEndPoint);


    // Label for number of points on the streamline line source
    RendererAddLabel(root, CEGUI::USize(CEGUI::UDim(1, 0), CEGUI::UDim(0, 30)), "Number of points:", "Number of points on the source line");
    // Editbox for number of points on streamline line source
    RendererAddEditbox(root, std::to_string(newRenderer->lineSourceSize), "Number of streamline sourcees",
        [this, newRenderer](const CEGUI::EventArgs &e)->bool
        {
            const CEGUI::WindowEventArgs &wargs = static_cast<const CEGUI::WindowEventArgs&>(e);
            CEGUI::Editbox* box = static_cast<CEGUI::Editbox*>(wargs.window);
            StreamLineRenderer* r = static_cast<StreamLineRenderer*>(newRenderer);
            std::stringstream sstm;
            sstm << box->getText();
            int value;
            sstm >> value;
            r->SetLineSize(value);
            // in case input was rejected, re-get real value
            box->setText(std::to_string(r->GetLineSize()));
            newRenderer->PrepareGeometry(this->_dataProvider);
            return true;
        }
    );

    // Label for the maximum length of the streamline
    RendererAddLabel(root, CEGUI::USize(CEGUI::UDim(1, 0), CEGUI::UDim(0, 30)), "Max Length:", "Maximal length of streamlines");
    // Editbox for streamline length
    RendererAddEditbox(root, std::to_string(newRenderer->maxStreamlineLength), "Maximum streamline length",
        [this, newRenderer](const CEGUI::EventArgs &e)->bool
        {
            const CEGUI::WindowEventArgs &wargs = static_cast<const CEGUI::WindowEventArgs&>(e);
            CEGUI::Editbox* box = static_cast<CEGUI::Editbox*>(wargs.window);
            StreamLineRenderer* r = static_cast<StreamLineRenderer*>(newRenderer);
            std::stringstream sstm;
            sstm << box->getText();
            double value;
            sstm >> value;
            r->SetLineLength(value);
            // in case input was rejected, re-get rel value
            box->setText(std::to_string(r->GetLineLength()));
            newRenderer->PrepareGeometry(this->_dataProvider);
            return true;
        }
    );
}

/// Creates the parameter controls used by the ProbabilitiesRenderer
void Compositor::AddProbabilitiesRendererPropertySheet(CEGUI::Window* root, CEGUI::Window* container, CEGUI::ToggleButton* visibleToggle, ProbabilitiesRenderer* newRenderer)
{
    // resize parameters container to make extra room for the new controls
    container->setSize(CEGUI::USize(CEGUI::UDim(1, 0), CEGUI::UDim(0, 225)));

    // add an additional subscriber to CheckStateChanged to shade/unshade parameter lists
    visibleToggle->subscribeEvent(CEGUI::ToggleButton::EventSelectStateChanged,
        [visibleToggle, container](const CEGUI::EventArgs &e)->bool
        {
            if (visibleToggle->isSelected())
            {
                static_cast<CEGUI::FrameWindow*>(container)->setRolledup(false);
                container->setMargin(CEGUI::UBox(CEGUI::UDim(0, 0)));
            }
            else
            {
                static_cast<CEGUI::FrameWindow*>(container)->setRolledup(true);
                container->setMargin(CEGUI::UBox(CEGUI::UDim(0, 0), CEGUI::UDim(0, 0), CEGUI::UDim(0, -225), CEGUI::UDim(0, 0)));
            }
            return true;
        }
    );

    RendererAddInterpolationCombobox(root, newRenderer);
    /*  Interpolation Bias setter */
    RendererAddSpinner(-4.0, 4.0, 0.5, 0.1, "", true, root, newRenderer,
        [newRenderer](const CEGUI::EventArgs &e)->bool
        {
            const CEGUI::WindowEventArgs &wargs = static_cast<const CEGUI::WindowEventArgs&>(e);
            CEGUI::Spinner* spinner = static_cast<CEGUI::Spinner*>(wargs.window);
            newRenderer->SetInterpolationBias(spinner->getCurrentValue());
            return true;
        }
    );

    /*  COLOR PICKERS */
    CEGUI::HorizontalLayoutContainer* picker_container = static_cast<CEGUI::HorizontalLayoutContainer*>(CEGUI::WindowManager::getSingleton().createWindow("HorizontalLayoutContainer"));
    root->addChild(picker_container);

    // maximum color
    RendererAddColorPicker(picker_container, "Max", CEGUI::Colour(1.0f, 0.0f, 0.0f, 1.0f),
        [newRenderer](const CEGUI::EventArgs &e)->bool
        {
            const CEGUI::WindowEventArgs &wargs = static_cast<const CEGUI::WindowEventArgs&>(e);
            CEGUI::ColourPicker* picker = static_cast<CEGUI::ColourPicker*>(wargs.window);
            CEGUI::Colour c = picker->getColour();
            newRenderer->SetMaxColor(c.getRed(), c.getGreen(), c.getBlue(), c.getAlpha());
            return true;
        }
    );

    // minimum color
    RendererAddColorPicker(picker_container, "Min", CEGUI::Colour(0.0f, 0.0f, 0.8f, 1.0f),
        [newRenderer](const CEGUI::EventArgs &e)->bool
        {
            const CEGUI::WindowEventArgs &wargs = static_cast<const CEGUI::WindowEventArgs&>(e);
            CEGUI::ColourPicker* picker = static_cast<CEGUI::ColourPicker*>(wargs.window);
            CEGUI::Colour c = picker->getColour();
            newRenderer->SetMinColor(c.getRed(), c.getGreen(), c.getBlue(), c.getAlpha());
            return true;
        }
    );

    /* ---------------------------------
        Probabilities-specific controls
       --------------------------------- */

    /* Start Point for selecting probabilities */
    RendererAddLabel(root, CEGUI::USize(CEGUI::UDim(1, 0), CEGUI::UDim(0, 30)), "Start point:", "Lower-left corner of region to render probabilities from");

    CEGUI::Window* startContainer = CEGUI::WindowManager::getSingleton().createWindow("HorizontalLayoutContainer");
    root->addChild(startContainer);
    startContainer->setSize(CEGUI::USize(CEGUI::UDim(1.0, 0), CEGUI::UDim(0, 40)));

    CEGUI::Editbox* startX = RendererAddEditbox(startContainer, "1.0", "Start X Coord");
    CEGUI::Editbox* startY = RendererAddEditbox(startContainer, "1.0", "Start Y Coord");
    CEGUI::Editbox* startZ = RendererAddEditbox(startContainer, "1.0", "Start Z Coord");

    std::function<bool(const CEGUI::EventArgs&)> updateStartPoint = [this, newRenderer, startX, startY, startZ](const CEGUI::EventArgs &e)->bool
    {
        double newX, newY, newZ;
        std::stringstream sstm;
        sstm << startX->getText() << " " << startY->getText() << " " << startZ->getText();
        sstm >> newX;
        sstm >> newY;
        sstm >> newZ;
        (static_cast<ProbabilitiesRenderer*>(newRenderer))->SetStartPoint(newX, newY, newZ);
        // in case new changes were rejected, get values again so they can be displayed in the editboxes
        std::vector<double> newPoint = (static_cast<ProbabilitiesRenderer*>(newRenderer))->GetStartPoint();
        startX->setText(std::to_string(newPoint[0]));
        startY->setText(std::to_string(newPoint[1]));
        startZ->setText(std::to_string(newPoint[2]));
        newRenderer->PrepareGeometry(this->_dataProvider);
        return true;

    };
    startX->subscribeEvent(CEGUI::Editbox::EventTextAccepted, updateStartPoint);
    startY->subscribeEvent(CEGUI::Editbox::EventTextAccepted, updateStartPoint);
    startZ->subscribeEvent(CEGUI::Editbox::EventTextAccepted, updateStartPoint);

    /* End Point for selecting probabilities */
    RendererAddLabel(root, CEGUI::USize(CEGUI::UDim(1, 0), CEGUI::UDim(0, 30)), "End point:", "Upper-right corner of region to render probabilities from");

    CEGUI::Window* endContainer = CEGUI::WindowManager::getSingleton().createWindow("HorizontalLayoutContainer");
    root->addChild(endContainer);
    endContainer->setSize(CEGUI::USize(CEGUI::UDim(1.0, 0.0), CEGUI::UDim(0.0, 40)));

    CEGUI::Editbox* endX = RendererAddEditbox(endContainer, "5.0", "End X Coord");
    CEGUI::Editbox* endY = RendererAddEditbox(endContainer, "5.0", "End Y Coord");
    CEGUI::Editbox* endZ = RendererAddEditbox(endContainer, "5.0", "End Z Coord");

    std::function<bool(const CEGUI::EventArgs&)> updateEndPoint = [this, newRenderer, endX, endY, endZ](const CEGUI::EventArgs &e)->bool
    {
        double newX, newY, newZ;
        std::stringstream sstm;
        sstm << endX->getText() << " " << endY->getText() << " " << endZ->getText();
        sstm >> newX;
        sstm >> newY;
        sstm >> newZ;
        (static_cast<ProbabilitiesRenderer*>(newRenderer))->SetEndPoint(newX, newY, newZ);
        newRenderer->PrepareGeometry(this->_dataProvider);
        return true;

    };
    endX->subscribeEvent(CEGUI::Editbox::EventTextAccepted, updateEndPoint);
    endY->subscribeEvent(CEGUI::Editbox::EventTextAccepted, updateEndPoint);
    endZ->subscribeEvent(CEGUI::Editbox::EventTextAccepted, updateEndPoint);
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
    CEGUI::System::getSingleton().getDefaultGUIContext().setDefaultFont("DejaVuSans-12-NoScale");
    CEGUI::System::getSingleton().getDefaultGUIContext().setDefaultTooltipType("TaharezLook/Tooltip");
    // force CEGUI's mouse position to (0,0)     /// TODO: do this in InputManager
    CEGUI::Vector2<float> mousePos = CEGUI::System::getSingleton().getDefaultGUIContext().getMouseCursor().getPosition();
    CEGUI::System::getSingleton().getDefaultGUIContext().injectMouseMove(-mousePos.d_x, -mousePos.d_y);

    // set root window
    CEGUI::System::getSingleton().getDefaultGUIContext().setRootWindow(guiRoot);
    guiRoot->setMousePassThroughEnabled(true);

    // load 'loading' popup
    CEGUI::Window* lWnd = CEGUI::WindowManager::getSingleton().loadLayoutFromFile("loading.layout");
    lWnd->setVisible(false);

    // load default window layout
    CEGUI::Window* fWnd = CEGUI::WindowManager::getSingleton().loadLayoutFromFile("default.layout");
    guiRoot->addChild(lWnd);
    guiRoot->addChild(fWnd);
    CEGUI::Window* data_window = fWnd->getChildRecursive("data_window"); // main window holding timestep controls, etc.

    // Configure the Load VTK button
    fWnd->getChildRecursive("LoadVTKbtn")->subscribeEvent(CEGUI::PushButton::EventClicked, 
                         [this, data_window](const CEGUI::EventArgs &e)->bool {
                            nfdchar_t* outPath = NULL;
                            nfdresult_t result = NFD_OpenDialog("vtk", NULL, &outPath);
                            setlocale(LC_NUMERIC, "C"); // GTK stomps on our locale and can break CEGUI, so reset it here

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
                            this->_dataProvider->NextTimeStepAsync();
                            this->_waitingForProvider = true;
                            this->ShowLoadingPopup(this->_waitingForProvider);
                            return true;
                        }
    );
    fWnd->getChildRecursive("btnPrevTimeStep")->subscribeEvent(CEGUI::PushButton::EventClicked,
                        [this, data_window](const CEGUI::EventArgs &e)->bool {
                            this->_dataProvider->PrevTimeStepAsync();
                            this->_waitingForProvider = true;
                            this->ShowLoadingPopup(this->_waitingForProvider);
                            return true;
                        }
    );
    fWnd->getChildRecursive("btnPlay")->subscribeEvent(CEGUI::PushButton::EventClicked,
                        [this](const CEGUI::EventArgs &e)->bool {
                            const CEGUI::WindowEventArgs &wargs = static_cast<const CEGUI::WindowEventArgs&>(e);
                            if (this->_autoplay)
                            {
                                wargs.window->setText(">>");
                            }
                            else
                            {
                                wargs.window->setText("||");
                            }
                            this->_autoplay = !this->_autoplay;
                            return true;
                        }
    );


    CEGUI::Window* add_popup = this->AddRendererPopup();
    // Configure the Add Renderer button
    fWnd->getChildRecursive("AddRendererbtn")->subscribeEvent(CEGUI::PushButton::EventClicked,
                        [add_popup](const CEGUI::EventArgs &e)->bool {
                            add_popup->show();
                            return true;
                        }
    );

    CEGUI::Window* settings_wnd = this->AddSettingsPopup();
    // Configure the Settings button
    fWnd->getChildRecursive("settings_btn")->subscribeEvent(CEGUI::PushButton::EventClicked,
                        [settings_wnd](const CEGUI::EventArgs &e)->bool {
                            settings_wnd->show();
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
    _axesShader.loadAndLink("shaders/_coordinateAxes.vertex", "shaders/_coordinateAxes.fragment");
    _axesShader.addUniform("MVP");

    _backgroundShader.loadAndLink("shaders/_gradient.vertex", "shaders/_gradient.fragment");
    _backgroundShader.addUniform("startColor");
    _backgroundShader.addUniform("endColor");

    _scalarMapShader.loadAndLink("shaders/scalarGradientMap1D.vertex", "shaders/scalarGradientMap1D.fragment");
    _scalarMapShader.addUniform("MVP");
    _scalarMapShader.addUniform("min_scalar");
    _scalarMapShader.addUniform("max_scalar");
    _scalarMapShader.addUniform("min_sizeScalar");
    _scalarMapShader.addUniform("max_sizeScalar");
    _scalarMapShader.addUniform("hotColor");
    _scalarMapShader.addUniform("coldColor");
    _scalarMapShader.addUniform("bias");         // used for exponential interpolation
    _scalarMapShader.addUniform("interpolator"); // used to select an interpolation mode
}

void Compositor::LoadVTK(std::string filename, CEGUI::Window* vtkWindowRoot)
{
    if (_dataProvider)
    {
        delete _dataProvider;
    }

    ShowLoadingPopup(true);
    _waitingForProvider = true;
    _dataProvider = new vtkLegacyReader(filename, [this, vtkWindowRoot](DataProvider* P){
        this->CenterCameraOnExtents(P->GetExtents());
    });
}

void Compositor::LoadLBM(std::string filename, CEGUI::Window* dataWindowRoot)
{
    if (_dataProvider)
    {
        delete _dataProvider;
    }

    ShowLoadingPopup(true);
    _waitingForProvider = true;
    _dataProvider = new lbsimWrapper(filename, [this, dataWindowRoot](DataProvider* P){
        this->CenterCameraOnExtents(P->GetExtents());
    });
}

void Compositor::UpdateDataGUI(CEGUI::Window* dataWindowRoot)
{
    vtkLegacyReader* legacyReader = dynamic_cast<vtkLegacyReader*>(_dataProvider);
    if (legacyReader)
    {
        dataWindowRoot->setText(legacyReader->GetFileName());
    }

    lbsimWrapper* lbsim = dynamic_cast<lbsimWrapper*>(_dataProvider);
    if (lbsim)
    {
        dataWindowRoot->setText(lbsim->GetFileName());
    }

    CEGUI::Window* timestep_label = dataWindowRoot->getChildRecursive("lblTimestep");
    CEGUI::Window* maxTimestep_label = dataWindowRoot->getChildRecursive("lblMaxTimestep");

    if (_dataProvider->GetTimeStep() >= 0)
        timestep_label->setText(std::to_string(_dataProvider->GetTimeStep()));
    else
        timestep_label->setText("N/A");

    if (_dataProvider->GetMaxTimeStep() > 0)
        maxTimestep_label->setText(std::to_string(_dataProvider->GetMaxTimeStep() - 1));
    else
        maxTimestep_label->setText("--");

}

CEGUI::Window* Compositor::AddRendererPopup()
{
    CEGUI::Window* addWnd = CEGUI::WindowManager::getSingleton().loadLayoutFromFile("add_renderer.layout");
    CEGUI::Listbox* renderer_list = static_cast<CEGUI::Listbox*>(addWnd->getChildRecursive("renderer_list"));

    // set selection highlight to a half transparent blue to red gradient.
    CEGUI::Colour selectColor1(0.0f, 0.8f, 0.5f, 0.4f);
    CEGUI::Colour selectColor2(0.1f, 0.0f, 0.0f, 0.1f);

    // Add list of renderers w/ IDs
    for (unsigned int i = 2; i < this->RendererStrs.size(); i++) /// HACK: start from 2 to skip Axes & Gradient Renderers
    {
        CEGUI::ListboxTextItem* renderer_entry = new CEGUI::ListboxTextItem(this->RendererStrs[i], i);
        renderer_entry->setSelectionColours(selectColor1, selectColor2, selectColor1, selectColor2);
        renderer_entry->setSelectionBrushImage("TaharezLook/ListboxSelectionBrush");
        renderer_list->addItem(renderer_entry);
    }

    // setup event for the AddRenderer button
    addWnd->getChildRecursive("btnAddRenderer")->subscribeEvent(CEGUI::PushButton::EventClicked,
                        [this, addWnd, renderer_list](const CEGUI::EventArgs &e)->bool {
                            CEGUI::ListboxItem* selected = renderer_list->getFirstSelectedItem();

                            if (selected)
                            {
                                this->AddRenderer((Renderers)selected->getID(), true);
                                addWnd->hide();
                            }
                            return true;
                        }
    );

    // setup event for the Cancel button
    addWnd->getChildRecursive("btnCancel")->subscribeEvent(CEGUI::PushButton::EventClicked,
                        [this, addWnd](const CEGUI::EventArgs &e)->bool {
                              addWnd->hide();
                            return true;
                        }
    );

    _guiRoot->addChild(addWnd);
    addWnd->setPosition(CEGUI::UVector2(CEGUI::UDim(0.5, 0), CEGUI::UDim(0.5, 0)));

    return addWnd;
}


CEGUI::Window* Compositor::AddSettingsPopup()
{
    CEGUI::Window* addWnd = CEGUI::WindowManager::getSingleton().loadLayoutFromFile("settings-dialog.layout");
    CEGUI::ToggleButton* axes_toggle = static_cast<CEGUI::ToggleButton*>(addWnd->getChildRecursive("chk_showAxes"));
    CEGUI::Window* color_container = static_cast<CEGUI::Window*>(addWnd->getChildRecursive("bgColor_container"));

    // configure close button
    addWnd->subscribeEvent(CEGUI::FrameWindow::EventCloseClicked,
        [] (const CEGUI::EventArgs &e)->bool
        {
            static_cast<const CEGUI::WindowEventArgs&>(e).window->setVisible(false);
            return true;
        }
    );

    // configure AxesRenderer toggle switch
    axes_toggle->subscribeEvent(CEGUI::ToggleButton::EventSelectStateChanged,
        [axes_toggle, this] (const CEGUI::EventArgs &e)->bool
        {
            if (axes_toggle->isSelected())
                this->_systemRenderers[1]->Enable();
            else
                this->_systemRenderers[1]->Disable();
            return true;
        }
    );

    // configure background color selection
    float startColor[4] = { 0.0f, 0.1f, 0.15f, 1.0f };  /// TODO: Load these from a configuration file someday
    float endColor[4]   = { 0.4f, 0.4f, 0.34f, 1.0f };
    RendererAddColorPicker(color_container, "Bot", CEGUI::Colour(startColor[0], startColor[1], startColor[2], startColor[3]), 
        [this] (const CEGUI::EventArgs &e)->bool
        {
            const CEGUI::WindowEventArgs &wargs = static_cast<const CEGUI::WindowEventArgs&>(e);
            CEGUI::ColourPicker* picker = static_cast<CEGUI::ColourPicker*>(wargs.window);
            CEGUI::Colour c = picker->getColour();
            static_cast<GradientRenderer*>(this->_systemRenderers[0])->SetStartColor(c.getRed(), c.getGreen(), c.getBlue(), c.getAlpha());
            return true;
        }
    );

    RendererAddColorPicker(color_container, "Top", CEGUI::Colour(endColor[0], endColor[1], endColor[2], endColor[3]), 
        [this] (const CEGUI::EventArgs &e)->bool
        {
            const CEGUI::WindowEventArgs &wargs = static_cast<const CEGUI::WindowEventArgs&>(e);
            CEGUI::ColourPicker* picker = static_cast<CEGUI::ColourPicker*>(wargs.window);
            CEGUI::Colour c = picker->getColour();
            static_cast<GradientRenderer*>(this->_systemRenderers[0])->SetEndColor(c.getRed(), c.getGreen(), c.getBlue(), c.getAlpha());
            return true;
        }
    );


/*
    // setup event for the AddRenderer button
    addWnd->getChildRecursive("btnAddRenderer")->subscribeEvent(CEGUI::PushButton::EventClicked,
                        [this, addWnd, renderer_list](const CEGUI::EventArgs &e)->bool {
                            CEGUI::ListboxItem* selected = renderer_list->getFirstSelectedItem();

                            if (selected)
                            {
                                this->AddRenderer((Renderers)selected->getID(), true);
                                addWnd->hide();
                            }
                            return true;
                        }
    );
*/
/*
    // setup event for the Cancel button
    addWnd->getChildRecursive("btnCancel")->subscribeEvent(CEGUI::PushButton::EventClicked,
                        [this, addWnd](const CEGUI::EventArgs &e)->bool {
                              addWnd->hide();
                            return true;
                        }
    );
*/
    _guiRoot->addChild(addWnd);
    addWnd->setPosition(CEGUI::UVector2(CEGUI::UDim(0.5, 0), CEGUI::UDim(0.5, 0)));

    return addWnd;
}

void Compositor::ShowLoadingPopup(bool show)
{
    CEGUI::Window* loadingPopup = _guiRoot->getChildRecursive("loadingInfo_window");
    loadingPopup->setVisible(show);
}

void Compositor::UpdateRenderers(DataProvider* provider)
{
    for (auto r : _renderers)
    {
        r->PrepareGeometry(provider);
    }
}

void Compositor::Update()
{
    // if the dataprovider has data we were waiting for, update renderers
    if (_waitingForProvider && _dataProvider->isReady())
    {
        _waitingForProvider = false;
        try
        {
            UpdateDataGUI(_guiRoot->getChildRecursive("data_window"));
            UpdateRenderers(_dataProvider);
        }
        catch (const ProviderBusy&)
        {
            _waitingForProvider = true;
        }

        ShowLoadingPopup(_waitingForProvider);
    }
}

void Compositor::Render(glm::mat4 MVP)
{
    static double timer = 0.0;

    // tell CEGUI how long its been since the last frame
    double dt = this->DeltaTime();
    CEGUI::System::getSingleton().injectTimePulse((float)dt);
    CEGUI::System::getSingleton().getDefaultGUIContext().injectTimePulse((float)dt);
    _lastFrameTime = glfwGetTime();

    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (auto r : _systemRenderers)
    {
        r->Draw(MVP);
    }

    for (auto r : _renderers)
    {
        r->Draw(MVP);
    }

    glDisable(GL_DEPTH_TEST); // no depth testing for GUIs
    // render GUI -- must be the LAST drawing call we make!
    CEGUI::System::getSingleton().renderAllGUIContexts();

    timer += dt;
    if (_autoplay && timer >= _autoplay_interval && !_waitingForProvider)
    {
        timer = 0;

        // if we've reached the maximal timestep, stop autoplaying
        if (!_waitingForProvider && (int)(_dataProvider->GetMaxTimeStep() - 1) == _dataProvider->GetTimeStep())
        {
            _autoplay = false;
            _guiRoot->getChildRecursive("btnPlay")->setText(">>"); /// HACK: Should probably have start/stop autoplaying functions to avoid so many hardcoded strings...
        }
        else
        {
            _dataProvider->NextTimeStepAsync();
            _waitingForProvider = true;
        }

        ShowLoadingPopup(_waitingForProvider);
    }
}

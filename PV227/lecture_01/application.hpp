#pragma once
#include "camera_ubo.hpp"
#include "light_ubo.hpp"
#include "pv227_application.hpp"
#include "scene_object.hpp"

class Application : public PV227Application {
    // ----------------------------------------------------------------------------
    // Variables (Geometry)
    // ----------------------------------------------------------------------------
protected:
    /** The scene object representing the light. */
    SceneObject light_object;
    /** The scene object representing the floor. */
    SceneObject floor_object;
    /** The remaining scene objects. */
    std::vector<SceneObject> scene_objects;

    // ----------------------------------------------------------------------------
    // Variables (Textures)
    // ----------------------------------------------------------------------------
protected:
    /** The wood texture. */
    GLuint wood_tex;
    /** The Lenna texture. */
    GLuint lenna_tex;

    // ----------------------------------------------------------------------------
    // Variables (Light)
    // ----------------------------------------------------------------------------
protected:
    /** The UBO storing the data about lights - positions, colors, etc. */
    PhongLightsUBO phong_lights_ubo;

    // ----------------------------------------------------------------------------
    // Variables (Camera)
    // ----------------------------------------------------------------------------
protected:
    /** The UBO storing the information about camera. */
    CameraUBO camera_ubo;

    // ----------------------------------------------------------------------------
    // Variables (Shaders)
    // ----------------------------------------------------------------------------
protected:
    /** The shader program used to render the extended objects in black. */
    ShaderProgram expand_program;

    // ----------------------------------------------------------------------------
    // Variables (GUI)
    // ----------------------------------------------------------------------------
protected:
    /** The light position set in the GUI. */
    float gui_light_position = glm::radians(60.f);

    // ----------------------------------------------------------------------------
    // Constructors
    // ----------------------------------------------------------------------------
public:
    Application(int initial_width, int initial_height, std::vector<std::string> arguments = {});

    /** Destroys the {@link Application} and releases the allocated resources. */
    virtual ~Application();

    // ----------------------------------------------------------------------------
    // Shaders
    // ----------------------------------------------------------------------------
    /**
     * {@copydoc PV227Application::compile_shaders}
     */
    void compile_shaders() override;

    // ----------------------------------------------------------------------------
    // Initialize Scene
    // ----------------------------------------------------------------------------
public:
    /** Prepares the required cameras. */
    void prepare_cameras();
    /** Prepares the required textures. */
    void prepare_textures();
    /** Prepares the lights. */
    void prepare_lights();
    /** Prepares the scene objects. */
    void prepare_scene();

    // ----------------------------------------------------------------------------
    // Update
    // ----------------------------------------------------------------------------
    /**
     * {@copydoc PV227Application::update}
     */
    void update(float delta) override;

    // ----------------------------------------------------------------------------
    // Render
    // ----------------------------------------------------------------------------
public:
    /** @copydoc PV227Application::render */
    void render() override;
    /** Renders the whole scene. */
    void render_scene();
    /**
     * Renders a single object using the specified program.
     *
     * @param 	object 	The object to render.
     * @param 	program	The program to be used.
     */
    void render_object(SceneObject& object, ShaderProgram& program) const;

    // ----------------------------------------------------------------------------
    // GUI
    // ----------------------------------------------------------------------------
public:
    /** @copydoc PV227Application::render_ui */
    void render_ui() override;
};

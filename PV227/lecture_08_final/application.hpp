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
    /** The geometry or a teapot defined using patches with 16 vertices each.*/
    TeapotPatches teapot_patches;
    /** The scene object representing the light. */
    SceneObject light_object;
    /** The scene object representing the floor. */
    SceneObject teapot_object;

    // ----------------------------------------------------------------------------
    // Variables (Textures)
    // ----------------------------------------------------------------------------
protected:
    /** The chesterfield texture containing the color information. */
    GLuint chesterfield_color_tex;
    /** The chesterfield texture containing the normal information. */
    GLuint chesterfield_normal_tex;
    /** The chesterfield texture containing the height information. */
    GLuint chesterfield_height_tex;

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

    // ----------------------------------------------------------------------------
    // Variables (GUI)
    // ----------------------------------------------------------------------------
protected:
    /** The light position set in the GUI. */
    float gui_light_position = glm::radians(60.f);
    /** The flag determining if we render the geometry using wireframe or fill geometry. */
    bool use_wireframe = false;

    /** The flag determining if we use normal mapping. */
    bool use_normal_mapping = false;
     /** The strength of the normal mapping. */
    float normal_factor = 1.0f;
    /** The flag determining if we use displacement mapping. */
    bool use_displacement_mapping = false;
     /** The strength of the displacement mapping. */
    float displacement_factor = 0.1f;
   

    /** The flag determining if the automatic or manual tessellation should be used. */
    bool use_auto_tessellation = false;
    /** The flag determining the amount of details during the manual tessellation. */
    float tessellation_factor = 10.f;
    /** The flag determining the requested triangle size for the automatic tessellation. */
    float triangle_size = 10.f;;

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

    /** Renders the teapot. */
    void render_teapot() const;

    /** Renders the light. */
    void render_light() const;

    // ----------------------------------------------------------------------------
    // GUI
    // ----------------------------------------------------------------------------
public:
    /** @copydoc PV227Application::render_ui */
    void render_ui() override;
};

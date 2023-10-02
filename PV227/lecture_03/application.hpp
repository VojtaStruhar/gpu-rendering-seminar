#pragma once

#include "camera_ubo.hpp"
#include "light_ubo.hpp"
#include "phong_material_ubo.hpp"
#include "pv227_application.hpp"
#include "scene_object.hpp"

class Application : public PV227Application {
    // ----------------------------------------------------------------------------
    // Variables (Geometry)
    // ----------------------------------------------------------------------------
protected:
    /** The scene objects. */
    std::vector<SceneObject> scene_objects;

    /** Parameters for generating the random scene. */
    glm::ivec3 grid_size = glm::ivec3(21, 1, 21);
    glm::ivec3 grid_start = glm::ivec3(-50, 0, -50);
    glm::ivec3 grid_spacing = glm::ivec3(5, 0, 5);

    // ----------------------------------------------------------------------------
    // Variables (Materials)
    // ----------------------------------------------------------------------------
protected:
    // The list of materials from which we will randomly choose materials for each object.
    std::vector<PhongMaterialUBO> color_ubos;

    // ----------------------------------------------------------------------------
    // Variables (Textures)
    // ----------------------------------------------------------------------------
protected:
    // The list of textures which we can choose for our scene.
    std::vector<GLuint> textures;

    // ----------------------------------------------------------------------------
    // Variables (Light)
    // ----------------------------------------------------------------------------
protected:

    /** The macro for setting the maximum number of lights. */
#define LIGHTS_MAX_COUNT 150
    /** The macro for setting the buffer type. Choose between GL_UNIFORM_BUFFER and GL_SHADER_STORAGE_BUFFER. */
#define LIGHTS_STORAGE GL_UNIFORM_BUFFER
    /** The UBO storing the data about lights - positions, colors, etc. */
    PhongLightsUBO phong_lights_ubo;
    std::vector<PhongLightData> all_lights;

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
    /** The shader program using forward shading. */
    ShaderProgram forward_program;
    /** The shader program used to fill G-buffer with required information for deferred shading. */
    ShaderProgram deferred_program;
    /** The shader program used to display the textures. */
    ShaderProgram display_texture_program;
    /** The shader program using deferred shading (by evaluating the screen quad). */
    ShaderProgram evaluate_quad_program;
    /** The shader program using deferred shading (by evaluating spheres). */
    ShaderProgram evaluate_sphere_program;

    // ----------------------------------------------------------------------------
    // Variables (Frame Buffers)
    // ----------------------------------------------------------------------------
protected:
    /** The framebuffer object that is used when rendering into the G-buffer. */
    GLuint gbuffer_fbo;
    /** The texture used in @link gbuffer_fbo to store positions. */
    GLuint gbuffer_position_texture = 0;
    /** The texture used in @link gbuffer_fbo to store normals. */
    GLuint gbuffer_normal_texture = 0;
    /** The texture used in @link gbuffer_fbo to store albedo. */
    GLuint gbuffer_albedo_texture = 0;
    /** The texture used in @link gbuffer_fbo to store depth. */
    GLuint gbuffer_depth_texture = 0;

    /** The framebuffer object that is used when evaluating deferred shading. */
    GLuint normal_scene_fbo;
    /** The texture used in @link normal_scene_fbo to store the final color. */
    GLuint final_color_texture = 0;

    // ----------------------------------------------------------------------------
    // Variables (GUI)
    // ----------------------------------------------------------------------------
protected:
    /** The constants identifying what can be displayed on the screen. */
    const int FORWARD_SHADING = 0;
    const int DISPLAY_POSITION = 1;
    const int DISPLAY_NORMAL = 2;
    const int DISPLAY_ALBEDO = 3;
    const int DEFERRED_SHADING_EVALUATE_QUAD = 4;
    const int DEFERRED_SHADING_EVALUATE_SPHERE = 5;
    /** The GUI labels for the constants above. */
    const char* DISPLAY_LABELS[6] = {"Forward Shading", "Positions", "Normals", "Albedo", "Deferred Shading - Quad", "Deferred Shading - Spheres"};

    /** The flag determining what will be displayed on the screen right now. */
    int what_to_display = FORWARD_SHADING;

    /** The requested number of lights. */
    int number_of_lights = 10;

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
    /** Prepares the required materials. */
    void prepare_materials();
    /** Prepares the required textures. */
    void prepare_textures();
    /** Prepares the lights. */
    void prepare_lights();
    /** Prepares the scene objects. */
    void prepare_scene();
    /** Prepares the frame buffer objects. */
    void prepare_framebuffers();
    /** Resizes the full screen textures match the window. */
    void resize_fullscreen_textures();

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
    /**
     * Renders the whole scene.
     *
     * @param 	program	The program to used when rendering the scene.
     */
    void render_scene(ShaderProgram& program);
    /** Sets up the frame buffer and renders all objects into it using forward shading. */
    void render_using_forward_shading();
    /** Sets up the G-buffer and renders all objects into it using deferred shading. */
    void render_into_g_buffer();
    /**
     * Sets up the frame buffer and renders one of the textures stored in G-buffer (i.e., position texture, normal
     * texture, or albedo texture).
     */
    void show_g_buffer_textures();
    /** Sets up the frame buffer and evaluates the lighting using a full-screen quad. */
    void evaluate_lighting_using_quad();
    /** Sets up the frame buffer and evaluates the lighting using spheres. */
    void evaluate_lighting_using_spheres();

    // ----------------------------------------------------------------------------
    // GUI
    // ----------------------------------------------------------------------------
public:
    /** @copydoc PV227Application::render_ui */
    void render_ui() override;

    // ----------------------------------------------------------------------------
    // Input Events
    // ----------------------------------------------------------------------------
public:
    /** @copydoc PV227Application::on_resize */
    void on_resize(int width, int height) override;
};

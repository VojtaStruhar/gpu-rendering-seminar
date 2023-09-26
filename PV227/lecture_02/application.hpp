#pragma once
#include "camera_ubo.hpp"
#include "light_ubo.hpp"
#include "model_ubo.hpp"
#include "pv227_application.hpp"
#include "scene_object.hpp"

class Application : public PV227Application {

    // ----------------------------------------------------------------------------
    // Variables (Geometry)
    // ----------------------------------------------------------------------------
  protected:
    /** The scene object representing the light. */
    SceneObject light_object;
    /** The scene object representing the torus #1. */
    SceneObject torus_object_1;
    /** The scene object representing the torus #2. */
    SceneObject torus_object_2;
    /** The scene object representing the torus #3. */
    SceneObject torus_object_3;
    /** The remaining scene objects. */
    std::vector<SceneObject> scene_objects;

    // ----------------------------------------------------------------------------
    // Variables (Textures)
    // ----------------------------------------------------------------------------
  protected:
    /** The wood texture. */
    GLuint wood_tex;

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
    /** The UBO storing the information about the light camera. */
    CameraUBO light_camera_data_ubo;
    /** The projection matrix of the light camera. */
    glm::mat4 light_camera_projection;
    /** The view matrix of the light camera. */
    glm::mat4 light_camera_view;

    // ----------------------------------------------------------------------------
    // Variables (Shaders)
    // ----------------------------------------------------------------------------
  protected:
    /** The shader program used to generate the shadow map. */
    ShaderProgram generate_shadow_program;
    /** The shader program used to display the shadow texture. */
    ShaderProgram display_texture_program;

    // ----------------------------------------------------------------------------
    // Variables (Frame Buffers)
    // ----------------------------------------------------------------------------
  protected:
    /** The framebuffer object that is used when rendering into the shadow texture. */
    GLuint shadow_fbo;
    /** The width and height of the shadow texture. */
    int shadow_tex_size = 1024;
    /** The shadow texture. */
    GLuint shadow_texture;
    /** The pre-multiplied shadow matrix. */
    glm::mat4 shadow_matrix;

    // ----------------------------------------------------------------------------
    // Variables (GUI)
    // ----------------------------------------------------------------------------
  protected:
    /** The light position set in the GUI. */
    float gui_light_position = glm::radians(45.f);

    /** The constants identifying what can be displayed on the screen. */
    static const int DISPLAY_VIEW_FROM_MAIN_CAMERA = 0;
    static const int DISPLAY_VIEW_FROM_LIGHT = 1;
    static const int DISPLAY_SHADOW_TEXTURE = 2;
    /** The GUI labels for the constants above. */
    const char* DISPLAY_LABELS[3] = {"Main Camera", "Light View", "Shadow Texture"};

    /** The flag determining what will be displayed on the screen right now. */
    int what_to_display = DISPLAY_VIEW_FROM_MAIN_CAMERA;

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
    /** Prepares the frame buffer objects. */
    void prepare_framebuffers();

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
     * @param 	from_light 	The flag that determines if we render the scene from the light camera.
     * @param 	gen_shadows	The flag that determines if we generate the shadows.
     */
    void render_scene(bool from_light, bool gen_shadows);
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

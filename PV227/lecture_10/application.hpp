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
    /** The scene objects. */
    SceneObject scene_objects[64];

    // Materials for the objects.
    glm::vec3 material_diffuses[64];
    glm::vec3 material_fresnels[64];
    float material_roughnesses[64];

    // UBO with random floats for sampling.
    GLuint random_floats_ubo;

    // ----------------------------------------------------------------------------
    // Variables (Textures)
    // ----------------------------------------------------------------------------
  protected:
    /** The mask texture. */
    GLuint mask_tex;
    /** The meadow skybox texture. */
    GLuint meadow_skybox_tex;
    /** The miramar skybox texture. */
    GLuint miramar_skybox_tex;
    /** The currently selected skybox texture. */
    GLuint current_skybox_tex;

    // ----------------------------------------------------------------------------
    // Variables (Light)
    // ----------------------------------------------------------------------------
  protected:
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
    ShaderProgram depth_prepass_program;
    ShaderProgram skybox_program;
    ShaderProgram object_program;

    // ----------------------------------------------------------------------------
    // Variables (GUI)
    // ----------------------------------------------------------------------------
  protected:
    /** The constants identifying what skybox texture to use. */
    static const int SKYBOX_MEADOW = 0;
    static const int SKYBOX_MIRAMAR = 1;
    /** The GUI labels for the constants above. */
    const char* SKYBOX_LABELS[2] = {"Medow", "Miramar"};
    /** The currently used skybox texture.*/
    int skybox_texture = SKYBOX_MIRAMAR;

    /** The constants identifying what object will be depicted in the middle. */
    static const int CENTRAL_OBJECT_CUBE = 0;
    static const int CENTRAL_OBJECT_SPHERE = 1;
    static const int CENTRAL_OBJECT_TORUS = 2;
    static const int CENTRAL_OBJECT_CYLINDER = 3;
    static const int CENTRAL_OBJECT_CAPSULE = 4;
    static const int CENTRAL_OBJECT_TEAPOT = 5;
    /** The GUI labels for the constants above. */
    const char* CENTRAL_OBJECT_LABELS[6] = {"Cube", "Sphere", "Torus", "Cylinder", "Capsule", "Teapot"};
    /** The currently displayed object in the middle. */
    int central_object = CENTRAL_OBJECT_TEAPOT;

    /** The amount of samples used for diffuse lightning. */
    int diffuse_sample_count = 64;
    /** The amount of samples used for specular lightning. */
    int specular_sample_count = 128;
    /** The flag determining whether a mask should be used. */
    bool mask_roughness = false;
    /** The flag determining whether a shiny layer should be depicted. */
    bool use_shiny_layer = false;
    /** The flag determining whether the shiny layer should be using a mask. */
    bool use_shiny_layer_mask = false;

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

    /** Prepares the scene objects. */
    void prepare_scene();

    /** Computes random sampling. */
    void compute_random_sampling();

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

    /** Renders the skybox. */
    void render_skybox() const;

    /** Renders the selected objects. */
    void render_objects() const;

    // ----------------------------------------------------------------------------
    // GUI
    // ----------------------------------------------------------------------------
  public:
    /** @copydoc PV227Application::render_ui */
    void render_ui() override;
};

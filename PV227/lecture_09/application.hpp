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

    /** The model matrix for the central object.*/
    ModelUBO central_object_ubo;

    // ----------------------------------------------------------------------------
    // Variables (Textures)
    // ----------------------------------------------------------------------------
  protected:
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
    ShaderProgram nolit_program;
    ShaderProgram blinn_phong_original_program;
    ShaderProgram blinn_phong_normalized_program;
    ShaderProgram cook_torrance_program;
    ShaderProgram oren_nayar_program;
    ShaderProgram ashikhmin_shirley_program;

    // ----------------------------------------------------------------------------
    // Variables (GUI)
    // ----------------------------------------------------------------------------
  protected:
    /** The light position and intensities set in the GUI. */
    float gui_light_position = glm::radians(60.f);
    float light_intensity = 1.0f;
    float ambient_light_intensity = 0.1f;

    /** The material settings. */
    glm::vec3 material_diffuse = glm::vec3(1.f, 0.f, 0.f);
    glm::vec3 material_fresnel = glm::vec3(0.1f, 0.1f, 0.1f);
    glm::vec3 material_specular = glm::vec3(1.f, 1.f, 1.f);
    float material_roughness = 0.5f;
    float material_shininess = 10;
    float material_shininess_tangent = 10;
    float material_shininess_bitangent = 10;

    /** The constants identifying individual presents. */
    static const int MATERIAL_PRESET_USER = 0;          // User values
    static const int MATERIAL_PRESET_PLASTIC_USER = 1;  // Fresnel = 0.04, user diffuse
    static const int MATERIAL_PRESET_PLASTIC_RED = 2;   // Fresnel = 0.04, red diffuse
    static const int MATERIAL_PRESET_PLASTIC_GREEN = 3; // Fresnel = 0.04, green diffuse
    static const int MATERIAL_PRESET_PLASTIC_BLUE = 4;  // Fresnel = 0.04, blue diffuse
    static const int MATERIAL_PRESET_IRON = 5;          // Fresnel = (0.56, 0.57, 0.58), diffuse = 0.0
    static const int MATERIAL_PRESET_COPPER = 6;        // Fresnel = (0.95, 0.64, 0.54), diffuse = 0.0
    static const int MATERIAL_PRESET_GOLD = 7;          // Fresnel = (1.00, 0.71, 0.29), diffuse = 0.0
    static const int MATERIAL_PRESET_ALUMINUM = 8;      // Fresnel = (0.91, 0.92, 0.92), diffuse = 0.0
    static const int MATERIAL_PRESET_SILVER = 9;        // Fresnel = (0.95, 0.93, 0.88), diffuse = 0.0

    /** The GUI labels for the constants above. */
    const char* MATERIAL_PRESET_LABELS[10] = {"User defined", "Plastic, user color", "Plastic red", "Plastic green", "Plastic blue", "Iron", "Copper", "Gold", "Aluminum", "Silver"};

    /** The current material preset. */
    int material_preset = MATERIAL_PRESET_USER;

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
    int central_object = CENTRAL_OBJECT_SPHERE;

    /** The constants defining individual materials (i.e., shaders). */
    static const int MATERIAL_TYPE_BLINN_PHONG_ORIGINAL = 0;
    static const int MATERIAL_TYPE_BLINN_PHONG_NORMALIZED = 1;
    static const int MATERIAL_TYPE_COOK_TORRANCE = 2;
    static const int MATERIAL_TYPE_OREN_NAYAR = 3;
    static const int MATERIAL_TYPE_ASHIKHMIN_SHIRLEY = 4;
    /** The GUI labels for the constants above. */
    const char* MATERIAL_TYPE_LABELS[5] = {"Blinn-Phong Original", "Blinn-Phong Normalized", "Cook-Torrance", "Oren-Nayar", "Ashikhmin-Shirley"};
    /** The material (i.e., shader) to be used. */
    int material_type = MATERIAL_TYPE_BLINN_PHONG_ORIGINAL;

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

    /** Sets the parameters for the currently selected material. */
    void set_materials();

    /** Renders the selected central object. */
    void render_central() const;

    /** Renders the light. */
    void render_light() const;

    // ----------------------------------------------------------------------------
    // GUI
    // ----------------------------------------------------------------------------
  public:
    /** @copydoc PV227Application::render_ui */
    void render_ui() override;
};

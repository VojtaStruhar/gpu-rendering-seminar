#pragma once
#include "camera_ubo.hpp"
#include "light_ubo.hpp"
#include "program_map.hpp"
#include "pv227_application.hpp"
#include "scene_object.hpp"

class Application : public PV227Application {

    // ----------------------------------------------------------------------------
    // Variables (Shaders)
    // ----------------------------------------------------------------------------
protected:

    /**The program for rendering the skybox. */
    ShaderProgram skybox_program;
    /** The shader for the central object, displays the cube map without reflections. */
    ShaderProgram display_cubemap_program;
    /** The shader for the central object, renders the object with texture and with reflections of the environment. */
    ShaderProgram reflection_program;
    /** The shaders that render all six faces of the cube map simultaneously and use instancing. */
    ShaderProgram object_to_cube_instancing_program;
    ShaderProgram skybox_to_cube_instancing_program;
    // Shader program maps and map keys.
    ShaderProgramMap object_program_map;
    ShaderProgramMap skybox_program_map;
    const int RENDER_STANDARD = 0;
    const int RENDER_TO_CUBE_INSTANCING = 1;

    // ----------------------------------------------------------------------------
    // Variables (Geometry)
    // ----------------------------------------------------------------------------
protected:
   /** The scene objects. */
    std::vector<SceneObject> scene_objects;
    SceneObject corner_object1;
    SceneObject corner_object2;
    SceneObject corner_object3;
    SceneObject corner_object4;
    SceneObject orbiting_object;

    /** The model matrix for the central object.*/
    ModelUBO central_object_ubo;

    /** Parameters for generating the random scene. */
    glm::ivec3 grid_size = glm::ivec3(9, 1, 9);
    glm::ivec3 grid_start = glm::ivec3(-20, -5, -20);
    glm::ivec3 grid_spacing = glm::ivec3(5, 0, 5);

    // ----------------------------------------------------------------------------
    // Variables (Materials)
    // ----------------------------------------------------------------------------
protected:
    // The list of materials from which we will randomly choose materials for each object.
    std::vector<PhongMaterialUBO> color_ubos;

    // ----------------------------------------------------------------------------
    // Variables (Camera)
    // ----------------------------------------------------------------------------
protected:
    /** The UBO storing the information about camera. */
    CameraUBO camera_ubo;
    /** The data of six cameras that are used when updating the cubemap - all in one buffer. */
    CameraUBO cubemap_all_camera_data_ubo;
    /** The data of six cameras that are used when updating the cubemap - each in separate buffer. */
    CameraUBO cubemap_separate_camera_data_ubo[6];

    // ----------------------------------------------------------------------------
    // Variables (Textures)
    // ----------------------------------------------------------------------------
protected:
    /* The list of textures which we can choose for our scene. */
    std::vector<GLuint> textures;

    /** The wood texture. */
    GLuint wood_tex;
    /** The Lenna texture. */
    GLuint lenna_tex;
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
    /** The UBO storing the data about lights - positions, colors, etc. */
    PhongLightsUBO phong_lights_ubo;

    // ----------------------------------------------------------------------------
    // Variables (Frame Buffers)
    // ----------------------------------------------------------------------------
protected:
    /** The size of the cube map. */
    const int cubemap_size = 1024;
    GLuint cubemap_fbo, cubemap_separate_fbo[6];
    GLuint cubemap_color_texture;
    GLuint cubemap_depth_texture;

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

    /** The constants identifying what method will be used to update the cubemap. */
    static const int CUBE_MAP_UPDATE_NO_UPDATE = 0;
    static const int CUBE_MAP_UPDATE_SEPARATE_FACES = 1;
    static const int CUBE_MAP_UPDATE_RENDER_TO_CUBE_MAP_INSTANCING = 2;
    /** The GUI labels for the constants above. */
    const char* CUBE_MAP_UPDATE_LABELS[3] = {"No Update", "Task 3: Separate Faces", "Task 4: Use Instancing"};
    /** The currently used method for updating the cubemap. */
    int cubemap_update_method = CUBE_MAP_UPDATE_NO_UPDATE;

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
    int central_object = CENTRAL_OBJECT_CUBE;

    /** The flag determining if reflections should be used. */
    bool use_reflections = false;

    float reflection_factor = 0.5f;

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

    /**
     * Updates the objects.
     */
    void udpate_objects();

    /**
     * Updates the skybox.
     */
    void udpate_skybox();

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
    void render_scene(ShaderProgram& program, bool use_instancing = false);

    /**
     * This function renderes the scene and the skybox. 
     * It does not render the central object.
     *
     * @param 	render_program	the type of the program to be used when rendering (e.g. to cube map, standard, ...)
     * @param 	use_instancing	when true, each object is rendered six times using instancing.
     */
    void render_skybox_and_scene(int render_program, bool use_instancing = false);

    /** Renders the scene six times - once for each face - and stores the result in the CubemapSeparateFBO. */
    void cube_map_update_separate_faces();

    /** Renders the scene only once - into all faces simultaneously - and stores the result in the CubemapFBO. */
    void cube_map_update_all_faces_at_once();

    /** Renders the final scene. */
    void render_final_scene();

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

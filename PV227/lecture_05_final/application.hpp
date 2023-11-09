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
    glm::ivec3 grid_size = glm::ivec3(17, 1, 17);
    glm::ivec3 grid_start = glm::ivec3(-32, 0, -32);
    glm::ivec3 grid_spacing = glm::ivec3(4, 0, 4);

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
    /** The shader program used to fill G-buffer with required information for deferred shading. */
    ShaderProgram deferred_program;
    /** The shader program used to display the textures. */
    ShaderProgram display_texture_program;
    /** The shader program using deferred shading (by evaluating the screen quad). */
    ShaderProgram evaluate_lighting_program;
    /** The shader program used to compute SSAO. */
    ShaderProgram evaluate_ssao_program;
    /** The shader program used to blur the SSAO texture. */
    ShaderProgram blur_ssao_texture_program;
    /** The shader program for blurring the final image using depth of field. */
    ShaderProgram dof_program;
    /** The shader program for HDR tone mapping */
    ShaderProgram tone_mapping_program;
    /** The shader program for blurring highlights. */
    ShaderProgram blurred_tone_mapping_program;
    /** The shader program for bloom effect - combines lit image with blurred highlights. */
    ShaderProgram bloom_program;

    // ----------------------------------------------------------------------------
    // Variables (Frame Buffers)
    // ----------------------------------------------------------------------------
protected:
    /** The framebuffer object that is used when rendering into the G-buffer. */
    GLuint gbuffer_fbo;
    /** The texture used in @link gbuffer_fbo to store positions in world space. */
    GLuint gbuffer_position_ws_texture;
    /** The texture used in @link gbuffer_fbo to store positions in view space. */
    GLuint gbuffer_position_vs_texture;
    /** The texture used in @link gbuffer_fbo to store normals in world space. */
    GLuint gbuffer_normal_ws_texture;
    /** The texture used in @link gbuffer_fbo to store normals in view space. */
    GLuint gbuffer_normal_vs_texture;
    /** The texture used in @link gbuffer_fbo to store albedo. */
    GLuint gbuffer_albedo_texture;
    /** The texture used in @link gbuffer_fbo to store depth. */
    GLuint gbuffer_depth_texture;

    /** The framebuffer object that is used when evaluating SSAO. */
    GLuint ssao_evaluation_fbo;
    /** The texture used in @link ssao_evaluation_fbo to store the ambient occlusion. */
    GLuint ssao_occlusion_texture;

    // The framebuffer object that is used to blur ambient occlusion texture.
    GLuint ssao_blurring_fbo;
    /** The texture used in @link ssao_blurring_fbo to store the blurred ambient occlusion. */
    GLuint ssao_blurred_occlusion_texture;

    // The UBO with random samples in the unit hemisphere for SSAO.
    GLuint ssao_samples_ubo;
    // The texture storing the random tangents directions (in view space).
    GLuint ssao_random_tangent_vs_texture;

    /** The framebuffer object used for evaluating the lighting of the scene. Used as basis for tone mapping and bloom. */
    GLuint hdr_fbo;
    /** HDR Texture with the color of the lit scene before applying post-effects.*/
    GLuint hdr_lit_scene_texture;
    /** HDR Texture with highlights only.*/
    GLuint hdr_highlights_texture;

    /** The framebuffer object used for storing the tone mapping only. Used as a basis for combination of tone mapping with DoF.  */
    GLuint tone_mapping_fbo;
    /** HDR Texture with the color of the lit scene with applied tone mapping.*/
    GLuint tone_mapping_scene_texture;

    /** The framebuffer objects that are used to blur highlights (needed for bloom effect). We have two buffers so we can ping-poing when blurring. */
    GLuint hrd_blurred_highlights_fbo[2];
    /** The textures with with the blurred highlights. We have two textures so we can ping-poing when blurring. */
    GLuint hdr_blurred_highlights_texture[2];

    /** The framebuffer object used for storing the bloom effect. Used as a basis for combination of bloom with DoF.  */
    GLuint bloom_fbo;
    /** HDR Texture with the color of the lit scene with applied tone mapping.*/
    GLuint bloom_scene_texture;

    // ----------------------------------------------------------------------------
    // Variables (GUI)
    // ----------------------------------------------------------------------------
protected:
    /** The constants identifying what can be displayed on the screen. */
    const int DISPLAY_POSITION_WS = 0;
    const int DISPLAY_POSITION_VS = 1;
    const int DISPLAY_NORMAL_WS = 2;
    const int DISPLAY_NORMAL_VS = 3;
    const int DISPLAY_ALBEDO = 4;
    const int DISPLAY_OCCLUSION = 5;
    const int DISPLAY_HIGHLIGHTS = 6;
    const int DISPLAY_BLURRED_HIGHLIGHTS = 7;
    const int DISPLAY_FINAL_IMAGE = 8;
    /** The GUI labels for the constants above. */
    const char* DISPLAY_LABELS[9] = {
        "Positions World Space",
        "Positions View Space",
        "Normals World Space",
        "Normals View Space",
        "Albedo",
        "Ambient Occlusion",
        "Highlights",
        "Blurred Highlights",
        "Final Image"};

    /** The flag determining what will be displayed on the screen right now. */
    int what_to_display = DISPLAY_FINAL_IMAGE;
    /** The flag determining if we use SSAO. */
    bool use_ssao = true;
    /** The flag determining if SSAO should be blurred. */
    bool blur_ssao = true;
    /** The radius used when computing the SSAO. */
    float ssao_radius = 0.5f;
    /** The flag determining if DoF (depth of field) should be used. */
    bool use_dof = false;
    /** The distance where the lenses are focused to. */
    int focus_distance = 10;
    /** The flag determining if tone mapping should be used. */
    bool use_tone_mapping = false;
    /** The exposure value. */
    float exposure = 1.0f;
    /** The gamma value. */
    float gamma = 2.2f;
    /** The flag determining if bloom should be used. */
    bool use_bloom = false;

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
     * {@copydoc OpenGLApplication::compile_shaders}
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

    /** Computes random positions of samples for SSAO and stores them in @link ssao_samples_ubo. */
    void compute_random_samples();

    /** Computes random tangent directions for SSAO and uploads them into @link ssao_random_tangent_vs_texture. */
    void compute_random_tangent_directions();

    // ----------------------------------------------------------------------------
    // Update
    // ----------------------------------------------------------------------------
    /**
     * {@copydoc OpenGLApplication::update}
     */
    void update(float delta) override;

    // ----------------------------------------------------------------------------
    // Render
    // ----------------------------------------------------------------------------
public:
    /** @copydoc OpenGLApplication::render */
    void render() override;

    /**
     * Renders the whole scene.
     *
     * @param 	program	The program to used when rendering the scene.
     */
    void render_scene(ShaderProgram& program);

    /** Sets up the G-buffer and renders all objects into it using deferred shading. */
    void render_into_g_buffer();

    /**
     * Sets up the frame buffer and renders one of the textures stored in G-buffer (i.e., position texture, normal
     * texture, or albedo texture).
     */
    void show_g_buffer_textures();

    /** Sets up the frame buffer and evaluates the lighting using a full-screen quad. */
    void evaluate_lighting_using_quad();

    /** Evaluates SSAO using @link evaluate_ssao_program. */
    void evaluate_ssao() const;

    /** Blurs the content of @link ssao_occlusion_texture and stores the result into @link ssao_blurred_occlusion_texture (via @link ssao_blurring_fbo). */
    void apply_blur_ssao() const;

    /** Clears SSAO frame buffers (@link ssao_evaluation_fbo, @link ssao_blurring_fbo) to ones, making the ambient occlusion not to appear. */
    void clear_ssao_buffers() const;

    /** Shows one of the textures stored in HDR buffers without tone mapping. */
    void render_without_tone_mapping();

    /** Shows one of the textures stored in HDR buffers with tone mapping. */
    void render_with_tone_mapping();

    /**  Blurs the highlights buffer. */
    void blur_highlights_buffer();

    /** Combines the blurred highlights with lit scene - i.e., applies the bloom effect. */
    void render_with_bloom();

    /** Evaluates DoF */
    void evaluate_dof() const;

    // ----------------------------------------------------------------------------
    // GUI
    // ----------------------------------------------------------------------------
public:
    /** @copydoc OpenGLApplication::render_ui */
    void render_ui() override;

    // ----------------------------------------------------------------------------
    // Input Events
    // ----------------------------------------------------------------------------
public:
    /** @copydoc OpenGLApplication::on_resize */
    void on_resize(int width, int height) override;
};

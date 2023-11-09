// ################################################################################
// Common Framework for Computer Graphics Courses at FI MUNI.
//
// Copyright (c) 2021-2022 Visitlab (https://visitlab.fi.muni.cz)
// All rights reserved.
// ################################################################################

#pragma once
#include "camera_ubo.hpp"
#include "light_ubo.hpp"
#include "pv227_application.hpp"
#include "scene_object.hpp"

enum EDisplayMode : int { NORMAL_SCENE, MIRRORED_SCENE, MASK_TEXTURE, FINAL_IMAGE };
inline const char* DisplayModeToText(int mode) {
    switch (static_cast<EDisplayMode>(mode)) {
    case FINAL_IMAGE:
        return "Final Image";
    case MASK_TEXTURE:
        return "Mask Texture";
    case NORMAL_SCENE:
        return "Normal Scene Texture";
    case MIRRORED_SCENE:
        return "Mirrored Scene";
    }
    return "";
}

class Application : public PV227Application {
    // ----------------------------------------------------------------------------
    // Variables (Geometry)
    // ----------------------------------------------------------------------------
  protected:
    /** The scene object representing vampire. */
    SceneObject vampire_object;
    /** The scene object representing SWAT head. */
    SceneObject swat_head_object;
    /** The scene object representing SWAT body. */
    SceneObject swat_body_object;
    /** The scene object representing SWAT helmet. */
    SceneObject swat_helmet_object;
    /** The scene object representing room. */
    SceneObject walls_object;
    /** The scene object representing door. */
    SceneObject door_object;
    /** The scene object representing table. */
    SceneObject table_object;
    /** The scene object representing chair. */
    SceneObject chair_object;
    /** The scene object representing window frame. */
    SceneObject window_frame_object;
    /** The scene object representing mirror glass. */
    SceneObject glass_object;
    /** The scene object representing first light. */
    SceneObject light_1_object;
    /** The scene object representing second light. */
    SceneObject light_2_object;
    /** The scene object representing floor.*/
    SceneObject floor_object;
    /** The scene object representing point light. */
    SceneObject point_light1_object;
    /** The scene object representing point light. */
    SceneObject point_light2_object;

    // ----------------------------------------------------------------------------
    // Variables (Materials)
    // ----------------------------------------------------------------------------
  protected:
    // ----------------------------------------------------------------------------
    // Variables (Textures)
    // ----------------------------------------------------------------------------
  protected:
    /** The albedo texture for SWAT head. */
    GLuint swat_head_albedo_texture;

    /** The albedo texture for SWAT body. */
    GLuint swat_body_albedo_texture;

    /** The albedo texture for vampire. */
    GLuint vampire_albedo_texture;

    /** The albedo texture for walls. */
    GLuint walls_albedo_texture;

    /** The albedo texture for door. */
    GLuint door_albedo_texture;

    /** The albedo texture for table. */
    GLuint table_albedo_texture;

    /** The albedo texture for chair. */
    GLuint chair_albedo_texture;

    /** The albedo texture for window. */
    GLuint window_albedo_texture;

    /** The albedo texture for lights. */
    GLuint light_albedo_texture;

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
    /** The camera projection matrix. */
    glm::mat4 projection_matrix;
    /** The UBO storing the information about the camera. */
    CameraUBO camera_ubo;

    // ----------------------------------------------------------------------------
    // Variables (Shaders)
    // ----------------------------------------------------------------------------
  protected:
    /** The program for rendering textures. */
    ShaderProgram display_texture_program;

    // ----------------------------------------------------------------------------
    // Variables (Frame Buffers)
    // ----------------------------------------------------------------------------
  protected:

    // ----------------------------------------------------------------------------
    // Variables (GUI)
    // ----------------------------------------------------------------------------
  protected:
    /** The flag determining what will be displayed on the screen right now. */
    EDisplayMode what_to_display = FINAL_IMAGE;
    /** The flag determining if the walls shall be transparent. */
    bool transparent_walls = false;
    /** The flag determining if the walls shall be transparent. */
    bool extra_reflection = false;

    // ----------------------------------------------------------------------------
    // Constructors
    // ----------------------------------------------------------------------------
  public:
    Application(int initial_width, int initial_height, std::vector<std::string> arguments = {});

    /** Destroys the {@link Application} and releases the allocated resources. */
    ~Application() override;

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

    /** Renders the whole scene. */
    void render_scene(const ShaderProgram& program) const;

    /** Renders one object. */
    void render_object(const SceneObject& object, const ShaderProgram& program) const;

    /** Renders a texture onto screen. */
    void display_texture(GLuint texture);

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

    /** @copydoc PV227Application::on_mouse_move */
    void on_mouse_move(double x, double y) override;
};

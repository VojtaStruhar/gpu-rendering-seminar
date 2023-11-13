#pragma once
#include "camera_ubo.hpp"
#include "light_ubo.hpp"
#include "pv227_application.hpp"
#include "scene_object.hpp"

class Application : public PV227Application {
    // ----------------------------------------------------------------------------
    // Variables (Particles)
    // ----------------------------------------------------------------------------
protected:
    /** The current number of particles in simulation. */
    int current_particle_count;
    /** The maximum number of particles for which the memory is allocated. */
    int max_particle_count = 131072;
    /**	The positions of all particles (on CPU). We need the positions twice (one for the previous frame, and one for the current frame) because the position of all particles in the previous frame affects the position of particles in the new frame. */
    std::vector<glm::vec4> particle_positions[2];
    /** The velocities of all particles (on CPU). The velocities of other particles do not affect velocity of one particle, so the velocities is present only once and they are updated in place. **/
    std::vector<glm::vec4> particle_velocities;
    /** The positions of all particles (on GPU). Again we need two buffers (see above). */
    GLuint particle_positions_bo[2];
    /** The velocities of all particles (on GPU). */
    GLuint particle_velocities_bo;
    /** The colors of all particles (on GPU). They do not change during the simulation. */
    GLuint particle_colors_bo;
    /** VAOs for rendering particles, one for each buffer with positions. */
    GLuint particle_vao[2];
    /* The index of the current position buffer to read from into. The values are 0/1. */
    int current_read = 0;
    /* The index of the current position buffer to write into. The values are 0/1. */
    int current_write = 1;

    // ----------------------------------------------------------------------------
    // Variables (Textures)
    // ----------------------------------------------------------------------------
protected:

    /** The particle texture. */
    GLuint particle_tex;

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
    /** The program for rendering particles as points. */
    ShaderProgram particle_simple_program;
    /** The program for rendering particles as a textured quad (generated in compute shader). */
    ShaderProgram particle_textured_program;
    /** The simple program for updating particles. */
    ShaderProgram update_particles_program;
    /** The more complex program for updating particles that is using shared memory. */
    ShaderProgram update_particles_shared_program;

    // ----------------------------------------------------------------------------
    // Variables (GUI)
    // ----------------------------------------------------------------------------
    /** The flag determining what to use to render particles: geometry shaders / simple points. */
    bool use_geometry_shaders = false;
    /** The size of one particle (in world/view space) when using geometry shaders. */
    float particle_size = 0.5f;
    /** The seed to use when generating random positions of particles. */
    int reset_particles_seed = 1;
    /** The number of particles to be used in simulation. */
    int desired_particle_count = 256;
    /** The acceleration factor used in the simulation - a constant that affects the final acceleration. */
    float acceleration_factor = 0.2f;
    /** The distance threshold used in the simulation - objects closer than this value exert no force to each other. */
    float distance_threshold = 0.01f;
    /** The time elapsed since the last frame of our simulation. */
    float time_delta = 0.005f;

     /** The constants identifying where the computation can run. */
    const int UPDATE_ON_CPU = 0;
    const int UPDATE_ON_GPU_SIMPLE = 1;
    const int UPDATE_ON_GPU_SHARED = 2;
    /** The GUI labels for the constants above. */
    const char* UPDATE_LABELS[3] = {"Update on CPU", "Update on GPU", "Update on GPU (Shared)"};

    /** The flag defining the current place where the computation runs (CPU / GPU / optimized GPU). */
    int current_update_method = UPDATE_ON_CPU;
    /** The flag defining the current place where the computation runs (CPU / GPU / optimized GPU). */
    int desired_update_method = UPDATE_ON_CPU;

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

    // ----------------------------------------------------------------------------
    // Update
    // ----------------------------------------------------------------------------
    /**
     * {@copydoc PV227Application::update}
     */
    void update(float delta) override;

    /** Resets the positions and velocities of all particles in the simulation. */
    void reset_particles();

    /** Updates the positions and velocities of all particles on CPU. */
    void update_particles_cpu();

    /** Updates the positions and velocities of all particles on GPU. */
    void update_particles_gpu();

    // ----------------------------------------------------------------------------
    // Render
    // ----------------------------------------------------------------------------
public:
    /** @copydoc PV227Application::render */
    void render() override;

    // ----------------------------------------------------------------------------
    // GUI
    // ----------------------------------------------------------------------------
public:
    /** @copydoc PV227Application::render_ui */
    void render_ui() override;
};

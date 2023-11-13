#include "application.hpp"
#include "utils.hpp"
#include <map>

Application::Application(int initial_width, int initial_height, std::vector<std::string> arguments)
    : PV227Application(initial_width, initial_height, arguments) {
    Application::compile_shaders();
    prepare_cameras();
    prepare_textures();
    prepare_scene();
}

Application::~Application() {
}

// ----------------------------------------------------------------------------
// Shaderes
// ----------------------------------------------------------------------------
void Application::compile_shaders() {
    particle_simple_program = ShaderProgram(lecture_shaders_path / "particle_simple.vert", lecture_shaders_path / "particle_simple.frag");

    particle_textured_program = ShaderProgram();
    particle_textured_program.add_vertex_shader(lecture_shaders_path / "particle_textured.vert");
    particle_textured_program.add_fragment_shader(lecture_shaders_path / "particle_textured.frag");
    particle_textured_program.add_geometry_shader(lecture_shaders_path / "particle_textured.geom");
    particle_textured_program.link();

    update_particles_program = ShaderProgram();
    update_particles_program.add_compute_shader(lecture_shaders_path / "nbody.comp");
    update_particles_program.link();

    update_particles_shared_program = ShaderProgram();
    update_particles_shared_program.add_compute_shader(lecture_shaders_path / "nbody_shared.comp");
    update_particles_program.link();

    std::cout << "Shaders are reloaded." << std::endl;
}

// ----------------------------------------------------------------------------
// Initialize Scene
// ----------------------------------------------------------------------------
void Application::prepare_cameras() {
    // Sets the default camera position.
    camera.set_eye_position(glm::radians(-45.f), glm::radians(20.f), 25.f);
    // Computes the projection matrix.
    camera_ubo.set_projection(glm::perspective(glm::radians(45.f), static_cast<float>(this->width) / static_cast<float>(this->height), 0.1f, 5000.0f));
    camera_ubo.update_opengl_data();
}

void Application::prepare_textures() {
    particle_tex = TextureUtils::load_texture_2d(lecture_textures_path / "18861-radial_gradient.png");
    // Particles are really small, use mipmaps for them.
    TextureUtils::set_texture_2d_parameters(particle_tex, GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
}

void Application::prepare_scene() {
    // Creates CPU buffers with the data of particles - positions and velocities.
    particle_positions[0].resize(max_particle_count, glm::vec4(0.0f));
    particle_positions[1].resize(max_particle_count, glm::vec4(0.0f));
    particle_velocities.resize(max_particle_count, glm::vec4(0.0f));

    // Computes random color for each particle.
    srand(12345);
    std::vector<glm::vec3> particle_colors(max_particle_count);
    for (int i = 0; i < max_particle_count; i++) {
        // Thanks to the fact that GLM has functions for HSV -> RGB conversion, we choose hue randomly.
        // We use 1.0 for saturation value, and convert it to RGB, thus obtaining a uniformly random color.
        particle_colors[i] = glm::rgbColor(glm::vec3(static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 360.0f, 1.0f, 1.0f));
    }

    // Allocates GPU buffers.
    glCreateBuffers(2, particle_positions_bo);
    glCreateBuffers(1, &particle_velocities_bo);
    glCreateBuffers(1, &particle_colors_bo);
    glNamedBufferStorage(particle_positions_bo[0], sizeof(float) * 4 * max_particle_count, nullptr, GL_DYNAMIC_STORAGE_BIT);
    glNamedBufferStorage(particle_positions_bo[1], sizeof(float) * 4 * max_particle_count, nullptr, GL_DYNAMIC_STORAGE_BIT);
    glNamedBufferStorage(particle_velocities_bo, sizeof(float) * 4 * max_particle_count, nullptr, GL_DYNAMIC_STORAGE_BIT);
    glNamedBufferStorage(particle_colors_bo, sizeof(float) * 3 * max_particle_count, particle_colors.data(), 0); // We can already upload the colors as they will not be changed.

    // Initialize positions and velocities, and uploads them into OpenGL buffers.
    current_particle_count = desired_particle_count;
    reset_particles();

    // Creates VAOs to render the particles. The positions are at location '0' and the colors at '1'.
    // Since there are two buffers with positions, we need two VAOs.
    glCreateVertexArrays(2, particle_vao);

    // --- particle_vao[0]
    glVertexArrayVertexBuffer(particle_vao[0], 0, particle_positions_bo[0], 0, 4 * sizeof(float));
    glVertexArrayVertexBuffer(particle_vao[0], 1, particle_colors_bo, 0, 4 * sizeof(float));

    glEnableVertexArrayAttrib(particle_vao[0], 0);
    glVertexArrayAttribFormat(particle_vao[0], 0, 4, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(particle_vao[0], 0, 0);

    glEnableVertexArrayAttrib(particle_vao[0], 1);
    glVertexArrayAttribFormat(particle_vao[0], 1, 4, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(particle_vao[0], 1, 1);

    // --- particle_vao[1]
    glVertexArrayVertexBuffer(particle_vao[1], 0, particle_positions_bo[0], 0, 4 * sizeof(float));
    glVertexArrayVertexBuffer(particle_vao[1], 1, particle_colors_bo, 0, 4 * sizeof(float));

    glEnableVertexArrayAttrib(particle_vao[1], 0);
    glVertexArrayAttribFormat(particle_vao[1], 0, 4, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(particle_vao[1], 0, 0);

    glEnableVertexArrayAttrib(particle_vao[1], 1);
    glVertexArrayAttribFormat(particle_vao[1], 1, 4, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(particle_vao[1], 1, 1);
}

// ----------------------------------------------------------------------------
// Update
// ----------------------------------------------------------------------------
void Application::update(float delta) {
    PV227Application::update(delta);

    // Updates the main camera.
    const glm::vec3 eye_position = camera.get_eye_position();
    camera_ubo.set_view(glm::lookAt(eye_position, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
    camera_ubo.update_opengl_data();

    // Recreates particles if the number changed or when the type of computation changed.
    if (current_particle_count != desired_particle_count || current_update_method != desired_update_method) {
        current_particle_count = desired_particle_count;
        current_update_method = desired_update_method;
        reset_particles();
    }

    // Updates the positions and velocities of all particles.
    if (current_update_method == UPDATE_ON_CPU) {
        update_particles_cpu();
    }
        // Simulates the particles on GPU using.
    else if (current_update_method == UPDATE_ON_GPU_SIMPLE || current_update_method == UPDATE_ON_GPU_SHARED) {
        update_particles_gpu();
    }
}

void Application::reset_particles() {
    // Sets the seed. We set it here to ensure that though all particles are at random places, they are always at the same places when they are reset.
    srand(reset_particles_seed);

    // The points are uniformly distributed on the surface of a unit sphere, and their initial velocity is zero.
    for (int i = 0; i < current_particle_count; i++) {
        const float alpha = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * static_cast<float>(M_PI);
        const float beta = asinf(static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f - 1.0f);
        glm::vec3 point_on_sphere = glm::vec3(cosf(alpha) * cosf(beta), sinf(alpha) * cosf(beta), sinf(beta));

        particle_positions[0][i] = glm::vec4(point_on_sphere, 1.0f);
        particle_positions[1][i] = glm::vec4(point_on_sphere, 1.0f);
        particle_velocities[i] = glm::vec4(0.0f);
    }

    // Updates the OpenGL buffers.
    glNamedBufferSubData(particle_positions_bo[0], 0, sizeof(glm::vec4) * current_particle_count, particle_positions[0].data());
    glNamedBufferSubData(particle_positions_bo[1], 0, sizeof(glm::vec4) * current_particle_count, particle_positions[1].data());
    glNamedBufferSubData(particle_velocities_bo, 0, sizeof(glm::vec4) * current_particle_count, particle_velocities.data());
}

/// Update the positions and velocities of all particles on CPU.
void Application::update_particles_cpu() {
    fps_gpu = 0; // We are not interested in GPU timing when updating on CPU.

    // "Renames" the variables so that their name was the same as their names in shaders.
    std::vector<glm::vec4>& particle_positions_read = particle_positions[current_read];
    std::vector<glm::vec4>& particle_positions_write = particle_positions[current_write];

    // TASK 1: Implement n-body simulation on CPU: for each particle, compute the force to each other particle, then the acceleration, and then update the position and velocity.
    //	 Hint: Variables:
    //          particle_positions_read   .. array of vec4, positions of particles to read from
	//			particle_positions_write  .. array of vec4, positions of particles to write into
	//			particle_velocities       .. array of vec4, velocities of particles to read from and to write into
	//			current_particle_count    .. integer, number of active particles in simulation
	//			distance_threshold        .. float, objects closer than this value exert no force to each other
	//			acceleration_factor       .. float, a constant that affects the final acceleration
	//			time_delta                .. float, time elapsed since the last frame of our simulation
    //	 Hint: You may use most of GLSL functions and structures, but remember that they are in glm namespace (like glm::vec3, glm::normalize, glm::dot, etc.). 
    //	       Unfortunately, there is no swizzling in C++.

    // Updates the position and velocity of each particle.
    for (int i = 0; i < current_particle_count; i++) {
        // Reads the initial position and velocity of the particle.
        glm::vec3 position = glm::vec3(particle_positions_read[i]);
        glm::vec3 velocity = glm::vec3(particle_velocities[i]);

        // Computes the acceleration of the particle. We simulate the gravitation force between all particles.
        // We compute a vector from this particle to each other particle, and if it is further than some distance
        // threshold (to avoid division by zero), we compute a force that attracts the particle. 
        // The force fades with the square of the distance, as in real world.
        glm::vec3 acceleration = glm::vec3(0.0f);
        for (int p = 0; p < current_particle_count; p++) {
            // Obtains position of the other particle.
            glm::vec3 other = glm::vec3(particle_positions_read[p]);
            // Computes vector between these two particles.
            glm::vec3 direction = other - position;
            // Computes square of the distance between these particles.
            float dist_sq = glm::dot(direction, direction);
            if (dist_sq > distance_threshold) {
                // Adds the force to acceleration. 
                // We must normalize the 'diff' to get the direction, and fade it with the square of the distance (division by 'dist_sq').
                acceleration += glm::normalize(direction) / dist_sq;
            }
        }
        // We multiply the acceleration with some constant. 
        // The constant contains all physics stuff like the mass of particles, gravitational constants, etc.
        acceleration *= acceleration_factor;

        // Updates the position and the velocity.
        position += velocity * time_delta + 0.5f * acceleration * time_delta * time_delta;
        velocity += acceleration * time_delta;

        // Stores the result. 
        // Note that we must store the positions into a different buffer than we read it from, because we need the initial values for simulation of other particles.
        particle_velocities[i] = glm::vec4(velocity, 0.0f);
        particle_positions_write[i] = glm::vec4(position, 1.0f);
    }

    // Updates the GPU buffer to render the positions.
    glNamedBufferSubData(particle_positions_bo[current_write], 0, sizeof(glm::vec4) * current_particle_count, particle_positions[current_write].data());
}

/// Update the positions and velocities of all particles on GPU.
void Application::update_particles_gpu() {
    // Bind all buffers to OpenGL
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, particle_positions_bo[current_read]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, particle_positions_bo[current_write]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, particle_velocities_bo);
    // Memory barrier, we need the data in the buffers to be ready
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // Chooses the proper program.
    ShaderProgram gpu_program_to_use;
    if (current_update_method == UPDATE_ON_GPU_SIMPLE) {
        gpu_program_to_use = update_particles_program;
    } else if (current_update_method == UPDATE_ON_GPU_SHARED) {
        gpu_program_to_use = update_particles_shared_program;
    }

    gpu_program_to_use.use();

    // Update uniform values
    gpu_program_to_use.uniform("current_particle_count", current_particle_count);
    gpu_program_to_use.uniform("time_delta", time_delta);
    gpu_program_to_use.uniform("acceleration_factor", acceleration_factor);
    gpu_program_to_use.uniform("distance_threshold", distance_threshold);

    // This must be the same as 'layout (local_size_x = 256) in;' in nbody_(shared).comp
    const int local_size_x = 256;

    // Dispatches the compute shader and measures the elapsed time.
    glBeginQuery(GL_TIME_ELAPSED, render_time_query);
    glDispatchCompute(current_particle_count / local_size_x, 1, 1);
    glEndQuery(GL_TIME_ELAPSED);

    // Waits for OpenGL - don't forget OpenGL is asynchronous.
    glFinish();

    // Evaluates the query.
    GLuint64 render_time;
    glGetQueryObjectui64v(render_time_query, GL_QUERY_RESULT, &render_time);
    fps_gpu = 1000.f / (static_cast<float>(render_time) * 1e-6f);
}

// ----------------------------------------------------------------------------
// Render
// ----------------------------------------------------------------------------
void Application::render() {
    // Binds the main window framebuffer.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);

    // Sets the clear values and clears the framebuffer.
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

     // Sets the data of the camera.
    camera_ubo.bind_buffer_base(CameraUBO::DEFAULT_CAMERA_BINDING);

    // Sets up very simple blending.
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    // Chooses the proper shader to render the particles
    if (use_geometry_shaders) {
        // Sets the necessary values
        particle_textured_program.use();
        particle_textured_program.uniform("particle_size_vs", particle_size);
        glBindTextureUnit(0, particle_tex);
    } else {
        // Sets the necessary values.
        particle_simple_program.use();
        glPointSize(particle_size);
    }

    // Binds the proper VAO (we use the VAO with the data we just wrote).
    glBindVertexArray(particle_vao[current_write]);
    // Memory barrier, we need the data in the buffers to be ready.
    glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
    // Draws the particles as points.
    glDrawArrays(GL_POINTS, 0, current_particle_count);

    // Disables blending.
    glDisable(GL_BLEND);

    // Swaps the indices of read and write buffers with positions.
    std::swap(current_read, current_write);

    // Resets the VAO and the program.
    glBindVertexArray(0);
    glUseProgram(0);
}

// ----------------------------------------------------------------------------
// GUI
// ----------------------------------------------------------------------------
void Application::render_ui() {
    const float unit = ImGui::GetFontSize();

    ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoDecoration);
    ImGui::SetWindowSize(ImVec2(20 * unit, 18 * unit));
    ImGui::SetWindowPos(ImVec2(2 * unit, 2 * unit));

    std::string fps_cpu_string = "FPS (CPU): ";
    ImGui::Text(fps_cpu_string.append(std::to_string(fps_cpu)).c_str());

    std::string fps_string = "FPS (GPU): ";
    ImGui::Text(fps_string.append(std::to_string(fps_gpu)).c_str());

    ImGui::PushItemWidth(150.f);
    ImGui::Checkbox("Use GS", &use_geometry_shaders);

    if (ImGui::Button("Reset Particles")) {
        reset_particles();
    }

    ImGui::SliderFloat("Particle Size", &particle_size, 0.05f, 3.0f, "%.2f");
    ImGui::SliderInt("Seed", &reset_particles_seed, 1, 100);

    const char* particle_labels[10] = {"256", "512", "1024", "2048", "4096", "8192", "16384", "32768", "65536", "131072"};
    int exponent = static_cast<int>(log2(current_particle_count) - 8); // -8 because we start at 256 = 2^8
    if (ImGui::Combo("Particle Count", &exponent, particle_labels, IM_ARRAYSIZE(particle_labels))) {
        desired_particle_count = static_cast<int>(glm::pow(2, exponent + 8)); // +8 because we start at 256 = 2^8
    }

    ImGui::Combo("Computation", &desired_update_method, UPDATE_LABELS, IM_ARRAYSIZE(UPDATE_LABELS));

    ImGui::SliderFloat("Acceleration Factor", &acceleration_factor, 0.01f, 1.0f, "%.2f");
    ImGui::SliderFloat("Distance Threshold", &distance_threshold, 0.01f, 1.0f, "%.2f");
    ImGui::SliderFloat("Time Delta", &time_delta, 0.001f, 0.01f, "%.3f");

    ImGui::End();
}

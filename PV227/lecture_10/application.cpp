#include "application.hpp"
#include "utils.hpp"
#include <map>

Application::Application(int initial_width, int initial_height, std::vector<std::string> arguments) : PV227Application(initial_width, initial_height, arguments) {
    Application::compile_shaders();
    prepare_cameras();
    prepare_textures();
    prepare_scene();
    compute_random_sampling();
}

Application::~Application() {}

// ----------------------------------------------------------------------------
// Shaderes
// ----------------------------------------------------------------------------
void Application::compile_shaders() {
    depth_prepass_program = ShaderProgram(lecture_shaders_path / "unlit.vert", lecture_shaders_path / "nothing.frag");
    skybox_program = ShaderProgram(lecture_shaders_path / "skybox.vert", lecture_shaders_path / "skybox.frag");
    object_program = ShaderProgram(lecture_shaders_path / "lit.vert", lecture_shaders_path / "lit.frag");

    std::cout << "Shaders are reloaded." << std::endl;
}

// ----------------------------------------------------------------------------
// Initialize Scene
// ----------------------------------------------------------------------------
void Application::prepare_cameras() {
    // Sets the default camera position.
    camera.set_eye_position(glm::radians(0.f), glm::radians(20.f), 15.f);
    // Computes the projection matrix.
    camera_ubo.set_projection(glm::perspective(glm::radians(45.f), static_cast<float>(this->width) / static_cast<float>(this->height), 0.1f, 1000.0f));
    camera_ubo.update_opengl_data();
}

void Application::prepare_textures() {
    mask_tex = TextureUtils::load_texture_2d(lecture_textures_path / "mask.png");
    TextureUtils::set_texture_2d_parameters(mask_tex, GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);

    meadow_skybox_tex = TextureUtils::load_texture_cube(1024, 1024, lecture_textures_path / "meadow_skybox/meadow_skybox_px.jpg", lecture_textures_path / "meadow_skybox/meadow_skybox_nx.jpg",
                                                        lecture_textures_path / "meadow_skybox/meadow_skybox_py.jpg", lecture_textures_path / "meadow_skybox/meadow_skybox_ny.jpg",
                                                        lecture_textures_path / "meadow_skybox/meadow_skybox_pz.jpg", lecture_textures_path / "meadow_skybox/meadow_skybox_nz.jpg");
    TextureUtils::set_texture_3d_parameters(meadow_skybox_tex, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

    // Loads the miramar skybox.
    miramar_skybox_tex = TextureUtils::load_texture_cube(1024, 1024, lecture_textures_path / "miramar_skybox/miramar_skybox_px.jpg", lecture_textures_path / "miramar_skybox/miramar_skybox_nx.jpg",
                                                         lecture_textures_path / "miramar_skybox/miramar_skybox_py.jpg", lecture_textures_path / "miramar_skybox/miramar_skybox_ny.jpg",
                                                         lecture_textures_path / "miramar_skybox/miramar_skybox_pz.jpg", lecture_textures_path / "miramar_skybox/miramar_skybox_nz.jpg");
    TextureUtils::set_texture_3d_parameters(miramar_skybox_tex, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

    // Chooses the initial skybox texture.
    current_skybox_tex = miramar_skybox_tex;
}

void Application::prepare_scene() {
    float x_start = -14.0f;
    float z_start = -14.0f;
    float x_spacing = 4.0f;
    float z_spacing = 4.0f;
    int x_grid_size = 8;
    int z_grid_size = 8;

    for (int i = 0; i < 64; i++) {
        scene_objects[i].get_model_ubo().set_matrix(
            glm::translate(glm::mat4(1.0f), glm::vec3(x_start + static_cast<float>(i % x_grid_size) * x_spacing, 0.0f, z_start + static_cast<float>(i / z_grid_size) * z_spacing)));
         scene_objects[i].get_model_ubo().update_opengl_data();

        switch (i % 8) {
        case 0:
            material_roughnesses[i] = 0.00f;
            break;
        case 1:
            material_roughnesses[i] = 0.05f;
            break;
        case 2:
            material_roughnesses[i] = 0.10f;
            break;
        case 3:
            material_roughnesses[i] = 0.15f;
            break;
        case 4:
            material_roughnesses[i] = 0.20f;
            break;
        case 5:
            material_roughnesses[i] = 0.30f;
            break;
        case 6:
            material_roughnesses[i] = 0.40f;
            break;
        case 7:
            material_roughnesses[i] = 0.50f;
            break;
        }
        switch (i / 8) {
        case 0:
            material_fresnels[i] = glm::vec3(0.04f, 0.04f, 0.04f);
            material_diffuses[i] = glm::vec3(1.0f, 0.0f, 0.0f);
            break; // Plastic Red
        case 1:
            material_fresnels[i] = glm::vec3(0.04f, 0.04f, 0.04f);
            material_diffuses[i] = glm::vec3(0.0f, 1.0f, 0.0f);
            break; // Plastic Green
        case 2:
            material_fresnels[i] = glm::vec3(0.04f, 0.04f, 0.04f);
            material_diffuses[i] = glm::vec3(0.0f, 0.0f, 1.0f);
            break; // Plastic Blue
        case 3:
            material_fresnels[i] = glm::vec3(0.56f, 0.57f, 0.58f);
            material_diffuses[i] = glm::vec3(0.0f, 0.0f, 0.0f);
            break; // Iron
        case 4:
            material_fresnels[i] = glm::vec3(0.95f, 0.64f, 0.54f);
            material_diffuses[i] = glm::vec3(0.0f, 0.0f, 0.0f);
            break; // Copper
        case 5:
            material_fresnels[i] = glm::vec3(1.00f, 0.71f, 0.29f);
            material_diffuses[i] = glm::vec3(0.0f, 0.0f, 0.0f);
            break; // Gold
        case 6:
            material_fresnels[i] = glm::vec3(0.91f, 0.92f, 0.92f);
            material_diffuses[i] = glm::vec3(0.0f, 0.0f, 0.0f);
            break; // Aluminum
        case 7:
            material_fresnels[i] = glm::vec3(0.95f, 0.93f, 0.88f);
            material_diffuses[i] = glm::vec3(0.0f, 0.0f, 0.0f);
            break; // Silver
        }
    }
}

void Application::compute_random_sampling() {
    srand(12345);
    std::vector<glm::vec4> random_floats(1024);
    for (size_t i = 0; i < random_floats.size(); i++) {
        random_floats[i] = glm::vec4(float(rand()) / float(RAND_MAX), float(rand()) / float(RAND_MAX), float(rand()) / float(RAND_MAX), float(rand()) / float(RAND_MAX));
    }
    // Creates a UBO for these random positions.
    glCreateBuffers(1, &random_floats_ubo);
    glNamedBufferStorage(random_floats_ubo, sizeof(float) * 4 * random_floats.size(), random_floats.data(), 0 /*static buffer*/);
}

// ----------------------------------------------------------------------------
// Update
// ----------------------------------------------------------------------------
void Application::update(float delta) {
    PV227Application::update(delta);

    // Updates the main camera.
    const glm::vec3 eye_position = camera.get_eye_position();
    camera_ubo.set_view(lookAt(eye_position, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
    camera_ubo.update_opengl_data();
}

// ----------------------------------------------------------------------------
// Render
// ----------------------------------------------------------------------------
void Application::render() {
    // Starts measuring the elapsed time.
    glBeginQuery(GL_TIME_ELAPSED, render_time_query);

    // Binds the main window framebuffer.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Sets the clear values and clears the framebuffer.
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, width, height);

    // Binds camera and lights.
    camera_ubo.bind_buffer_base(CameraUBO::DEFAULT_CAMERA_BINDING);

    // Binds the buffer with samples.
    glBindBufferBase(GL_UNIFORM_BUFFER, 4, random_floats_ubo);
    glBindTextureUnit(0, current_skybox_tex);
    glBindTextureUnit(1, mask_tex);

    // Renders the skybox.
    render_skybox();
    // Renders the objects.
    render_objects();

    // Resets the VAO and the program.
    glBindVertexArray(0);
    glUseProgram(0);

    // Stops measuring the elapsed time.
    glEndQuery(GL_TIME_ELAPSED);

    // Waits for OpenGL - don't forget OpenGL is asynchronous.
    glFinish();

    // Evaluates the query.
    GLuint64 render_time;
    glGetQueryObjectui64v(render_time_query, GL_QUERY_RESULT, &render_time);
    fps_gpu = 1000.f / (static_cast<float>(render_time) * 1e-6f);
}

void Application::render_skybox() const {
    skybox_program.use();

    // Binds the skybox texture.
    glBindTextureUnit(0, current_skybox_tex);

    // Render the skybox only into the color buffer, do not use depth test, do not update depth buffer
    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);

    // We render the skybox using a fullscreen quad, and we compute the view direction in vertex shader.
    // Binds an empty VAO as we do not need any state.
    glBindVertexArray(empty_vao);
    // Calls a draw command with 3 vertices that are generated in vertex shader.
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Restore the depth test and the depth buffer writes
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
}

void Application::render_objects() const {

    // Choose proper object
    const Geometry* geom = nullptr;
    switch (central_object) {
    case CENTRAL_OBJECT_CUBE:
        geom = &cube;
        break;
    case CENTRAL_OBJECT_SPHERE:
        geom = &sphere;
        break;
    case CENTRAL_OBJECT_TORUS:
        geom = &torus;
        break;
    case CENTRAL_OBJECT_CYLINDER:
        geom = &cylinder;
        break;
    case CENTRAL_OBJECT_CAPSULE:
        geom = &capsule;
        break;
    case CENTRAL_OBJECT_TEAPOT:
        geom = &teapot;
        break;
    }

    // Depth pass - We render only into depth buffer.
    glDepthFunc(GL_LEQUAL);
    depth_prepass_program.use();
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    geom->bind_vao();
    for (int i = 0; i < 64; i++) {
        scene_objects[i].get_model_ubo().bind_buffer_base(ModelUBO::DEFAULT_MODEL_BINDING);
        geom->draw();
    }

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    // Render pass - We render all objects in the scene.
    glDepthFunc(GL_EQUAL);

    object_program.use();
    object_program.uniform("diffuse_sample_count", diffuse_sample_count);
    object_program.uniform("specular_sample_count", specular_sample_count);
    object_program.uniform("mask_roughness", mask_roughness);
    object_program.uniform("use_shiny_layer", use_shiny_layer);
    object_program.uniform("use_shiny_layer_mask", use_shiny_layer_mask);

    geom->bind_vao();
    for (int i = 0; i < 64; i++) {
        object_program.uniform("material_fresnel", material_fresnels[i]);
        object_program.uniform("material_diffuse", material_diffuses[i]);
        object_program.uniform("material_roughness", material_roughnesses[i]);
        scene_objects[i].get_model_ubo().bind_buffer_base(ModelUBO::DEFAULT_MODEL_BINDING);
        geom->draw();
    }
}

// ----------------------------------------------------------------------------
// GUI
// ----------------------------------------------------------------------------
void Application::render_ui() {
    const float unit = ImGui::GetFontSize();

    ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoDecoration);
    ImGui::SetWindowSize(ImVec2(15 * unit, 15 * unit));
    ImGui::SetWindowPos(ImVec2(2 * unit, 2 * unit));

    std::string fps_cpu_string = "FPS (CPU): ";
    ImGui::Text(fps_cpu_string.append(std::to_string(fps_cpu)).c_str());

    std::string fps_string = "FPS (GPU): ";
    ImGui::Text(fps_string.append(std::to_string(fps_gpu)).c_str());

    ImGui::PushItemWidth(100);
    if (ImGui::Combo("Skybox", &skybox_texture, SKYBOX_LABELS, IM_ARRAYSIZE(SKYBOX_LABELS))) {
        if (skybox_texture == SKYBOX_MEADOW) {
            current_skybox_tex = meadow_skybox_tex;
        } else if (skybox_texture == SKYBOX_MIRAMAR) {
            current_skybox_tex = miramar_skybox_tex;
        }
    }
    ImGui::Combo("Central Object", &central_object, CENTRAL_OBJECT_LABELS, IM_ARRAYSIZE(CENTRAL_OBJECT_LABELS));

    // Builds up the lookup tables to map particle counts to string labels.
    const char* samples_labels[8] = {"32", "64", "128", "256", "384", "512", "768", "1024"};
    std::map<int, int> labels_to_values{{0, 32}, {1, 64}, {2, 128}, {3, 256}, {4, 384}, {5, 512}, {6, 768}, {7, 1024}};

    std::map<int, int> values_to_labels{{32, 0}, {64, 1}, {128, 2}, {256, 3}, {384, 4}, {512, 5}, {768, 6}, {1024, 7}};

    int currently_selected_number_of_diffuse_samples = values_to_labels.at(diffuse_sample_count);
    if (ImGui::Combo("Diffuse Samples", &currently_selected_number_of_diffuse_samples, samples_labels, IM_ARRAYSIZE(samples_labels))) {
        diffuse_sample_count = labels_to_values.at(currently_selected_number_of_diffuse_samples);
    }

    int currently_selected_number_of_specular_samples = values_to_labels.at(specular_sample_count);
    if (ImGui::Combo("Specular Samples", &currently_selected_number_of_specular_samples, samples_labels, IM_ARRAYSIZE(samples_labels))) {
        specular_sample_count = labels_to_values.at(currently_selected_number_of_specular_samples);
    }

    ImGui::Checkbox("Use Mask for Roughness", &mask_roughness);
    ImGui::Checkbox("Use Shiny Layer", &use_shiny_layer);
    ImGui::Checkbox("Use Mask for Shiny Layer", &use_shiny_layer_mask);

    ImGui::End();
}
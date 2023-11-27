#include "application.hpp"
#include "utils.hpp"

Application::Application(int initial_width, int initial_height, std::vector<std::string> arguments)
    : PV227Application(initial_width, initial_height, arguments) {
    Application::compile_shaders();
    prepare_cameras();
    prepare_textures();
    prepare_lights();
    prepare_scene();
}

Application::~Application() {
}

// ----------------------------------------------------------------------------
// Shaderes
// ----------------------------------------------------------------------------
void Application::compile_shaders() {
    default_unlit_program = ShaderProgram(lecture_shaders_path / "unlit.vert", lecture_shaders_path / "unlit.frag");

    default_lit_program = ShaderProgram();
    default_lit_program.add_vertex_shader(lecture_shaders_path / "teapot.vert");
    default_lit_program.add_tess_control_shader(lecture_shaders_path / "teapot_control.tesc");
    default_lit_program.add_tess_evaluation_shader(lecture_shaders_path / "teapot_evaluation.tese");
    default_lit_program.add_fragment_shader(lecture_shaders_path / "teapot.frag");
    default_lit_program.link();

    std::cout << "Shaders are reloaded." << std::endl;
}

// ----------------------------------------------------------------------------
// Initialize Scene
// ----------------------------------------------------------------------------
void Application::prepare_cameras() {
    // Sets the default camera position.
    camera.set_eye_position(glm::radians(-0.f), glm::radians(20.f), 15.f);
    // Computes the projection matrix.
    camera_ubo.set_projection(glm::perspective(glm::radians(45.f), static_cast<float>(this->width) / static_cast<float>(this->height), 0.1f, 1000.0f));
    camera_ubo.update_opengl_data();
}

void Application::prepare_textures() {
    chesterfield_color_tex = TextureUtils::load_texture_2d(lecture_textures_path / "chesterfield/color.png");
    TextureUtils::set_texture_2d_parameters(chesterfield_color_tex, GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);

    chesterfield_normal_tex = TextureUtils::load_texture_2d(lecture_textures_path / "chesterfield/normal.png");
    TextureUtils::set_texture_2d_parameters(chesterfield_normal_tex, GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);

    chesterfield_height_tex = TextureUtils::load_texture_2d(lecture_textures_path / "chesterfield/height.png");
    TextureUtils::set_texture_2d_parameters(chesterfield_height_tex, GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);
}

void Application::prepare_lights() {
    // The rest is set in the update scene method.
    phong_lights_ubo.set_global_ambient(glm::vec3(0.0f));
}

void Application::prepare_scene() {
    // Prepares the teapot object.
    teapot_object = SceneObject(cube, ModelUBO(translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.7f, 0.0f))), white_material_ubo, chesterfield_color_tex);

     // The light model is placed based on the value from UI so we update its model matrix later.
    light_object = SceneObject(sphere, ModelUBO(), white_material_ubo);
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

    // Computes the light position.
    const glm::vec3 light_position =
        glm::vec3(5, 5, 5) * glm::vec3(cosf(gui_light_position / 6.0f) * sinf(gui_light_position), sinf(gui_light_position / 6.0f), cosf(gui_light_position / 6.0f) * cosf(gui_light_position));

    // Updates the light model visible in the scene.
    light_object.get_model_ubo().set_matrix(translate(glm::mat4(1.0f), light_position) * scale(glm::mat4(1.0f), glm::vec3(0.2f)));
    light_object.get_model_ubo().update_opengl_data();

    // Updates the OpenGL buffer storing the information about the light.
    phong_lights_ubo.clear();
    phong_lights_ubo.add(PhongLightData::CreateDirectionalLight(light_position, glm::vec3(0.1f), glm::vec3(0.9f), glm::vec3(1.0f)));
    phong_lights_ubo.update_opengl_data();
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
    phong_lights_ubo.bind_buffer_base(PhongLightsUBO::DEFAULT_LIGHTS_BINDING);

    // Renders the scene.
    render_teapot();
    render_light();

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

void Application::render_teapot() const {
    default_lit_program.use();
    default_lit_program.uniform("use_normal_mapping", use_normal_mapping);
    default_lit_program.uniform("normal_factor", normal_factor);
    default_lit_program.uniform("use_displacement_mapping", use_displacement_mapping);
    default_lit_program.uniform("displacement_factor", displacement_factor);
    default_lit_program.uniform("auto_tessellation", use_auto_tessellation);
    default_lit_program.uniform("tessellation_factor", tessellation_factor);
    default_lit_program.uniform("triangle_size", triangle_size);
    default_lit_program.uniform("window_size", static_cast<float>(width), static_cast<float>(height));
    default_lit_program.uniform("has_texture", true);

    glBindTextureUnit(0, chesterfield_color_tex);
    glBindTextureUnit(1, chesterfield_normal_tex);
    glBindTextureUnit(2, chesterfield_height_tex);

    teapot_object.get_model_ubo().bind_buffer_base(ModelUBO::DEFAULT_MODEL_BINDING);

    if (use_wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    teapot_patches.bind_vao();
    teapot_patches.draw();

    if (use_wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

void Application::render_light() const {
    default_unlit_program.use();
    light_object.get_material().bind_buffer_base(PhongMaterialUBO::DEFAULT_MATERIAL_BINDING);
    light_object.get_model_ubo().bind_buffer_base(ModelUBO::DEFAULT_MODEL_BINDING);
    light_object.get_geometry().bind_vao();
    light_object.get_geometry().draw();
}

// ----------------------------------------------------------------------------
// GUI
// ----------------------------------------------------------------------------
void Application::render_ui() {
    const float unit = ImGui::GetFontSize();

    ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoDecoration);
    ImGui::SetWindowSize(ImVec2(20 * unit, 19 * unit));
    ImGui::SetWindowPos(ImVec2(2 * unit, 2 * unit));

    std::string fps_cpu_string = "FPS (CPU): ";
    ImGui::Text(fps_cpu_string.append(std::to_string(fps_cpu)).c_str());

    std::string fps_string = "FPS (GPU): ";
    ImGui::Text(fps_string.append(std::to_string(fps_gpu)).c_str());

    ImGui::PushItemWidth(150);
    ImGui::SliderAngle("Light Position", &gui_light_position, 0);

    ImGui::Checkbox("Wireframe", &use_wireframe);

    ImGui::Checkbox("Normal Mapping", &use_normal_mapping);
    ImGui::SliderFloat("Normal Factor", &normal_factor, 0.0f, 1.0f, "%.1f");
    ImGui::Checkbox("Displacement Mapping", &use_displacement_mapping);
    ImGui::SliderFloat("Displacement Factor", &displacement_factor, 0.0f, 0.1f, "%.2f");

    ImGui::Checkbox("Automatic Tessellation", &use_auto_tessellation);
    ImGui::SliderFloat("Tessellation Factor", &tessellation_factor, 0.0f, 64.0f, "%.1f");
    ImGui::SliderFloat("Triangle Size", &triangle_size, 0.0f, 100.0f, "%.1f");

    ImGui::End();
}

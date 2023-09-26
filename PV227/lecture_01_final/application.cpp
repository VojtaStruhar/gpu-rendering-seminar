#include "application.hpp"
#include "utils.hpp"

Application::Application(int initial_width, int initial_height, std::vector<std::string> arguments) : PV227Application(initial_width, initial_height, arguments) {
    Application::compile_shaders();
    prepare_cameras();
    prepare_textures();
    prepare_lights();
    prepare_scene();
}

Application::~Application() {}

// ----------------------------------------------------------------------------
// Shaderes
// ----------------------------------------------------------------------------
void Application::compile_shaders() {
    default_unlit_program = ShaderProgram(lecture_shaders_path / "object.vert", lecture_shaders_path / "unlit.frag");
    default_lit_program = ShaderProgram(lecture_shaders_path / "object.vert", lecture_shaders_path / "lit.frag");

    // TASK 4b: Create a new shader program with a vertex shader that enlarges the object along its normal.
    //    Hint: You can use unlit.frag as the fragment shader since it already renders the object with a single color.
    expand_program = ShaderProgram(lecture_shaders_path / "expand.vert", lecture_shaders_path / "unlit.frag");

    std::cout << "Shaders are reloaded." << std::endl;
}

// ----------------------------------------------------------------------------
// Initialize Scene
// ----------------------------------------------------------------------------
void Application::prepare_cameras() {
    // Sets the default camera position.
    camera.set_eye_position(glm::radians(-45.f), glm::radians(20.f), 25.f);
    // Computes the projection matrix.
    camera_ubo.set_projection(glm::perspective(glm::radians(45.f), static_cast<float>(this->width) / static_cast<float>(this->height), 0.1f, 1000.0f));
    camera_ubo.update_opengl_data();
}

void Application::prepare_textures() {
    wood_tex = TextureUtils::load_texture_2d(framework_textures_path / "wood.png");
    TextureUtils::set_texture_2d_parameters(wood_tex, GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);

    lenna_tex = TextureUtils::load_texture_2d(framework_textures_path / "lenna.png");
    TextureUtils::set_texture_2d_parameters(lenna_tex, GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);
}

void Application::prepare_lights() {
    // The rest is set in the update scene method.
    phong_lights_ubo.set_global_ambient(glm::vec3(0.0f));
}

void Application::prepare_scene() {
    const ModelUBO cube_model_ubo(glm::translate(glm::mat4(1.0f), glm::vec3(-4.0f, 1.0f, 0.0f)));
    const SceneObject cube_object(cube, cube_model_ubo, white_material_ubo, lenna_tex);
    scene_objects.push_back(cube_object);

    const SceneObject torus_object(torus, ModelUBO(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, 4.0f))), blue_material_ubo);
    scene_objects.push_back(torus_object);

    const SceneObject sphere_object(sphere, ModelUBO(glm::translate(glm::mat4(1.0f), glm::vec3(4.0f, 1.0f, 0.0f))), white_material_ubo, wood_tex);
    scene_objects.push_back(sphere_object);

    const SceneObject cylinder_object(cylinder, ModelUBO(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, -4.0f))), green_material_ubo);
    scene_objects.push_back(cylinder_object);

    const SceneObject teapot_object(teapot, ModelUBO(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f))), red_material_ubo);
    scene_objects.push_back(teapot_object);

    // The floor object is rendered separately so we do not insert it into the scene_objects.
    floor_object = SceneObject(cube, ModelUBO(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.1f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(10.0f, 0.1f, 10.0f))), gray_material_ubo);

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
    camera_ubo.set_view(glm::lookAt(eye_position, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
    camera_ubo.update_opengl_data();

    // Computes the light position.
    glm::vec3 light_position =
        glm::vec3(15, 15, 15) * glm::vec3(cosf(gui_light_position / 6.0f) * sinf(gui_light_position), sinf(gui_light_position / 6.0f), cosf(gui_light_position / 6.0f) * cosf(gui_light_position));

    // Updates the light model visible in the scene.
    light_object.get_model_ubo().set_matrix(glm::translate(glm::mat4(1.0f), light_position) * glm::scale(glm::mat4(1.0f), glm::vec3(0.2f)));
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
    glViewport(0, 0, width, height);

    // Sets the clear values and clears the framebuffer.
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    // Renders the scene.
    render_scene();

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

void Application::render_scene() {
    camera_ubo.bind_buffer_base(CameraUBO::DEFAULT_CAMERA_BINDING);
    phong_lights_ubo.bind_buffer_base(PhongLightsUBO::DEFAULT_LIGHTS_BINDING);

    // TASK 4b: Render each object twice, first only the back faces (i.e. cull away the front faces) with black material and enlarged,
    //          and then only the front faces (i.e. cull away the black faces) and in a standard way.
    //			Hint: Remember OpenGL functions glEnable(GL_CULL_FACE), glDisable(GL_CULL_FACE), glCullFace(GL_FRONT), and glCullFace(GL_BACK).

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    expand_program.use();
    black_material_ubo.bind_buffer_base(PhongMaterialUBO::DEFAULT_MATERIAL_BINDING);
    for (SceneObject& object : scene_objects) {
        object.get_model_ubo().bind_buffer_base(ModelUBO::DEFAULT_MODEL_BINDING);
        object.get_geometry().bind_vao();
        object.get_geometry().draw();
    }
    glCullFace(GL_BACK);

    default_lit_program.use();
    for (SceneObject& object : scene_objects) {
        render_object(object, default_lit_program);
    }
    // We render the floor separately because we do not want outlines on it.
    render_object(floor_object, default_lit_program);
    render_object(light_object, default_unlit_program);

    glDisable(GL_CULL_FACE);
}

void Application::render_object(SceneObject& object, ShaderProgram& program) const {
    program.use();
    // Handles the textures.
    program.uniform("has_texture", object.has_texture());
    glBindTextureUnit(0, object.get_texture());
    // Binds the buffers with materials and model matrix.
    object.get_material().bind_buffer_base(PhongMaterialUBO::DEFAULT_MATERIAL_BINDING);
    object.get_model_ubo().bind_buffer_base(ModelUBO::DEFAULT_MODEL_BINDING);
    // Binds VAO and renders the object.
    object.get_geometry().bind_vao();
    object.get_geometry().draw();
}

// ----------------------------------------------------------------------------
// GUI
// ----------------------------------------------------------------------------
void Application::render_ui() {
    const float unit = ImGui::GetFontSize();

    ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoDecoration);
    ImGui::SetWindowSize(ImVec2(25 * unit, 6 * unit));
    ImGui::SetWindowPos(ImVec2(2 * unit, 2 * unit));

    std::string fps_cpu_string = "FPS (CPU): ";
    ImGui::Text(fps_cpu_string.append(std::to_string(fps_cpu)).c_str());

    std::string fps_string = "FPS (GPU): ";
    ImGui::Text(fps_string.append(std::to_string(fps_gpu)).c_str());

    ImGui::SliderAngle("Light Position", &gui_light_position, 0);
    ImGui::End();
}

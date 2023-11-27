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
    default_unlit_program = ShaderProgram(lecture_shaders_path / "unlit.vert", lecture_shaders_path / "unlit.frag");
    blinn_phong_original_program = ShaderProgram(lecture_shaders_path / "lit.vert", lecture_shaders_path / "blinn_phong_original.frag");
    blinn_phong_normalized_program = ShaderProgram(lecture_shaders_path / "lit.vert", lecture_shaders_path / "blinn_phong_normalized.frag");
    cook_torrance_program = ShaderProgram(lecture_shaders_path / "lit.vert", lecture_shaders_path / "cook_torrance.frag");
    oren_nayar_program = ShaderProgram(lecture_shaders_path / "lit.vert", lecture_shaders_path / "oren_nayar.frag");
    ashikhmin_shirley_program = ShaderProgram(lecture_shaders_path / "lit.vert", lecture_shaders_path / "ashikhmin_shirley.frag");

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

void Application::prepare_textures() {}

void Application::prepare_lights() {
    phong_lights_ubo.set_global_ambient(glm::vec3(0.2f));
}

void Application::prepare_scene() {
    // Sets the model matrix for the central object.
    central_object_ubo.set_matrix(scale(glm::mat4(1.0f), glm::vec3(2.0f)));
    central_object_ubo.update_opengl_data();

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
    phong_lights_ubo.add(PhongLightData::CreateDirectionalLight(light_position, glm::vec3(ambient_light_intensity), glm::vec3(light_intensity), glm::vec3(light_intensity)));
    phong_lights_ubo.update_opengl_data();

    // Update the data of the preset material
    switch (material_preset) {
    case MATERIAL_PRESET_USER:
        break; // Do nothing
    case MATERIAL_PRESET_PLASTIC_USER:
        material_fresnel = glm::vec3(0.04f, 0.04f, 0.04f);
        break;
    case MATERIAL_PRESET_PLASTIC_RED:
        material_fresnel = glm::vec3(0.04f, 0.04f, 0.04f);
        material_diffuse = glm::vec3(1.0f, 0.0f, 0.0f);
        break;
    case MATERIAL_PRESET_PLASTIC_GREEN:
        material_fresnel = glm::vec3(0.04f, 0.04f, 0.04f);
        material_diffuse = glm::vec3(0.0f, 1.0f, 0.0f);
        break;
    case MATERIAL_PRESET_PLASTIC_BLUE:
        material_fresnel = glm::vec3(0.04f, 0.04f, 0.04f);
        material_diffuse = glm::vec3(0.0f, 0.0f, 1.0f);
        break;
    case MATERIAL_PRESET_IRON:
        material_fresnel = glm::vec3(0.56f, 0.57f, 0.58f);
        material_diffuse = glm::vec3(0.0f, 0.0f, 0.0f);
        break;
    case MATERIAL_PRESET_COPPER:
        material_fresnel = glm::vec3(0.95f, 0.64f, 0.54f);
        material_diffuse = glm::vec3(0.0f, 0.0f, 0.0f);
        break;
    case MATERIAL_PRESET_GOLD:
        material_fresnel = glm::vec3(1.00f, 0.71f, 0.29f);
        material_diffuse = glm::vec3(0.0f, 0.0f, 0.0f);
        break;
    case MATERIAL_PRESET_ALUMINUM:
        material_fresnel = glm::vec3(0.91f, 0.92f, 0.92f);
        material_diffuse = glm::vec3(0.0f, 0.0f, 0.0f);
        break;
    case MATERIAL_PRESET_SILVER:
        material_fresnel = glm::vec3(0.95f, 0.93f, 0.88f);
        material_diffuse = glm::vec3(0.0f, 0.0f, 0.0f);
        break;
    default:
        break;
    }
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
    set_materials();
    render_central();
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

/// Sets the parameters for the currently selected material.
void Application::set_materials() {
    switch (material_type) {
    case MATERIAL_TYPE_BLINN_PHONG_ORIGINAL:
        blinn_phong_original_program.use();
        blinn_phong_original_program.uniform("material_diffuse", material_diffuse);
        blinn_phong_original_program.uniform("material_specular", material_specular);
       blinn_phong_original_program.uniform("material_shininess", material_shininess);
        break;
    case MATERIAL_TYPE_BLINN_PHONG_NORMALIZED:
        blinn_phong_normalized_program.use();
        blinn_phong_normalized_program.uniform("material_diffuse", material_diffuse);
        blinn_phong_normalized_program.uniform("material_specular", material_specular);
        blinn_phong_normalized_program.uniform("material_shininess", material_shininess);
        break;
    case MATERIAL_TYPE_COOK_TORRANCE:
        cook_torrance_program.use();
        glUniform3fv(cook_torrance_program.get_uniform_location("material_fresnel"), 1, glm::value_ptr(material_fresnel));
        glUniform3fv(cook_torrance_program.get_uniform_location("material_diffuse"), 1, glm::value_ptr(material_diffuse));
        cook_torrance_program.uniform("material_roughness", material_roughness);
        break;
    case MATERIAL_TYPE_OREN_NAYAR:
        oren_nayar_program.use();
        oren_nayar_program.uniform("material_diffuse", material_diffuse);
        oren_nayar_program.uniform("material_roughness", material_roughness);
        break;
    case MATERIAL_TYPE_ASHIKHMIN_SHIRLEY:
        ashikhmin_shirley_program.use();
        ashikhmin_shirley_program.uniform("material_fresnel", material_fresnel);
        ashikhmin_shirley_program.uniform("material_diffuse", material_diffuse);
        ashikhmin_shirley_program.uniform("material_shininess_tangent", material_shininess_tangent);
        ashikhmin_shirley_program.uniform("material_shininess_bitangent", material_shininess_bitangent);
        break;
    }
}

void Application::render_central() const {
    // Sets central object's position.
    central_object_ubo.bind_buffer_base(ModelUBO::DEFAULT_MODEL_BINDING);

    // Renders a selected central object.
    switch (central_object) {
    case CENTRAL_OBJECT_CUBE:
        cube.bind_vao();
        cube.draw();
        break;
    case CENTRAL_OBJECT_SPHERE:
        sphere.bind_vao();
        sphere.draw();
        break;
    case CENTRAL_OBJECT_TORUS:
        torus.bind_vao();
        torus.draw();
        break;
    case CENTRAL_OBJECT_CYLINDER:
        cylinder.bind_vao();
        cylinder.draw();
        break;
    case CENTRAL_OBJECT_CAPSULE:
        capsule.bind_vao();
        capsule.draw();
        break;
    case CENTRAL_OBJECT_TEAPOT:
        teapot.bind_vao();
        teapot.draw();
        break;
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
    ImGui::SetWindowSize(ImVec2(23 * unit, 25 * unit));
    ImGui::SetWindowPos(ImVec2(2 * unit, 2 * unit));

    std::string fps_cpu_string = "FPS (CPU): ";
    ImGui::Text(fps_cpu_string.append(std::to_string(fps_cpu)).c_str());

    std::string fps_string = "FPS (GPU): ";
    ImGui::Text(fps_string.append(std::to_string(fps_gpu)).c_str());

    ImGui::PushItemWidth(180);
    ImGui::SliderAngle("Light Position", &gui_light_position, 0);
    ImGui::SliderFloat("Light Intensity", &light_intensity, 0, 2, "%.2f");
    ImGui::SliderFloat("Ambient Intensity", &ambient_light_intensity, 0, 0.1, "%.2f");

    ImGui::Combo("Central Object", &central_object, CENTRAL_OBJECT_LABELS, IM_ARRAYSIZE(CENTRAL_OBJECT_LABELS));

    ImGui::Combo("Material Preset", &material_preset, MATERIAL_PRESET_LABELS, IM_ARRAYSIZE(MATERIAL_PRESET_LABELS));

    ImGui::ColorEdit3("Diffuse Color", reinterpret_cast<float*>(&material_diffuse));
    ImGui::ColorEdit3("Fresnel Color", reinterpret_cast<float*>(&material_fresnel));
    ImGui::ColorEdit3("Specular Color", reinterpret_cast<float*>(&material_specular));

    ImGui::SliderFloat("Roughness", &material_roughness, 0, 1, "%.2f");
    ImGui::SliderFloat("Shininess", &material_shininess, 0, 500, "%.0f");
    ImGui::SliderFloat("Shininess (Tangent)", &material_shininess_tangent, 0, 500, "%.0f");
    ImGui::SliderFloat("Shininess (Bitangent)", &material_shininess_bitangent, 0, 500, "%.0f");

    ImGui::Combo("Material", &material_type, MATERIAL_TYPE_LABELS, IM_ARRAYSIZE(MATERIAL_TYPE_LABELS));

    ImGui::End();
}

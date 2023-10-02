#include "application.hpp"
#include "utils.hpp"

Application::Application(int initial_width, int initial_height, std::vector<std::string> arguments)
    : PV227Application(initial_width, initial_height, arguments) {
    Application::compile_shaders();
    prepare_cameras();
    prepare_textures();
    prepare_lights();
    prepare_scene();
    prepare_framebuffers();
}

Application::~Application() {}

// ----------------------------------------------------------------------------
// Shaderes
// ----------------------------------------------------------------------------
void Application::compile_shaders() {
    default_unlit_program = ShaderProgram(lecture_shaders_path / "object.vert", lecture_shaders_path / "unlit.frag");
    default_lit_program = ShaderProgram(lecture_shaders_path / "object.vert", lecture_shaders_path / "lit.frag");
    display_texture_program = ShaderProgram(lecture_shaders_path / "full_screen_quad.vert", lecture_shaders_path / "display_texture.frag");
    // TASK 3: Prepare special shader program for rendering into the shadow map.
    generate_shadow_program = ShaderProgram(lecture_shaders_path / "object.vert", lecture_shaders_path / "nothing.frag");

    std::cout << "Shaders are reloaded." << std::endl;
}

// ----------------------------------------------------------------------------
// Initialize Scene
// ----------------------------------------------------------------------------
void Application::prepare_cameras() {
    // Sets the default camera position.
    camera.set_eye_position(glm::radians(-45.f), glm::radians(20.f), 25.f);
    // Computes the projection matrix.
    camera_ubo.set_projection(
        glm::perspective(glm::radians(45.f), static_cast<float>(this->width) / static_cast<float>(this->height), 0.1f, 1000.0f));
    camera_ubo.update_opengl_data();

    // TASK 1: Initialize the projection matrix for the light camera using glm::perspective function.
    //   Hint: Use glm::perspective, use 80 degrees as field of view, 2.0 as near plane, and 30.0 as far plane.
    //		   Think about the value of aspect ratio - what value should you use?
    light_camera_projection = glm::perspective(glm::radians(80.0f), 1.0f, 2.0f, 30.0f);
    light_camera_view = glm::mat4(1.0f);
    light_camera_data_ubo.set_projection(light_camera_projection);
    light_camera_data_ubo.set_view(light_camera_view);
    light_camera_data_ubo.update_opengl_data();
}

void Application::prepare_textures() {
    wood_tex = TextureUtils::load_texture_2d(framework_textures_path / "wood.png");
    TextureUtils::set_texture_2d_parameters(wood_tex, GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);
}

void Application::prepare_lights() {
    // The rest is set in the update scene method.
    phong_lights_ubo.set_global_ambient(glm::vec3(0.0f));
}

void Application::prepare_scene() {
    ModelUBO floor_model_ubo(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.1f, 0.0f)) *
                             glm::scale(glm::mat4(1.0f), glm::vec3(10.0f, 0.1f, 10.0f)));
    SceneObject floor_object(cube, floor_model_ubo, gray_material_ubo);
    scene_objects.push_back(floor_object);

    ModelUBO teapot_model_ubo(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)));
    SceneObject teapot_object(teapot, teapot_model_ubo, red_material_ubo);
    scene_objects.push_back(teapot_object);

    ModelUBO cube1_model_ubo(glm::translate(glm::mat4(1.0f), glm::vec3(-5.0f, 1.0f, 0.0f)));
    SceneObject cube_object1(cube, cube1_model_ubo, white_material_ubo, wood_tex);
    scene_objects.push_back(cube_object1);

    ModelUBO cube2_model_ubo(glm::translate(glm::mat4(1.0f), glm::vec3(-2.5f, 1.0f, -4.5f)));
    SceneObject cube_object2(cube, cube2_model_ubo, white_material_ubo, wood_tex);
    scene_objects.push_back(cube_object2);

    ModelUBO cube3_model_ubo(glm::translate(glm::mat4(1.0f), glm::vec3(-2.5f, 1.0f, 4.5f)));
    SceneObject cube_object3(cube, cube3_model_ubo, white_material_ubo, wood_tex);
    scene_objects.push_back(cube_object3);

    // The tori are animated so we update their model matrix later.
    torus_object_1 = SceneObject(torus, ModelUBO(), blue_material_ubo);
    scene_objects.push_back(torus_object_1);
    torus_object_2 = SceneObject(torus, ModelUBO(), blue_material_ubo);
    scene_objects.push_back(torus_object_2);
    torus_object_3 = SceneObject(torus, ModelUBO(), blue_material_ubo);
    scene_objects.push_back(torus_object_3);

    // The light model is placed based on the value from UI so we update its model matrix later.
    // We are also not adding it to the scene_objects since we render it separately.
    light_object = SceneObject(sphere, ModelUBO(), white_material_ubo);
}

void Application::prepare_framebuffers() {
    // Creates a texture for storing the shadow map.
    glCreateTextures(GL_TEXTURE_2D, 1, &shadow_texture);
    glTextureStorage2D(shadow_texture, 1, GL_DEPTH_COMPONENT24, shadow_tex_size, shadow_tex_size);
    TextureUtils::set_texture_2d_parameters(shadow_texture, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);

    // Creates a framebuffer for rendering the shadow map.
    glCreateFramebuffers(1, &shadow_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo);
    glNamedFramebufferTexture(shadow_fbo, GL_DEPTH_ATTACHMENT, shadow_texture, 0);
    FBOUtils::check_framebuffer_status(shadow_fbo, "ShadowFBO");
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
        glm::vec3(15, 20, 15) * glm::vec3(cosf(gui_light_position / 6.0f) * sinf(gui_light_position), sinf(gui_light_position / 6.0f),
                                          cosf(gui_light_position / 6.0f) * cosf(gui_light_position));

    // Updates the light model visible in the scene.
    light_object.get_model_ubo().set_matrix(glm::translate(glm::mat4(1.0f), light_position) * glm::scale(glm::mat4(1.0f), glm::vec3(0.2f)));
    light_object.get_model_ubo().update_opengl_data();

    // Updates the OpenGL buffer storing the information about the light.
    phong_lights_ubo.clear();
    phong_lights_ubo.add(PhongLightData::CreateSpotLight(light_position, glm::vec3(0.05f), glm::vec3(0.95f), glm::vec3(1.0f),
                                                         -normalize(light_position), 20.0f, cosf(glm::radians(40.0f))));
    phong_lights_ubo.update_opengl_data();

    // Animates the objects.
    torus_object_1.get_model_ubo().set_matrix(
        glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 1.5f, 0.0f)) *
        glm::rotate(glm::mat4(1.0f), static_cast<float>(elapsed_time) * 0.002f, glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
    torus_object_2.get_model_ubo().set_matrix(
        glm::translate(glm::mat4(1.0f), glm::vec3(2.5f, 1.5f, -4.5f)) *
        glm::rotate(glm::mat4(1.0f), static_cast<float>(elapsed_time) * 0.002f, glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
    torus_object_3.get_model_ubo().set_matrix(
        glm::translate(glm::mat4(1.0f), glm::vec3(2.5f, 1.5f, 4.5f)) *
        glm::rotate(glm::mat4(1.0f), static_cast<float>(elapsed_time) * 0.002f, glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
    torus_object_1.get_model_ubo().update_opengl_data();
    torus_object_2.get_model_ubo().update_opengl_data();
    torus_object_3.get_model_ubo().update_opengl_data();

    // TASK 1: Update the view matrix for the light camera using glm::lookAt function.
    //	 Hint: Use data of the first light from phong_lights_ubo (phong_lights_ubo.get_lights()[0]).
    //	       The light contains 'position' and 'spot_direction' variables that can be quite handy.
    light_camera_view = glm::lookAt(glm::vec3(phong_lights_ubo.get_lights()[0].position),
                                    glm::vec3(phong_lights_ubo.get_lights()[0].position) + phong_lights_ubo.get_lights()[0].spot_direction,
                                    glm::vec3(0.0f, 1.0f, 0.0f));
    light_camera_data_ubo.set_view(light_camera_view);
    light_camera_data_ubo.update_opengl_data();

    // TASK 4: Compute and update shadow matrix.
    //	 Hint: glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z))
    //	 Hint: glm::scale(glm::mat4(1.0f), glm::vec3(x, y, z))
    // TASK 7: Add additional translation to the shadow matrix.
    shadow_matrix =
        glm::translate(glm::mat4(1.0f), glm::vec3(0.5f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.5f)) * light_camera_projection *
        light_camera_view;
}

// ----------------------------------------------------------------------------
// Render
// ----------------------------------------------------------------------------
void Application::render() {
    // Starts measuring the elapsed time.
    glBeginQuery(GL_TIME_ELAPSED, render_time_query);

    //--------------------------------------
    // Renders into the shadow texture.
    //--------------------------------------

    // TASK 2: Render the scene into the shadow map.
    //   Hint: Do the same as if you rendered into the main window, just use correct FBO and change the associated parameters.
    //   Hint: See the definition of 'shadow_fbo' and 'shadow_tex_size';

    glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo);
    glViewport(0, 0, shadow_tex_size, shadow_tex_size);

    // Clears the framebuffer -> clears only the depth (there is no color).
    glClearDepth(1.0);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    light_camera_data_ubo.bind_buffer_base(CameraUBO::DEFAULT_CAMERA_BINDING);

    render_scene(true, true);

    //--------------------------------------
    // Renders into the main window.
    //--------------------------------------

    // Binds the main window framebuffer.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);

    // Clear the framebuffer, clear the color and depth
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    switch (what_to_display) {
    case DISPLAY_VIEW_FROM_MAIN_CAMERA: {
        // We use the main camera.
        camera_ubo.bind_buffer_base(CameraUBO::DEFAULT_CAMERA_BINDING);
        // Sets the lights.
        phong_lights_ubo.bind_buffer_base(PhongLightsUBO::DEFAULT_LIGHTS_BINDING);
        // Renders the scene.
        render_scene(false, false);
        break;
    }
    case DISPLAY_VIEW_FROM_LIGHT: {
        // TASK 1: Render the scene from the position of the light.
        //   Hint: See the code above for 'DISPLAY_VIEW_FROM_MAIN_CAMERA'.

        // We use the light camera.
        light_camera_data_ubo.bind_buffer_base(CameraUBO::DEFAULT_CAMERA_BINDING);
        // Sets the lights.
        phong_lights_ubo.bind_buffer_base(PhongLightsUBO::DEFAULT_LIGHTS_BINDING);
        // Renders the scene.
        render_scene(true, false);
        break;
    }
    case DISPLAY_SHADOW_TEXTURE: {
        // Uses a special shader and render the shadow texture in grayscale.
        display_texture_program.use();
        glDisable(GL_DEPTH_TEST);

        // TASK 2: Display the shadow map (see also the TASK 2 in the display_texture.frag).
        //	 Hint: Use glBindTextureUnit as if it was any other texture.
        glBindTextureUnit(0, shadow_texture);
        // TASK 8: Use GL_NONE as GL_TEXTURE_COMPARE_MODE when using shadow texture
        //	 Hint: It means calling glTextureParameteri(shadow_texture, GL_TEXTURE_COMPARE_MODE, GL_NONE).

        // Renders the full screen quad to evaluate every pixel.
        // Binds an empty VAO as we do not need any state.
        glBindVertexArray(empty_vao);
        // Calls a draw command with 3 vertices that are generated in vertex shader.
        glDrawArrays(GL_TRIANGLES, 0, 3);
        break;
    }
    }

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

void Application::render_scene(bool from_light, bool gen_shadows) {
    // TASK 1: Do not render the light object when rendering from the light.
    //	 Hint: Utilize parameters of this function.
    // TASK 3: Use special shader program for rendering into the shadow map.
    //   Hint: Utilize parameters of this function.
    // TASK 4: Send shadow matrix into appropriate shader program.
    //	 Hint: To update uniform variable in shader program, use my_program.uniform_matrix("shadow_matrix", shadow_matrix);
    //		   Call it when the program is active.
    // TASK 5: Send 'shadow_texture' into appropriate shader program.
    //	 Hint: We use texture unit 1 for all shaders, so that the texture could be bound only once, at the beginning of this function.
    // TASK 7: Use glPolygonOffset(2.0f, 0.0f)
    //	 Hint: Don't forget to glEnable/glDisable(GL_POLYGON_OFFSET_FILL)
    // TASK 7: Cull front faces.
    //	 Hint: Use glEnable/glDisable(GL_CULL_FACE), glCullFace(GL_FRONT) functions.
    // TASK 8: Use GL_COMPARE_REF_TO_TEXTURE as GL_TEXTURE_COMPARE_MODE when using shadow texture.
    //	 Hint: It means calling glTextureParameteri(shadow_texture, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE).
    //	 Hint: Set also the compare function with glTextureParameteri(shadow_texture, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL).

    default_lit_program.uniform_matrix("shadow_matrix", shadow_matrix);
    for (SceneObject& object : scene_objects) {
        render_object(object, gen_shadows ? generate_shadow_program : default_lit_program);
    }

    if (!from_light) {
        render_object(light_object, default_unlit_program);
    }
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
    ImGui::SetWindowSize(ImVec2(25 * unit, 7 * unit));
    ImGui::SetWindowPos(ImVec2(2 * unit, 2 * unit));

    std::string fps_cpu_string = "FPS (CPU): ";
    ImGui::Text(fps_cpu_string.append(std::to_string(fps_cpu)).c_str());

    std::string fps_string = "FPS (GPU): ";
    ImGui::Text(fps_string.append(std::to_string(fps_gpu)).c_str());

    ImGui::SliderAngle("Light Position", &gui_light_position, 0);

    ImGui::Combo("Display", &what_to_display, DISPLAY_LABELS, IM_ARRAYSIZE(DISPLAY_LABELS));

    ImGui::End();
}

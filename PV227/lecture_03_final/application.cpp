#include "application.hpp"
#include "utils.hpp"

Application::Application(int initial_width, int initial_height, std::vector<std::string> arguments) : PV227Application(initial_width, initial_height, arguments) {
    Application::compile_shaders();
    prepare_cameras();
    prepare_materials();
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
    forward_program = ShaderProgram(lecture_shaders_path / "object.vert", lecture_shaders_path / "forward.frag");
    deferred_program = ShaderProgram(lecture_shaders_path / "object.vert", lecture_shaders_path / "deferred.frag");
    display_texture_program = ShaderProgram(lecture_shaders_path / "full_screen_quad.vert", lecture_shaders_path / "display_texture.frag");
    evaluate_quad_program = ShaderProgram(lecture_shaders_path / "full_screen_quad.vert", lecture_shaders_path / "evaluate_lighting_quad.frag");
    evaluate_sphere_program = ShaderProgram(lecture_shaders_path / "evaluate_lightning_sphere.vert", lecture_shaders_path / "evaluate_lightning_sphere.frag");

    std::cout << "Shaders are reloaded." << std::endl;
}

// ----------------------------------------------------------------------------
// Initialize Scene
// ----------------------------------------------------------------------------
void Application::prepare_cameras() {
    // Sets the default camera position.
    camera.set_eye_position(glm::radians(-45.f), glm::radians(20.f), 50.f);
    // Computes the projection matrix.
    camera_ubo.set_projection(glm::perspective(glm::radians(45.f), static_cast<float>(this->width) / static_cast<float>(this->height), 0.1f, 1000.0f));
    camera_ubo.update_opengl_data();
}

void Application::prepare_materials() {
    color_ubos.push_back(red_material_ubo);
    color_ubos.push_back(green_material_ubo);
    color_ubos.push_back(blue_material_ubo);
    color_ubos.push_back(cyan_material_ubo);
    color_ubos.push_back(magenta_material_ubo);
    color_ubos.push_back(yellow_material_ubo);
    color_ubos.push_back(white_material_ubo);
}

void Application::prepare_textures() {
    const GLuint wood_tex = TextureUtils::load_texture_2d(framework_textures_path / "wood.png");
    TextureUtils::set_texture_2d_parameters(wood_tex, GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);

    const GLuint lenna_tex = TextureUtils::load_texture_2d(framework_textures_path / "lenna.png");
    TextureUtils::set_texture_2d_parameters(lenna_tex, GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);

    textures.push_back(wood_tex);
    textures.push_back(lenna_tex);
}

void Application::prepare_lights() {
    phong_lights_ubo = PhongLightsUBO(LIGHTS_MAX_COUNT, LIGHTS_STORAGE);
    phong_lights_ubo.set_global_ambient(glm::vec3(0.0f));

    // Creates all lights, including those that will not be used.
    glm::vec3 lights_start = glm::vec3(grid_start.x - grid_spacing.x * 0.5f, 1.0f, grid_start.z - grid_spacing.z * 0.5f);
    glm::vec3 lights_size = glm::vec3(grid_spacing.x * static_cast<float>(grid_size.x), 2.0f, grid_spacing.z * static_cast<float>(grid_size.z));
    for (int i = 0; i < LIGHTS_MAX_COUNT; i++) {
        // Places the light at a random location int the scene.
        glm::vec3 light_position = lights_start + lights_size * glm::vec3(static_cast<float>(rand()), static_cast<float>(rand()), static_cast<float>(rand())) / static_cast<float>(RAND_MAX);

        // Gives the light a random color. Thanks to the fact that GLM has functions for HSV -> RGB conversion,
        // we choose hue randomly, use 1.0 for saturation and value, and convert it to RGB, thus obtaining
        // a uniformly random colors.
        glm::vec3 light_color = glm::rgbColor(glm::vec3(static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 360.0f, 1.0f, 1.0f));

        // We use black ambient (the scene contains hundreds of lights, non-black ambient would cause "overexposure"),
        // We also use attenuation (again, without attenuation, the objects would be "overexposed").
        PhongLightData light = PhongLightData::CreatePointLight(light_position, glm::vec3(0.0f), light_color, light_color, 0.2f, 0.0f, 1.0f);
        phong_lights_ubo.add(light);
        all_lights.push_back(light);
    }
    phong_lights_ubo.update_opengl_data();
}

void Application::prepare_scene() {
    // Creates a random scene. We create a grid of random objects with random materials.
    srand(12345);

    // Prepares a list of geometries to pick from.
    // We also define matrices that describe how to transform the objects so that they "sit" on XZ plane.
    std::vector<std::pair<Geometry*, glm::mat4>> geometries;
    geometries.push_back(std::make_pair(&cube, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, 0.0f))));
    geometries.push_back(std::make_pair(&sphere, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, 0.0f))));
    geometries.push_back(std::make_pair(&torus, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.5f, 0.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f))));
    geometries.push_back(std::make_pair(&cylinder, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, 0.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f))));
    geometries.push_back(std::make_pair(&capsule, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, 0.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f))));
    geometries.push_back(std::make_pair(&teapot, glm::mat4(1.0f)));

    for (int x = 0; x < grid_size.x; x++)
        for (int z = 0; z < grid_size.z; z++) {
            // Chooses randomly the geometry.
            const int geometry = rand() % static_cast<int>(geometries.size());
            // Rotates the object randomly around the y axis.
            glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * static_cast<float>(M_PI) * 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));
            // Places the object at the proper place in the grid
            glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3(grid_start.x + static_cast<float>(x) * grid_spacing.x, 0.0f, grid_start.z + static_cast<float>(z) * grid_spacing.z));
            // Chooses the material randomly, choose randomly between the colors and the textures.
            const int material = rand() % static_cast<int>(color_ubos.size() + textures.size());
            // Computes the model matrix.
            ModelUBO model_ubo(translation * rotation * geometries[geometry].second);
            // Creates the scene object and add it into the list.
            SceneObject scene_object;
            if (material < static_cast<int>(color_ubos.size())) {
                // Objects without textures.
                scene_object = SceneObject(*geometries[geometry].first, model_ubo, color_ubos[material]);
            } else {
                // Objects with texture.
                scene_object = SceneObject(*geometries[geometry].first, model_ubo, white_material_ubo, textures[material - static_cast<int>(color_ubos.size())]);
            }
            scene_objects.push_back(scene_object);
        }

    // Prepares the floor model.
    const ModelUBO floor_model_ubo(
        glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.1f, 0.0f)) *
        glm::scale(glm::mat4(1.0f), glm::vec3(grid_spacing.x * static_cast<float>(grid_size.x) / 2.0f + 5.0f, 0.1f, grid_spacing.z * static_cast<float>(grid_size.z) / 2.0f + 5.0f)));
    const SceneObject floor_object = SceneObject(cube, floor_model_ubo, gray_material_ubo);
    scene_objects.push_back(floor_object);
}

void Application::prepare_framebuffers() {
    // Creates the framebuffer for rendering into the G-buffer.
    glCreateFramebuffers(1, &gbuffer_fbo);
    // Specifies a list of color buffers to be drawn into.
    glNamedFramebufferDrawBuffers(gbuffer_fbo, 3, FBOUtils::draw_buffers_constants);

    // Creates the framebuffer for evaluation of the G-buffer.
    glCreateFramebuffers(1, &normal_scene_fbo);
    // Specifies a list of color buffers to be drawn into.
    glNamedFramebufferDrawBuffers(normal_scene_fbo, 1, FBOUtils::draw_buffers_constants);

    // Creates and binds the required textures.
    resize_fullscreen_textures();
}

void Application::resize_fullscreen_textures() {
    // Removes the previously allocated textures (if any).
    glDeleteTextures(1, &gbuffer_position_texture);
    glDeleteTextures(1, &gbuffer_normal_texture);
    glDeleteTextures(1, &gbuffer_albedo_texture);
    glDeleteTextures(1, &gbuffer_depth_texture);
    glDeleteTextures(1, &final_color_texture);

    // Creates new textures for G-buffer and set their basic parameters.
    glCreateTextures(GL_TEXTURE_2D, 1, &gbuffer_position_texture);
    glCreateTextures(GL_TEXTURE_2D, 1, &gbuffer_normal_texture);
    glCreateTextures(GL_TEXTURE_2D, 1, &gbuffer_albedo_texture);
    glCreateTextures(GL_TEXTURE_2D, 1, &gbuffer_depth_texture);
    glCreateTextures(GL_TEXTURE_2D, 1, &final_color_texture);

    // Initializes the immutable storage.
    glTextureStorage2D(gbuffer_position_texture, 1, GL_RGBA32F, width, height);
    glTextureStorage2D(gbuffer_normal_texture, 1, GL_RGBA32F, width, height);
    glTextureStorage2D(gbuffer_albedo_texture, 1, GL_RGBA32F, width, height);
    glTextureStorage2D(gbuffer_depth_texture, 1, GL_DEPTH_COMPONENT24, width, height);
    glTextureStorage2D(final_color_texture, 1, GL_RGBA32F, width, height);

    // Sets the texture parameters.
    TextureUtils::set_texture_2d_parameters(gbuffer_position_texture, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
    TextureUtils::set_texture_2d_parameters(gbuffer_normal_texture, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
    TextureUtils::set_texture_2d_parameters(gbuffer_albedo_texture, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
    TextureUtils::set_texture_2d_parameters(gbuffer_depth_texture, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
    TextureUtils::set_texture_2d_parameters(final_color_texture, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);

    // Binds the texture to the G-buffer.
    glNamedFramebufferTexture(gbuffer_fbo, GL_COLOR_ATTACHMENT0, gbuffer_position_texture, 0);
    glNamedFramebufferTexture(gbuffer_fbo, GL_COLOR_ATTACHMENT1, gbuffer_normal_texture, 0);
    glNamedFramebufferTexture(gbuffer_fbo, GL_COLOR_ATTACHMENT2, gbuffer_albedo_texture, 0);
    glNamedFramebufferTexture(gbuffer_fbo, GL_DEPTH_ATTACHMENT, gbuffer_depth_texture, 0);
    FBOUtils::check_framebuffer_status(gbuffer_fbo, "G-buffer");

    // Binds the textures to the evaluation buffer.
    glNamedFramebufferTexture(normal_scene_fbo, GL_COLOR_ATTACHMENT0, final_color_texture, 0);
    glNamedFramebufferTexture(normal_scene_fbo, GL_DEPTH_ATTACHMENT, gbuffer_depth_texture, 0);
    FBOUtils::check_framebuffer_status(normal_scene_fbo, "Evaluate G-buffer");
}

// ----------------------------------------------------------------------------
// Update
// ----------------------------------------------------------------------------
void Application::update(float delta) {
    PV227Application::update(delta);

    // Updates the list of lights.
    phong_lights_ubo.clear();
    for (int i = 0; i < number_of_lights; i++) {
        phong_lights_ubo.add(all_lights[i]);
    }
    phong_lights_ubo.update_opengl_data();

    // Updates the main camera.
    const glm::vec3 eye_position = camera.get_eye_position();
    camera_ubo.set_view(glm::lookAt(eye_position, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
    camera_ubo.update_opengl_data();
}

// ----------------------------------------------------------------------------
// Render
// ----------------------------------------------------------------------------
void Application::render() {
    // Starts measuring the elapsed time.
    glBeginQuery(GL_TIME_ELAPSED, render_time_query);

    if (what_to_display == FORWARD_SHADING) {
        // Uses standard forward shading approach.
        render_using_forward_shading();
    } else {
        // First, we need to render into the G-buffer.
        render_into_g_buffer();

        if (what_to_display == DISPLAY_POSITION || what_to_display == DISPLAY_NORMAL || what_to_display == DISPLAY_ALBEDO) {
            // Displays the position texture, normal texture, or albedo texture.
            show_g_buffer_textures();
        } else if (what_to_display == DEFERRED_SHADING_EVALUATE_QUAD) {
            // Evaluates the lighting using a full-screen quad.
            evaluate_lighting_using_quad();
        } else if (what_to_display == DEFERRED_SHADING_EVALUATE_SPHERE) {
            // Evaluates the lighting using a spheres
            evaluate_lighting_using_spheres();
        }
    }

    // Resets the VAO and the program.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

void Application::render_scene(ShaderProgram& program) {
    program.use();

    camera_ubo.bind_buffer_base(CameraUBO::DEFAULT_CAMERA_BINDING);
    phong_lights_ubo.bind_buffer_base(PhongLightsUBO::DEFAULT_LIGHTS_BINDING);

    for (SceneObject& object : scene_objects) {
        // Handles the textures.
        program.uniform("has_texture", object.has_texture());
        if (object.has_texture()) {
            glBindTextureUnit(0, object.get_texture());
        }

        // Calls the standard rendering functions.
        object.get_model_ubo().bind_buffer_base(ModelUBO::DEFAULT_MODEL_BINDING);
        object.get_material().bind_buffer_base(PhongMaterialUBO::DEFAULT_MATERIAL_BINDING);
        object.get_geometry().bind_vao();
        object.get_geometry().draw();
    }
}

void Application::render_using_forward_shading() {
    // Binds the main window framebuffer.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);

    // Sets the clear values and clears the framebuffer.
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    render_scene(forward_program);
}

void Application::render_into_g_buffer() {
    // Binds G-Buffer.
    glBindFramebuffer(GL_FRAMEBUFFER, gbuffer_fbo);
    glViewport(0, 0, width, height);

    // Clears the G-buffer, clear all textures to (0,0,0,0) and depth to 1.0.
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    // Renders all objects in the scene using deferred shading program.
    render_scene(deferred_program);
}

void Application::show_g_buffer_textures() {
    // Binds the main window framebuffer.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);

    // Clears the framebuffer color.
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    // We do not need depth and depth test.
    glDisable(GL_DEPTH_TEST);

    // Use the proper program
    display_texture_program.use();
    if (what_to_display == DISPLAY_POSITION) {
        // Sets the transformation matrix - scale the position to 0.02.
        display_texture_program.uniform_matrix("transformation", glm::scale(glm::mat4(1.0f), glm::vec3(0.02f)));
        // Binds the proper texture.
        glBindTextureUnit(0, gbuffer_position_texture);
    } else if (what_to_display == DISPLAY_NORMAL) {
        // Sets the transformation matrix - no scale.
        display_texture_program.uniform_matrix("transformation", glm::mat4(1.0f));
        // Binds the proper texture.
        glBindTextureUnit(0, gbuffer_normal_texture);
    } else if (what_to_display == DISPLAY_ALBEDO) {
        // Sets the transformation matrix - no scale.
        display_texture_program.uniform_matrix("transformation", glm::mat4(1.0f));
        // Binds the proper texture.
        glBindTextureUnit(0, gbuffer_albedo_texture);
    }

    // Renders the full screen quad to evaluate every pixel.
    // Binds an empty VAO as we do not need any state.
    glBindVertexArray(empty_vao);
    // Calls a draw command with 3 vertices that are generated in vertex shader.
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void Application::evaluate_lighting_using_quad() {
    // Binds the main window framebuffer.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);

    // Clears the framebuffer color.
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    // We do not need depth and depth test.
    glDisable(GL_DEPTH_TEST);

    // Uses the proper program.
    evaluate_quad_program.use();

    // Binds all three textures.
    glBindTextureUnit(0, gbuffer_position_texture);
    glBindTextureUnit(1, gbuffer_normal_texture);
    glBindTextureUnit(2, gbuffer_albedo_texture);

    // Binds the data with the camera and the lights.
    camera_ubo.bind_buffer_base(CameraUBO::DEFAULT_CAMERA_BINDING);
    phong_lights_ubo.bind_buffer_base(PhongLightsUBO::DEFAULT_LIGHTS_BINDING);

    // Binds the data with specular and shininess (this is a disadvantage of this approach, all materials must have the same material).
    white_material_ubo.bind_buffer_base(PhongMaterialUBO::DEFAULT_MATERIAL_BINDING);

    // Renders the full screen quad to evaluate every pixel.
    // Binds an empty VAO as we do not need any state.
    glBindVertexArray(empty_vao);
    // Calls a draw command with 3 vertices that are generated in vertex shader.
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void Application::evaluate_lighting_using_spheres() {
    // Binds the framebuffer for evaluation.
    glBindFramebuffer(GL_FRAMEBUFFER, normal_scene_fbo);
    glViewport(0, 0, width, height);

    // Clears the framebuffer color.
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // TASK 3: Configure the depth test, blending, and culling.
    //	 Hint: glEnable/glDisable(GL_DEPTH_TEST/GL_BLEND/GL_CULL_FACE),
    //         glDepthMask(GL_TRUE/GL_FALSE),
    //	       glCullFace(GL_FRONT/GL_BACK),
    //         glBlendFunc(GL_ZERO/GL_ONE/GL_SRC_ALPHA/GL_ONE_MINUS_SRC_ALPHA)

    // Uses depth test, but do not render into the depth buffer.
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    // Uses blending, i.e. enables blending and sets the blend function.
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    // Culls away back faces.
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // Uses the proper program.
    evaluate_sphere_program.use();

    // Binds all three textures.
    glBindTextureUnit(0, gbuffer_position_texture);
    glBindTextureUnit(1, gbuffer_normal_texture);
    glBindTextureUnit(2, gbuffer_albedo_texture);

    // Binds the data with the camera and the lights.
    camera_ubo.bind_buffer_base(CameraUBO::DEFAULT_CAMERA_BINDING);
    phong_lights_ubo.bind_buffer_base(PhongLightsUBO::DEFAULT_LIGHTS_BINDING);

    // Binds the data with specular and shininess (this is a disadvantage of this approach, all objects must have the same material).
    white_material_ubo.bind_buffer_base(PhongMaterialUBO::DEFAULT_MATERIAL_BINDING);

    // Render spheres using instancing, one sphere for each active light
    sphere.bind_vao();
    sphere.draw_instanced(phong_lights_ubo.get_lights().size());

    // TASK 3: Reset the depth test, blending, and culling.
    //	 Hint: Disable blending and culling, enable writing into the depth buffer.
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);

    // Copies the result into the main window. We do this, because we needed access to the depth texture from the first pass. 
    glBindFramebuffer(GL_READ_FRAMEBUFFER, normal_scene_fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

// ----------------------------------------------------------------------------
// GUI
// ----------------------------------------------------------------------------
void Application::render_ui() {
    const float unit = ImGui::GetFontSize();

    ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoDecoration);
    ImGui::SetWindowSize(ImVec2(25 * unit, 8 * unit));
    ImGui::SetWindowPos(ImVec2(2 * unit, 2 * unit));

    std::string fps_cpu_string = "FPS (CPU): ";
    ImGui::Text(fps_cpu_string.append(std::to_string(fps_cpu)).c_str());

    std::string fps_string = "FPS (GPU): ";
    ImGui::Text(fps_string.append(std::to_string(fps_gpu)).c_str());

    ImGui::Combo("Display", &what_to_display, DISPLAY_LABELS, IM_ARRAYSIZE(DISPLAY_LABELS));
    ImGui::SliderInt("Lights", &number_of_lights, 1, LIGHTS_MAX_COUNT);

    ImGui::End();
}

// ----------------------------------------------------------------------------
// Input Events
// ----------------------------------------------------------------------------
void Application::on_resize(int width, int height) {
    PV227Application::on_resize(width, height);
    resize_fullscreen_textures();
}
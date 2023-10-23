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
    compute_random_samples();
    compute_random_tangent_directions();
}

Application::~Application() {}

// ----------------------------------------------------------------------------
// Shaderes
// ----------------------------------------------------------------------------
void Application::compile_shaders() {

    deferred_program = ShaderProgram(lecture_shaders_path / "object.vert", lecture_shaders_path / "deferred.frag");
    display_texture_program = ShaderProgram(lecture_shaders_path / "full_screen_quad.vert", lecture_shaders_path / "display_texture.frag");
    evaluate_lighting_program = ShaderProgram(lecture_shaders_path / "full_screen_quad.vert", lecture_shaders_path / "evaluate_lighting_quad.frag");
    evaluate_ssao_program = ShaderProgram(lecture_shaders_path / "full_screen_quad.vert", lecture_shaders_path / "evaluate_ssao.frag");
    blur_ssao_texture_program = ShaderProgram(lecture_shaders_path / "full_screen_quad.vert", lecture_shaders_path / "blur_ssao_texture.frag");
    dof_program = ShaderProgram(lecture_shaders_path / "full_screen_quad.vert", lecture_shaders_path / "dof_fragment.frag");
    tone_mapping_program = ShaderProgram(lecture_shaders_path / "full_screen_quad.vert", lecture_shaders_path / "tone_mapping_fragment.frag");
    blurred_tone_mapping_program = ShaderProgram(lecture_shaders_path / "full_screen_quad.vert", lecture_shaders_path / "blur_highlights_fragment.frag");
    bloom_program = ShaderProgram(lecture_shaders_path / "full_screen_quad.vert", lecture_shaders_path / "bloom_fragment.frag");

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
    // We update the default materials so they look a bit better in the scene with high lighting.
    red_material_ubo.set_material(PhongMaterialData(glm::vec3(1.0f, 0.1f, 0.1f), true, 200.0f, 1.0f));
    green_material_ubo.set_material(PhongMaterialData(glm::vec3(0.1f, 1.0f, 0.1f), true, 200.0f, 1.0f));
    blue_material_ubo.set_material(PhongMaterialData(glm::vec3(0.1f, 0.1f, 1.0f), true, 200.0f, 1.0f));
    cyan_material_ubo.set_material(PhongMaterialData(glm::vec3(0.1f, 1.0f, 1.0f), true, 200.0f, 1.0f));
    magenta_material_ubo.set_material(PhongMaterialData(glm::vec3(1.0f, 0.1f, 1.0f), true, 200.0f, 1.0f));
    yellow_material_ubo.set_material(PhongMaterialData(glm::vec3(1.0f, 1.0f, 0.1f), true, 200.0f, 1.0f));
    red_material_ubo.update_opengl_data();
    green_material_ubo.update_opengl_data();
    blue_material_ubo.update_opengl_data();
    cyan_material_ubo.update_opengl_data();
    magenta_material_ubo.update_opengl_data();
    yellow_material_ubo.update_opengl_data();

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
    phong_lights_ubo.set_global_ambient(glm::vec3(0.0f));
    phong_lights_ubo.add(PhongLightData::CreatePointLight(glm::vec3(20.0f, 5.0f, 1.0f),                          // position
                                                          glm::vec3(0.0f), glm::vec3(200.0f), glm::vec3(200.0f), // intensities
                                                          0.0f, 0.0f, 1.0f));                                    // attenuation
    phong_lights_ubo.add(PhongLightData::CreatePointLight(glm::vec3(-20.0f, 5.0f, 1.0f),                         // position
                                                          glm::vec3(0.0f), glm::vec3(2.5f), glm::vec3(2.5f),     // intensities
                                                          0.0f, 0.0f, 1.0f));                                    // attenuation
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
    glNamedFramebufferDrawBuffers(gbuffer_fbo, 5, FBOUtils::draw_buffers_constants);

    // Creates the framebuffer for evaluation of the SSAO.
    glCreateFramebuffers(1, &ssao_evaluation_fbo);
    // Specifies a list of color buffers to be drawn into.
    glNamedFramebufferDrawBuffers(ssao_evaluation_fbo, 1, FBOUtils::draw_buffers_constants);

    // Creates the framebuffer for blurring of the SSAO.
    glCreateFramebuffers(1, &ssao_blurring_fbo);
    // Specifies a list of color buffers to be drawn into.
    glNamedFramebufferDrawBuffers(ssao_blurring_fbo, 1, FBOUtils::draw_buffers_constants);

    // Creates the framebuffer for computing the HDR scene.
    glCreateFramebuffers(1, &hdr_fbo);
    // Specifies a list of color buffers to be drawn into.
    glNamedFramebufferDrawBuffers(hdr_fbo, 2, FBOUtils::draw_buffers_constants);

    // Creates the framebuffer for computing the scene with tone mapping applied.
    glCreateFramebuffers(1, &tone_mapping_fbo);
    // Specifies a list of color buffers to be drawn into.
    glNamedFramebufferDrawBuffers(tone_mapping_fbo, 1, FBOUtils::draw_buffers_constants);

    // Creates the framebuffers for computing the HDR scene.
    glCreateFramebuffers(2, hrd_blurred_highlights_fbo);
    // Specifies a list of color buffers to be drawn into for each of the two buffers.
    glNamedFramebufferDrawBuffers(hrd_blurred_highlights_fbo[0], 1, FBOUtils::draw_buffers_constants);
    glNamedFramebufferDrawBuffers(hrd_blurred_highlights_fbo[1], 1, FBOUtils::draw_buffers_constants);

    // Creates the framebuffer for computing the scene with bloom effect applied.
    glCreateFramebuffers(1, &bloom_fbo);
    // Specifies a list of color buffers to be drawn into.
    glNamedFramebufferDrawBuffers(bloom_fbo, 1, FBOUtils::draw_buffers_constants);

    // Creates and binds the required textures.
    resize_fullscreen_textures();
}

void Application::resize_fullscreen_textures() {
    // Removes the previously allocated textures (if any).
    glDeleteTextures(1, &gbuffer_position_ws_texture);
    glDeleteTextures(1, &gbuffer_position_vs_texture);
    glDeleteTextures(1, &gbuffer_normal_ws_texture);
    glDeleteTextures(1, &gbuffer_normal_vs_texture);
    glDeleteTextures(1, &gbuffer_albedo_texture);
    glDeleteTextures(1, &gbuffer_depth_texture);
    glDeleteTextures(1, &ssao_occlusion_texture);
    glDeleteTextures(1, &ssao_blurred_occlusion_texture);
    glDeleteTextures(1, &hdr_lit_scene_texture);
    glDeleteTextures(1, &hdr_highlights_texture);
    glDeleteTextures(1, &tone_mapping_scene_texture);
    glDeleteTextures(2, hdr_blurred_highlights_texture);
    glDeleteTextures(1, &bloom_scene_texture);

    // Creates new textures for G-buffer and set their basic parameters.
    glCreateTextures(GL_TEXTURE_2D, 1, &gbuffer_position_ws_texture);
    glCreateTextures(GL_TEXTURE_2D, 1, &gbuffer_position_vs_texture);
    glCreateTextures(GL_TEXTURE_2D, 1, &gbuffer_normal_ws_texture);
    glCreateTextures(GL_TEXTURE_2D, 1, &gbuffer_normal_vs_texture);
    glCreateTextures(GL_TEXTURE_2D, 1, &gbuffer_albedo_texture);
    glCreateTextures(GL_TEXTURE_2D, 1, &gbuffer_depth_texture);
    glCreateTextures(GL_TEXTURE_2D, 1, &ssao_occlusion_texture);
    glCreateTextures(GL_TEXTURE_2D, 1, &ssao_blurred_occlusion_texture);
    glCreateTextures(GL_TEXTURE_2D, 1, &hdr_lit_scene_texture);
    glCreateTextures(GL_TEXTURE_2D, 1, &hdr_highlights_texture);
    glCreateTextures(GL_TEXTURE_2D, 1, &tone_mapping_scene_texture);
    glCreateTextures(GL_TEXTURE_2D, 2, hdr_blurred_highlights_texture);
    glCreateTextures(GL_TEXTURE_2D, 1, &bloom_scene_texture);

    // Initializes the immutable storage.
    glTextureStorage2D(gbuffer_position_ws_texture, 1, GL_RGBA32F, width, height);
    glTextureStorage2D(gbuffer_position_vs_texture, 1, GL_RGBA32F, width, height);
    glTextureStorage2D(gbuffer_normal_ws_texture, 1, GL_RGBA16F, width, height);
    glTextureStorage2D(gbuffer_normal_vs_texture, 1, GL_RGBA16F, width, height);
    glTextureStorage2D(gbuffer_albedo_texture, 1, GL_RGBA8, width, height);
    glTextureStorage2D(gbuffer_depth_texture, 1, GL_DEPTH_COMPONENT24, width, height);
    glTextureStorage2D(ssao_occlusion_texture, 1, GL_R16F, width, height);
    glTextureStorage2D(ssao_blurred_occlusion_texture, 1, GL_R16F, width, height);
    glTextureStorage2D(tone_mapping_scene_texture, 1, GL_RGBA8, width, height);
    glTextureStorage2D(bloom_scene_texture, 1, GL_RGBA8, width, height);

    // TASK 1: Increase precision of all HDR buffers.
    glTextureStorage2D(hdr_lit_scene_texture, 1, GL_RGBA16F, width, height);
    glTextureStorage2D(hdr_highlights_texture, 1, GL_RGBA16F, width, height);
    glTextureStorage2D(hdr_blurred_highlights_texture[0], 1, GL_RGBA16F, width, height);
    glTextureStorage2D(hdr_blurred_highlights_texture[1], 1, GL_RGBA16F, width, height);

    // Sets the texture parameters.
    TextureUtils::set_texture_2d_parameters(gbuffer_position_ws_texture, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
    TextureUtils::set_texture_2d_parameters(gbuffer_position_vs_texture, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
    TextureUtils::set_texture_2d_parameters(gbuffer_normal_ws_texture, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
    TextureUtils::set_texture_2d_parameters(gbuffer_normal_vs_texture, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
    TextureUtils::set_texture_2d_parameters(gbuffer_albedo_texture, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
    TextureUtils::set_texture_2d_parameters(gbuffer_depth_texture, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
    TextureUtils::set_texture_2d_parameters(ssao_occlusion_texture, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
    TextureUtils::set_texture_2d_parameters(ssao_blurred_occlusion_texture, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
    TextureUtils::set_texture_2d_parameters(hdr_lit_scene_texture, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);
    TextureUtils::set_texture_2d_parameters(hdr_highlights_texture, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);
    TextureUtils::set_texture_2d_parameters(tone_mapping_scene_texture, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);
    TextureUtils::set_texture_2d_parameters(hdr_blurred_highlights_texture[0], GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);
    TextureUtils::set_texture_2d_parameters(hdr_blurred_highlights_texture[1], GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);
    TextureUtils::set_texture_2d_parameters(bloom_scene_texture, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);

    // Binds the texture to the G-buffer.
    glNamedFramebufferTexture(gbuffer_fbo, GL_COLOR_ATTACHMENT0, gbuffer_position_ws_texture, 0);
    glNamedFramebufferTexture(gbuffer_fbo, GL_COLOR_ATTACHMENT1, gbuffer_position_vs_texture, 0);
    glNamedFramebufferTexture(gbuffer_fbo, GL_COLOR_ATTACHMENT2, gbuffer_normal_ws_texture, 0);
    glNamedFramebufferTexture(gbuffer_fbo, GL_COLOR_ATTACHMENT3, gbuffer_normal_vs_texture, 0);
    glNamedFramebufferTexture(gbuffer_fbo, GL_COLOR_ATTACHMENT4, gbuffer_albedo_texture, 0);
    glNamedFramebufferTexture(gbuffer_fbo, GL_DEPTH_ATTACHMENT, gbuffer_depth_texture, 0);
    FBOUtils::check_framebuffer_status(gbuffer_fbo, "G-buffer");

    // Binds the textures to the framebuffer for evaluating SSAO.
    glNamedFramebufferTexture(ssao_evaluation_fbo, GL_COLOR_ATTACHMENT0, ssao_occlusion_texture, 0);
    FBOUtils::check_framebuffer_status(ssao_evaluation_fbo, "Evaluate SSAO");

    // Binds the textures to the framebuffer for blurring SSAO.
    glNamedFramebufferTexture(ssao_blurring_fbo, GL_COLOR_ATTACHMENT0, ssao_blurred_occlusion_texture, 0);
    FBOUtils::check_framebuffer_status(ssao_blurring_fbo, "Blurring SSAO");

    //  Binds the textures to the framebuffer for computing the HDR scene.
    glNamedFramebufferTexture(hdr_fbo, GL_COLOR_ATTACHMENT0, hdr_lit_scene_texture, 0);
    glNamedFramebufferTexture(hdr_fbo, GL_COLOR_ATTACHMENT1, hdr_highlights_texture, 0);
    FBOUtils::check_framebuffer_status(hdr_fbo, "HRD");

    //  Binds the textures to the framebuffer for computing the scene with tone mapping.
    glNamedFramebufferTexture(tone_mapping_fbo, GL_COLOR_ATTACHMENT0, tone_mapping_scene_texture, 0);
    FBOUtils::check_framebuffer_status(tone_mapping_fbo, "Tone Mapping");

    // Binds the textures to the framebuffers for bluring the highlights.
    glNamedFramebufferTexture(hrd_blurred_highlights_fbo[0], GL_COLOR_ATTACHMENT0, hdr_blurred_highlights_texture[0], 0);
    FBOUtils::check_framebuffer_status(hrd_blurred_highlights_fbo[0], "Blurring HDR #1");
    glNamedFramebufferTexture(hrd_blurred_highlights_fbo[1], GL_COLOR_ATTACHMENT0, hdr_blurred_highlights_texture[1], 0);
    FBOUtils::check_framebuffer_status(hrd_blurred_highlights_fbo[1], "Blurring HDR #2");

    // Binds the textures to the framebuffer for computing the scene with bloom.
    glNamedFramebufferTexture(bloom_fbo, GL_COLOR_ATTACHMENT0, bloom_scene_texture, 0);
    FBOUtils::check_framebuffer_status(bloom_fbo, "Bloom");
}

void Application::compute_random_samples() {
    const int number_of_ssao_samples = 64;

    std::vector<glm::vec4> ssao_samples(number_of_ssao_samples);
    for (size_t i = 0; i < ssao_samples.size(); i++) {
        // Create a uniform point on a hemisphere (unit sphere, cut by xy plane, hemisphere in +z direction)
        const float alpha = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * static_cast<float>(M_PI);
        const float beta = asinf(static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
        glm::vec3 point_on_hemisphere = glm::vec3(cosf(alpha) * cosf(beta), sinf(alpha) * cosf(beta), sinf(beta));

        // Place the point on the hemisphere inside the hemisphere, so that the points in the hemisphere were uniformly
        // distributed in the hemisphere. This is true when the distance of the points from the center of the hemisphere
        // is the cube root of a uniformly distributed random number from interval [0,1].
        const float radius = powf(static_cast<float>(rand()) / static_cast<float>(RAND_MAX), 1.0f / 3.0f);
        glm::vec3 point_in_hemisphere = point_on_hemisphere * radius;

        ssao_samples[i] = glm::vec4(point_in_hemisphere, 0.0f);
    }

    // Creates a buffer for storing the random positions.
    glCreateBuffers(1, &ssao_samples_ubo);
    glNamedBufferStorage(ssao_samples_ubo, sizeof(float) * 4 * ssao_samples.size(), ssao_samples.data(), 0 /* no updates needed*/);
}

void Application::compute_random_tangent_directions() {
    std::vector<glm::vec3> SSAORandomTangent(16);
    for (int i = 0; i < 16; i++) {
        // Generates the tangents as random directions in xy plane.
        const float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * static_cast<float>(M_PI);
        SSAORandomTangent[i] = glm::vec3(sinf(angle), cosf(angle), 0.0f);
    }
    // Creates a 4x4 texture for these random directions.
    glCreateTextures(GL_TEXTURE_2D, 1, &ssao_random_tangent_vs_texture);
    glTextureStorage2D(ssao_random_tangent_vs_texture, 1, GL_RGBA32F, 4, 4);
    glTextureSubImage2D(ssao_random_tangent_vs_texture, 0, 0, 0, 4, 4, GL_RGB, GL_FLOAT, SSAORandomTangent.data());
    TextureUtils::set_texture_2d_parameters(ssao_random_tangent_vs_texture, GL_REPEAT, GL_REPEAT, GL_NEAREST, GL_NEAREST);
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

    // First, we need to render into the G-buffer.
    render_into_g_buffer();

    // Disables the depth test, other rendering passes do not use it.
    glDisable(GL_DEPTH_TEST);

    // SSAO
    if ((what_to_display == DISPLAY_OCCLUSION || what_to_display == DISPLAY_FINAL_IMAGE) && use_ssao) {
        // We either evaluate SSAO texture and blur it...
        evaluate_ssao();
    } else {
        // .. or we clear SSAO textures to ones, making the ambient occlusion not to appear.
        clear_ssao_buffers();
    }

    if (what_to_display == DISPLAY_POSITION_WS || what_to_display == DISPLAY_POSITION_VS || what_to_display == DISPLAY_NORMAL_WS || what_to_display == DISPLAY_NORMAL_VS ||
        what_to_display == DISPLAY_ALBEDO || what_to_display == DISPLAY_OCCLUSION) {
        // Displays the position texture, the normal texture, the albedo texture, or the ambient occlusion texture.
        show_g_buffer_textures();
    } else if (what_to_display == DISPLAY_HIGHLIGHTS || what_to_display == DISPLAY_BLURRED_HIGHLIGHTS || what_to_display == DISPLAY_FINAL_IMAGE) {
        // Evaluates the lighting using a full-screen quad while also applying SSAO.
        evaluate_lighting_using_quad();

        // Displays the highlights or bloom texture (without the tone mapping).
        // Note that the final image without tone mapping was already rendered in evaluate_lighting_using_quad (if requested).
        if (!use_tone_mapping && (what_to_display == DISPLAY_HIGHLIGHTS || what_to_display == DISPLAY_BLURRED_HIGHLIGHTS)) {
            render_without_tone_mapping();
        }

        // Displays the highlights, bloom texture or the final image (with the tone mapping).
        if (use_tone_mapping && (what_to_display == DISPLAY_FINAL_IMAGE || what_to_display == DISPLAY_HIGHLIGHTS || what_to_display == DISPLAY_BLURRED_HIGHLIGHTS)) {
            render_with_tone_mapping();
        }

        // Compute highlights and bloom if needed.
        if (use_bloom || what_to_display == DISPLAY_HIGHLIGHTS || what_to_display == DISPLAY_BLURRED_HIGHLIGHTS) {
            // Blurs the highlights buffer.
            if (what_to_display != DISPLAY_HIGHLIGHTS) {
                blur_highlights_buffer();
            }

            // Combines blurred highlights with the lit scene.
            if (use_bloom && what_to_display == DISPLAY_FINAL_IMAGE) {
                render_with_bloom();
            }
        }

        // Adds depth of field.
        if (use_dof) {
            evaluate_dof();
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

void Application::render_into_g_buffer() {
    // Binds G-Buffer.
    glBindFramebuffer(GL_FRAMEBUFFER, gbuffer_fbo);
    glViewport(0, 0, width, height);

    // Clears the G-buffer, clear all textures.
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    // Renders all objects in the scene.
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
    if (what_to_display == DISPLAY_POSITION_WS) {
        // Sets the transformation matrix - scale the position to 0.02.
        display_texture_program.uniform_matrix("transformation", scale(glm::mat4(1.0f), glm::vec3(0.02f)));
        // Binds the proper texture.
        glBindTextureUnit(0, gbuffer_position_ws_texture);
    } else if (what_to_display == DISPLAY_POSITION_VS) {
        // Sets the transformation matrix - scale the position to 0.02.
        display_texture_program.uniform_matrix("transformation", scale(glm::mat4(1.0f), glm::vec3(0.02f)));
        // Binds the proper texture.
        glBindTextureUnit(0, gbuffer_position_vs_texture);
    } else if (what_to_display == DISPLAY_NORMAL_WS) {
        // Sets the transformation matrix - no scale.
        display_texture_program.uniform_matrix("transformation", glm::mat4(1.0f));
        // Binds the proper texture.
        glBindTextureUnit(0, gbuffer_normal_ws_texture);
    } else if (what_to_display == DISPLAY_NORMAL_VS) {
        // Sets the transformation matrix - no scale.
        display_texture_program.uniform_matrix("transformation", glm::mat4(1.0f));
        // Binds the proper texture.
        glBindTextureUnit(0, gbuffer_normal_vs_texture);
    } else if (what_to_display == DISPLAY_ALBEDO) {
        // Sets the transformation matrix - no scale.
        display_texture_program.uniform_matrix("transformation", glm::mat4(1.0f));
        // Binds the proper texture.
        glBindTextureUnit(0, gbuffer_albedo_texture);
    } else if (what_to_display == DISPLAY_OCCLUSION) {
        // Sets the transformation matrix - duplicates red channel, the same as .rrr in swizzling.
        display_texture_program.uniform_matrix("transformation", glm::mat4(1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f));
        // Binds the proper texture.
        glBindTextureUnit(0, blur_ssao ? ssao_blurred_occlusion_texture : ssao_occlusion_texture);
    }

    // Renders the full screen quad to evaluate every pixel.
    // Binds an empty VAO as we do not need any state.
    glBindVertexArray(empty_vao);
    // Calls a draw command with 3 vertices that are generated in vertex shader.
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void Application::evaluate_lighting_using_quad() {
    if (what_to_display == DISPLAY_FINAL_IMAGE && !use_tone_mapping && !use_bloom && !use_dof) {
        // Without any post effects we render directly into the main window...
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    } else {
        // ...otherwise we bind the HDR_FBO to render lit scene to texture (with high dynamic range).
        glBindFramebuffer(GL_FRAMEBUFFER, hdr_fbo);
    }
    glViewport(0, 0, width, height);

    // Clears the framebuffer color.
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    // We do not need depth and depth test.
    glDisable(GL_DEPTH_TEST);

    // Uses the proper program.
    evaluate_lighting_program.use();

    // Binds all required textures.
    glBindTextureUnit(0, gbuffer_position_ws_texture);
    glBindTextureUnit(1, gbuffer_normal_ws_texture);
    glBindTextureUnit(2, gbuffer_albedo_texture);
    glBindTextureUnit(3, blur_ssao ? ssao_blurred_occlusion_texture : ssao_occlusion_texture);

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

void Application::evaluate_ssao() const {
    // Binds the proper framebuffer.
    glBindFramebuffer(GL_FRAMEBUFFER, ssao_evaluation_fbo);
    glViewport(0, 0, width, height);

    // Binds all required textures.
    glBindTextureUnit(0, gbuffer_position_vs_texture);
    glBindTextureUnit(1, gbuffer_normal_vs_texture);
    glBindTextureUnit(2, ssao_random_tangent_vs_texture);

    // Binds all UBOs that we need.
    camera_ubo.bind_buffer_base(CameraUBO::DEFAULT_CAMERA_BINDING);
    // Although the binding point 1 is usually used by the data of the lights, we do not need the lights here, so we may use this binding point.
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, ssao_samples_ubo);

    // Uses the proper program and set its uniform variables.
    evaluate_ssao_program.use();
    evaluate_ssao_program.uniform("ssao_radius", ssao_radius);

    // Renders the full screen quad to evaluate every pixel.
    // Binds an empty VAO as we do not need any state.
    glBindVertexArray(empty_vao);
    // Calls a draw command with 3 vertices that are generated in vertex shader.
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Blur SSAO texture if requested.
    if (blur_ssao) {
        apply_blur_ssao();
    }
}

void Application::apply_blur_ssao() const {
    // Binds the proper framebuffer.
    glBindFramebuffer(GL_FRAMEBUFFER, ssao_blurring_fbo);
    glViewport(0, 0, width, height);

    // Binds all required textures.
    glBindTextureUnit(0, ssao_occlusion_texture);

    // Uses the proper program.
    blur_ssao_texture_program.use();

    // Renders the full screen quad to evaluate every pixel.
    // Binds an empty VAO as we do not need any state.
    glBindVertexArray(empty_vao);
    // Calls a draw command with 3 vertices that are generated in vertex shader.
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void Application::clear_ssao_buffers() const {
    // Clears the evaluation framebuffer.
    glBindFramebuffer(GL_FRAMEBUFFER, ssao_evaluation_fbo);
    glViewport(0, 0, width, height);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Clears the blurring framebuffer.
    glBindFramebuffer(GL_FRAMEBUFFER, ssao_blurring_fbo);
    glViewport(0, 0, width, height);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Application::render_without_tone_mapping() {
    // Binds the main window framebuffer.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);

    // Uses the simple render texture program.
    display_texture_program.use();

    // Sets the transformation matrix - no scale.
    display_texture_program.uniform_matrix("transformation", glm::mat4(1.0f));

    // Binds the proper texture.
    if (what_to_display == DISPLAY_HIGHLIGHTS) {
        glBindTextureUnit(0, hdr_highlights_texture);
    }
    if (what_to_display == DISPLAY_BLURRED_HIGHLIGHTS) {
        glBindTextureUnit(0, hdr_blurred_highlights_texture[0]);
    }

    // Renders the full screen quad to evaluate every pixel.
    // Binds an empty VAO as we do not need any state.
    glBindVertexArray(empty_vao);
    // Calls a draw command with 3 vertices that are generated in vertex shader.
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void Application::render_with_tone_mapping() {
    if (!use_dof) {
        // Binds the main window framebuffer.
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    } else {
        // Render offscreen so we can use the result for DoF.
        glBindFramebuffer(GL_FRAMEBUFFER, tone_mapping_fbo);
    }
    glViewport(0, 0, width, height);

    // Uses the proper program and set its uniform variables.
    tone_mapping_program.use();
    tone_mapping_program.uniform("exposure", exposure);
    tone_mapping_program.uniform("gamma", gamma);

    // Binds the proper texture.
    if (what_to_display == DISPLAY_HIGHLIGHTS) {
        glBindTextureUnit(0, hdr_highlights_texture);
    } else if (what_to_display == DISPLAY_BLURRED_HIGHLIGHTS) {
        glBindTextureUnit(0, hdr_blurred_highlights_texture[0]);
    } else if (what_to_display == DISPLAY_FINAL_IMAGE) {
        glBindTextureUnit(0, hdr_lit_scene_texture);
    }

    // Renders the full screen quad to evaluate every pixel.
    // Binds an empty VAO as we do not need any state.
    glBindVertexArray(empty_vao);
    // Calls a draw command with 3 vertices that are generated in vertex shader.
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void Application::blur_highlights_buffer() {
    // TASK 4a: Simple, one pass blurring - the C++ part is already implemented here, see shader.

    // Binds the main window framebuffer.
    glBindFramebuffer(GL_FRAMEBUFFER, hrd_blurred_highlights_fbo[0]);
    glViewport(0, 0, width, height);

    // Binds all required textures.
    glBindTextureUnit(0, hdr_highlights_texture);

    // Uses the proper program and set its uniform variables.
    blurred_tone_mapping_program.use();

    // Renders the full screen quad to evaluate every pixel.
    // Binds an empty VAO as we do not need any state.
    glBindVertexArray(empty_vao);
    // Calls a draw command with 3 vertices that are generated in vertex shader.
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // TASK 4b: Blur multiple times using two buffers.
    //    Hint: Use HDR_Blurred_Highlights_Texture[0 and 1]
}

void Application::render_with_bloom() {
    if (!use_dof) {
        // Binds the main window framebuffer.
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    } else {
        // Render offscreen so we can use the result for DoF.
        glBindFramebuffer(GL_FRAMEBUFFER, bloom_fbo);
    }
    glViewport(0, 0, width, height);

    // Bind all textures that we need
    glBindTextureUnit(0, hdr_lit_scene_texture);
    glBindTextureUnit(1, hdr_blurred_highlights_texture[0]);

    // Use the proper program and set its uniform variables
    bloom_program.use();
    bloom_program.uniform("exposure", exposure);
    bloom_program.uniform("gamma", gamma);

    // Renders the full screen quad to evaluate every pixel.
    // Binds an empty VAO as we do not need any state.
    glBindVertexArray(empty_vao);
    // Calls a draw command with 3 vertices that are generated in vertex shader.
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void Application::evaluate_dof() const {
    // Binds the main window framebuffer.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);

    // Binds all required textures.
    glBindTextureUnit(0, gbuffer_position_vs_texture);
    if (use_bloom) {
        // Bloom uses tone mapping by default so it goes first
        glBindTextureUnit(1, bloom_scene_texture);
    } else if (use_tone_mapping) {
        // The scene with tone mapping active.
        glBindTextureUnit(1, tone_mapping_scene_texture);
    } else {
        // The scene without tone mapping.
        glBindTextureUnit(1, hdr_lit_scene_texture);
    }

    // Use the proper program and set its uniform variables
    dof_program.use();
    dof_program.uniform("focus_distance", focus_distance);

    // Renders the full screen quad to evaluate every pixel.
    // Binds an empty VAO as we do not need any state.
    glBindVertexArray(empty_vao);
    // Calls a draw command with 3 vertices that are generated in vertex shader.
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

// ----------------------------------------------------------------------------
// GUI
// ----------------------------------------------------------------------------
void Application::render_ui() {
    const float unit = ImGui::GetFontSize();

    ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoDecoration);
    ImGui::SetWindowSize(ImVec2(18 * unit, 20 * unit));
    ImGui::SetWindowPos(ImVec2(2 * unit, 2 * unit));

    std::string fps_cpu_string = "FPS (CPU): ";
    ImGui::Text(fps_cpu_string.append(std::to_string(fps_cpu)).c_str());

    std::string fps_string = "FPS (GPU): ";
    ImGui::Text(fps_string.append(std::to_string(fps_gpu)).c_str());

    ImGui::PushItemWidth(150.f);
    ImGui::Checkbox("Use SSAO", &use_ssao);
    ImGui::SliderFloat("SSAO Radius", &ssao_radius, 0.1f, 2.0f, "%.2f");
    ImGui::Checkbox("Blur SSAO", &blur_ssao);

    ImGui::Checkbox("Use DoF", &use_dof);
    ImGui::SliderInt("Focus Distance", &focus_distance, 5.f, 100.f);

    ImGui::Checkbox("Use Tone Mapping", &use_tone_mapping);
    ImGui::SliderFloat("Exposure", &exposure, 0.0f, 100.0f, "%.1f");
    ImGui::SliderFloat("Gamma", &gamma, 0.5f, 2.8f, "%.1f");
    ImGui::Checkbox("Use Bloom", &use_bloom);

    ImGui::Combo("Display", &what_to_display, DISPLAY_LABELS, IM_ARRAYSIZE(DISPLAY_LABELS));

    ImGui::End();
}

// ----------------------------------------------------------------------------
// Input Events
// ----------------------------------------------------------------------------
void Application::on_resize(int width, int height) {
    PV227Application::on_resize(width, height);
    resize_fullscreen_textures();
}

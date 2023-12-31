#include "application.hpp"
#include "utils.hpp"
#include <map>

Application::Application(int initial_width, int initial_height, std::vector<std::string> arguments)
        : PV227Application(initial_width, initial_height, arguments) {
    Application::compile_shaders();
    prepare_cameras();
    prepare_materials();
    prepare_textures();
    prepare_lights();
    prepare_scene();
    prepare_framebuffers();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
}

Application::~Application() {}

// ----------------------------------------------------------------------------
// Shaderes
// ----------------------------------------------------------------------------
void Application::compile_shaders() {
    default_unlit_program = ShaderProgram(lecture_shaders_path / "object.vert", lecture_shaders_path / "unlit.frag");
    default_lit_program = ShaderProgram(lecture_shaders_path / "object.vert", lecture_shaders_path / "lit.frag");

    composite_program = ShaderProgram(lecture_shaders_path / "full_screen_quad.vert",
                                      lecture_shaders_path / "composite_textures.frag");

    masking_program = ShaderProgram(lecture_shaders_path / "object.vert", lecture_shaders_path / "masking.frag");

    display_texture_program = ShaderProgram(lecture_shaders_path / "full_screen_quad.vert",
                                            lecture_shaders_path / "display_texture.frag");


    std::cout << "Shaders are reloaded." << std::endl;
}

// ----------------------------------------------------------------------------
// Initialize Scene
// ----------------------------------------------------------------------------
void Application::prepare_cameras() {
    // Sets the default camera position.
    camera.set_eye_position(glm::radians(-45.f), glm::radians(20.f), 400.f);

    // Sets the projection matrix for the normal and mirrored cameras.
    projection_matrix =
            glm::perspective(glm::radians(45.f), static_cast<float>(this->width) / static_cast<float>(this->height),
                             0.1f, 5000.0f);


    camera_ubo.set_projection(projection_matrix);
}

void Application::prepare_materials() {
    white_material_ubo.set_material(PhongMaterialData(glm::vec3(1.0f), true, 20.0f, 1.0f));
    white_material_ubo.update_opengl_data();
}

void Application::prepare_textures() {
    vampire_albedo_texture =
            TextureUtils::load_texture_2d(lecture_textures_path / "low" / "vampire_albedo.png");
    TextureUtils::set_texture_2d_parameters(vampire_albedo_texture, GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);

    swat_head_albedo_texture =
            TextureUtils::load_texture_2d(lecture_textures_path / "low" / "swat_head_albedo.png");
    TextureUtils::set_texture_2d_parameters(vampire_albedo_texture, GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);

    swat_body_albedo_texture =
            TextureUtils::load_texture_2d(lecture_textures_path / "low" / "swat_body_albedo.png");
    TextureUtils::set_texture_2d_parameters(vampire_albedo_texture, GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);

    walls_albedo_texture = TextureUtils::load_texture_2d(lecture_textures_path / "low" / "walls_albedo.png");
    TextureUtils::set_texture_2d_parameters(walls_albedo_texture, GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);

    door_albedo_texture = TextureUtils::load_texture_2d(lecture_textures_path / "low" / "door_albedo.png");
    TextureUtils::set_texture_2d_parameters(door_albedo_texture, GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);

    table_albedo_texture = TextureUtils::load_texture_2d(lecture_textures_path / "low" / "table_albedo.png");
    TextureUtils::set_texture_2d_parameters(table_albedo_texture, GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);

    chair_albedo_texture = TextureUtils::load_texture_2d(lecture_textures_path / "low" / "chair_albedo.png");
    TextureUtils::set_texture_2d_parameters(chair_albedo_texture, GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);

    window_albedo_texture = TextureUtils::load_texture_2d(lecture_textures_path / "low" / "window_albedo.png");
    TextureUtils::set_texture_2d_parameters(window_albedo_texture, GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);

    light_albedo_texture = TextureUtils::load_texture_2d(lecture_textures_path / "low" / "light_albedo.png");
    TextureUtils::set_texture_2d_parameters(window_albedo_texture, GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);
}

void Application::prepare_lights() {
    // The rest is set in the update scene method.
    phong_lights_ubo.set_global_ambient(glm::vec3(0.0f));
    const glm::vec3 light1 = glm::vec3(0, 54, 36.5);
    const glm::vec3 light2 = glm::vec3(0, 54, -36.5);

    point_light1_object = SceneObject(sphere, ModelUBO(glm::translate(glm::mat4(1.0f), light1)), white_material_ubo);
    point_light2_object = SceneObject(sphere, ModelUBO(glm::translate(glm::mat4(1.0f), light2)), white_material_ubo);

    phong_lights_ubo.clear();
    phong_lights_ubo.add(PhongLightData::CreatePointLight(light1, glm::vec3(0.1f), glm::vec3(0.3f), glm::vec3(0.2f)));
    phong_lights_ubo.add(PhongLightData::CreatePointLight(light2, glm::vec3(0.1f), glm::vec3(0.3f), glm::vec3(0.2f)));
    phong_lights_ubo.update_opengl_data();
}

void Application::prepare_framebuffers() {
    // Creates and binds the required textures.

    {
        glGenFramebuffers(1, &mask_framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, mask_framebuffer);

        std::cout << "Created FBO: " << mask_framebuffer << std::endl;

        glGenTextures(1, &mask_texture);
        glBindTexture(GL_TEXTURE_2D, mask_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mask_texture, 0);

        glGenTextures(1, &mask_depth);
        glBindTexture(GL_TEXTURE_2D, mask_depth);

        // Define the depth texture data
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                     width, height, 0, GL_DEPTH_COMPONENT,
                     GL_UNSIGNED_BYTE, NULL);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                               GL_TEXTURE_2D, mask_depth, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Error: Mask framebuffer is not complete!" << std::endl;
    }

    {
        glGenFramebuffers(1, &base_framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, base_framebuffer);

        std::cout << "Created FBO: " << base_framebuffer << std::endl;

        glGenTextures(1, &base_texture);
        glBindTexture(GL_TEXTURE_2D, base_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, base_texture, 0);

        glGenTextures(1, &base_depth);
        glBindTexture(GL_TEXTURE_2D, base_depth);

        // Define the depth texture data
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                     width, height, 0, GL_DEPTH_COMPONENT,
                     GL_UNSIGNED_BYTE, NULL);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                               GL_TEXTURE_2D, base_depth, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Error: Base framebuffer is not complete!" << std::endl;
    }
    {
        glGenFramebuffers(1, &mirror_framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, mirror_framebuffer);

        std::cout << "Created FBO: " << mirror_framebuffer << std::endl;

        glGenTextures(1, &mirror_texture);
        glBindTexture(GL_TEXTURE_2D, mirror_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mirror_texture, 0);

        glGenTextures(1, &mirror_depth);
        glBindTexture(GL_TEXTURE_2D, mirror_depth);

        // Define the depth texture data
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                     width, height, 0, GL_DEPTH_COMPONENT,
                     GL_UNSIGNED_BYTE, NULL);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                               GL_TEXTURE_2D, mirror_depth, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Error: Mirror framebuffer is not complete!" << std::endl;
    }
    {
        glGenFramebuffers(1, &glass_framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, glass_framebuffer);

        std::cout << "Created FBO: " << glass_framebuffer << std::endl;

        glGenTextures(1, &glass_texture);
        glBindTexture(GL_TEXTURE_2D, glass_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, glass_texture, 0);

        glGenTextures(1, &glass_depth);
        glBindTexture(GL_TEXTURE_2D, glass_depth);

        // Define the depth texture data
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                     width, height, 0, GL_DEPTH_COMPONENT,
                     GL_UNSIGNED_BYTE, NULL);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                               GL_TEXTURE_2D, glass_depth, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Error: Glass framebuffer is not complete!" << std::endl;
    }

    resize_fullscreen_textures();

}

void Application::resize_fullscreen_textures() {
    glViewport(0, 0, width, height);

    {
        glDeleteTextures(1, &mask_texture);
        glDeleteTextures(1, &mask_depth);

        glBindFramebuffer(GL_FRAMEBUFFER, mask_framebuffer);

        glGenTextures(1, &mask_texture);
        glBindTexture(GL_TEXTURE_2D, mask_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mask_texture, 0);

        glGenTextures(1, &mask_depth);
        glBindTexture(GL_TEXTURE_2D, mask_depth);

        // Define the depth texture data
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                     width, height, 0, GL_DEPTH_COMPONENT,
                     GL_UNSIGNED_BYTE, NULL);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                               GL_TEXTURE_2D, mask_depth, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Error: Mask framebuffer is not complete!" << std::endl;
    }

    {
        glDeleteTextures(1, &base_texture);
        glDeleteTextures(1, &base_depth);

        glBindFramebuffer(GL_FRAMEBUFFER, base_framebuffer);

        glGenTextures(1, &base_texture);
        glBindTexture(GL_TEXTURE_2D, base_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, base_texture, 0);

        glGenTextures(1, &base_depth);
        glBindTexture(GL_TEXTURE_2D, base_depth);

        // Define the depth texture data
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                     width, height, 0, GL_DEPTH_COMPONENT,
                     GL_UNSIGNED_BYTE, NULL);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                               GL_TEXTURE_2D, base_depth, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Error: Base framebuffer is not complete!" << std::endl;
    }
    {
        glDeleteTextures(1, &mirror_texture);
        glDeleteTextures(1, &mirror_depth);

        glBindFramebuffer(GL_FRAMEBUFFER, mirror_framebuffer);

        glGenTextures(1, &mirror_texture);
        glBindTexture(GL_TEXTURE_2D, mirror_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mirror_texture, 0);

        glGenTextures(1, &mirror_depth);
        glBindTexture(GL_TEXTURE_2D, mirror_depth);

        // Define the depth texture data
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                     width, height, 0, GL_DEPTH_COMPONENT,
                     GL_UNSIGNED_BYTE, NULL);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                               GL_TEXTURE_2D, mirror_depth, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Error: Mirror framebuffer is not complete!" << std::endl;
    }
    {
        glDeleteTextures(1, &glass_texture);
        glDeleteTextures(1, &glass_depth);

        glBindFramebuffer(GL_FRAMEBUFFER, glass_framebuffer);

        glGenTextures(1, &glass_texture);
        glBindTexture(GL_TEXTURE_2D, glass_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, glass_texture, 0);

        glGenTextures(1, &glass_depth);
        glBindTexture(GL_TEXTURE_2D, glass_depth);

        // Define the depth texture data
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                     width, height, 0, GL_DEPTH_COMPONENT,
                     GL_UNSIGNED_BYTE, NULL);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                               GL_TEXTURE_2D, glass_depth, 0);


        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Error: Glass framebuffer is not complete!" << std::endl;
    }

}

void Application::prepare_scene() {

    float floor_offset = -50;
    const ModelUBO floor_model_ubo(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, floor_offset - 0.5f, 0.0f)) *
                                   glm::scale(glm::mat4(1.0f), glm::vec3(200, 0.1f, 200)));
    floor_object = SceneObject(cube, floor_model_ubo, gray_material_ubo);

    const Geometry vampire = Geometry::from_file(lecture_folder_path / "models/vampire.obj", false);
    vampire_object = SceneObject(vampire, ModelUBO(glm::translate(glm::mat4(1.0f), glm::vec3(0, floor_offset, 0))),
                                 white_material_ubo,
                                 vampire_albedo_texture);

    const Geometry swat_head = Geometry::from_file(lecture_folder_path / "models/swat_head.obj", false);
    swat_head_object = SceneObject(swat_head, ModelUBO(glm::translate(glm::mat4(1.0f), glm::vec3(0, floor_offset, 0))),
                                   white_material_ubo,
                                   swat_head_albedo_texture);

    const Geometry swat_body = Geometry::from_file(lecture_folder_path / "models/swat_body.obj", false);
    swat_body_object = SceneObject(swat_body, ModelUBO(glm::translate(glm::mat4(1.0f), glm::vec3(0, floor_offset, 0))),
                                   white_material_ubo,
                                   swat_body_albedo_texture);

    const Geometry swat_helmet = Geometry::from_file(lecture_folder_path / "models/swat_helmet.obj", false);
    swat_helmet_object = SceneObject(swat_helmet,
                                     ModelUBO(glm::translate(glm::mat4(1.0f), glm::vec3(0, floor_offset, 0))),
                                     white_material_ubo, swat_body_albedo_texture);

    const Geometry walls = Geometry::from_file(lecture_folder_path / "models/walls.obj", false);
    walls_object = SceneObject(walls, ModelUBO(glm::translate(glm::mat4(1.0f), glm::vec3(0, floor_offset, 0))),
                               white_material_ubo,
                               walls_albedo_texture);

    const Geometry door = Geometry::from_file(lecture_folder_path / "models/door.obj", false);
    door_object = SceneObject(door, ModelUBO(glm::translate(glm::mat4(1.0f), glm::vec3(0, floor_offset, 0))),
                              white_material_ubo,
                              door_albedo_texture);

    const Geometry table = Geometry::from_file(lecture_folder_path / "models/table.obj", false);
    table_object = SceneObject(table, ModelUBO(glm::translate(glm::mat4(1.0f), glm::vec3(0, floor_offset, 0))),
                               white_material_ubo,
                               table_albedo_texture);

    const Geometry chair = Geometry::from_file(lecture_folder_path / "models/chair.obj", false);
    chair_object = SceneObject(chair, ModelUBO(glm::translate(glm::mat4(1.0f), glm::vec3(0, floor_offset, 0))),
                               white_material_ubo,
                               chair_albedo_texture);

    const Geometry mirror_frame = Geometry::from_file(lecture_folder_path / "models/window_frame.obj", false);
    window_frame_object = SceneObject(mirror_frame,
                                      ModelUBO(glm::translate(glm::mat4(1.0f), glm::vec3(0, floor_offset, 0))),
                                      white_material_ubo, window_albedo_texture);

    const Geometry light_1 = Geometry::from_file(lecture_folder_path / "models/light_1.obj", false);
    light_1_object = SceneObject(light_1, ModelUBO(glm::translate(glm::mat4(1.0f), glm::vec3(0, floor_offset, 0))),
                                 white_material_ubo,
                                 light_albedo_texture);

    const Geometry light_2 = Geometry::from_file(lecture_folder_path / "models/light_2.obj", false);
    light_2_object = SceneObject(light_2, ModelUBO(glm::translate(glm::mat4(1.0f), glm::vec3(0, floor_offset, 0))),
                                 white_material_ubo,
                                 light_albedo_texture);

    const Geometry glass = Geometry::from_file(lecture_folder_path / "models/glass.obj", false);
    glass_object = SceneObject(glass, ModelUBO(glm::translate(glm::mat4(1.0f), glm::vec3(0, floor_offset, 0))),
                               white_material_ubo,
                               window_albedo_texture);
}

// ----------------------------------------------------------------------------
// Update
// ----------------------------------------------------------------------------
void Application::update(float delta) {
    PV227Application::update(delta);

    // Updates the main camera.
    const glm::vec3 eye_position = camera.get_eye_position();
    const glm::mat4 view_matrix = glm::lookAt(eye_position, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    camera_ubo.set_view(view_matrix);
    camera_ubo.update_opengl_data();
}

// ----------------------------------------------------------------------------
// Render
// ----------------------------------------------------------------------------
void Application::render() {
    // Starts measuring the elapsed time.
    glBeginQuery(GL_TIME_ELAPSED, render_time_query);


    default_lit_program.uniform("is_mirror", what_to_display == MIRRORED_SCENE);

    switch (what_to_display) {
        case NORMAL_SCENE:
            glBindFramebuffer(GL_FRAMEBUFFER, base_framebuffer);
            render_scene(default_lit_program);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            display_texture(base_texture);
            break;
        case MASK_TEXTURE:
            glBindFramebuffer(GL_FRAMEBUFFER, mask_framebuffer);
            render_scene_mask();
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            display_texture(mask_texture);
            break;
        case MIRRORED_SCENE:
            glBindFramebuffer(GL_FRAMEBUFFER, mirror_framebuffer);
            render_scene(default_lit_program);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            display_texture(mirror_texture);
            break;
        case GLASS_TEXTURE:
            glBindFramebuffer(GL_FRAMEBUFFER, glass_framebuffer);
            glEnable(GL_DEPTH_TEST);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            render_object(glass_object, default_lit_program);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            display_texture(glass_texture);
            break;
        case FINAL_IMAGE:
            /// I didn't put this in a separate method, it's a bit too specific.

            /// Basic view of the scene
            glBindFramebuffer(GL_FRAMEBUFFER, base_framebuffer);
            render_scene(default_lit_program);

            /// Mirrored view of the scene
            what_to_display = MIRRORED_SCENE;
            default_lit_program.uniform("is_mirror", true);
            glBindFramebuffer(GL_FRAMEBUFFER, mirror_framebuffer);
            render_scene(default_lit_program);


            what_to_display = FINAL_IMAGE;
            /// The mirror mask
            glBindFramebuffer(GL_FRAMEBUFFER, mask_framebuffer);
            render_scene_mask();

            /// Render the glass object separatedly - for outside viewing
            glBindFramebuffer(GL_FRAMEBUFFER, glass_framebuffer);
            glEnable(GL_DEPTH_TEST);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            render_object(glass_object, default_lit_program);

            /// Finally, render the final result to the screen.
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            display_final();
            break;
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

/**
 * Basic - renders the scene with currently bound framebuffer. Differentiates between mirrored and "normal" mode.
 * @param program
 */
void Application::render_scene(const ShaderProgram &program) const {

    camera_ubo.bind_buffer_base(CameraUBO::DEFAULT_CAMERA_BINDING);
    phong_lights_ubo.bind_buffer_base(PhongLightsUBO::DEFAULT_LIGHTS_BINDING);

    // Binds the buffer and clears it.
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    render_object(floor_object, program);
    if (transparent_walls) {
        glEnable(GL_CULL_FACE);
        glCullFace(what_to_display == MIRRORED_SCENE ? GL_FRONT : GL_BACK);
        render_object(walls_object, program);
        glDisable(GL_CULL_FACE);
        glCullFace(what_to_display == MIRRORED_SCENE ? GL_BACK : GL_FRONT);
    } else {
        render_object(walls_object, program);
    }
    render_object(table_object, program);
    render_object(chair_object, program);
    render_object(window_frame_object, program);
    render_object(light_1_object, program);
    render_object(light_2_object, program);
    render_object(door_object, program);
    if (what_to_display != MIRRORED_SCENE) {
        render_object(swat_body_object, program);
        render_object(swat_head_object, program);
        render_object(swat_helmet_object, program);
        render_object(vampire_object, program);
    }

    // render_object(glass_object, program);
}

/**
 * Specialized mode to render the mirror mask - the program is hardcoded.
 *
 * Writes red color to the "inside" part of the mirror and green to the "outside" part via face culling.
 */
void Application::render_scene_mask() const {
    camera_ubo.bind_buffer_base(CameraUBO::DEFAULT_CAMERA_BINDING);

    // Binds the buffer and clears it.
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto program = masking_program;


    program.uniform("write_color", glm::vec3(0.0f));

    render_object(floor_object, program);  // get outta here
    if (transparent_walls) {
        glEnable(GL_CULL_FACE);
        glCullFace(what_to_display == MIRRORED_SCENE ? GL_FRONT : GL_BACK);
        render_object(walls_object, program);
        glDisable(GL_CULL_FACE);
        glCullFace(what_to_display == MIRRORED_SCENE ? GL_BACK : GL_FRONT);
    } else {
        render_object(walls_object, program);
    }
    render_object(table_object, program);
    render_object(chair_object, program);
    render_object(window_frame_object, program);
    render_object(light_1_object, program);
    render_object(light_2_object, program);
    render_object(door_object, program);

    render_object(vampire_object, program);

    render_object(swat_body_object, program);
    render_object(swat_head_object, program);
    render_object(swat_helmet_object, program);
    render_object(vampire_object, program);


    glEnable(GL_CULL_FACE);
    glCullFace(what_to_display == MIRRORED_SCENE ? GL_FRONT : GL_BACK);
    program.uniform("write_color", glm::vec3(1.0f, 0.0f, 0.0f));
    render_object(glass_object, program);
    glCullFace(what_to_display == MIRRORED_SCENE ? GL_BACK : GL_FRONT);
    program.uniform("write_color", glm::vec3(0.0f, 1.0f, 0.0f));
    render_object(glass_object, program);
    glDisable(GL_CULL_FACE);

}

void Application::render_object(const SceneObject &object, const ShaderProgram &program) const {
    program.use();

    // Handles the textures.
    program.uniform("has_texture", object.has_texture());

    if (object.has_texture()) {
        glBindTextureUnit(0, object.get_texture());
    } else {
        glBindTextureUnit(0, 0);
    }

    object.get_model_ubo().bind_buffer_base(ModelUBO::DEFAULT_MODEL_BINDING);
    object.get_material().bind_buffer_base(PhongMaterialUBO::DEFAULT_MATERIAL_BINDING);
    object.get_geometry().bind_vao();
    object.get_geometry().draw();
}

void Application::display_texture(GLuint texture) {
    // Binds the main framebuffer and clears it.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    // Disables depth test.
    glDisable(GL_DEPTH_TEST);

    // Uses the proper program.
    display_texture_program.use();
    // Binds the texture.
    glBindTextureUnit(0, texture);

    // Renders the full screen quad to evaluate every pixel.
    // Binds an empty VAO as we do not need any state.
    glBindVertexArray(empty_vao);
    // Calls a draw command with 3 vertices that are generated in vertex shader.
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

/**
 * Rendering of the final image - take all the separate textures and stitch them together in a fragment shader.
 * The program is once again hardcoded here and so is the framebuffer. This is not supposed to be rendered anywhere
 * but the screen.
 */
void Application::display_final() {
    // Binds the main framebuffer and clears it.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    // Disables depth test.
    glDisable(GL_DEPTH_TEST);

    // Uses the proper program.
    composite_program.use();
    // Binds the texture.
    glBindTextureUnit(0, base_texture);
    glBindTextureUnit(1, mirror_texture);
    glBindTextureUnit(2, mask_texture);
    glBindTextureUnit(3, glass_texture);


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
    ImGui::SetWindowSize(ImVec2(20 * unit, 17 * unit));
    ImGui::SetWindowPos(ImVec2(2 * unit, 2 * unit));

    ImGui::PushItemWidth(150.f);

    std::string fps_cpu_string = "FPS (CPU): ";
    ImGui::Text(fps_cpu_string.append(std::to_string(fps_cpu)).c_str());

    std::string fps_string = "FPS (GPU): ";
    ImGui::Text(fps_string.append(std::to_string(fps_gpu)).c_str());

    const char *display_labels[5] = {
            DisplayModeToText(0),
            DisplayModeToText(1),
            DisplayModeToText(2),
            DisplayModeToText(3),
            DisplayModeToText(4),
    };

    /** Improving the controls. Press W to toggle of Wall visibility. Move viewing modes with tab / shift tab. */
    {
        if (ImGui::IsKeyPressed(ImGuiKey_Tab)) {
            if (ImGui::GetIO().KeyShift) {
                what_to_display = static_cast<EDisplayMode>((static_cast<int>(what_to_display) + 4) % 5);
            } else {
                what_to_display = static_cast<EDisplayMode>((static_cast<int>(what_to_display) + 1) % 5);
            }
        }

        if (ImGui::IsKeyPressed(ImGuiKey_W)) {
            transparent_walls = !transparent_walls;
        }
    }
    ImGui::Combo("Display", reinterpret_cast<int *>(&what_to_display), display_labels, IM_ARRAYSIZE(display_labels));

    ImGui::Checkbox("Transparent Walls", &transparent_walls);

    ImGui::Checkbox("Extra Reflections (corrective only)", &extra_reflection);

    ImGui::End();
}

// ----------------------------------------------------------------------------
// Input Events
// ----------------------------------------------------------------------------
void Application::on_resize(int width, int height) {
    PV227Application::on_resize(width, height);
    resize_fullscreen_textures();
}

void Application::on_mouse_move(double x, double y) { PV227Application::on_mouse_move(x, y); }
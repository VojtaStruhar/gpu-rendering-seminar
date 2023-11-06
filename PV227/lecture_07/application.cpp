#include "application.hpp"
#include "utils.hpp"
#include <map>

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
    // Initializes the shader programs.
    //
    // The rendering is getting more and more sophisticated, so we need more and more shaders.
    // We use the following shaders:
    //		default_lit_program		- objects with and without textures
    //		skybox_*_program	    - skybox in the background
    //
    //		display_cubemap_program		- for the central object, displays the cube map without reflections
    //		reflection_program			- for the central object, renders the object with texture and with reflections of the environment
    //
    //		*_{nothing}_program					- shaders for simple rendering as we did in the last lectures
    //		*_to_cube_instancing_no_gs_program	- shaders that render all six faces of the cube map simultaneously using instancing
    //
    // In addition to that, we use ShaderProgramMap to manage the shaders.
    //		RENDER_STANDARD					- use *_{nothing}_program
    //		RENDER_TO_CUBE_INSTANCING_NO_GS	- use *_to_cube_instancing_program

    default_lit_program = ShaderProgram(lecture_shaders_path / "object.vert", lecture_shaders_path / "lit.frag");
    skybox_program = ShaderProgram(lecture_shaders_path / "skybox.vert", lecture_shaders_path / "skybox.frag");
    display_cubemap_program = ShaderProgram(lecture_shaders_path / "display_cubemap.vert", lecture_shaders_path / "skybox.frag");
    reflection_program = ShaderProgram(lecture_shaders_path / "object.vert", lecture_shaders_path / "reflection.frag");

    // We need this extension.
    if (Utils::is_open_gl_extension_present("GL_ARB_shader_viewport_layer_array")) {
        object_to_cube_instancing_program = ShaderProgram(lecture_shaders_path / "object_to_cube_instancing.vert", lecture_shaders_path / "lit.frag");
        skybox_to_cube_instancing_program = ShaderProgram(lecture_shaders_path / "skybox_to_cube_instancing.vert", lecture_shaders_path / "skybox.frag");

    } else {
        // The extension is not present, we cannot load these shaders.
        object_to_cube_instancing_program.delete_program();
        skybox_to_cube_instancing_program.delete_program();
    }

    // Setups the shader maps.
    object_program_map.remove_all();
    object_program_map.add(RENDER_STANDARD, &default_lit_program);
    object_program_map.add(RENDER_TO_CUBE_INSTANCING, &object_to_cube_instancing_program);

    skybox_program_map.remove_all();
    skybox_program_map.add(RENDER_STANDARD, &skybox_program);
    skybox_program_map.add(RENDER_TO_CUBE_INSTANCING, &skybox_to_cube_instancing_program);

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

    // Creates six separate cameras to use when rendering six faces of the cube map.
    // All six cameras have the same projection.
    const glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 1000.0f);
    for (int face = 0; face < 6; face++) {
        cubemap_separate_camera_data_ubo[face].set_projection(projection);
    }
    // Each camera looks in specific direction (POSITIVE/NEGATIVE_X/Y/Z) and is properly oriented.
    cubemap_separate_camera_data_ubo[0].set_view(lookAt(glm::vec3(0.0f), glm::vec3(+1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    cubemap_separate_camera_data_ubo[1].set_view(lookAt(glm::vec3(0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    cubemap_separate_camera_data_ubo[2].set_view(lookAt(glm::vec3(0.0f), glm::vec3(0.0f, +1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
    cubemap_separate_camera_data_ubo[3].set_view(lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
    cubemap_separate_camera_data_ubo[4].set_view(lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, +1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    cubemap_separate_camera_data_ubo[5].set_view(lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    // Updates OpenGL buffers.
    for (int face = 0; face < 6; face++) {
        cubemap_separate_camera_data_ubo[face].update_opengl_data();
    }

    // Creates a group of six cameras to use when rendering six faces of the cube map.
    const std::vector<CameraData> cameras = {CameraData(projection, lookAt(glm::vec3(0.0f), glm::vec3(+1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)), glm::vec3(0.0f)),
                                             CameraData(projection, lookAt(glm::vec3(0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)), glm::vec3(0.0f)),
                                             CameraData(projection, lookAt(glm::vec3(0.0f), glm::vec3(0.0f, +1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)), glm::vec3(0.0f)),
                                             CameraData(projection, lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)), glm::vec3(0.0f)),
                                             CameraData(projection, lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, +1.0f), glm::vec3(0.0f, -1.0f, 0.0f)), glm::vec3(0.0f)),
                                             CameraData(projection, lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)), glm::vec3(0.0f))};

    cubemap_all_camera_data_ubo = CameraUBO(cameras);
    cubemap_all_camera_data_ubo.update_opengl_data();
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
    wood_tex = TextureUtils::load_texture_2d(framework_textures_path / "wood.png");
    TextureUtils::set_texture_2d_parameters(wood_tex, GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);

    lenna_tex = TextureUtils::load_texture_2d(framework_textures_path / "lenna.png");
    TextureUtils::set_texture_2d_parameters(lenna_tex, GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);

    meadow_skybox_tex = TextureUtils::load_texture_cube(1024, 1024, lecture_textures_path / "meadow_skybox/meadow_skybox_px.jpg", lecture_textures_path / "meadow_skybox/meadow_skybox_nx.jpg",
                                                        lecture_textures_path / "meadow_skybox/meadow_skybox_py.jpg", lecture_textures_path / "meadow_skybox/meadow_skybox_ny.jpg",
                                                        lecture_textures_path / "meadow_skybox/meadow_skybox_pz.jpg", lecture_textures_path / "meadow_skybox/meadow_skybox_nz.jpg");
    TextureUtils::set_texture_3d_parameters(meadow_skybox_tex, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);

    // Loads the miramar skybox.
    miramar_skybox_tex = TextureUtils::load_texture_cube(1024, 1024, lecture_textures_path / "miramar_skybox/miramar_skybox_px.jpg", lecture_textures_path / "miramar_skybox/miramar_skybox_nx.jpg",
                                                         lecture_textures_path / "miramar_skybox/miramar_skybox_py.jpg", lecture_textures_path / "miramar_skybox/miramar_skybox_ny.jpg",
                                                         lecture_textures_path / "miramar_skybox/miramar_skybox_pz.jpg", lecture_textures_path / "miramar_skybox/miramar_skybox_nz.jpg");
    TextureUtils::set_texture_3d_parameters(miramar_skybox_tex, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);

    // Chooses the initial skybox texture
    current_skybox_tex = miramar_skybox_tex;

    textures.push_back(wood_tex);
    textures.push_back(lenna_tex);
}

void Application::prepare_lights() {
    phong_lights_ubo.set_global_ambient(glm::vec3(0.1f));
    phong_lights_ubo.add(PhongLightData::CreateDirectionalLight(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(1.0f)));
    phong_lights_ubo.update_opengl_data();
}

void Application::prepare_scene() {
    // Creates a random scene. We create a grid of random objects with random materials.
    srand(12345);

    // Prepares a list of geometries to pick from.
    // We also define matrices that describe how to transform the objects so that they "sit" on XZ plane.
    std::vector<std::pair<Geometry*, glm::mat4>> Geometries;
    Geometries.push_back(std::make_pair(&cube, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, 0.0f))));
    Geometries.push_back(std::make_pair(&sphere, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, 0.0f))));
    Geometries.push_back(std::make_pair(&torus, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.5f, 0.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f))));
    Geometries.push_back(std::make_pair(&cylinder, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, 0.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f))));
    Geometries.push_back(std::make_pair(&capsule, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, 0.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f))));
    Geometries.push_back(std::make_pair(&teapot, glm::mat4(1.0f)));

    for (int x = 0; x < grid_size.x; x++)
        for (int z = 0; z < grid_size.z; z++) {

            glm::ivec3 xyz_grid_empty_size = glm::ivec3(3, 1, 3);
            glm::ivec3 xyz_grid_empty_start = (grid_size - xyz_grid_empty_size) / 2;

            glm::ivec3 xyz = glm::ivec3(x, 0, z);
            if (all(greaterThanEqual(xyz, xyz_grid_empty_start)) && all(lessThan(xyz, xyz_grid_empty_start + xyz_grid_empty_size)))
                continue;

            // Chooses randomly the geometry.
            const int geometry = rand() % static_cast<int>(Geometries.size());
            // Rotates the object randomly around the y axis.
            glm::mat4 rotation = rotate(glm::mat4(1.0f), static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * static_cast<float>(M_PI) * 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));
            // Places the object at the proper place in the grid
            glm::mat4 translation = translate(glm::mat4(1.0f), glm::vec3(grid_start.x + static_cast<float>(x) * grid_spacing.x, grid_start.y, grid_start.z + static_cast<float>(z) * grid_spacing.z));
            // Chooses the material randomly, choose randomly between the colors and the textures.
            const int material = rand() % static_cast<int>(color_ubos.size() + textures.size());
            // Computes the model matrix.
            ModelUBO model_ubo(translation * rotation * Geometries[geometry].second);
            // Creates the scene object and add it into the list.
            SceneObject scene_object;
            if (material < static_cast<int>(color_ubos.size())) {
                // Objects without textures.
                scene_object = SceneObject(*Geometries[geometry].first, model_ubo, color_ubos[material]);
            } else {
                // Objects with texture.
                scene_object = SceneObject(*Geometries[geometry].first, model_ubo, white_material_ubo, textures[material - static_cast<int>(color_ubos.size())]);
            }
            scene_objects.push_back(scene_object);
        }

    // Prepares the floor model.
    ModelUBO floor_model_ubo(translate(glm::mat4(1.0f), glm::vec3(0.0f, grid_start.y - 0.1f, 0.0f)) * scale(glm::mat4(1.0f), glm::vec3(40.0f, 0.1f, 40.0f)));
    const SceneObject floor_object = SceneObject(cube, floor_model_ubo, white_material_ubo, wood_tex);
    scene_objects.push_back(floor_object);

    // Prepares the corner models.
    corner_object1 = SceneObject(teapot, ModelUBO(), green_material_ubo);
    scene_objects.push_back(corner_object1);

    corner_object2 = SceneObject(capsule, ModelUBO(), blue_material_ubo);
    scene_objects.push_back(corner_object2);

    corner_object3 = SceneObject(torus, ModelUBO(), white_material_ubo, wood_tex);
    scene_objects.push_back(corner_object3);

    corner_object4 = SceneObject(cube, ModelUBO(), white_material_ubo, lenna_tex);
    scene_objects.push_back(corner_object4);

    // Prepares the orbiting object.
    orbiting_object = SceneObject(cube, ModelUBO(), white_material_ubo, wood_tex);
    scene_objects.push_back(orbiting_object);

    // Sets the model matrix for the central object.
    central_object_ubo.set_matrix(scale(glm::mat4(1.0f), glm::vec3(3.0f)));
    central_object_ubo.update_opengl_data();
}

void Application::prepare_framebuffers() {
    // Prepares a cubemap texture into which we render the environment.
    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &cubemap_color_texture);
    glTextureStorage2D(cubemap_color_texture, 1, GL_RGBA8, cubemap_size, cubemap_size);
    TextureUtils::set_texture_3d_parameters(cubemap_color_texture, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);

    // Prepares also a depth cubemap texture which we need for the depth test.
    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &cubemap_depth_texture);
    glTextureStorage2D(cubemap_depth_texture, 1, GL_DEPTH_COMPONENT24, cubemap_size, cubemap_size);
    TextureUtils::set_texture_3d_parameters(cubemap_depth_texture, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);

    // Creates six frame buffers, each with one cubemap face.
    glCreateFramebuffers(6, cubemap_separate_fbo);
    for (int face = 0; face < 6; face++) {
        // We attach a single face.
        glNamedFramebufferTextureLayer(cubemap_separate_fbo[face], GL_COLOR_ATTACHMENT0, cubemap_color_texture, 0, face);
        glNamedFramebufferTextureLayer(cubemap_separate_fbo[face], GL_DEPTH_ATTACHMENT, cubemap_depth_texture, 0, face);
        glNamedFramebufferDrawBuffers(cubemap_separate_fbo[face], 1, FBOUtils::draw_buffers_constants);
        FBOUtils::check_framebuffer_status(cubemap_separate_fbo[face], "CubemapSeparateFBO " + face);
    }

    // Creates a single frame buffer with the whole cubemap.
    glCreateFramebuffers(1, &cubemap_fbo);
    // We use glNamedFramebufferTexture to attach the whole cubemap texture.
    glNamedFramebufferTexture(cubemap_fbo, GL_COLOR_ATTACHMENT0, cubemap_color_texture, 0);
    glNamedFramebufferTexture(cubemap_fbo, GL_DEPTH_ATTACHMENT, cubemap_depth_texture, 0);
    glNamedFramebufferDrawBuffers(cubemap_fbo, 1, FBOUtils::draw_buffers_constants);
    FBOUtils::check_framebuffer_status(cubemap_fbo, "CubemapFBO");
}

void Application::resize_fullscreen_textures() {}

// ----------------------------------------------------------------------------
// Update
// ----------------------------------------------------------------------------
void Application::update(float delta) {
    // Updates the main camera.
    PV227Application::update(delta);

    const glm::vec3 eye_position = camera.get_eye_position();
    camera_ubo.set_view(lookAt(eye_position, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
    camera_ubo.update_opengl_data();

    // Animates all objects.
    udpate_objects();
}

void Application::udpate_objects() {
    // Application time in seconds.
    const float app_time_s = elapsed_time * 0.001;
    glm::mat4 matrix;

    // 0. object is a teapot
    matrix =
        // Sets object position and orientation in the world (it rotates around its y-axis).
        translate(glm::mat4(1.0f), glm::vec3(-30.0f, -5.0f, -30.0f)) * rotate(glm::mat4(1.0f), app_time_s * 1.5f, glm::vec3(0.0f, 1.0f, 0.0f)) *
        // Sets object size.
        scale(glm::mat4(1.0f), glm::vec3(5.0f)) *
        // Sets the base position and orientation (so that it sits on the floor).
        glm::mat4(1.0f);
    corner_object1.get_model_ubo().set_matrix(matrix);
    corner_object1.get_model_ubo().update_opengl_data();

    // 1. object is a capsule
    matrix =
        // Sets object position and orientation in the world (it rotates around its y-axis).
        translate(glm::mat4(1.0f), glm::vec3(30.0f, -5.0f, -30.0f)) * rotate(glm::mat4(1.0f), app_time_s * 1.1f, glm::vec3(0.0f, 1.0f, 0.0f)) *
        // Sets object size.
        scale(glm::mat4(1.0f), glm::vec3(5.0f)) *
        // Sets the base position and orientation (so that it sits on the floor).
        translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, 0.0f)) * rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    corner_object2.get_model_ubo().set_matrix(matrix);
    corner_object2.get_model_ubo().update_opengl_data();

    // 2. object is a torus
    matrix =
        // Sets object position and orientation in the world (it rotates around its y-axis).
        translate(glm::mat4(1.0f), glm::vec3(-30.0f, -5.0f, 30.0f)) * rotate(glm::mat4(1.0f), app_time_s * 2.1f, glm::vec3(0.0f, 1.0f, 0.0f)) *
        // Sets object size.
        scale(glm::mat4(1.0f), glm::vec3(5.0f)) *
        // Sets the base position and orientation (so that it sits on the floor).
        translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.5f, 0.0f)) * rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    corner_object3.get_model_ubo().set_matrix(matrix);
    corner_object3.get_model_ubo().update_opengl_data();

    // 3. object is a cube
    matrix =
        // Sets object position and orientation in the world (it rotates around its y-axis).
        translate(glm::mat4(1.0f), glm::vec3(30.0f, -5.0f, 30.0f)) * rotate(glm::mat4(1.0f), app_time_s * 1.9f, glm::vec3(0.0f, 1.0f, 0.0f)) *
        // Sets object size.
        scale(glm::mat4(1.0f), glm::vec3(5.0f)) *
        // Sets the base position and orientation (so that it sits on the floor).
        translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    corner_object4.get_model_ubo().set_matrix(matrix);
    corner_object4.get_model_ubo().update_opengl_data();

    // Orbiting object
    matrix =
        // Sets object position and orientation in the world (it rotates around central y-axis and also chaotically around its own center)
        translate(glm::mat4(1.0f), glm::vec3(0.0f, 10.0f, 0.0f)) * rotate(glm::mat4(1.0f), app_time_s * 2.0f, glm::vec3(0.0f, 1.0f, 0.0f)) * translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 20.0f)) *
        rotate(glm::mat4(1.0f), app_time_s * 1.2f, glm::vec3(1.0f, 0.0f, 0.0f)) * rotate(glm::mat4(1.0f), app_time_s * 1.4f, glm::vec3(0.0f, 1.0f, 0.0f)) *
        rotate(glm::mat4(1.0f), app_time_s * 1.6f, glm::vec3(0.0f, 0.0f, 1.0f)) *
        // Sets object size.
        scale(glm::mat4(1.0f), glm::vec3(3.0f));
    orbiting_object.get_model_ubo().set_matrix(matrix);
    orbiting_object.get_model_ubo().update_opengl_data();
}

// ----------------------------------------------------------------------------
// Render
// ----------------------------------------------------------------------------
void Application::render() {
    // Starts measuring the elapsed time.
    glBeginQuery(GL_TIME_ELAPSED, render_time_query);

    // Renders the scene six times - once for each face.
    if (cubemap_update_method == CUBE_MAP_UPDATE_SEPARATE_FACES) {
        cube_map_update_separate_faces();
    }
    // Renders the scene only once - into all faces simultaneously.
    else if (cubemap_update_method == CUBE_MAP_UPDATE_RENDER_TO_CUBE_MAP_INSTANCING) {
        cube_map_update_all_faces_at_once();
    }

    // Renders the final scene.
    render_final_scene();

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

void Application::render_scene(ShaderProgram& program, bool use_instancing) {
    program.use();

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
        if (use_instancing) {
            object.get_geometry().draw_instanced(6);
        } else {
            object.get_geometry().draw();
        }
    }
}

void Application::render_skybox_and_scene(int render_program, bool use_instancing) {
    // Renders the skybox.
    if (skybox_program_map.use(render_program)) {
        glBindTextureUnit(0, current_skybox_tex);

        // Renders the skybox only into the color buffer, does not use depth test, does not update depth buffer.
        glDisable(GL_DEPTH_TEST);

        // We render the skybox using a fullscreen quad, and we compute the view direction in vertex shader.
        // Binds an empty VAO as we do not need any state.
        glBindVertexArray(empty_vao);
        if (use_instancing) {
            // Calls a draw command that creates 6 instances of geometry with 3 vertices each that are generated in vertex shader.
            glDrawArraysInstanced(GL_TRIANGLES, 0, 3, 6);
        } else {
            // Calls a draw command with 3 vertices that are generated in vertex shader.
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }

        // Restore the depth test and the depth buffer writes
        glEnable(GL_DEPTH_TEST);
    }

    render_scene(*object_program_map.get_program(render_program), use_instancing);
}

void Application::cube_map_update_separate_faces() {
    // TASK 3: Update the faces of the cubemap separately.
    //  Hints: The individual sides of the cubemap are mapped as color attachments to frame buffers stored in cubemap_separate_fbo[] array.
    //         The size of the cubemap (i.e., width and height of each side) is stored in 'cubemap_size'.
    //         Corresponding cameras are stored in cubemap_separate_camera_data_ubo[] array.
    //         Lights are stored in phong_lights_ubo.
    //         To bind a UBO call 'ubo_object.bind_buffer_base(binding_point)' - the binding point can be found in the shader.
    //         Use 'render_skybox_and_scene(RENDER_STANDARD, false)' to render the scene.
    //         You may also inspire yourself in the function below where we update all the sides at the same time.

    // solve the task 3 here:
    for (int face = 0; face < 6; face++) {
        glBindFramebuffer(GL_FRAMEBUFFER, cubemap_separate_fbo[face]);
        glViewport(0, 0, cubemap_size, cubemap_size);

        // Clears the frame buffer.
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClearDepth(1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        // Sets the data of the camera and the lights.
        cubemap_separate_camera_data_ubo[face].bind_buffer_base(CameraUBO::DEFAULT_CAMERA_BINDING);
        phong_lights_ubo.bind_buffer_base(PhongLightsUBO::DEFAULT_LIGHTS_BINDING);

        // Renders the scene.
        render_skybox_and_scene(RENDER_STANDARD, false);
    }
}

void Application::cube_map_update_all_faces_at_once() {
    // TASK 4: No need to implement anything here, just notice that we omit the for loop and instead we render 
    //         6 copies of the scene with glDrawArraysInstanced. We also map all the cameras at the same time.
    glBindFramebuffer(GL_FRAMEBUFFER, cubemap_fbo);
    glViewport(0, 0, cubemap_size, cubemap_size);

    // Clears the frame buffer.
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    // Sets the data of the camera and the lights.
    cubemap_all_camera_data_ubo.bind_buffer_base(CameraUBO::DEFAULT_CAMERA_BINDING);
    phong_lights_ubo.bind_buffer_base(PhongLightsUBO::DEFAULT_LIGHTS_BINDING);

    // Renders the scene.
    render_skybox_and_scene(RENDER_TO_CUBE_INSTANCING, true);
}

void Application::render_final_scene() {
    // Renders into the main window.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Clears the frame buffer.
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, width, height);

    // Sets the data of the camera and the lights.
    camera_ubo.bind_buffer_base(CameraUBO::DEFAULT_CAMERA_BINDING);
    phong_lights_ubo.bind_buffer_base(PhongLightsUBO::DEFAULT_LIGHTS_BINDING);

    // Renders the scene.
    render_skybox_and_scene(RENDER_STANDARD, false);

    // Chooses the proper cubemap texture:
    // - when the texture is no being updated, we use the loaded texture;
    // - when the texture is updated, we use the updated texture.
    const GLuint reflection_texture = (cubemap_update_method == CUBE_MAP_UPDATE_NO_UPDATE) ? current_skybox_tex : cubemap_color_texture;

    // Chooses a proper shader and sets proper textures depending whether the reflection is on or off.
    if (use_reflections) {
        reflection_program.use();
        reflection_program.uniform("has_texture", true);
        reflection_program.uniform("reflection_factor", reflection_factor);
        glBindTextureUnit(0, wood_tex);
        glBindTextureUnit(1, reflection_texture);
    } else {
        display_cubemap_program.use();
        glBindTextureUnit(0, reflection_texture);
    }

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

// ----------------------------------------------------------------------------
// GUI
// ----------------------------------------------------------------------------
void Application::render_ui() {
    const float unit = ImGui::GetFontSize();

    ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoDecoration);
    ImGui::SetWindowSize(ImVec2(26 * unit, 13 * unit));
    ImGui::SetWindowPos(ImVec2(2 * unit, 2 * unit));

    std::string fps_cpu_string = "FPS (CPU): ";
    ImGui::Text(fps_cpu_string.append(std::to_string(fps_cpu)).c_str());

    std::string fps_string = "FPS (GPU): ";
    ImGui::Text(fps_string.append(std::to_string(fps_gpu)).c_str());

    if (ImGui::Combo("Skybox", &skybox_texture, SKYBOX_LABELS, IM_ARRAYSIZE(SKYBOX_LABELS))) {
        if (skybox_texture == SKYBOX_MEADOW) {
            current_skybox_tex = meadow_skybox_tex;

            // Sets the light's position to correspond to the position of the sun in skybox texture.
            phong_lights_ubo.get_lights()[0].position = glm::vec4(1.0f, 0.25f, 1.0f, 0.0f);
            phong_lights_ubo.update_opengl_data();
        } else if (skybox_texture == SKYBOX_MIRAMAR) {
            current_skybox_tex = miramar_skybox_tex;

            // Sets the light's position to correspond to the position of the sun in skybox texture.
            phong_lights_ubo.get_lights()[0].position = glm::vec4(1.0f, 2.0f, -1.0f, 0.0f);
            phong_lights_ubo.update_opengl_data();
        }
    }
    ImGui::Checkbox("Task 2: Reflections", &use_reflections);
    if (use_reflections) {
        ImGui::SliderFloat("Reflection Factor", &reflection_factor, 0.0f, 1.0f);
    }
    ImGui::Combo("Update Method", &cubemap_update_method, CUBE_MAP_UPDATE_LABELS, IM_ARRAYSIZE(CUBE_MAP_UPDATE_LABELS));
    ImGui::Combo("Central Object", &central_object, CENTRAL_OBJECT_LABELS, IM_ARRAYSIZE(CENTRAL_OBJECT_LABELS));

    ImGui::End();
}

// ----------------------------------------------------------------------------
// Input Events
// ----------------------------------------------------------------------------
void Application::on_resize(int width, int height) {
    PV227Application::on_resize(width, height);
    resize_fullscreen_textures();
}

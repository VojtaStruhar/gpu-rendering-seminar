#include "application.hpp"
#include <texture_utils.hpp>
#include <vector>

Application::SpectralSystesisInfo::SpectralSystesisInfo()
    : noise_frequency(1.0f), sampling_frequency(75U), start_amplitude(1.0f), amplitude_multiplier(0.5f), frequency_multiplier(2.0f),
      arg0(10.0f), light_position(164.051f), tree_density(0.25f), trees(true), textures(false) {}

std::map<std::string, Application::noise_function_type> const Application::noise_functions_map{
    {"1. Plain noise",
     [](glm::vec3 const& pos, SpectralSystesisInfo const& info) { return Application::perlin_noise_improved(info.noise_frequency * pos); }},
    {"2. Fractal sum", Application::fractalsum},
    {"3. Turbulence", Application::turbulence},
    { "4. Marble", Application::marble },
};

Application::Application(int initial_width, int initial_height, std::vector<std::string> arguments)
    : OpenGLApplication(initial_width, initial_height, arguments), synthesis_info(), is_systesis_info_changed(true),
      current_noise_function_name(noise_functions_map.begin()->first),

      light_shader(lecture_shaders_path / "light.vert", lecture_shaders_path / "light.frag"), noise_texture(0U) {

    camera.set_eye_position(0, 0, 140);

    // Defines the number of triangles on one side.
    const int triangles = size - 1;

    // Generates the mesh vertices.
    std::vector<float> vertices = {};
    for (int z = 0; z < size; z++) {
        for (int x = 0; x < size; x++) {
            vertices.push_back(x - (triangles / 2.f));
            vertices.push_back(0.0f);
            vertices.push_back(z - (triangles / 2.f));
        }
    }

    // Generates the mesh triangles.
    std::vector<uint32_t> indices = {};
    for (int z = 0; z < triangles; z++) {
        for (int x = 0; x < triangles; x++) {
            uint32_t i1 = x + (z * size);

            indices.push_back(i1);
            indices.push_back(i1 + size);
            indices.push_back(i1 + 1);

            indices.push_back(i1 + 1);
            indices.push_back(i1 + size);
            indices.push_back(i1 + size + 1);
        }
    }

    // generates the uvs.
    std::vector<float> uvs = {};
    for (float j = 0; j < size; j++) {
        for (float i = 0; i < size; i++) {
            uvs.push_back(i / size);
            uvs.push_back(j / size);
        }
    }

    prepare_textures();

    // Generates the terrain geometry.
    geometry_terrain = Geometry_3_3(GL_TRIANGLES, vertices, indices, {}, {}, uvs, {}, {}, Geometry_Base::DEFAULT_POSITION_LOC, -1, -1,
                                    Geometry_Base::DEFAULT_TEX_COORD_LOC, -1, -1);
    // Loads the tree model.
    geometry_tree_bark = std::make_shared<Geometry>(Geometry::from_file(lecture_folder_path / "objects/bark.obj", false));
    geometry_tree_leaves = std::make_shared<Geometry>(Geometry::from_file(lecture_folder_path / "objects/leaves.obj", false));

    create_ubo_on_gpu();

    glClearColor(0.25f, 0.25f, 0.25f, 1.0f);

    compile_shaders();
}

void Application::compile_shaders() {
    shader_terrain = ShaderProgram(lecture_shaders_path / "terrain.vert", lecture_shaders_path / "terrain.frag");
    shader_terrain_noise_texture =
        ShaderProgram(lecture_shaders_path / "terrain.vert", lecture_shaders_path / "terrain_noise_texture.frag");
    shader_tree = ShaderProgram(lecture_shaders_path / "tree.vert", lecture_shaders_path / "tree.frag");
}

void Application::prepare_textures() {
    grass_texture = TextureUtils::load_texture_2d(lecture_textures_path / "grass_texture2.jpg");
    TextureUtils::set_texture_2d_parameters(grass_texture, GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);
    mountain_texture = TextureUtils::load_texture_2d(lecture_textures_path / "mountain_texture.jpg");
    TextureUtils::set_texture_2d_parameters(mountain_texture, GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);

    grass_texture_normals = TextureUtils::load_texture_2d(lecture_textures_path / "grass_texture_normal2.jpg");
    TextureUtils::set_texture_2d_parameters(grass_texture_normals, GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);
    mountain_texture_normals = TextureUtils::load_texture_2d(lecture_textures_path / "mountain_texture_normal.jpg");
    TextureUtils::set_texture_2d_parameters(mountain_texture_normals, GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);

    // tree
    tree_bark_texture = TextureUtils::load_texture_2d(lecture_textures_path / "bark.jpg");
    TextureUtils::set_texture_2d_parameters(tree_bark_texture, GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);

    tree_branch_texture = TextureUtils::load_texture_2d(lecture_textures_path / "branch.png");
    TextureUtils::set_texture_2d_parameters(tree_branch_texture, GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);
}

GLuint Application::make_noise_texture(SpectralSystesisInfo const& info, noise_function_type const& content_generator) {

    std::vector<unsigned char> data;
    data.resize((4U * info.sampling_frequency * info.sampling_frequency) * 4U, 127U);
    auto const coord_fn = [&info](int c) { return (c - (float)info.sampling_frequency) / (float)info.sampling_frequency; };
    std::function<glm::vec3(int, int)> position = [&coord_fn,&info](int i, int j) -> glm::vec3 {
        return glm::vec3(coord_fn(i), coord_fn(j), info.slice);
    };
    auto const clamp = [](float x, float a, float b) -> float { return x < a ? a : (x > b) ? b : x; };

    for (int i = 0; i != 2U * info.sampling_frequency; ++i)
        for (int j = 0; j != 2U * info.sampling_frequency; ++j) {
            glm::vec3 pos = position(i, j);
            float const noise = content_generator(pos, info);
            float const intensity = clamp((noise + 1.0f) / 2.0f, 0.0f, 1.0f);
            unsigned char colour = (unsigned char)(255.0f * intensity);
            unsigned char* rgb = &data.at((j * (2U * info.sampling_frequency) + i) * 4U);
            rgb[0] = rgb[1] = rgb[2] = rgb[3] = colour;
        }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2U * info.sampling_frequency, 2U * info.sampling_frequency, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 data.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return texture;
}

GLuint Application::rebuild_noise_texture() {
    if (noise_texture != 0 && noise_texture != -1)
        glDeleteTextures(1, &noise_texture);
    noise_texture = make_noise_texture(synthesis_info, noise_functions_map.at(current_noise_function_name));
    return noise_texture;
}

void Application::update(float delta) {
    if (is_systesis_info_changed) {
        is_systesis_info_changed = false;
        rebuild_noise_texture();
    }
    if (is_light_position_changed) {
        is_light_position_changed = false;

        light_position =
            glm::vec3(60, 60, 100) *
            glm::vec3(cosf(glm::radians(synthesis_info.light_position / 6.0f)) * sinf(glm::radians(synthesis_info.light_position)),
                      sinf(glm::radians(synthesis_info.light_position / 6.0f)),
                      cosf(glm::radians(synthesis_info.light_position / 6.0f)) * cosf(glm::radians(synthesis_info.light_position)));
    }

    if (is_tree_density_changed) {
        is_tree_density_changed = false;
        make_tree_model_matrices();
        fill_ubo_on_gpu();
    }
}

void Application::render_light() {
    glm::mat4 projection_matrix =
        glm::perspective(glm::radians(45.0f), static_cast<float>(width) / static_cast<float>(height), 1.0f, 1000.0f);
    glm::mat4 view_matrix = lookAt(camera.get_eye_position(), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 model_matrix = translate(glm::mat4(1.0f), glm::vec3(light_position));

    light_shader.use();
    glUniformMatrix4fv(light_shader.get_uniform_location("projection"), 1, GL_FALSE, value_ptr(projection_matrix));
    glUniformMatrix4fv(light_shader.get_uniform_location("view"), 1, GL_FALSE, value_ptr(view_matrix));
    glUniformMatrix4fv(light_shader.get_uniform_location("model"), 1, GL_FALSE, value_ptr(model_matrix));

    sphere.draw();
}

void Application::render_terrain(ShaderProgram shader) {
    glm::mat4 projection_matrix =
        glm::perspective(glm::radians(45.0f), static_cast<float>(width) / static_cast<float>(height), 1.0f, 1000.0f);
    glm::mat4 view_matrix = lookAt(camera.get_eye_position(), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 model_matrix = translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    glm::mat4 light_model_matrix = translate(glm::mat4(1.0f), glm::vec3(light_position));

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    shader.use();

    glUniformMatrix4fv(shader.get_uniform_location("projection"), 1, GL_FALSE, value_ptr(projection_matrix));
    glUniformMatrix4fv(shader.get_uniform_location("view"), 1, GL_FALSE, value_ptr(view_matrix));
    glUniformMatrix4fv(shader.get_uniform_location("model"), 1, GL_FALSE, value_ptr(model_matrix));

    // light
    glm::vec3 eye_position = camera.get_eye_position();
    glUniform3f(shader.get_uniform_location("eye_position"), eye_position.x, eye_position.y, eye_position.z);
    glUniform4f(shader.get_uniform_location("light_position"), light_position.x, light_position.y, light_position.z, 1.0f);
    glUniform3f(shader.get_uniform_location("light_ambient_color"), 0.08f, 0.08f, 0.08f);
    glUniform3f(shader.get_uniform_location("light_diffuse_color"), 0.5f, 0.5f, 0.5f);
    glUniform3f(shader.get_uniform_location("light_specular_color"), 0.5f, 0.5f, 0.5f);

    // material
    glUniform3f(shader.get_uniform_location("material_ambient_color"), 0.4f, 0.4f, 0.4f);
    glUniform3f(shader.get_uniform_location("material_diffuse_color"), 0.5f, 0.5f, 0.5f);
    glUniform3f(shader.get_uniform_location("material_specular_color"), 0.2f, 0.2f, 0.2f);
    glUniform1f(shader.get_uniform_location("material_shininess"), 4.0f);

    shader.uniform("noise_texture", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, noise_texture);

    shader.uniform("grass_texture", 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, grass_texture);

    shader.uniform("mountain_texture", 2);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, mountain_texture);

    shader.uniform("mountain_texture_normals", 3);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, mountain_texture_normals);

    shader.uniform("grass_texture_normals", 4);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, grass_texture_normals);

    // Sets the terrain height multiplier uniform.
    shader.uniform("terrain_height_multiplier", synthesis_info.terrain_height_multiplier);

    geometry_terrain.draw();
}

void Application::make_tree_model_matrices() {
    tree_model_matrices = {};
    srand(74);
    for (float x = -size / 2.f + 1; x < size / 2.f - 1; x += 5.f) {
        for (float z = -size / 2.f + 1; z < size / 2.f - 1; z += 5.f) {
            float rx = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2 - 1;
            float rz = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2 - 1;
            float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            if (r > 1.0f - synthesis_info.tree_density) {
                glm::mat4 tree_model = glm::translate(glm::mat4(1.0f), glm::vec3(rx + x, 0, rz + z));
                tree_model_matrices.push_back(tree_model);
                tree_model_matrices.push_back(transpose(inverse(tree_model)));
            }
        }
    }
}

void Application::create_ubo_on_gpu() {
    glGenBuffers(1, &tree_models_buffer);
    glBindBuffer(GL_UNIFORM_BUFFER, tree_models_buffer);
    glBufferData(GL_UNIFORM_BUFFER, 2 * 500 * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Application::fill_ubo_on_gpu() {
    glBindBuffer(GL_UNIFORM_BUFFER, tree_models_buffer);

    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4) * tree_model_matrices.size(), tree_model_matrices.data());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Application::render_trees() {

    glm::mat4 projection_matrix =
        glm::perspective(glm::radians(45.0f), static_cast<float>(width) / static_cast<float>(height), 1.0f, 1000.0f);
    glm::mat4 view_matrix = lookAt(camera.get_eye_position(), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    shader_tree.use();

    // matrices
    glUniformMatrix4fv(shader_tree.get_uniform_location("projection"), 1, GL_FALSE, value_ptr(projection_matrix));
    glUniformMatrix4fv(shader_tree.get_uniform_location("view"), 1, GL_FALSE, value_ptr(view_matrix));
    int locationModels = shader_tree.get_uniform_block_index("Models");
    glUniformBlockBinding(shader_tree.get_opengl_program(), locationModels, tree_models_buffer);
    glBindBufferBase(GL_UNIFORM_BUFFER, tree_models_buffer, tree_models_buffer);

    // lights
    glm::vec3 eye_position = camera.get_eye_position();
    glUniform3f(shader_tree.get_uniform_location("eye_position"), eye_position.x, eye_position.y, eye_position.z);
    glUniform4f(shader_tree.get_uniform_location("light_position"), light_position.x, light_position.y, light_position.z, 1.0f);
    glUniform3f(shader_tree.get_uniform_location("light_ambient_color"), 0.08f, 0.08f, 0.08f);
    glUniform3f(shader_tree.get_uniform_location("light_diffuse_color"), 0.5f, 0.5f, 0.5f);
    glUniform3f(shader_tree.get_uniform_location("light_specular_color"), 0.5f, 0.5f, 0.5f);

    // material
    glUniform3f(shader_tree.get_uniform_location("material_ambient_color"), 0.4f, 0.4f, 0.4f);
    glUniform3f(shader_tree.get_uniform_location("material_diffuse_color"), 0.5f, 0.5f, 0.5f);
    glUniform3f(shader_tree.get_uniform_location("material_specular_color"), 0.2f, 0.2f, 0.2f);
    glUniform1f(shader_tree.get_uniform_location("material_shininess"), 4.0f);

    // Sets the terrain height multiplier uniform.
    shader_tree.uniform("terrain_height_multiplier", synthesis_info.terrain_height_multiplier);

    // bark texture
    shader_tree.uniform("tree_texture", 5);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, tree_bark_texture);

    shader_tree.uniform("noise_texture", 6);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, noise_texture);
    geometry_tree_bark->draw_instanced(static_cast<int>(tree_model_matrices.size()));

    // branches texture
    shader_tree.uniform("tree_texture", 5);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, tree_branch_texture);

    //glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    geometry_tree_leaves->draw_instanced(static_cast<int>(tree_model_matrices.size()));

    glDisable(GL_BLEND);
    //glDepthMask(GL_TRUE);
}

void Application::render() {
    if (!shader_terrain.is_valid())
        return;
    activate_render_buffer();
    glViewport(0, 0, width, height);

    render_light();
    if (synthesis_info.textures) {
        render_terrain(shader_terrain_noise_texture);
    } else {
        render_terrain(shader_terrain);
        if (synthesis_info.trees) {
            render_trees();
        }
    }
}

void Application::render_ui() {
    ImGui::Begin("Settings", nullptr);

    static bool size_set = false;
    if (!size_set) {
        const float unit = ImGui::GetFontSize();
        ImGui::SetWindowSize(ImVec2(30 * unit, 24 * unit));
        ImGui::SetWindowPos(ImVec2(0, 0));
        size_set = true;
    }

    bool b00 = false;
    if (ImGui::BeginCombo("Noise generator", current_noise_function_name.c_str())) {
        for (auto const& name_and_level : noise_functions_map) {
            bool const is_selected = name_and_level.first == current_noise_function_name;
            if (ImGui::Selectable(name_and_level.first.c_str(), is_selected)) {
                if (current_noise_function_name != name_and_level.first)
                    b00 = true;
                current_noise_function_name = name_and_level.first;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    bool b01 = ImGui::SliderInt("Sampling frequency", (int*)&synthesis_info.sampling_frequency, 1U, 128U);
    bool b02 = ImGui::SliderFloat("Noise frequency", &synthesis_info.noise_frequency, 1.0f, 3.0f);
    bool b05 = ImGui::SliderFloat("Frequency multiplier", &synthesis_info.frequency_multiplier, 1.75f, 5.0f);
    bool b03 = ImGui::InputFloat("Start amplitude", &synthesis_info.start_amplitude, 0.1f);
    bool b04 = ImGui::InputFloat("Amplitude multiplier", &synthesis_info.amplitude_multiplier, 0.1f);
    bool b06 = ImGui::InputFloat("Arg 0", &synthesis_info.arg0, 0.1f);
    bool b07 = ImGui::SliderFloat("Light position", &synthesis_info.light_position, 0.0f, 360.0f);
    bool b10 = ImGui::SliderFloat("Tree density", &synthesis_info.tree_density, 0.0f, 0.5f);
    bool b11 = ImGui::SliderFloat("Height multiplier", &synthesis_info.terrain_height_multiplier, 10.0f, 50.0f);
    bool b12 = ImGui::SliderFloat("Slice", &synthesis_info.slice, 0.0f, 10.0f);
    bool b08 = ImGui::Checkbox("Trees", &synthesis_info.trees);
    bool b09 = ImGui::Checkbox("Show Noise Texture", &synthesis_info.textures);

    if (b00 || b01 || b02 || b03 || b04 || b05 || b06 || b12)
        is_systesis_info_changed = true;

    if (b07)
        is_light_position_changed = true;

    if (b10)
        is_tree_density_changed = true;
    ImGui::End();
}

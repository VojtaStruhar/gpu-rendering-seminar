#pragma once

#include "geometry_3_3.hpp"
#include "opengl_application.hpp"
#include "program.hpp"
#include "scene_object.hpp"
#include <functional>
#include <map>
#include <string>

class Application : public OpenGLApplication {

    // ----------------------------------------------------------------------------
    // The assignment
    // ----------------------------------------------------------------------------

  public:
    // Holds configuration of a spectral synthesis noise functions: fractal sum, turbulence, marble.
    struct SpectralSystesisInfo {
        SpectralSystesisInfo();

        // The (initial) frequency for the composed octaves.
        // Usage: for (f = this->noise_frequency; ...; f *= this->frequency_multiplier) { ... perlin_noise_improved(f * pos) ... }
        float noise_frequency;
        // Number of texels between two neighbour lattice points.
        unsigned int sampling_frequency;
        // The (initial) amplitude for the composed octaves.
        // Usage: this->start_amplitude * perlin_noise_improved(...)
        float start_amplitude;
        // An amplitude-change coefficient. Typically, 0 < this->amplitude_multiplier < 1.
        // Usage: for (a = this->start_amplitude; ...; a *= this->amplitude_multiplier) { ... a * perlin_noise_improved(...) ... }
        float amplitude_multiplier;
        // A noise frequency-change coefficient. Typically, this->frequency_multiplier > 1.
        // Usage: for (f = this->noise_frequency; ...; f *= this->frequency_multiplier) { ... perlin_noise_improved(f * pos) ... }
        float frequency_multiplier;
        // An extra data, which can be used by some more complex noise function, e.g. the "alpha" coefficient of marble.
        float arg0;

        float light_position;

        bool trees;

        bool textures;

        float tree_density;
        float terrain_height_multiplier = 50;
        float slice = 1.0;
    };

    //
    // Your homework is to implement four functions below in the "application.cpp" file. See details there.
    //

    // Implementation of the "3D Perlin Noise Improved" noise function.
    static float perlin_noise_improved(glm::vec3 pos);
    // Implementation of the "Fractal sum" noise function.
    static float fractalsum(glm::vec3 const& pos, SpectralSystesisInfo const& info);
    // Implementation of the "Turbulence" noise function.
    static float turbulence(glm::vec3 const& pos, SpectralSystesisInfo const& info);
    // Implementation of the "Marble" noise function.
    static float marble(glm::vec3 const& pos, SpectralSystesisInfo const& info);

    // ----------------------------------------------------------------------------
    // Implementation details - Not necessary for understanding and completing the assignment.
    // ----------------------------------------------------------------------------

  public:
    using noise_function_type = std::function<float(glm::vec3 const&, SpectralSystesisInfo const&)>;

    Application(int initial_width, int initial_height, std::vector<std::string> arguments);

    void compile_shaders() override;

    GLuint rebuild_noise_texture();

    void prepare_textures();

    void update(float delta) override;

    void render() override;

    void render_light();

    void render_terrain(ShaderProgram shader);

    void make_tree_model_matrices();

    void render_trees();

    void create_ubo_on_gpu();

    void fill_ubo_on_gpu();

    void render_ui() override;

    static GLuint make_noise_texture(SpectralSystesisInfo const& info, noise_function_type const& content_generator);

    static std::map<std::string, noise_function_type> const noise_functions_map;

    glm::vec3 light_position;

    struct Vertex {
        glm::vec3 position;
    };

  private:
    SpectralSystesisInfo synthesis_info;
    bool is_systesis_info_changed;
    bool is_light_position_changed;
    bool is_tree_density_changed;
    std::string current_noise_function_name;
    ShaderProgram shader_terrain;
    ShaderProgram shader_terrain_noise_texture;
    ShaderProgram shader_tree;
    ShaderProgram light_shader;

    Geometry_3_3 geometry_noise;
    Geometry_3_3 geometry_terrain;
    std::shared_ptr<Geometry> geometry_tree_bark;
    std::shared_ptr<Geometry> geometry_tree_leaves;

    std::vector<glm::mat4> tree_model_matrices;
    GLuint tree_models_buffer;
    GLuint noise_texture;
    GLuint grass_texture;
    GLuint mountain_texture;
    GLuint tree_texture;
    GLuint tree_bark_texture;
    GLuint tree_branch_texture;

    GLuint grass_texture_normals;
    GLuint mountain_texture_normals;
    GLuint tree_texture_normals;

    /** Defines the number of vertices on the edge.*/
    const int size = 100;

    SceneObject tree_object;
};

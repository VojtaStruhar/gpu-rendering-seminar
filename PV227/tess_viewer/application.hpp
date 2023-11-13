#pragma once

#include "camera_ubo.hpp"
#include "light_ubo.hpp"
#include "model_ubo.hpp"
#include "pv227_application.hpp"
#include "scene_object.hpp"

class Application : public PV227Application {

    // ----------------------------------------------------------------------------
    // Variables (Shaders)
    // ----------------------------------------------------------------------------
protected:

    ShaderProgram IL_FOS_program;    // isolines, fractional_odd_spacing
  ShaderProgram IL_FOS_PM_program; // isolines, fractional_odd_spacing, point_mode
  ShaderProgram IL_FES_program;    // isolines, fractional_even_spacing
  ShaderProgram IL_FES_PM_program; // isolines, fractional_even_spacing, point_mode
  ShaderProgram IL_ES_program;     // isolines, equal_spacing
  ShaderProgram IL_ES_PM_program;  // isolines, equal_spacing, point_mode
  ShaderProgram T_FOS_program;     // triangles, fractional_odd_spacing
  ShaderProgram T_FOS_PM_program;  // triangles, fractional_odd_spacing, point_mode
  ShaderProgram T_FES_program;     // triangles, fractional_even_spacing
  ShaderProgram T_FES_PM_program;  // triangles, fractional_even_spacing, point_mode
  ShaderProgram T_ES_program;      // triangles, equal_spacing
  ShaderProgram T_ES_PM_program;   // triangles, equal_spacing, point_mode
  ShaderProgram Q_FOS_program;     // quads, fractional_odd_spacing
  ShaderProgram Q_FOS_PM_program;  // quads, fractional_odd_spacing, point_mode
  ShaderProgram Q_FES_program;     // quads, fractional_even_spacing
  ShaderProgram Q_FES_PM_program;  // quads, fractional_even_spacing, point_mode
  ShaderProgram Q_ES_program;      // quads, equal_spacing
  ShaderProgram Q_ES_PM_program;   // quads, equal_spacing, point_mode

    // ----------------------------------------------------------------------------
    // Variables (GUI)
    // ----------------------------------------------------------------------------
protected:
  
  const int PRIMITIVE_ISOLINES = 0;
  const int PRIMITIVE_QUADS = 1;
  const int PRIMITIVE_TRIANGLES = 2;
  const char* PRIMITIVE_LABELS[3] = {"Isolines", "Quads", "Triangles"};
  int primitive = PRIMITIVE_QUADS;

  
  const int EQUAL_SPACING = 0;
  const int FRACTIONAL_EVEN_SPACING = 1;
  const int FRACTIONAL_ODD_SPACING = 2;
  const char* SPACING_LABELS[3] = {"Equal Spacing", "Fractional Even Spacing", "Fractional Odd Spacing"};
  int spacing = EQUAL_SPACING;

    bool point_mode = false;

  float TessLevelInner[2] = {10.f, 10.f};
    float TessLevelOuter[4] = {10.f, 10.f, 10.f, 10.f};


    // ----------------------------------------------------------------------------
    // Constructors
    // ----------------------------------------------------------------------------
public:
    Application(int initial_width, int initial_height, std::vector<std::string> arguments = {});

    /** Destroys the {@link Application} and releases the allocated resources. */
    virtual ~Application();

    // ----------------------------------------------------------------------------
    // Shaders
    // ----------------------------------------------------------------------------
    /**
     * {@copydoc PV227Application::compile_shaders}
     */
    void compile_shaders() override;

    // ----------------------------------------------------------------------------
    // Initialize Scene
    // ----------------------------------------------------------------------------
public:
    // ----------------------------------------------------------------------------
    // Update
    // ----------------------------------------------------------------------------
    /**
     * {@copydoc PV227Application::update}
     */
    void update(float delta) override;

    // ----------------------------------------------------------------------------
    // Render
    // ----------------------------------------------------------------------------
public:

    /** @copydoc PV227Application::render */
    void render() override;

    // ----------------------------------------------------------------------------
    // GUI
    // ----------------------------------------------------------------------------
public:
    /** @copydoc PV227Application::render_ui */
    void render_ui() override;
};

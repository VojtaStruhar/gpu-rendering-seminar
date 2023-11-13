#include "application.hpp"
#include "utils.hpp"

Application::Application(int initial_width, int initial_height, std::vector<std::string> arguments)
    : PV227Application(initial_width, initial_height, arguments) {
    Application::compile_shaders();
}

Application::~Application() {
}

// ----------------------------------------------------------------------------
// Shaderes
// ----------------------------------------------------------------------------
void Application::compile_shaders() {
    IL_FOS_program.add_vertex_shader(lecture_shaders_path / "tesselation.vert");
    IL_FOS_program.add_tess_control_shader(lecture_shaders_path / "IL.tesc");
    IL_FOS_program.add_tess_evaluation_shader(lecture_shaders_path / "IL_FOS.tese");
    IL_FOS_program.add_fragment_shader(lecture_shaders_path / "tesselation.frag");
    IL_FOS_program.link();
    IL_FOS_PM_program.add_vertex_shader(lecture_shaders_path / "tesselation.vert");
    IL_FOS_PM_program.add_tess_control_shader(lecture_shaders_path / "IL.tesc");
    IL_FOS_PM_program.add_tess_evaluation_shader(lecture_shaders_path / "IL_FOS_PM.tese");
    IL_FOS_PM_program.add_fragment_shader(lecture_shaders_path / "tesselation.frag");
    IL_FOS_PM_program.link();

    IL_FES_program.add_vertex_shader(lecture_shaders_path / "tesselation.vert");
    IL_FES_program.add_tess_control_shader(lecture_shaders_path / "IL.tesc");
    IL_FES_program.add_tess_evaluation_shader(lecture_shaders_path / "IL_FES.tese");
    IL_FES_program.add_fragment_shader(lecture_shaders_path / "tesselation.frag");
    IL_FES_program.link();
    IL_FES_PM_program.add_vertex_shader(lecture_shaders_path / "tesselation.vert");
    IL_FES_PM_program.add_tess_control_shader(lecture_shaders_path / "IL.tesc");
    IL_FES_PM_program.add_tess_evaluation_shader(lecture_shaders_path / "IL_FES_PM.tese");
    IL_FES_PM_program.add_fragment_shader(lecture_shaders_path / "tesselation.frag");
    IL_FES_PM_program.link();

    IL_ES_program.add_vertex_shader(lecture_shaders_path / "tesselation.vert");
    IL_ES_program.add_tess_control_shader(lecture_shaders_path / "IL.tesc");
    IL_ES_program.add_tess_evaluation_shader(lecture_shaders_path / "IL_ES.tese");
    IL_ES_program.add_fragment_shader(lecture_shaders_path / "tesselation.frag");
    IL_ES_program.link();
    IL_ES_PM_program.add_vertex_shader(lecture_shaders_path / "tesselation.vert");
    IL_ES_PM_program.add_tess_control_shader(lecture_shaders_path / "IL.tesc");
    IL_ES_PM_program.add_tess_evaluation_shader(lecture_shaders_path / "IL_ES_PM.tese");
    IL_ES_PM_program.add_fragment_shader(lecture_shaders_path / "tesselation.frag");
    IL_ES_PM_program.link();

    T_FOS_program.add_vertex_shader(lecture_shaders_path / "tesselation.vert");
    T_FOS_program.add_tess_control_shader(lecture_shaders_path / "T.tesc");
    T_FOS_program.add_tess_evaluation_shader(lecture_shaders_path / "T_FOS.tese");
    T_FOS_program.add_fragment_shader(lecture_shaders_path / "tesselation.frag");
    T_FOS_program.link();
    T_FOS_PM_program.add_vertex_shader(lecture_shaders_path / "tesselation.vert");
    T_FOS_PM_program.add_tess_control_shader(lecture_shaders_path / "T.tesc");
    T_FOS_PM_program.add_tess_evaluation_shader(lecture_shaders_path / "T_FOS_PM.tese");
    T_FOS_PM_program.add_fragment_shader(lecture_shaders_path / "tesselation.frag");
    T_FOS_PM_program.link();

    T_FES_program.add_vertex_shader(lecture_shaders_path / "tesselation.vert");
    T_FES_program.add_tess_control_shader(lecture_shaders_path / "T.tesc");
    T_FES_program.add_tess_evaluation_shader(lecture_shaders_path / "T_FES.tese");
    T_FES_program.add_fragment_shader(lecture_shaders_path / "tesselation.frag");
    T_FES_program.link();
    T_FES_PM_program.add_vertex_shader(lecture_shaders_path / "tesselation.vert");
    T_FES_PM_program.add_tess_control_shader(lecture_shaders_path / "T.tesc");
    T_FES_PM_program.add_tess_evaluation_shader(lecture_shaders_path / "T_FES_PM.tese");
    T_FES_PM_program.add_fragment_shader(lecture_shaders_path / "tesselation.frag");
    T_FES_PM_program.link();

    T_ES_program.add_vertex_shader(lecture_shaders_path / "tesselation.vert");
    T_ES_program.add_tess_control_shader(lecture_shaders_path / "T.tesc");
    T_ES_program.add_tess_evaluation_shader(lecture_shaders_path / "T_ES.tese");
    T_ES_program.add_fragment_shader(lecture_shaders_path / "tesselation.frag");
    T_ES_program.link();
    T_ES_PM_program.add_vertex_shader(lecture_shaders_path / "tesselation.vert");
    T_ES_PM_program.add_tess_control_shader(lecture_shaders_path / "T.tesc");
    T_ES_PM_program.add_tess_evaluation_shader(lecture_shaders_path / "T_ES_PM.tese");
    T_ES_PM_program.add_fragment_shader(lecture_shaders_path / "tesselation.frag");
    T_ES_PM_program.link();

    Q_FOS_program.add_vertex_shader(lecture_shaders_path / "tesselation.vert");
    Q_FOS_program.add_tess_control_shader(lecture_shaders_path / "Q.tesc");
    Q_FOS_program.add_tess_evaluation_shader(lecture_shaders_path / "Q_FOS.tese");
    Q_FOS_program.add_fragment_shader(lecture_shaders_path / "tesselation.frag");
    Q_FOS_program.link();
    Q_FOS_PM_program.add_vertex_shader(lecture_shaders_path / "tesselation.vert");
    Q_FOS_PM_program.add_tess_control_shader(lecture_shaders_path / "Q.tesc");
    Q_FOS_PM_program.add_tess_evaluation_shader(lecture_shaders_path / "Q_FOS_PM.tese");
    Q_FOS_PM_program.add_fragment_shader(lecture_shaders_path / "tesselation.frag");
    Q_FOS_PM_program.link();

    Q_FES_program.add_vertex_shader(lecture_shaders_path / "tesselation.vert");
    Q_FES_program.add_tess_control_shader(lecture_shaders_path / "Q.tesc");
    Q_FES_program.add_tess_evaluation_shader(lecture_shaders_path / "Q_FES.tese");
    Q_FES_program.add_fragment_shader(lecture_shaders_path / "tesselation.frag");
    Q_FES_program.link();
    Q_FES_PM_program.add_vertex_shader(lecture_shaders_path / "tesselation.vert");
    Q_FES_PM_program.add_tess_control_shader(lecture_shaders_path / "Q.tesc");
    Q_FES_PM_program.add_tess_evaluation_shader(lecture_shaders_path / "Q_FES_PM.tese");
    Q_FES_PM_program.add_fragment_shader(lecture_shaders_path / "tesselation.frag");
    Q_FES_PM_program.link();

    Q_ES_program.add_vertex_shader(lecture_shaders_path / "tesselation.vert");
    Q_ES_program.add_tess_control_shader(lecture_shaders_path / "Q.tesc");
    Q_ES_program.add_tess_evaluation_shader(lecture_shaders_path / "Q_ES.tese");
    Q_ES_program.add_fragment_shader(lecture_shaders_path / "tesselation.frag");
    Q_ES_program.link();
    Q_ES_PM_program.add_vertex_shader(lecture_shaders_path / "tesselation.vert");
    Q_ES_PM_program.add_tess_control_shader(lecture_shaders_path / "Q.tesc");
    Q_ES_PM_program.add_tess_evaluation_shader(lecture_shaders_path / "Q_ES_PM.tese");
    Q_ES_PM_program.add_fragment_shader(lecture_shaders_path / "tesselation.frag");
    Q_ES_PM_program.link();

    std::cout << "Shaders are reloaded." << std::endl;
}

// ----------------------------------------------------------------------------
// Update
// ----------------------------------------------------------------------------
void Application::update(float delta) {
    PV227Application::update(delta);
}

// ----------------------------------------------------------------------------
// Render
// ----------------------------------------------------------------------------
void Application::render() {
    // Starts measuring the elapsed time.
    glBeginQuery(GL_TIME_ELAPSED, render_time_query);

    // Render into the main window
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);

    // Clear the framebuffer
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ShaderProgram* current_program = nullptr;
    if ((primitive == PRIMITIVE_ISOLINES) && (spacing == FRACTIONAL_ODD_SPACING) && !point_mode)
        current_program = &IL_FOS_program;
    if ((primitive == PRIMITIVE_ISOLINES) && (spacing == FRACTIONAL_ODD_SPACING) && point_mode)
        current_program = &IL_FOS_PM_program;
    if ((primitive == PRIMITIVE_ISOLINES) && (spacing == FRACTIONAL_EVEN_SPACING) && !point_mode)
        current_program = &IL_FES_program;
    if ((primitive == PRIMITIVE_ISOLINES) && (spacing == FRACTIONAL_EVEN_SPACING) && point_mode)
        current_program = &IL_FES_PM_program;
    if ((primitive == PRIMITIVE_ISOLINES) && (spacing == EQUAL_SPACING) && !point_mode)
        current_program = &IL_ES_program;
    if ((primitive == PRIMITIVE_ISOLINES) && (spacing == EQUAL_SPACING) && point_mode)
        current_program = &IL_ES_PM_program;
    if ((primitive == PRIMITIVE_TRIANGLES) && (spacing == FRACTIONAL_ODD_SPACING) && !point_mode)
        current_program = &T_FOS_program;
    if ((primitive == PRIMITIVE_TRIANGLES) && (spacing == FRACTIONAL_ODD_SPACING) && point_mode)
        current_program = &T_FOS_PM_program;
    if ((primitive == PRIMITIVE_TRIANGLES) && (spacing == FRACTIONAL_EVEN_SPACING) && !point_mode)
        current_program = &T_FES_program;
    if ((primitive == PRIMITIVE_TRIANGLES) && (spacing == FRACTIONAL_EVEN_SPACING) && point_mode)
        current_program = &T_FES_PM_program;
    if ((primitive == PRIMITIVE_TRIANGLES) && (spacing == EQUAL_SPACING) && !point_mode)
        current_program = &T_ES_program;
    if ((primitive == PRIMITIVE_TRIANGLES) && (spacing == EQUAL_SPACING) && point_mode)
        current_program = &T_ES_PM_program;
    if ((primitive == PRIMITIVE_QUADS) && (spacing == FRACTIONAL_ODD_SPACING) && !point_mode)
        current_program = &Q_FOS_program;
    if ((primitive == PRIMITIVE_QUADS) && (spacing == FRACTIONAL_ODD_SPACING) && point_mode)
        current_program = &Q_FOS_PM_program;
    if ((primitive == PRIMITIVE_QUADS) && (spacing == FRACTIONAL_EVEN_SPACING) && !point_mode)
        current_program = &Q_FES_program;
    if ((primitive == PRIMITIVE_QUADS) && (spacing == FRACTIONAL_EVEN_SPACING) && point_mode)
        current_program = &Q_FES_PM_program;
    if ((primitive == PRIMITIVE_QUADS) && (spacing == EQUAL_SPACING) && !point_mode)
        current_program = &Q_ES_program;
    if ((primitive == PRIMITIVE_QUADS) && (spacing == EQUAL_SPACING) && point_mode)
        current_program = &Q_ES_PM_program;

    if (current_program) {
        current_program->use();
        current_program->uniform("TessLevelInner[0]", TessLevelInner[0]);
        current_program->uniform("TessLevelInner[1]", TessLevelInner[1]);
        current_program->uniform("TessLevelOuter[0]", TessLevelOuter[0]);
        current_program->uniform("TessLevelOuter[1]", TessLevelOuter[1]);
        current_program->uniform("TessLevelOuter[2]", TessLevelOuter[2]);
        current_program->uniform("TessLevelOuter[3]", TessLevelOuter[3]);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Always in wireframe
        glPointSize(3.0f);

        glBindVertexArray(empty_vao);
        glPatchParameteri(GL_PATCH_VERTICES, 1);
        glDrawArrays(GL_PATCHES, 0, 1);
    }

    // Reset the VAO and the program
    glBindVertexArray(0);
    glUseProgram(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Stops measuring the elapsed time.
    glEndQuery(GL_TIME_ELAPSED);

    // Waits for OpenGL - don't forget OpenGL is asynchronous.
    glFinish();

    // Evaluates the query.
    GLuint64 render_time;
    glGetQueryObjectui64v(render_time_query, GL_QUERY_RESULT, &render_time);
    fps_gpu = 1000.f / (static_cast<float>(render_time) * 1e-6f);
}



// ----------------------------------------------------------------------------
// GUI
// ----------------------------------------------------------------------------
void Application::render_ui() {
    const float unit = ImGui::GetFontSize();

    ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoDecoration);
    ImGui::SetWindowSize(ImVec2(20 * unit, 22 * unit));
    ImGui::SetWindowPos(ImVec2(2 * unit, 2 * unit));

    std::string fps_cpu_string = "FPS (CPU): ";
    ImGui::Text(fps_cpu_string.append(std::to_string(fps_cpu)).c_str());

    std::string fps_string = "FPS (GPU): ";
    ImGui::Text(fps_string.append(std::to_string(fps_gpu)).c_str());

    ImGui::PushItemWidth(150);

    ImGui::Combo("Primitive", &primitive, PRIMITIVE_LABELS, IM_ARRAYSIZE(PRIMITIVE_LABELS));
    ImGui::Combo("Spacing", &spacing, SPACING_LABELS, IM_ARRAYSIZE(SPACING_LABELS));

    ImGui::Checkbox("Point Mode", &point_mode);

    ImGui::SliderFloat("TessLevelInner[0]", &TessLevelInner[0], 0.0f, 64.0f, "%.1f");
    ImGui::SliderFloat("TessLevelInner[1]", &TessLevelInner[1], 0.0f, 64.0f, "%.1f");
    ImGui::SliderFloat("TessLevelOuter[0]", &TessLevelOuter[0], 0.0f, 64.0f, "%.1f");
    ImGui::SliderFloat("TessLevelOuter[1]", &TessLevelOuter[1], 0.0f, 64.0f, "%.1f");
    ImGui::SliderFloat("TessLevelOuter[2]", &TessLevelOuter[2], 0.0f, 64.0f, "%.1f");
    ImGui::SliderFloat("TessLevelOuter[3]", &TessLevelOuter[3], 0.0f, 64.0f, "%.1f");

    ImGui::End();
}

#include "application.hpp"

namespace std {

template <typename T1, typename T2>
struct hash<pair<T1, T2>> {
    std::size_t operator()(std::pair<T1, T2> const& p) const {
        return std::hash<T1>()(p.first) ^ std::hash<T2>()(p.second);
    }
};

}

Application::SubdivisionTriangleMeshBuilder::SubdivisionTriangleMeshBuilder(SubdivisionTriangleMesh* const mesh_)
    : mesh(mesh_) {
    mesh->vertices.clear();
    mesh->edges.clear();
    mesh->faces.clear();
}

Application::Vertex* Application::SubdivisionTriangleMeshBuilder::insert_vertex(glm::vec3 const& pos, Vertex* const src_mesh_vertex) {
    mesh->vertices.push_back({pos, nullptr});
    if (src_mesh_vertex != nullptr)
        from_src_vertices_to_dst_vertices[src_mesh_vertex] = &mesh->vertices.back();
    return &mesh->vertices.back();
}

Application::Vertex* Application::SubdivisionTriangleMeshBuilder::insert_vertex(glm::vec3 const& pos, Edge* const src_mesh_edge) {
    mesh->vertices.push_back({pos, nullptr});
    if (src_mesh_edge != nullptr) {
        from_src_edges_to_dst_vertices[src_mesh_edge] = &mesh->vertices.back();
        from_src_edges_to_dst_vertices[src_mesh_edge->twin] = &mesh->vertices.back();
    }
    return &mesh->vertices.back();
}

Application::Vertex* Application::SubdivisionTriangleMeshBuilder::find_dst_vertex_of(Vertex* const src_mesh_vertex) const {
    auto const it = from_src_vertices_to_dst_vertices.find(src_mesh_vertex);
    return it == from_src_vertices_to_dst_vertices.end() ? nullptr : it->second;
}

Application::Vertex* Application::SubdivisionTriangleMeshBuilder::find_dst_vertex_of(Edge* const src_mesh_edge) const {
    auto const it = from_src_edges_to_dst_vertices.find(src_mesh_edge);
    return it == from_src_edges_to_dst_vertices.end() ? nullptr : it->second;
}

void Application::SubdivisionTriangleMeshBuilder::insert_triangle(Vertex* const v0, Vertex* const v1, Vertex* const v2) {
    triangles.push_back({v0, v1, v2});
}

void Application::SubdivisionTriangleMeshBuilder::finalize() {
    std::unordered_map<std::pair<Vertex const*, Vertex const*>, Edge*> edge_map;

    auto const insert_edge = [this, &edge_map](Vertex* const v0, Vertex* const v1) -> Edge*
    {
        mesh->edges.push_back({v0, v1, nullptr, nullptr, nullptr, nullptr});
        Edge* const e = &mesh->edges.back();
        if (v0->edge == nullptr)
            v0->edge = e;
        edge_map[{v0, v1}] = e;
        return e;
    };

    auto const insert_face = [this](Edge* const e) -> Face*
    {
        mesh->faces.push_back({e});
        return &mesh->faces.back();
    };

    for (Triangle const& t : triangles) {
        std::array<Edge*, 3> const e = {
            insert_edge(t[0], t[1]),
            insert_edge(t[1], t[2]),
            insert_edge(t[2], t[0])
        };

        Face* const f = insert_face(e[0]);

        for (std::size_t i = 0, j = 2, k = 1; i != 3; ++i, j = (j + 1) % 3, k = (k + 1) % 3) {
            auto const it = edge_map.find({e[i]->end, e[i]->start});
            if (it != edge_map.end()) {
                e[i]->twin = it->second;
                it->second->twin = e[i];
            }
            e[i]->next = e[k];
            e[i]->prev = e[j];
            e[i]->face = f;
        }
    }
}

std::shared_ptr<Application::SubdivisionTriangleMesh> createMeshTetrahedton() {
    std::shared_ptr<Application::SubdivisionTriangleMesh> mesh = std::make_shared<Application::SubdivisionTriangleMesh>();
    Application::SubdivisionTriangleMeshBuilder bld(mesh.get());

    Application::Vertex* const v0 = bld.insert_vertex(glm::vec3(glm::sqrt(8.0f / 9.0f), 0, -1.0f / 3.0f), static_cast<Application::Vertex*>(nullptr));
    Application::Vertex* const v1 = bld.insert_vertex(glm::vec3(-glm::sqrt(2.0f / 9.0f), glm::sqrt(2.0f / 3.0f), -1.0f / 3.0f), static_cast<Application::Vertex*>(nullptr));
    Application::Vertex* const v2 = bld.insert_vertex(glm::vec3(-glm::sqrt(2.0f / 9.0f), -glm::sqrt(2.0f / 3.0f), -1.0f / 3.0f), static_cast<Application::Vertex*>(nullptr));
    Application::Vertex* const v3 = bld.insert_vertex(glm::vec3(0, 0, 1), static_cast<Application::Vertex*>(nullptr));

    bld.insert_triangle(v0, v2, v1);
    bld.insert_triangle(v0, v1, v3);
    bld.insert_triangle(v1, v2, v3);
    bld.insert_triangle(v2, v0, v3);

    bld.finalize();
    return mesh;
}

std::shared_ptr<Application::SubdivisionTriangleMesh> createMeshTShape() {
    std::shared_ptr<Application::SubdivisionTriangleMesh> mesh = std::make_shared<Application::SubdivisionTriangleMesh>();
    Application::SubdivisionTriangleMeshBuilder bld(mesh.get());

    // bottom vertices - in CCW order when looking in -Z axis
    Application::Vertex* const v0 = bld.insert_vertex(0.5f * glm::vec3(-2 / 3.0f, -3 / 3.0f, -1 / 3.0f), static_cast<Application::Vertex*>(nullptr));
    Application::Vertex* const v1 = bld.insert_vertex(0.5f * glm::vec3(0 / 3.0f, -3 / 3.0f, -1 / 3.0f), static_cast<Application::Vertex*>(nullptr));
    Application::Vertex* const v2 = bld.insert_vertex(0.5f * glm::vec3(0 / 3.0f, -1 / 3.0f, -1 / 3.0f), static_cast<Application::Vertex*>(nullptr));
    Application::Vertex* const v3 = bld.insert_vertex(0.5f * glm::vec3(2 / 3.0f, -1 / 3.0f, -1 / 3.0f), static_cast<Application::Vertex*>(nullptr));
    Application::Vertex* const v4 = bld.insert_vertex(0.5f * glm::vec3(2 / 3.0f, 1 / 3.0f, -1 / 3.0f), static_cast<Application::Vertex*>(nullptr));
    Application::Vertex* const v5 = bld.insert_vertex(0.5f * glm::vec3(0 / 3.0f, 1 / 3.0f, -1 / 3.0f), static_cast<Application::Vertex*>(nullptr));
    Application::Vertex* const v6 = bld.insert_vertex(0.5f * glm::vec3(0 / 3.0f, 3 / 3.0f, -1 / 3.0f), static_cast<Application::Vertex*>(nullptr));
    Application::Vertex* const v7 = bld.insert_vertex(0.5f * glm::vec3(-2 / 3.0f, 3 / 3.0f, -1 / 3.0f), static_cast<Application::Vertex*>(nullptr));
    Application::Vertex* const v8 = bld.insert_vertex(0.5f * glm::vec3(-2 / 3.0f, 1 / 3.0f, -1 / 3.0f), static_cast<Application::Vertex*>(nullptr));
    Application::Vertex* const v9 = bld.insert_vertex(0.5f * glm::vec3(-2 / 3.0f, -1 / 3.0f, -1 / 3.0f), static_cast<Application::Vertex*>(nullptr));

    // top vertices - in CCW order when looking in -Z axis
    Application::Vertex* const v10 = bld.insert_vertex(0.5f * glm::vec3(-2 / 3.0f, -3 / 3.0f, 1 / 3.0f), static_cast<Application::Vertex*>(nullptr));
    Application::Vertex* const v11 = bld.insert_vertex(0.5f * glm::vec3(0 / 3.0f, -3 / 3.0f, 1 / 3.0f), static_cast<Application::Vertex*>(nullptr));
    Application::Vertex* const v12 = bld.insert_vertex(0.5f * glm::vec3(0 / 3.0f, -1 / 3.0f, 1 / 3.0f), static_cast<Application::Vertex*>(nullptr));
    Application::Vertex* const v13 = bld.insert_vertex(0.5f * glm::vec3(2 / 3.0f, -1 / 3.0f, 1 / 3.0f), static_cast<Application::Vertex*>(nullptr));
    Application::Vertex* const v14 = bld.insert_vertex(0.5f * glm::vec3(2 / 3.0f, 1 / 3.0f, 1 / 3.0f), static_cast<Application::Vertex*>(nullptr));
    Application::Vertex* const v15 = bld.insert_vertex(0.5f * glm::vec3(0 / 3.0f, 1 / 3.0f, 1 / 3.0f), static_cast<Application::Vertex*>(nullptr));
    Application::Vertex* const v16 = bld.insert_vertex(0.5f * glm::vec3(0 / 3.0f, 3 / 3.0f, 1 / 3.0f), static_cast<Application::Vertex*>(nullptr));
    Application::Vertex* const v17 = bld.insert_vertex(0.5f * glm::vec3(-2 / 3.0f, 3 / 3.0f, 1 / 3.0f), static_cast<Application::Vertex*>(nullptr));
    Application::Vertex* const v18 = bld.insert_vertex(0.5f * glm::vec3(-2 / 3.0f, 1 / 3.0f, 1 / 3.0f), static_cast<Application::Vertex*>(nullptr));
    Application::Vertex* const v19 = bld.insert_vertex(0.5f * glm::vec3(-2 / 3.0f, -1 / 3.0f, 1 / 3.0f), static_cast<Application::Vertex*>(nullptr));

    // bottom faces
    bld.insert_triangle(v0, v2, v1);
    bld.insert_triangle(v0, v9, v2);

    bld.insert_triangle(v9, v5, v2);
    bld.insert_triangle(v9, v8, v5);

    bld.insert_triangle(v2, v4, v3);
    bld.insert_triangle(v2, v5, v4);

    bld.insert_triangle(v8, v6, v5);
    bld.insert_triangle(v8, v7, v6);

    // top faces
    bld.insert_triangle(v10, v11, v12);
    bld.insert_triangle(v10, v12, v19);

    bld.insert_triangle(v19, v12, v15);
    bld.insert_triangle(v19, v15, v18);

    bld.insert_triangle(v12, v13, v14);
    bld.insert_triangle(v12, v14, v15);

    bld.insert_triangle(v18, v15, v16);
    bld.insert_triangle(v18, v16, v17);

    // back faces
    bld.insert_triangle(v0, v10, v19);
    bld.insert_triangle(v0, v19, v9);

    bld.insert_triangle(v9, v19, v18);
    bld.insert_triangle(v9, v18, v8);

    bld.insert_triangle(v8, v18, v17);
    bld.insert_triangle(v8, v17, v7);

    // front faces
    bld.insert_triangle(v1, v2, v12);
    bld.insert_triangle(v1, v12, v11);

    bld.insert_triangle(v3, v4, v14);
    bld.insert_triangle(v3, v14, v13);

    bld.insert_triangle(v5, v6, v16);
    bld.insert_triangle(v5, v16, v15);

    // left faces
    bld.insert_triangle(v0, v1, v11);
    bld.insert_triangle(v0, v11, v10);

    bld.insert_triangle(v2, v3, v13);
    bld.insert_triangle(v2, v13, v12);

    // right faces
    bld.insert_triangle(v4, v5, v15);
    bld.insert_triangle(v4, v15, v14);

    bld.insert_triangle(v6, v7, v17);
    bld.insert_triangle(v6, v17, v16);

    bld.finalize();
    return mesh;
}

std::shared_ptr<Application::SubdivisionTriangleMesh> createMeshTorus() {
    std::shared_ptr<Application::SubdivisionTriangleMesh> mesh = std::make_shared<Application::SubdivisionTriangleMesh>();
    Application::SubdivisionTriangleMeshBuilder bld(mesh.get());

    // bottom vertices, outer loop - in CCW order when looking in -Z axis
    Application::Vertex* const v0 = bld.insert_vertex(0.25f * glm::vec3(-2, -2, -1 / 2.0f), static_cast<Application::Vertex*>(nullptr));
    Application::Vertex* const v1 = bld.insert_vertex(0.25f * glm::vec3(2, -2, -1 / 2.0f), static_cast<Application::Vertex*>(nullptr));
    Application::Vertex* const v2 = bld.insert_vertex(0.25f * glm::vec3(2, 2, -1 / 2.0f), static_cast<Application::Vertex*>(nullptr));
    Application::Vertex* const v3 = bld.insert_vertex(0.25f * glm::vec3(-2, 2, -1 / 2.0f), static_cast<Application::Vertex*>(nullptr));

    // bottom vertices, inner loop - in CCW order when looking in -Z axis
    Application::Vertex* const v4 = bld.insert_vertex(0.25f * glm::vec3(-1, -1, -1 / 2.0f), static_cast<Application::Vertex*>(nullptr));
    Application::Vertex* const v5 = bld.insert_vertex(0.25f * glm::vec3(1, -1, -1 / 2.0f), static_cast<Application::Vertex*>(nullptr));
    Application::Vertex* const v6 = bld.insert_vertex(0.25f * glm::vec3(1, 1, -1 / 2.0f), static_cast<Application::Vertex*>(nullptr));
    Application::Vertex* const v7 = bld.insert_vertex(0.25f * glm::vec3(-1, 1, -1 / 2.0f), static_cast<Application::Vertex*>(nullptr));

    // top vertices, outer loop - in CCW order when looking in -Z axis
    Application::Vertex* const v8 = bld.insert_vertex(0.25f * glm::vec3(-2, -2, 1 / 2.0f), static_cast<Application::Vertex*>(nullptr));
    Application::Vertex* const v9 = bld.insert_vertex(0.25f * glm::vec3(2, -2, 1 / 2.0f), static_cast<Application::Vertex*>(nullptr));
    Application::Vertex* const v10 = bld.insert_vertex(0.25f * glm::vec3(2, 2, 1 / 2.0f), static_cast<Application::Vertex*>(nullptr));
    Application::Vertex* const v11 = bld.insert_vertex(0.25f * glm::vec3(-2, 2, 1 / 2.0f), static_cast<Application::Vertex*>(nullptr));

    // top vertices, inner loop - in CCW order when looking in -Z axis
    Application::Vertex* const v12 = bld.insert_vertex(0.25f * glm::vec3(-1, -1, 1 / 2.0f), static_cast<Application::Vertex*>(nullptr));
    Application::Vertex* const v13 = bld.insert_vertex(0.25f * glm::vec3(1, -1, 1 / 2.0f), static_cast<Application::Vertex*>(nullptr));
    Application::Vertex* const v14 = bld.insert_vertex(0.25f * glm::vec3(1, 1, 1 / 2.0f), static_cast<Application::Vertex*>(nullptr));
    Application::Vertex* const v15 = bld.insert_vertex(0.25f * glm::vec3(-1, 1, 1 / 2.0f), static_cast<Application::Vertex*>(nullptr));

    // bottom faces
    bld.insert_triangle(v0, v5, v1);
    bld.insert_triangle(v0, v4, v5);

    bld.insert_triangle(v1, v6, v2);
    bld.insert_triangle(v1, v5, v6);

    bld.insert_triangle(v2, v7, v3);
    bld.insert_triangle(v2, v6, v7);

    bld.insert_triangle(v3, v4, v0);
    bld.insert_triangle(v3, v7, v4);

    // top faces
    bld.insert_triangle(v8, v9, v13);
    bld.insert_triangle(v8, v13, v12);

    bld.insert_triangle(v9, v10, v14);
    bld.insert_triangle(v9, v14, v13);

    bld.insert_triangle(v10, v11, v15);
    bld.insert_triangle(v10, v15, v14);

    bld.insert_triangle(v11, v8, v12);
    bld.insert_triangle(v11, v12, v15);

    // back faces
    bld.insert_triangle(v0, v8, v11);
    bld.insert_triangle(v0, v11, v3);

    bld.insert_triangle(v5, v13, v14);
    bld.insert_triangle(v5, v14, v6);

    // front faces
    bld.insert_triangle(v1, v2, v10);
    bld.insert_triangle(v1, v10, v9);

    bld.insert_triangle(v4, v7, v15);
    bld.insert_triangle(v4, v15, v12);

    // left faces
    bld.insert_triangle(v0, v1, v9);
    bld.insert_triangle(v0, v9, v8);

    bld.insert_triangle(v7, v6, v14);
    bld.insert_triangle(v7, v14, v15);

    // right faces
    bld.insert_triangle(v2, v3, v11);
    bld.insert_triangle(v2, v11, v10);

    bld.insert_triangle(v5, v4, v12);
    bld.insert_triangle(v5, v12, v13);

    bld.finalize();
    return mesh;
}

void create_original_meshes(std::map<std::string, std::shared_ptr<Application::SubdivisionLevel>>& original_meshes) {
    original_meshes["Tetrahedton"] = std::make_shared<Application::SubdivisionLevel>(createMeshTetrahedton(), nullptr);
    original_meshes["T-Shape"] = std::make_shared<Application::SubdivisionLevel>(createMeshTShape(), nullptr);
    original_meshes["Torus"] = std::make_shared<Application::SubdivisionLevel>(createMeshTorus(), nullptr);
}

std::shared_ptr<Geometry_3_3> createGeometryFromMesh(Application::SubdivisionTriangleMesh const& mesh) {
    std::unordered_map<Application::Vertex const*, uint32_t> from_vertices_to_indices;
    std::vector<float> vertices;
    uint32_t idx = 0U;
    for (Application::Vertex const& vertex : mesh.vertices) {
        vertices.push_back(vertex.position.x);
        vertices.push_back(vertex.position.y);
        vertices.push_back(vertex.position.z);
        from_vertices_to_indices.insert({&vertex, idx++});
    }
    std::vector<uint32_t> indices;
    for (Application::Face const& face : mesh.faces) {
        Application::Edge const* e = face.edge;
        do {
            indices.push_back(from_vertices_to_indices.at(e->end));
            e = e->next;
        }
        while (e != face.edge);

    }
    return std::make_shared<Geometry_3_3>(GL_TRIANGLES, vertices, indices);
}

Application::Application(int initial_width, int initial_height, std::vector<std::string> arguments)
    : OpenGLApplication(initial_width, initial_height, arguments)
      , subdivision_levels{}
      , current_mesh_name("T-Shape")
      , current_level(0),
      shader(lecture_shaders_path / "main.vert", lecture_shaders_path / "main.frag")
      , wireframe(false)
      , is_gui_active(false) {
    create_original_meshes(original_meshes);
    for (auto& name_and_level : original_meshes)
        name_and_level.second->geometry = createGeometryFromMesh(*name_and_level.second->mesh);
    subdivision_levels.push_back(original_meshes[current_mesh_name]);
}

void Application::update(float delta) {
    if (current_level >= static_cast<int>(subdivision_levels.size())) {
        std::shared_ptr<SubdivisionTriangleMesh> const dst_mesh = std::make_shared<SubdivisionTriangleMesh>();
        loop_subdivision(*subdivision_levels.back()->mesh, *dst_mesh);
        if (!dst_mesh->faces.empty())
            subdivision_levels.push_back(std::make_shared<SubdivisionLevel>(dst_mesh, createGeometryFromMesh(*dst_mesh)));
    }
}

void Application::render() {
    if (!shader.is_valid())
        return;

    activate_render_buffer();

    shader.use();

    glm::mat4 projection_matrix = glm::perspective(glm::radians(45.0f), static_cast<float>(width) / static_cast<float>(height), 0.1f, 100.0f);
    glm::mat4 view_matrix = lookAt(camera.get_eye_position(), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 model_matrix = translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));

    glUniformMatrix4fv(shader.get_uniform_location("projection"), 1, GL_FALSE, value_ptr(projection_matrix));
    glUniformMatrix4fv(shader.get_uniform_location("view"), 1, GL_FALSE, value_ptr(view_matrix));
    glUniformMatrix4fv(shader.get_uniform_location("model"), 1, GL_FALSE, value_ptr(model_matrix));
    glUniform1f(shader.get_uniform_location("zshift"), 0.0f);

    Geometry_3_3 const& geometry = *subdivision_levels.at(std::max(0, std::min(current_level, static_cast<int>(subdivision_levels.size()) - 1)))->geometry;

    glClearColor(0.25f, 0.25f, 0.25f, 1.0f);

    glDisable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glUniform4f(shader.get_uniform_location("input_color"), 0.8f, 0.8f, 0.8f, 1.0f);
    geometry.draw();

    if (!wireframe) {
        glUniform1f(shader.get_uniform_location("zshift"), 0.001f);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glUniform4f(shader.get_uniform_location("input_color"), 0.5f, 0.5f, 0.5f, 1.0f);
        geometry.draw();
    }
}

void Application::render_ui() {
    ImGui::Begin("Loop subdivision", nullptr);

    static bool size_set = false;
    if (!size_set) {
        const float unit = ImGui::GetFontSize();
        ImGui::SetWindowSize(ImVec2(13 * unit, 7 * unit));
        ImGui::SetWindowPos(ImVec2(2 * unit, 2 * unit));
        size_set = true;
    }

    ImGui::SliderInt("Level", &current_level, 0, 5);
    if (ImGui::BeginCombo("Mesh", current_mesh_name.c_str())) {
        for (auto const& name_and_level : original_meshes) {
            bool const is_selected = name_and_level.first == current_mesh_name;
            if (ImGui::Selectable(name_and_level.first.c_str(), is_selected)) {
                current_mesh_name = name_and_level.first;
                subdivision_levels.clear();
                subdivision_levels.push_back(original_meshes[current_mesh_name]);
                current_level = 0;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGui::Checkbox("Wireframe", &wireframe);

    is_gui_active = ImGui::IsWindowHovered();

    ImGui::End();
}

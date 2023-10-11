#include "application.hpp"
#include <cassert>
#include <unordered_set>

/**
 * Accepts a vertex of the source (coarse) mesh and applies the vertex rule of the Loop subdivision to compute a
 * position of a new vertex of the destination (refined) mesh.
 * <p>
 * NOTE: The source mesh uses the "half-edge" representation.
 *
 * @param 	vertex	The vertex to process.
 * @return	A position of the new vertex.
 */
glm::vec3 Application::vertex_rule(Vertex const* const vertex) {

    Edge* starting_edge = vertex->edge;

    std::vector<Vertex const*> vertices;
    {
        // TODO: Collect all vertices incident with the passed one via an edge.
        // HINTS:
        //  - vertices.push_back(v) - appends the passed vertex "v" into the vector "vertices".
        //  - For the half-edge definition see the Application.hpp file.

        auto edge = vertex->edge;

        do {
            vertices.push_back(edge->end);
            edge = edge->twin->next;

        } while (edge != starting_edge && edge->start == vertex);
    }


    glm::vec3 result = glm::vec3(0, 0, 0);

    {
        // TODO: Use the vertices collected above to compute a position of the resulting vertex.
        // HINTS:
        //  - glm::pow(x,y) - computes x^y, i.e. y-th power of x.
        //  - glm::cos(x) - computes cosine of the passed angle x in radians.
        //  - glm::pi<float>() - the constant PI (3.1415...)
        //  - vertices.size() - the number of vertices stored in the vector "vertices".
        //  - For the half-edge definition see the Application.hpp file.

        float n = (float) vertices.size();


        float center = (0.375f + (0.25f * cos(2.0f * glm::pi<float>() / n)));
        float beta = (0.625f - (center * center)) / n;

        glm::vec3 position_sum = glm::vec3(0.0f);
        for (auto vert: vertices) {
            position_sum += vert->position;
        }

        result = vertex->position * (1 - n * beta) + position_sum * beta;
    }

    return result;
}

/**
 * Accepts an edge of the source (coarse) mesh and applies the edge rule of the Loop subdivision to compute a position of a new vertex of the destination (refined) mesh. 
 * <p>
 * NOTE: The source mesh uses the "half-edge" representation.
 *
 * @param 	edge	The edge to process.
 * @return	A position of the new vertex.
 */
glm::vec3 Application::edge_rule(Edge const* const edge) {
    glm::vec3 result = glm::vec3(0, 0, 0);
    {
        // TODO: Implement the edge rule (see slides from the lecture).
        //HINTS:
        //  - For the half-edge definition see the Application.hpp file.

        auto neighbor_left = edge->start;
        auto neighbor_right = edge->end;

        auto opposite_top = edge->next->end;
        auto opposite_bottom = edge->twin->next->end;

        result += neighbor_left->position * 3/8.0f;
        result += neighbor_right->position * 3/8.0f;
        result += opposite_bottom->position * 1/8.0f;
        result += opposite_top->position * 1/8.0f;
    }

    return result;
}

/**
 * Given a source (coarse) mesh "src_mesh" the method applies the Loop subdivision scheme in order to obtain the
 * destination (refined) mesh "dst_mesh". The destination mesh "dst_mesh" passed to the method must be empty, and
 * the method then builds the content according to the Loop subdivision scheme.
 * <p>
 * NOTE: The source mesh "src_mesh" uses the "half-edge" representation. 
 *       The destination mesh "dst_mesh" must also use the "half-edge" representation.
 *
 * @param 	src_mesh	The source mesh.
 * @param 	dst_mesh	the destination mesh.
 */
void Application::loop_subdivision(SubdivisionTriangleMesh const& src_mesh, SubdivisionTriangleMesh& dst_mesh) {
    SubdivisionTriangleMeshBuilder bld(&dst_mesh);

    for (Face const& face : src_mesh.faces) {
        std::vector<Vertex*> vertex_rule_vertices, edge_rule_vertices;

        {
            // TODO: Compute subdivision vertices using the rules, i.e. methods "vertex_rule" and "edge_rule" above. 
            //       Insert the computed vertices into the builder and also to vectors "vertex_rule_vertices" and "edge_rule_vertices".

            // HINTS:
            //  - For each edge and vertex of the face apply "Application::edge_rule" and "Application::vertex_rule" respectively.
            //  - However, be careful not to recompute already computed vertices => use "bld.find_dst_vertex_of" to check whether
            //    the vertex was computed already (otherwise the method returns "nullptr").

            //  - Note there are 2 "bld.find_dst_vertex_of" methods: One accepts vertices from the source mesh and the other edges
            //    from the source mesh.
            //  - For more details look at "SubdivisionTriangleMeshBuilder::find_dst_vertex_of" methods in the Application.hpp file.

            //  - Use methods "bld.insert_vertex" of the builder to insert a new vertex into the destination mesh and simultaneously
            //    to mark the vertex as being computed for the corresponsing source mesh element (vetex or edge).
            //  - Note there are 2 "bld.insert_vertex" methods: One accepts vertices passed to the vertex rule and the other edges
            //    passed to the edge rule.
            //	- For more details look at "SubdivisionTriangleMeshBuilder::insert_vertex" methods in the Application.hpp file.

            //  - vertex_rule_vertices.push_back(v) - appends the vertex v into the vector vertex_rule_vertices.
            //  - edge_rule_vertices.push_back(v) - appends the vertex v into the vector edge_rule_vertices.
            //
            //  - For the half-edge definition see the Application.hpp file.

            /// Current working edge :)
            Edge* edge = face.edge;

            for (int i = 0; i < 3; i++) {
                Vertex* vertex = edge->start;

                auto er_vertex = bld.find_dst_vertex_of(edge);
                if (er_vertex == nullptr) {
                    glm::vec3 edge_r = edge_rule(edge);
                    er_vertex = bld.insert_vertex(edge_r, edge);
                }
                edge_rule_vertices.push_back(er_vertex);


                auto vr_vertex = bld.find_dst_vertex_of(vertex);
                if (vr_vertex == nullptr) {
                    glm::vec3 vertex_r = vertex_rule(vertex);
                    vr_vertex = bld.insert_vertex(vertex_r, vertex);
                }
                vertex_rule_vertices.push_back(vr_vertex);

                /// Move to the next edge
                edge = edge->next;
            }


        }

        // Note: We are inserting 4 triangles, so 4 calls to the method will be made

        // TODO: Insert triangles between subdivision vertices obtained above.
        // HINTS:
        //  - The vetrices of all the triangles are collected in vectors "vertex_rule_vertices" and "edge_rule_vertices" computed above.
        //  - Call "bld.insert_triangle" to insert a triangle into the builder (i.e. the destination mesh).
        //  - There should be 4 triangles inserted into the builder.
        //	- For the half-edge definition see the Application.hpp file.

        bld.insert_triangle(vertex_rule_vertices[0], edge_rule_vertices[0], edge_rule_vertices[2]);
        bld.insert_triangle(vertex_rule_vertices[1], edge_rule_vertices[1], edge_rule_vertices[0]);
        bld.insert_triangle(vertex_rule_vertices[2], edge_rule_vertices[2], edge_rule_vertices[1]);

        bld.insert_triangle(edge_rule_vertices[0], edge_rule_vertices[1], edge_rule_vertices[2]);
    }

    bld.finalize();

    check_mesh_invariants(dst_mesh);
}

/**
 * This is helper function which checks whether basic invariants of the subdivided mesh hold true or not.
 * <p>
 * NOTE: This method is not part of the assignment. So, nothing to implement here. But its assertions can give you
 * an idea what is wrong, if you make a mistake.
 *
 * @param 	mesh	The mesh to check.
 */
void Application::check_mesh_invariants(SubdivisionTriangleMesh const& mesh) {
    if (mesh.vertices.empty()) {
        assert(("The empty mesh is also valid mesh", mesh.edges.empty() && mesh.faces.empty()));
        return;
    }

    assert(("Non-empty mesh should be at least tetrahedron", mesh.edges.size() >= 4 && mesh.edges.size() >= 12 && mesh.faces.size() > 4));

    std::unordered_set<Vertex const*> vertices;
    for (Vertex const& v : mesh.vertices)
        vertices.insert(&v);

    std::unordered_map<Edge const*, int> edges;
    for (Edge const& e : mesh.edges)
        edges.insert({&e, 0});

    std::unordered_set<Face const*> faces;
    for (Face const& f : mesh.faces)
        faces.insert(&f);

    for (Vertex const& v : mesh.vertices) {
        assert(("Vertices of coarsest meshes fit into unit cube. Subdivided meshes should fit there too.", glm::length(v.position) < glm::sqrt(3.0f)));
        assert(("The vertex must have valid edge.", edges.count(v.edge) != 0 && v.edge->start == &v));
    }

    for (Edge const& e : mesh.edges) {
        assert(("The edge must have valid vertices.", vertices.count(e.start) != 0 && vertices.count(e.end) != 0 && e.start != e.end));
        assert(("The edge must have valid twin edge.", edges.count(e.twin) != 0 && &e != e.twin && e.start == e.twin->end && e.end == e.twin->start));
        assert(("The edge must have valid next edge.", edges.count(e.next) != 0 && &e != e.next && e.next != e.prev && e.end == e.next->start && e.start != e.next->end));
        assert(("The edge must have valid prev edge.", edges.count(e.prev) != 0 && &e != e.prev && e.prev != e.next && e.start == e.prev->end && e.end != e.prev->start));
        assert(("The edge must have valid face.", faces.count(e.face) != 0));
    }

    for (Face const& f : mesh.faces) {
        assert(("The face must have valid edge.", edges.count(f.edge) != 0));
        ++edges[f.edge];
    }

    for (auto const& edge_and_count : edges) {
        assert(("An edge can be references from at most one face.", edge_and_count.second < 2));
    }
}

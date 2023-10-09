#pragma once

#include "geometry_3_3.hpp"
#include "program.hpp"
#include <array>
#include <list>
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>
#include "gui_application.h"
#include "opengl_application.hpp"

class Application : public OpenGLApplication {

    // ----------------------------------------------------------------------------
    // The assignment
    // ----------------------------------------------------------------------------

public:

    // --- Half-edge mesh representation. ---

    struct Vertex;
    struct Edge;
    struct Face;

    struct Vertex {
        glm::vec3 position; // The positions of the vertex in 3D (model) space.
        Edge* edge;         // An edge whose "start" vertex is this vertex. INVARIANT: this == this->edge->start;
    };

    struct Edge // The half-edge representation.
    {
        Vertex* start; // The start corner vertex of the edge.
        Vertex* end;   // The end corner vertex of the edge.
        Edge* twin;    // The twin edge of this one, oriented in the opposite direction. INVARIANT: this == this->twin->twin;
        Edge* next;    // The successor edge along the "face" this edge belongs to. INVARIANT: this == this->next->prev;
        Edge* prev;    // The predecessor edge along the "face" this edge belongs to. INVARIANT: this == this->prev->next;
        Face* face;    // The face this edge belongs to.
    };

    struct Face {
        Edge* edge; // One of the edges of this face.
    };

    struct SubdivisionTriangleMesh {
        std::list<Vertex> vertices; // All vertices of the mesh.
        std::list<Edge> edges;      // All half-edges of the mesh.
        std::list<Face> faces;      // All faces of the mesh.
    };

    // --- Helper class for construction of a subdivided mesh. ---

    // Provides an assistance in construction of half-edge representation of a mesh.
    // A user initializes the builder by passing it a reference to a mesh to be built.
    // Then, the content of the mesh is built by calling methods "insert_vertex" and
    // "insert_triangle" of the builder. And lastly, the method "finalize" must be
    // called to complete the half-edge representation.
    // NOTE: Observe that user only inserts vertices and triangles into the builder.
    //       Edges are computed automatically in the method "finalize".
    struct SubdivisionTriangleMeshBuilder {
        // Initialize the instance with the destination mesh to be constructed by the builder.
        SubdivisionTriangleMeshBuilder(SubdivisionTriangleMesh* mesh_);

        // Inserts a new vertex into the destination mesh. The passed position "pos" must be
        // computed by the method "Application::vertex_rule" (see below) for the vertex
        // "src_mesh_vertex" of the source mesh. The new vertex is then returned.
        Vertex* insert_vertex(glm::vec3 const& pos, Vertex* src_mesh_vertex);

        // Inserts a new vertex into the destination mesh. The passed position "pos" must be
        // computed by the method "Application::edge_rule" (see below) for the edge
        // "src_mesh_edge" of the source mesh. The new vertex is then returned.
        Vertex* insert_vertex(glm::vec3 const& pos, Edge* src_mesh_edge);

        // Accepts a vertex "src_mesh_vertex" of the source mesh and checks, whether the
        // method "insert_vertex(glm::vec3 const&, Vertex* const)" above was called for
        // that "src_mesh_vertex". If yes, then the vertex already computed by that method
        // is returned. Otherwise, "nullptr" is returned.
        Vertex* find_dst_vertex_of(Vertex* src_mesh_vertex) const;

        // Accepts an edge "src_mesh_edge" of the source mesh and checks, whether the
        // method "insert_vertex(glm::vec3 const&, Edge* const)" above was called for
        // that "src_mesh_edge". If yes, then the vertex already computed by that method
        // is returned. Otherwise, "nullptr" is returned.
        Vertex* find_dst_vertex_of(Edge* src_mesh_edge) const;

        // Given 3 vertices "v0", "v1", "v2" already inserted to the destination mesh
        // via any of the "insert_vertex" methods above, this method inserts a new
        // triangle (defined by the passed vertices) into the destination mesh.
        void insert_triangle(Vertex* v0, Vertex* v1, Vertex* v2);

        // This method must be called after all vertices and all triangles were
        // inserted into the destination mesh via methods "insert_vertex" and
        // "insert_triangle" respectively. This method completes the half-edge
        // representation of the destination mesh.
        void finalize();

    private:
        /** The definition of the Triangle type as an array of 3 vertices. */
        using Triangle = std::array<Vertex*, 3>;
        /** The mesh to process. */
        SubdivisionTriangleMesh* mesh;
        /** The list of triangles. */
        std::vector<Triangle> triangles;
        /** The map of all vertices from source mesh to the destination mesh. */
        std::unordered_map<Vertex const*, Vertex*> from_src_vertices_to_dst_vertices;
        /** The map of all edges from source mesh to the destination mesh. */
        std::unordered_map<Edge const*, Vertex*> from_src_edges_to_dst_vertices;
    };

    // --- Subdivision methods to be implemented in the file application.cpp ---

    // Implement the "vertex rule" of the Loop subdivision schema here.
    static glm::vec3 vertex_rule(Vertex const* vertex);

    // Implement the "edge rule" of the Loop subdivision schema here.
    static glm::vec3 edge_rule(Edge const* edge);

    // Compute a content of the destination mesh "dstMesh" from the source "srcMesh"
    // by applying the subdivision rules "vertex_rule" and "edge_rule" for all vertices
    // and edges of the source mesh respectively.
    // NOTE: "SubdivisionTriangleMeshBuilder" can be used to simplify the construction
    // of the half-edge representation of the destination mesh.
    static void loop_subdivision(SubdivisionTriangleMesh const& srcMesh, SubdivisionTriangleMesh& dstMesh);

    // --- Checking whether basic invariants of the subdivided mesh hold true or not. ---

    static void check_mesh_invariants(SubdivisionTriangleMesh const& mesh);

    // ----------------------------------------------------------------------------
    // Implementation details - Not necessary for understanding and completing the assignment.
    // ----------------------------------------------------------------------------

public:
    Application(int initial_width, int initial_height, std::vector<std::string> arguments);

    void update(float delta) override;

    void render() override;

    void render_ui() override;

    struct SubdivisionLevel {
        std::shared_ptr<SubdivisionTriangleMesh> mesh;
        std::shared_ptr<Geometry_3_3> geometry;
        SubdivisionLevel(std::shared_ptr<SubdivisionTriangleMesh> mesh, std::shared_ptr<Geometry_3_3> geometry) : mesh(mesh), geometry(geometry) {};
    };

private:
    std::map<std::string, std::shared_ptr<SubdivisionLevel>> original_meshes;
    std::vector<std::shared_ptr<SubdivisionLevel>> subdivision_levels;
    std::string current_mesh_name;
    int current_level;
    ShaderProgram shader;
    bool wireframe;
    bool is_gui_active;
};

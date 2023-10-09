#include "../application.hpp"
#include <cassert>
#include <functional>
#include <gtest/gtest.h>
#include <unordered_set>

/**
 * IMPORTANT NOTE:
 * The purpose of the test is to check correctness of the subdivision,
 * NOT the half-edge representation. We therefore introduce type
 * ComparableMesh capturing necessary minimum of the mesh topology.
 * The test then compares instances of this type.
 */
namespace std {

template <typename T1, typename T2> struct hash<tuple<T1, T2>> {
    std::size_t operator()(std::tuple<T1, T2> const& t) const { return std::hash<T1>()(std::get<0>(t)) ^ std::hash<T2>()(std::get<1>(t)); }
};

template <typename T1, typename T2, typename T3> struct hash<tuple<T1, T2, T3>> {
    std::size_t operator()(std::tuple<T1, T2, T3> const& t) const { return std::hash<T1>()(std::get<0>(t)) ^ std::hash<T2>()(std::get<1>(t)) ^ std::hash<T3>()(std::get<2>(t)); }
};

} // namespace std

struct ComparableMesh {
    using Vertex = std::tuple<int, int, int>;
    using Edge = std::tuple<std::size_t, std::size_t>;
    using Face = std::tuple<std::size_t, std::size_t, std::size_t>;

    ComparableMesh() : vertices(), edges(), faces() {}

    ComparableMesh(Application::SubdivisionTriangleMesh const& mesh) : ComparableMesh() {
        std::unordered_map<Application::Vertex const*, std::size_t> vertex_indices;
        std::size_t i = 0U;
        for (auto it = mesh.vertices.begin(); it != mesh.vertices.end(); ++it, ++i) {
            // We compare positions of vertices in mm precision.
            vertices.push_back({static_cast<int>(std::roundf(it->position.x * 10000.0f) / 10.0f), static_cast<int>(std::roundf(it->position.y * 10000.0f) / 10.0f),
                                static_cast<int>(std::roundf(it->position.z * 10000.0f) / 10.0f)});
            vertex_indices[&*it] = i;
        }

        for (Application::Edge const& e : mesh.edges) {
            std::size_t const v0 = vertex_indices.at(e.start);
            std::size_t const v1 = vertex_indices.at(e.end);
            if (edges.count({v0, v1}) == 0UL && edges.count({v1, v0}) == 0UL)
                edges.insert({v0, v1});
        }

        for (Application::Face const& f : mesh.faces) {
            std::vector<std::size_t> v;
            Application::Edge const* e = f.edge;
            do {
                v.push_back(vertex_indices.at(e->end));
                e = e->next;
            } while (e != f.edge);
            EXPECT_EQ(v.size(), 3UL) << "The faces must be triangles.";
            faces.insert({v[0], v[1], v[2]});
        }
    }

    std::vector<Vertex> vertices;
    std::unordered_set<Edge> edges;
    std::unordered_set<Face> faces;
};

static void save(std::ofstream& ostr, ComparableMesh const& mesh) {
    ostr << mesh.vertices.size() << '\n';
    for (ComparableMesh::Vertex const& v : mesh.vertices)
        ostr << std::get<0>(v) << '\n' << std::get<1>(v) << '\n' << std::get<2>(v) << '\n';

    ostr << mesh.edges.size() << '\n';
    for (ComparableMesh::Edge const& e : mesh.edges)
        ostr << std::get<0>(e) << '\n' << std::get<1>(e) << '\n';

    ostr << mesh.faces.size() << '\n';
    for (ComparableMesh::Face const& f : mesh.faces)
        ostr << std::get<0>(f) << '\n' << std::get<1>(f) << '\n' << std::get<2>(f) << '\n';
}

static void load(std::ifstream& istr, ComparableMesh& mesh) {
    auto const read_int = [&istr]() -> int {
        std::string line;
        const bool validity = !std::getline(istr, line).fail();
        EXPECT_TRUE(validity) << "Failed to read mesh file.";
        return std::stoi(line);
    };
    auto const read_uint = [&read_int]() -> std::size_t { return static_cast<std::size_t>(read_int()); };

    for (std::size_t i = 0, n = read_uint(); i < n; ++i)
        mesh.vertices.push_back({read_int(), read_int(), read_int()});

    for (std::size_t i = 0, n = read_uint(); i < n; ++i)
        mesh.edges.insert({read_uint(), read_uint()});

    for (std::size_t i = 0, n = read_uint(); i < n; ++i)
        mesh.faces.insert({read_uint(), read_uint(), read_uint()});
}

static void process_meshes(std::function<void(std::string const&, Application::SubdivisionTriangleMesh const&)> const& processor) {
    std::map<std::string, std::shared_ptr<Application::SubdivisionLevel>> original_meshes;
    extern void create_original_meshes(std::map<std::string, std::shared_ptr<Application::SubdivisionLevel>>&);
    create_original_meshes(original_meshes);
    for (auto const& name_and_level : original_meshes) {
        Application::SubdivisionTriangleMesh dst_mesh;
        Application::loop_subdivision(*name_and_level.second->mesh, dst_mesh);
        processor(name_and_level.first, dst_mesh);
    }
}

static std::filesystem::path data_dir;
static bool regenerate = false;

static void save_mesh(std::string const& name, Application::SubdivisionTriangleMesh const& mesh) {
    ComparableMesh const cmp_mesh(mesh);
    std::ofstream ostr((data_dir / name).string());
    save(ostr, cmp_mesh);
}

static void test_mesh(std::string const& name, Application::SubdivisionTriangleMesh const& mesh) {
    ComparableMesh tested_mesh(mesh);

    ComparableMesh correct_mesh;
    std::ifstream istr((data_dir / name).string());
    load(istr, correct_mesh);

    // equal(correct_mesh, tested_mesh);

    EXPECT_EQ(correct_mesh.vertices.size(), tested_mesh.vertices.size()) << "The meshes must have the same number of vertices";
    EXPECT_EQ(correct_mesh.edges.size(), tested_mesh.edges.size()) << "The meshes must have the same number of vertices";
    EXPECT_EQ(correct_mesh.faces.size(), tested_mesh.faces.size()) << "The meshes must have the same number of vertices";

    std::unordered_map<ComparableMesh::Vertex, std::size_t> from_correct_to_tested;
    {
        std::unordered_map<ComparableMesh::Vertex, std::size_t> right_indices;
        for (std::size_t i = 0U; i != tested_mesh.vertices.size(); ++i)
            right_indices[tested_mesh.vertices.at(i)] = i;
        for (std::size_t i = 0U; i != correct_mesh.vertices.size(); ++i) {
            ComparableMesh::Vertex const& v = correct_mesh.vertices.at(i);
            auto const it = right_indices.find(v);
            EXPECT_NE(it, right_indices.end()) << "Each expected vertex must be found in the tested mesh.";
            if (it != right_indices.end()) {
                from_correct_to_tested[v] = it->second;
            }
        }
    }

    for (ComparableMesh::Edge const& left_e : correct_mesh.edges) {
        auto const v0_it = from_correct_to_tested.find(correct_mesh.vertices.at(std::get<0>(left_e)));
        auto const v1_it = from_correct_to_tested.find(correct_mesh.vertices.at(std::get<1>(left_e)));
        EXPECT_NE(v0_it, from_correct_to_tested.end()) << "The start vertex of an expected edge must be found in the tested mesh.";
        EXPECT_NE(v1_it, from_correct_to_tested.end()) << "The end vertex of an expected edge must be found in the tested mesh.";
        if (v0_it != from_correct_to_tested.end() && v1_it != from_correct_to_tested.end()) {
            EXPECT_FALSE(tested_mesh.edges.count({v0_it->second, v1_it->second}) == 0UL && tested_mesh.edges.count({v1_it->second, v0_it->second}) == 0UL)
                << "Each expected edge must be found in the tested mesh.";
        }
    }

    for (ComparableMesh::Face const& left_f : correct_mesh.faces) {
        auto const v0_it = from_correct_to_tested.find(correct_mesh.vertices.at(std::get<0>(left_f)));
        auto const v1_it = from_correct_to_tested.find(correct_mesh.vertices.at(std::get<1>(left_f)));
        auto const v2_it = from_correct_to_tested.find(correct_mesh.vertices.at(std::get<2>(left_f)));
        EXPECT_NE(v0_it, from_correct_to_tested.end()) << "The first vertex of an expected face must be found in the tested mesh.";
        EXPECT_NE(v1_it, from_correct_to_tested.end()) << "The second vertex of an expected face must be found in the tested mesh.";
        EXPECT_NE(v2_it, from_correct_to_tested.end()) << "The third vertex of an expected face must be found in the tested mesh.";
        if (v0_it != from_correct_to_tested.end() && v1_it != from_correct_to_tested.end() && v2_it != from_correct_to_tested.end()) {
            EXPECT_FALSE(tested_mesh.faces.count({v0_it->second, v1_it->second, v2_it->second}) == 0UL && tested_mesh.faces.count({v2_it->second, v0_it->second, v1_it->second}) == 0UL &&
                         tested_mesh.faces.count({v1_it->second, v2_it->second, v0_it->second}) == 0UL)
                << "Each expected face must be found in the tested mesh.";
        }
    }
}

TEST(SubdivisionTestSuite, subdivision_test) {
    if (regenerate)
        process_meshes(save_mesh);
    process_meshes(test_mesh);
}

int main(int argc, char** argv) {
    data_dir = Configuration(argv[0]).get_path("data");
    regenerate = argc >= 2 && strncmp(argv[argc - 1], "--generate", 10) == 0;
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

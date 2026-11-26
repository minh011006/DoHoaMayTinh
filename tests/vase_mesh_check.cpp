#include "vase_mesh.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <vector>

namespace {

using vase::Mesh;
using vase::ProfilePoint;
using vase::TriangleIndex;
using vase::Vec3;

void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

double dot(const Vec3& a, const Vec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

void checkGeometry(const Mesh& mesh) {
    require(mesh.positions.size() == mesh.normals.size(), "Missing vertex normals");
    require(mesh.indices.size() % 3 == 0, "Incomplete index triple");
    for (const auto& normal : mesh.normals) {
        require(std::abs(dot(normal, normal) - 1.0) < 1e-12, "Normal is not a unit vector");
    }
    for (std::size_t i = 0; i < mesh.indices.size(); i += 3) {
        const auto a = mesh.indices[i];
        const auto b = mesh.indices[i + 1];
        const auto c = mesh.indices[i + 2];
        require(a < mesh.positions.size() && b < mesh.positions.size() &&
            c < mesh.positions.size(), "Vertex index out of bounds");
        const auto& p = mesh.positions[a];
        const auto& q = mesh.positions[b];
        const auto& r = mesh.positions[c];
        const Vec3 u{q.x - p.x, q.y - p.y, q.z - p.z};
        const Vec3 v{r.x - p.x, r.y - p.y, r.z - p.z};
        const Vec3 face{u.y * v.z - u.z * v.y, u.z * v.x - u.x * v.z,
            u.x * v.y - u.y * v.x};
        require(dot(face, face) > 0.0, "Triangle has zero area");
        require(dot(face, mesh.normals[a]) > 0.0 && dot(face, mesh.normals[b]) > 0.0 &&
            dot(face, mesh.normals[c]) > 0.0, "Triangle winding disagrees with vertex normals");
    }
}

void checkCylinder() {
    constexpr int slices = 8;
    const auto mesh = vase::generateVaseMesh({{2.0, 0.0}, {2.0, 3.0}}, slices);
    require(mesh.positions.size() == 16, "Cylinder has the wrong vertex count");
    require(mesh.indices.size() == 48, "Open cylinder must have 16 triangles");
    const TriangleIndex seam[] = {7, 15, 0, 0, 15, 8};
    require(std::equal(std::begin(seam), std::end(seam), mesh.indices.end() - 6),
        "Cylinder seam does not connect the last slice to the first");
    checkGeometry(mesh);
    for (std::size_t i = 0; i < mesh.positions.size(); ++i) {
        const auto& p = mesh.positions[i];
        const auto& n = mesh.normals[i];
        const double angle = 2.0 * std::acos(-1.0) * static_cast<double>(i % slices) / slices;
        require(std::abs(p.x - 2.0 * std::cos(angle)) < 1e-12 &&
            p.y == (i < slices ? 0.0 : 3.0) &&
            std::abs(p.z - 2.0 * std::sin(angle)) < 1e-12, "Cylinder position is wrong");
        require(std::abs(n.x - p.x / 2.0) < 1e-12 && n.y == 0.0 &&
            std::abs(n.z - p.z / 2.0) < 1e-12, "Cylinder normal is not radial and outward");
    }
    std::cout << "PASS cylinder: 16 vertices, 16 triangles, closed seam, outward normals\n";
}

void checkPoles() {
    constexpr int slices = 8;
    const auto bottom = vase::generateVaseMesh({{0.0, 0.0}, {2.0, 0.0}, {1.0, 3.0}}, slices);
    require(bottom.positions.size() == 17 && bottom.indices.size() == 72,
        "Bottom-pole mesh must have 17 vertices and 24 triangles");
    require(std::count(bottom.indices.begin(), bottom.indices.end(), 0u) == slices &&
        bottom.normals.front().y == -1.0, "Bottom pole must belong to eight outward triangles");
    checkGeometry(bottom);
    std::cout << "PASS bottom pole: 8 fan triangles, 24 nondegenerate triangles total\n";

    const auto both = vase::generateVaseMesh({{0.0, 0.0}, {2.0, 1.0}, {0.0, 3.0}}, slices);
    require(both.positions.size() == 10 && both.indices.size() == 48,
        "Two-pole mesh must have 10 vertices and 16 triangles");
    require(std::count(both.indices.begin(), both.indices.end(), 9u) == slices &&
        both.normals.back().y == 1.0, "Top pole must belong to eight outward triangles");
    checkGeometry(both);
    std::cout << "PASS top pole: 8 fan triangles with outward winding\n";
}

void expectInvalid(const std::vector<ProfilePoint>& profile, int slices) {
    try {
        vase::generateVaseMesh(profile, slices);
    } catch (const std::invalid_argument& error) {
        require(*error.what() != '\0', "Missing validation message");
        return;
    }
    throw std::runtime_error("Invalid profile or slice count was accepted");
}

void checkInvalidInputs() {
    expectInvalid({{0.0, 0.0}}, 8);
    expectInvalid({{1.0, 0.0}, {1.0, 1.0}}, 2);
    expectInvalid({{1.0, 0.0}, {-1.0, 1.0}}, 8);
    expectInvalid({{1.0, 0.0}, {1.0, std::numeric_limits<double>::infinity()}}, 8);
    expectInvalid({{1.0, 0.0}, {0.0, 1.0}, {1.0, 2.0}}, 8);
    expectInvalid({{1.0, 0.0}, {1.0, 0.0}}, 8);
    expectInvalid({{0.0, 0.0}, {0.0, 1.0}}, 8);
    expectInvalid({{1.0, 1.0}, {1.0, 0.0}}, 8);
    expectInvalid({{1.0, 0.0}, {2.0, 0.0}, {1.0, 0.0}}, 8);
    std::cout << "PASS invalid inputs: 9 cases rejected with std::invalid_argument\n";
}

}  // namespace

int main() {
    try {
        checkCylinder();
        checkPoles();
        checkInvalidInputs();
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
    return 0;
}

#pragma once

#include <cstdint>
#include <vector>

namespace vase {

struct ProfilePoint {
    double r;
    double y;
};

struct Vec3 {
    double x;
    double y;
    double z;
};

using TriangleIndex = std::uint32_t;

struct Mesh {
    std::vector<Vec3> positions;
    std::vector<Vec3> normals;            // One outward unit normal per position.
    std::vector<TriangleIndex> indices;   // Each triple is counterclockwise from outside.
};

// Revolve the ordered profile from base to mouth, with at least three slices.
// A zero radius is allowed at either endpoint. Positive-radius ends stay open.
// Throws std::invalid_argument for invalid geometry or std::length_error if too large.
Mesh generateVaseMesh(const std::vector<ProfilePoint>& profile, int sliceCount);

}  // namespace vase

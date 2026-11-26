#include "vase_mesh.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace vase {
namespace {

struct ProfileNormal {
    double radial;
    double vertical;
};

ProfileNormal unitProfileNormal(double radial, double vertical) {
    const double scale = std::max(std::abs(radial), std::abs(vertical));
    const double length = std::hypot(radial / scale, vertical / scale);
    return {radial / scale / length, vertical / scale / length};
}

ProfileNormal segmentNormal(const ProfilePoint& from, const ProfilePoint& to) {
    const double dr = to.r - from.r;
    const double dy = to.y - from.y;
    if (dr == 0.0 && dy == 0.0) {
        throw std::invalid_argument("Consecutive profile points must be distinct.");
    }
    if (!std::isfinite(dy)) {
        return unitProfileNormal(to.y / 2.0 - from.y / 2.0, -dr / 2.0);
    }
    return unitProfileNormal(dy, -dr);
}

}  // namespace

Mesh generateVaseMesh(const std::vector<ProfilePoint>& profile, int sliceCount) {
    if (profile.size() < 2) {
        throw std::invalid_argument("The profile must contain at least two points.");
    }
    if (sliceCount < 3) {
        throw std::invalid_argument("The slice count must be at least three.");
    }
    for (std::size_t i = 0; i < profile.size(); ++i) {
        const auto& point = profile[i];
        if (!std::isfinite(point.r) || !std::isfinite(point.y) || point.r < 0.0) {
            throw std::invalid_argument("Profile coordinates must be finite and radii nonnegative.");
        }
        if (point.r == 0.0 && i != 0 && i + 1 != profile.size()) {
            throw std::invalid_argument("Only the first and last profile points may lie on the axis.");
        }
    }
    if (profile.back().y < profile.front().y) {
        throw std::invalid_argument("The mouth must be at or above the base.");
    }

    const bool firstOnAxis = profile.front().r == 0.0;
    const bool lastOnAxis = profile.back().r == 0.0;
    const std::size_t poleCount = static_cast<std::size_t>(firstOnAxis) + lastOnAxis;
    const std::size_t ringCount = profile.size() - poleCount;
    if (ringCount == 0) {
        throw std::invalid_argument("The profile must contain a positive radius.");
    }

    const std::size_t slices = static_cast<std::size_t>(sliceCount);
    const std::size_t trianglesPerSlice = 2 * (ringCount - 1) + poleCount;
    Mesh mesh;
    const auto vertexLimit = std::min(mesh.positions.max_size(),
        static_cast<std::size_t>(std::numeric_limits<TriangleIndex>::max()));
    if (ringCount > (vertexLimit - poleCount) / slices ||
        trianglesPerSlice > mesh.indices.max_size() / 3 / slices) {
        throw std::length_error("The generated mesh exceeds the supported vertex or index count.");
    }

    const std::size_t vertexCount = ringCount * slices + poleCount;
    mesh.positions.reserve(vertexCount);
    mesh.normals.reserve(vertexCount);
    mesh.indices.reserve(trianglesPerSlice * slices * 3);

    const double angleStep = 2.0 * std::acos(-1.0) / sliceCount;
    struct CirclePoint {
        double x;
        double z;
    };
    std::vector<CirclePoint> circle;
    circle.reserve(slices);
    for (std::size_t j = 0; j < slices; ++j) {
        const double angle = angleStep * static_cast<double>(j);
        circle.push_back({std::cos(angle), std::sin(angle)});
    }

    TriangleIndex previousStart = 0;
    ProfileNormal incoming{};
    for (std::size_t i = 0; i < profile.size(); ++i) {
        const auto& point = profile[i];
        const bool first = i == 0;
        const bool last = i + 1 == profile.size();
        const bool onAxis = point.r == 0.0;
        const auto start = static_cast<TriangleIndex>(mesh.positions.size());
        const ProfileNormal outgoing = last ? incoming : segmentNormal(point, profile[i + 1]);
        if (onAxis) {
            mesh.positions.push_back({0.0, point.y, 0.0});
            mesh.normals.push_back({0.0, first ? -1.0 : 1.0, 0.0});
        } else {
            ProfileNormal normal = outgoing;
            if (!first && !last) {
                const double radial = incoming.radial + outgoing.radial;
                const double vertical = incoming.vertical + outgoing.vertical;
                if (radial == 0.0 && vertical == 0.0) {
                    throw std::invalid_argument("The profile cannot reverse direction at a point.");
                }
                normal = unitProfileNormal(radial, vertical);
            }
            for (const auto& around : circle) {
                mesh.positions.push_back({point.r * around.x, point.y, point.r * around.z});
                mesh.normals.push_back({normal.radial * around.x, normal.vertical,
                    normal.radial * around.z});
            }
        }

        if (!first) {
            const bool previousOnAxis = profile[i - 1].r == 0.0;
            for (TriangleIndex j = 0; j < slices; ++j) {
                const TriangleIndex next = (j + 1) % static_cast<TriangleIndex>(slices);
                // Split the quad along the diagonal from upper[j] to lower[next].
                if (!previousOnAxis) {
                    mesh.indices.insert(mesh.indices.end(),
                        {previousStart + j, onAxis ? start : start + j, previousStart + next});
                }
                if (!onAxis) {
                    mesh.indices.insert(mesh.indices.end(),
                        {previousOnAxis ? previousStart : previousStart + next, start + j, start + next});
                }
            }
        }
        incoming = outgoing;
        previousStart = start;
    }
    return mesh;
}

}  // namespace vase

#pragma once
#include "vase_mesh.hpp"
#include <array>
#include <string>
namespace vase {
class BezierProfile {
public:
    std::array<ProfilePoint, 4> controls{{{0.35,0.0},{1.8,0.8},{0.25,2.2},{0.8,3.0}}};
    void moveControl(int index, double r, double y);
    ProfilePoint evaluate(double t) const;
    std::vector<ProfilePoint> sample(int count = 101) const;
};
void writeProfileCsv(const std::string& path, const std::vector<ProfilePoint>& points);
}

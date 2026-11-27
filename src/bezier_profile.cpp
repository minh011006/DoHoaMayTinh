#include "bezier_profile.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <stdexcept>
namespace vase {
void BezierProfile::moveControl(int index, double r, double y) {
    if (index < 0 || index > 3 || !std::isfinite(r) || !std::isfinite(y))
        throw std::invalid_argument("Invalid control point");
    const double low = index == 0 ? 0.0 : controls[index-1].y;
    const double high = index == 3 ? 4.0 : controls[index+1].y;
    // Keep the profile open and prevent negative radii.
    controls[index] = {std::clamp(r, 0.05, 2.5), std::clamp(y, low, high)};
    // Keep a nonzero height between base and mouth.
    if (index == 0) controls[0].y = std::min(controls[0].y, controls[3].y - 0.05);
    if (index == 3) controls[3].y = std::max(controls[3].y, controls[0].y + 0.05);
}
ProfilePoint BezierProfile::evaluate(double t) const {
    if (!std::isfinite(t) || t < 0 || t > 1) throw std::invalid_argument("t outside [0,1]");
    auto p = controls;
    for (int level=3; level>0; --level)
        for (int i=0; i<level; ++i)
            p[i] = {(1-t)*p[i].r+t*p[i+1].r, (1-t)*p[i].y+t*p[i+1].y};
    return p[0];
}
std::vector<ProfilePoint> BezierProfile::sample(int count) const {
    if (count < 2) throw std::invalid_argument("Need at least two samples");
    std::vector<ProfilePoint> result;
    result.reserve(count);
    for (int i=0; i<count; ++i) result.push_back(evaluate(double(i)/(count-1)));
    return result;
}
void writeProfileCsv(const std::string& path, const std::vector<ProfilePoint>& points) {
    std::ofstream out(path, std::ios::trunc);
    out << std::setprecision(17) << "r,y\n";
    for (const auto& p : points) out << p.r << ',' << p.y << '\n';
    out.close();
    if (!out) throw std::runtime_error("Cannot write CSV: " + path);
}
}

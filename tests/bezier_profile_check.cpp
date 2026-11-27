#include "bezier_profile.hpp"
#include <cmath>
#include <fstream>
#include <iostream>
#include <stdexcept>
void require(bool b) { if(!b)throw std::runtime_error("Profile check failed"); }
int main() {
    vase::BezierProfile p;
    auto s=p.sample();require(s.size()==101);require(s.front().r==p.controls[0].r);require(s.back().y==p.controls[3].y);
    auto mid=p.evaluate(.5);
    require(std::abs(mid.r-(p.controls[0].r+3*p.controls[1].r+3*p.controls[2].r+p.controls[3].r)/8)<1e-12);
    for(int k=0;k<200;++k) {
        p.moveControl(k%4, (k%9-2)*.5, (k%13-3)*.5);
        s=p.sample();
        for(size_t i=1;i<s.size();++i) require(s[i].y>=s[i-1].y && s[i].r>=0);
        auto mesh=vase::generateVaseMesh(s,48);require(!mesh.indices.empty());
    }
    vase::writeProfileCsv("profile_test.csv",s);
    std::ifstream in("profile_test.csv");std::string line;int count=0;
    std::getline(in,line);require(line=="r,y");while(std::getline(in,line))++count;require(count==101);
    std::cout<<"PASS: Bezier formula, endpoints, 200 constrained drags, mesh integration, CSV\n";
}

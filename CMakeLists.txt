cmake_minimum_required(VERSION 3.16)
project(DoHoaMayTinh LANGUAGES CXX)

add_library(vase_mesh STATIC src/vase_mesh.cpp)
target_include_directories(vase_mesh PUBLIC include)
target_compile_features(vase_mesh PUBLIC cxx_std_17)

include(CTest)
if(BUILD_TESTING)
    add_executable(vase_mesh_check tests/vase_mesh_check.cpp)
    target_link_libraries(vase_mesh_check PRIVATE vase_mesh)
    add_test(NAME vase_mesh_check COMMAND vase_mesh_check)
endif()

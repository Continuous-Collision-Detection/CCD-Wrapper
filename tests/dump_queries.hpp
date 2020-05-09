#pragma once

#include <ECCD.hpp>
#include <doubleCCD/double_subfunctions.h>
#include <fmt/format.h>
#include <highfive/H5Easy.hpp>

inline Eigen::Matrix<double, 8, 3> construct_points(
    const Eigen::Vector3d& x0,
    const Eigen::Vector3d& x1,
    const Eigen::Vector3d& x2,
    const Eigen::Vector3d& x3,
    const Eigen::Vector3d& x4,
    const Eigen::Vector3d& x5,
    const Eigen::Vector3d& x6,
    const Eigen::Vector3d& x7)
{
    Eigen::Matrix<double, 8, 3> points;
    points.row(0) = x0;
    points.row(1) = x1;
    points.row(2) = x2;
    points.row(3) = x3;
    points.row(4) = x4;
    points.row(5) = x5;
    points.row(6) = x6;
    points.row(7) = x7;
    return points;
}

void dump_vf_query(
    const std::string& name,
    const Eigen::Vector3d& vertex_start,
    const Eigen::Vector3d& face_vertex0_start,
    const Eigen::Vector3d& face_vertex1_start,
    const Eigen::Vector3d& face_vertex2_start,
    const Eigen::Vector3d& vertex_end,
    const Eigen::Vector3d& face_vertex0_end,
    const Eigen::Vector3d& face_vertex1_end,
    const Eigen::Vector3d& face_vertex2_end,
    bool expected_result)
{
    HighFive::File file(
        fmt::format("vertex-face-collisions.hdf5"),
        HighFive::File::OpenOrCreate);

    Eigen::Matrix<double, 8, 3> points = construct_points(
        vertex_start, face_vertex0_start, face_vertex1_start,
        face_vertex2_start, vertex_end, face_vertex0_end, face_vertex1_end,
        face_vertex2_end);
    H5Easy::dump(
        file, fmt::format("/{}/points", name), points,
        H5Easy::DumpMode::Overwrite);
    H5Easy::dump(
        file, fmt::format("/{}/result", name), (unsigned char)expected_result,
        H5Easy::DumpMode::Overwrite);

    doubleccd::vf_pair input_vf_pair(
        vertex_start, face_vertex0_start, face_vertex1_start,
        face_vertex2_start, vertex_end, face_vertex0_end, face_vertex1_end,
        face_vertex2_end);
    doubleccd::vf_pair shifted_vf_pair;
    double shift_err
        = doubleccd::shift_vertex_face(input_vf_pair, shifted_vf_pair);
    if (shift_err != 0) {
        std::cout << "---------------------------------------------------------"
                     "--------------------------------->"
                  << name << std::endl;
    }
    H5Easy::dump(
        file, fmt::format("/{}/shifted/error", name), shift_err,
        H5Easy::DumpMode::Overwrite);
    points = construct_points(
        shifted_vf_pair.x0, shifted_vf_pair.x1, shifted_vf_pair.x2,
        shifted_vf_pair.x3, shifted_vf_pair.x0b, shifted_vf_pair.x1b,
        shifted_vf_pair.x2b, shifted_vf_pair.x3b);
    H5Easy::dump(
        file, fmt::format("/{}/shifted/points", name), points,
        H5Easy::DumpMode::Overwrite);
    expected_result = eccd::vertexFaceCCD(
        shifted_vf_pair.x0, shifted_vf_pair.x1, shifted_vf_pair.x2,
        shifted_vf_pair.x3, shifted_vf_pair.x0b, shifted_vf_pair.x1b,
        shifted_vf_pair.x2b, shifted_vf_pair.x3b);
    H5Easy::dump(
        file, fmt::format("/{}/shifted/result", name),
        (unsigned char)expected_result, H5Easy::DumpMode::Overwrite);
}

void dump_ee_query(
    const std::string& name,
    const Eigen::Vector3d& edge0_vertex0_start,
    const Eigen::Vector3d& edge0_vertex1_start,
    const Eigen::Vector3d& edge1_vertex0_start,
    const Eigen::Vector3d& edge1_vertex1_start,
    const Eigen::Vector3d& edge0_vertex0_end,
    const Eigen::Vector3d& edge0_vertex1_end,
    const Eigen::Vector3d& edge1_vertex0_end,
    const Eigen::Vector3d& edge1_vertex1_end,
    bool expected_result)
{
    HighFive::File file(
        fmt::format("edge-edge-collisions.hdf5"), HighFive::File::OpenOrCreate);
    Eigen::Matrix<double, 8, 3> points = construct_points(
        edge0_vertex0_start, edge0_vertex1_start, edge1_vertex0_start,
        edge1_vertex1_start, edge0_vertex0_end, edge0_vertex1_end,
        edge1_vertex0_end, edge1_vertex1_end);
    H5Easy::dump(
        file, fmt::format("/{}/points", name), points,
        H5Easy::DumpMode::Overwrite);
    H5Easy::dump(
        file, fmt::format("/{}/result", name), (unsigned char)expected_result,
        H5Easy::DumpMode::Overwrite);

    doubleccd::ee_pair input_ee_pair(
        edge0_vertex0_start, edge0_vertex1_start, edge1_vertex0_start,
        edge1_vertex1_start, edge0_vertex0_end, edge0_vertex1_end,
        edge1_vertex0_end, edge1_vertex1_end);
    doubleccd::ee_pair shifted_ee_pair;
    double shift_err
        = doubleccd::shift_edge_edge(input_ee_pair, shifted_ee_pair);
    if (shift_err != 0) {
        std::cout << "---------------------------------------------------------"
                     "--------------------------------->"
                  << name << std::endl;
    }
    H5Easy::dump(
        file, fmt::format("/{}/shifted/error", name), shift_err,
        H5Easy::DumpMode::Overwrite);
    points = construct_points(
        shifted_ee_pair.a0, shifted_ee_pair.a1, shifted_ee_pair.b0,
        shifted_ee_pair.b1, shifted_ee_pair.a0b, shifted_ee_pair.a1b,
        shifted_ee_pair.b0b, shifted_ee_pair.b1b);
    H5Easy::dump(
        file, fmt::format("/{}/shifted/points", name), points,
        H5Easy::DumpMode::Overwrite);
    expected_result = eccd::edgeEdgeCCD(
        shifted_ee_pair.a0, shifted_ee_pair.a1, shifted_ee_pair.b0,
        shifted_ee_pair.b1, shifted_ee_pair.a0b, shifted_ee_pair.a1b,
        shifted_ee_pair.b0b, shifted_ee_pair.b1b);
    H5Easy::dump(
        file, fmt::format("/{}/shifted/result", name),
        (unsigned char)expected_result, H5Easy::DumpMode::Overwrite);
}

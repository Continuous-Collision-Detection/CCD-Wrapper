#pragma once

#include <Eigen/Core>

namespace msccd {
namespace root_finder {

    bool vertexFaceMSCCD(
        const Eigen::Vector3d& vertex_start,
        const Eigen::Vector3d& face_vertex0_start,
        const Eigen::Vector3d& face_vertex1_start,
        const Eigen::Vector3d& face_vertex2_start,
        const Eigen::Vector3d& vertex_end,
        const Eigen::Vector3d& face_vertex0_end,
        const Eigen::Vector3d& face_vertex1_end,
        const Eigen::Vector3d& face_vertex2_end,
        double min_distance,
        double& toi);

    bool edgeEdgeMSCCD(
        const Eigen::Vector3d& edge0_vertex0_start,
        const Eigen::Vector3d& edge0_vertex1_start,
        const Eigen::Vector3d& edge1_vertex0_start,
        const Eigen::Vector3d& edge1_vertex1_start,
        const Eigen::Vector3d& edge0_vertex0_end,
        const Eigen::Vector3d& edge0_vertex1_end,
        const Eigen::Vector3d& edge1_vertex0_end,
        const Eigen::Vector3d& edge1_vertex1_end,
        double min_distance,
        double& toi);

    ///////////////////////////////////////////////////////////////////////////
    // Helper functions

    int areProximate(
        const Eigen::Vector3d& x0,
        const Eigen::Vector3d& x1,
        const Eigen::Vector3d& x2,
        const Eigen::Vector3d& x3,
        const Eigen::Vector3d& v0,
        const Eigen::Vector3d& v1,
        const Eigen::Vector3d& v2,
        const Eigen::Vector3d& v3,
        double h,
        double* dt);

    int arePointPointIntersecting(
        const Eigen::Vector3d& x0,
        const Eigen::Vector3d& x1,
        const Eigen::Vector3d& v0,
        const Eigen::Vector3d& v1,
        double h,
        double* dt);

    int arePointLineIntersecting(
        const Eigen::Vector3d& x0,
        const Eigen::Vector3d& x1,
        const Eigen::Vector3d& x2,
        const Eigen::Vector3d& v0,
        const Eigen::Vector3d& v1,
        const Eigen::Vector3d& v2,
        double h,
        double* dt);

    double vertexFaceDist(
        const Eigen::Vector3d& p,
        const Eigen::Vector3d& a,
        const Eigen::Vector3d& b,
        const Eigen::Vector3d& c,
        double& t1,
        double& t2,
        double& t3);

    int areEEProximate(
        const Eigen::Vector3d& x0,
        const Eigen::Vector3d& x1,
        const Eigen::Vector3d& x2,
        const Eigen::Vector3d& x3,
        const Eigen::Vector3d& v0,
        const Eigen::Vector3d& v1,
        const Eigen::Vector3d& v2,
        const Eigen::Vector3d& v3,
        double h,
        double* dt);

    double edgeEdgeDist(
        const Eigen::Vector3d& p1,
        const Eigen::Vector3d& q1,
        const Eigen::Vector3d& p2,
        const Eigen::Vector3d& q2,
        double& s,
        double& t);

    int areEdgeEdgeIntersecting(
        const Eigen::Vector3d& x0,
        const Eigen::Vector3d& x1,
        const Eigen::Vector3d& x2,
        const Eigen::Vector3d& x3,
        const Eigen::Vector3d& v0,
        const Eigen::Vector3d& v1,
        const Eigen::Vector3d& v2,
        const Eigen::Vector3d& v3,
        double h,
        double* dt);

    int areLineLineIntersecting(
        const Eigen::Vector3d& x0,
        const Eigen::Vector3d& x1,
        const Eigen::Vector3d& x2,
        const Eigen::Vector3d& x3,
        const Eigen::Vector3d& v0,
        const Eigen::Vector3d& v1,
        const Eigen::Vector3d& v2,
        const Eigen::Vector3d& v3,
        double h,
        double* dt);

} // namespace root_finder
} // namespace msccd

// Etienne Vouga's CCD using a root finder in floating points

#include "floating_point_root_finder_ccd.hpp"

#if CCD_WRAPPER_WITH_FPRF

#include <CTCD.h>

bool FloatingPointRootFinderCCD::point_triangle_ccd(
    const Eigen::Vector3d& p_t0,
    const Eigen::Vector3d& t0_t0,
    const Eigen::Vector3d& t1_t0,
    const Eigen::Vector3d& t2_t0,
    const Eigen::Vector3d& p_t1,
    const Eigen::Vector3d& t0_t0,
    const Eigen::Vector3d& t1_t0,
    const Eigen::Vector3d& t2_t0) const override
{
    double toi;
    return point_triangle_ccd(
        p_t0, t0_t0, t1_t0, t2_t0, p_t1, t0_t0, t1_t0, t2_t0, toi);
}

bool FloatingPointRootFinderCCD::point_triangle_ccd(
    const Eigen::Vector3d& p_t0,
    const Eigen::Vector3d& t0_t0,
    const Eigen::Vector3d& t1_t0,
    const Eigen::Vector3d& t2_t0,
    const Eigen::Vector3d& p_t1,
    const Eigen::Vector3d& t0_t0,
    const Eigen::Vector3d& t1_t0,
    const Eigen::Vector3d& t2_t0,
    double& toi) const override
{
    return point_triangle_ccd(
        p_t0, t0_t0, t1_t0, t2_t0, p_t1, t0_t0, t1_t0, t2_t0, /*eta=*/0, toi);
}

bool FloatingPointRootFinderCCD::point_triangle_ccd(
    const Eigen::Vector3d& p_t0,
    const Eigen::Vector3d& t0_t0,
    const Eigen::Vector3d& t1_t0,
    const Eigen::Vector3d& t2_t0,
    const Eigen::Vector3d& p_t1,
    const Eigen::Vector3d& t0_t0,
    const Eigen::Vector3d& t1_t0,
    const Eigen::Vector3d& t2_t0,
    const double minimum_separation,
    double& toi) const override
{
    return CTCD::vertexFaceCTCD(
        p_t0, t0_t0, t1_t0, t2_t0, p_t1, t0_t0, t1_t0, t2_t0,
        minimum_separation, toi);
}

bool FloatingPointRootFinderCCD::edge_edge_ccd(
    const Eigen::Vector3d& ea0_t0,
    const Eigen::Vector3d& ea1_t0,
    const Eigen::Vector3d& eb0_t0,
    const Eigen::Vector3d& eb1_t0,
    const Eigen::Vector3d& ea0_t1,
    const Eigen::Vector3d& ea1_t1,
    const Eigen::Vector3d& eb0_t1,
    const Eigen::Vector3d& eb1_t1) const override
{
    double toi;
    return edge_edge_ccd(
        ea0_t0, ea1_t0, eb0_t0, eb1_t0, ea0_t1, ea1_t1, eb0_t1, eb1_t1, toi);
}

bool FloatingPointRootFinderCCD::edge_edge_ccd(
    const Eigen::Vector3d& ea0_t0,
    const Eigen::Vector3d& ea1_t0,
    const Eigen::Vector3d& eb0_t0,
    const Eigen::Vector3d& eb1_t0,
    const Eigen::Vector3d& ea0_t1,
    const Eigen::Vector3d& ea1_t1,
    const Eigen::Vector3d& eb0_t1,
    const Eigen::Vector3d& eb1_t1,
    double& toi) const override
{
    return edge_edge_ccd(
        ea0_t0, ea1_t0, eb0_t0, eb1_t0, ea0_t1, ea1_t1, eb0_t1, eb1_t1,
        /*minimum_separation=*/0, toi);
}

bool FloatingPointRootFinderCCD::edge_edge_ccd(
    const Eigen::Vector3d& ea0_t0,
    const Eigen::Vector3d& ea1_t0,
    const Eigen::Vector3d& eb0_t0,
    const Eigen::Vector3d& eb1_t0,
    const Eigen::Vector3d& ea0_t1,
    const Eigen::Vector3d& ea1_t1,
    const Eigen::Vector3d& eb0_t1,
    const Eigen::Vector3d& eb1_t1,
    const double minimum_separation,
    double& toi) const override
{
    return CTCD::edgeEdgeCTCD(
        ea0_t0, ea1_t0, eb0_t0, eb1_t0, ea0_t1, ea1_t1, eb0_t1, eb1_t1,
        minimum_separation, toi);
}

#endif

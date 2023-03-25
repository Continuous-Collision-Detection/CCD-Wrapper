// Teseo's reimplementation of Brochu et al. [2012] using rationals
#include "rational_root_parity_ccd.hpp"

#if CCD_WRAPPER_WITH_RRP

#include <ECCD.hpp>

bool RootParityCCD::point_triangle_ccd(
    const Eigen::Vector3d& p_t0,
    const Eigen::Vector3d& t0_t0,
    const Eigen::Vector3d& t1_t0,
    const Eigen::Vector3d& t2_t0,
    const Eigen::Vector3d& p_t1,
    const Eigen::Vector3d& t0_t0,
    const Eigen::Vector3d& t1_t0,
    const Eigen::Vector3d& t2_t0) const override
{
    return eccd::vertexFaceCCD(
        p_t0, t0_t0, t1_t0, t2_t0, p_t1, t0_t0, t1_t0, t2_t0);
}

bool RootParityCCD::edge_edge_ccd(
    const Eigen::Vector3d& ea0_t0,
    const Eigen::Vector3d& ea1_t0,
    const Eigen::Vector3d& eb0_t0,
    const Eigen::Vector3d& eb1_t0,
    const Eigen::Vector3d& ea0_t1,
    const Eigen::Vector3d& ea1_t1,
    const Eigen::Vector3d& eb0_t1,
    const Eigen::Vector3d& eb1_t1) const override
{
    return eccd::edgeEdgeCCD(
        ea0_t0, ea1_t0, eb0_t0, eb1_t0, ea0_t1, ea1_t1, eb0_t1, eb1_t1);
}

#endif

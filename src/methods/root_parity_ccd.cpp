// Root parity method of Brochu et al. [2012]
#include "root_parity_ccd.hpp"

#if CCD_WRAPPER_WITH_RP

#include <rootparitycollisiontest.h>

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
    return rootparity::RootParityCollisionTest(
               Vec3d(p_t0.data()), Vec3d(t0_t0.data()), Vec3d(t1_t0.data()),
               Vec3d(t2_t0.data()), Vec3d(p_t1.data()), Vec3d(t0_t0.data()),
               Vec3d(t1_t0.data()), Vec3d(t2_t0.data()),
               /*is_edge_edge=*/false)
        .run_test();
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
    return rootparity::RootParityCollisionTest(
               Vec3d(ea0_t0.data()), Vec3d(ea1_t0.data()), Vec3d(eb0_t0.data()),
               Vec3d(eb1_t0.data()), Vec3d(ea0_t1.data()), Vec3d(ea1_t1.data()),
               Vec3d(eb0_t1.data()), Vec3d(eb1_t1.data()),
               /*is_edge_edge=*/true)
        .run_test();
}

#endif

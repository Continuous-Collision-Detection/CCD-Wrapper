// Eigen wrappers for different CCD methods

#include "ccd.hpp"

// Ettien Vouga's CCD using a root finder in floating points
#include <CTCD.h>
// Root parity method of Brochu et al. [2012]
#include <rootparitycollisiontest.h>
// Teseo's reimplementation of Brochu et al. [2012] using rationals
#include <ECCD.hpp>
// Bernstein sign classification method of Tang et al. [2014]
#include <bsc.h>
// TightCCD method of Wang et al. [2015]
#include <bsc_tightbound.h>

namespace ccd {

// Detect collisions between a vertex and a triangular face.
bool vertexFaceCCD(
    const Eigen::Vector3d& vertex_start,
    const Eigen::Vector3d& face_vertex0_start,
    const Eigen::Vector3d& face_vertex1_start,
    const Eigen::Vector3d& face_vertex2_start,
    const Eigen::Vector3d& vertex_end,
    const Eigen::Vector3d& face_vertex0_end,
    const Eigen::Vector3d& face_vertex1_end,
    const Eigen::Vector3d& face_vertex2_end,
    const CCDMethod method)
{
    switch (method) {
    case CCDMethod::FLOAT:
        double toi;
        return CTCD::vertexFaceCTCD(
            // Point at t=0
            vertex_start,
            // Triangle at t = 0
            face_vertex0_start, face_vertex1_start, face_vertex2_start,
            // Point at t=1
            vertex_end,
            // Triangle at t = 1
            face_vertex0_end, face_vertex1_end, face_vertex2_end,
            /*eta=*/0, toi);
    case CCDMethod::ROOT_PARITY:
        return rootparity::RootParityCollisionTest(
                   // Point at t=0
                   Vec3d(vertex_start.data()),
                   // Triangle at t = 0
                   Vec3d(face_vertex1_start.data()),
                   Vec3d(face_vertex0_start.data()),
                   Vec3d(face_vertex2_start.data()),
                   // Point at t=1
                   Vec3d(vertex_end.data()),
                   // Triangle at t = 1
                   Vec3d(face_vertex1_end.data()),
                   Vec3d(face_vertex0_end.data()),
                   Vec3d(face_vertex2_end.data()),
                   /* is_edge_edge = */ false)
            .run_test();
    case CCDMethod::RATIONAL_ROOT_PARITY:
        return eccd::vertexFaceCCD(
            // Point at t=0
            vertex_start,
            // Triangle at t = 0
            face_vertex0_start, face_vertex1_start, face_vertex2_start,
            // Point at t=1
            vertex_end,
            // Triangle at t = 1
            face_vertex0_end, face_vertex1_end, face_vertex2_end);
    case CCDMethod::BSC:
        return bsc::Intersect_VF_robust(
            // Triangle at t = 0
            Vec3d(face_vertex0_start.data()), Vec3d(face_vertex1_start.data()),
            Vec3d(face_vertex2_start.data()),
            // Point at t=0
            Vec3d(vertex_start.data()),
            // Triangle at t = 1
            Vec3d(face_vertex0_end.data()), Vec3d(face_vertex1_end.data()),
            Vec3d(face_vertex2_end.data()),
            // Point at t=1
            Vec3d(vertex_end.data()));
    case CCDMethod::TIGHT_CCD:
        return bsc_tightbound::Intersect_VF_robust(
            // Triangle at t = 0
            Vec3d(face_vertex0_start.data()), Vec3d(face_vertex1_start.data()),
            Vec3d(face_vertex2_start.data()),
            // Point at t=0
            Vec3d(vertex_start.data()),
            // Triangle at t = 1
            Vec3d(face_vertex0_end.data()), Vec3d(face_vertex1_end.data()),
            Vec3d(face_vertex2_end.data()),
            // Point at t=1
            Vec3d(vertex_end.data()));
    default:
        return false;
    }
}

// Detect collisions between two edges as they move.
bool edgeEdgeCCD(
    const Eigen::Vector3d& edge0_vertex0_start,
    const Eigen::Vector3d& edge0_vertex1_start,
    const Eigen::Vector3d& edge1_vertex0_start,
    const Eigen::Vector3d& edge1_vertex1_start,
    const Eigen::Vector3d& edge0_vertex0_end,
    const Eigen::Vector3d& edge0_vertex1_end,
    const Eigen::Vector3d& edge1_vertex0_end,
    const Eigen::Vector3d& edge1_vertex1_end,
    const CCDMethod method)
{
    switch (method) {
    case CCDMethod::FLOAT:
        double toi;
        return CTCD::edgeEdgeCTCD(
            // Edge 1 at t=0
            edge0_vertex0_start, edge0_vertex1_start,
            // Edge 2 at t=0
            edge1_vertex0_start, edge1_vertex1_start,
            // Edge 1 at t=1
            edge0_vertex0_end, edge0_vertex1_end,
            // Edge 2 at t=1
            edge1_vertex0_end, edge1_vertex1_end,
            /*eta=*/0, toi);
    case CCDMethod::ROOT_PARITY:
        return rootparity::RootParityCollisionTest(
                   // Edge 1 at t=0
                   Vec3d(edge0_vertex0_start.data()),
                   Vec3d(edge0_vertex1_start.data()),
                   // Edge 2 at t=0
                   Vec3d(edge1_vertex0_start.data()),
                   Vec3d(edge1_vertex1_start.data()),
                   // Edge 1 at t=1
                   Vec3d(edge0_vertex0_end.data()),
                   Vec3d(edge0_vertex1_end.data()),
                   // Edge 2 at t=1
                   Vec3d(edge1_vertex0_end.data()),
                   Vec3d(edge1_vertex1_end.data()),
                   /* is_edge_edge = */ true)
            .run_test();
    case CCDMethod::RATIONAL_ROOT_PARITY:
        return eccd::edgeEdgeCCD(
            // Edge 1 at t=0
            edge0_vertex0_start, edge0_vertex1_start,
            // Edge 2 at t=0
            edge1_vertex0_start, edge1_vertex1_start,
            // Edge 1 at t=1
            edge0_vertex0_end, edge0_vertex1_end,
            // Edge 2 at t=1
            edge1_vertex0_end, edge1_vertex1_end);
    case CCDMethod::BSC:
        return bsc::Intersect_EE_robust(
            // Edge 1 at t=0
            Vec3d(edge0_vertex0_start.data()),
            Vec3d(edge0_vertex1_start.data()),
            // Edge 2 at t=0
            Vec3d(edge1_vertex0_start.data()),
            Vec3d(edge1_vertex1_start.data()),
            // Edge 1 at t=1
            Vec3d(edge0_vertex0_end.data()), Vec3d(edge0_vertex1_end.data()),
            // Edge 2 at t=1
            Vec3d(edge1_vertex0_end.data()), Vec3d(edge1_vertex1_end.data()));
    case CCDMethod::TIGHT_CCD:
        return bsc_tightbound::Intersect_EE_robust(
            // Edge 1 at t=0
            Vec3d(edge0_vertex0_start.data()),
            Vec3d(edge0_vertex1_start.data()),
            // Edge 2 at t=0
            Vec3d(edge1_vertex0_start.data()),
            Vec3d(edge1_vertex1_start.data()),
            // Edge 1 at t=1
            Vec3d(edge0_vertex0_end.data()), Vec3d(edge0_vertex1_end.data()),
            // Edge 2 at t=1
            Vec3d(edge1_vertex0_end.data()), Vec3d(edge1_vertex1_end.data()));
    default:
        return false;
    }
}

} // namespace ccd

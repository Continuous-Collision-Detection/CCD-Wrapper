// Eigen wrappers for different CCD methods
#include "ccd.hpp"

// Etienne Vouga's CCD using a root finder in floating points
#include <CTCD.h>
// Root parity method of Brochu et al. [2012]
#include <rootparitycollisiontest.h>
// Teseo's reimplementation of Brochu et al. [2012] using rationals
#include <ECCD.hpp>
// Bernstein sign classification method of Tang et al. [2014]
#include <bsc.h>
// TightCCD method of Wang et al. [2015]
#include <bsc_tightbound.h>
// Exact Minimum Separation CCD of Wang et al. [2020]
#include <CCD/ccd.hpp>
#include <doubleCCD/doubleccd.hpp>
#include <interval_ccd/interval_ccd.hpp>

#include <min_separation_ccd/min_separation_ccd.hpp>

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
    double toi; // Computed by some methods but never returned
    try {
        switch (method) {
        case CCDMethod::FLOAT:
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
                Vec3d(face_vertex0_start.data()),
                Vec3d(face_vertex1_start.data()),
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
                Vec3d(face_vertex0_start.data()),
                Vec3d(face_vertex1_start.data()),
                Vec3d(face_vertex2_start.data()),
                // Point at t=0
                Vec3d(vertex_start.data()),
                // Triangle at t = 1
                Vec3d(face_vertex0_end.data()), Vec3d(face_vertex1_end.data()),
                Vec3d(face_vertex2_end.data()),
                // Point at t=1
                Vec3d(vertex_end.data()));
        case CCDMethod::FLOAT_MIN_SEPARATION:
        case CCDMethod::EXACT_RATIONAL_MIN_SEPARATION:
        case CCDMethod::EXACT_DOUBLE_MIN_SEPARATION:
            return vertexFaceMSCCD(
                // Point at t=0
                vertex_start,
                // Triangle at t = 0
                face_vertex0_start, face_vertex1_start, face_vertex2_start,
                // Point at t=1
                vertex_end,
                // Triangle at t = 1
                face_vertex0_end, face_vertex1_end, face_vertex2_end,
                /*minimum_distance=*/DEFAULT_MIN_DISTANCE, method);
        default:
            throw "Invalid CCDMethod";
        }
    } catch (const char* err) {
        // Conservative answer upon failure.
        std::cerr << "Vertex-face CCD failed because \"" << err << "\" for "
                  << method_names[method] << std::endl;
        return true;
    } catch (...) {
        // Conservative answer upon failure.
        std::cerr << "Vertex-face CCD failed for unknown reason when using "
                  << method_names[method] << std::endl;
        return true;
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
    double toi; // Computed by some methods but never returned
    try {
        switch (method) {
        case CCDMethod::FLOAT:
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
                Vec3d(edge0_vertex0_end.data()),
                Vec3d(edge0_vertex1_end.data()),
                // Edge 2 at t=1
                Vec3d(edge1_vertex0_end.data()),
                Vec3d(edge1_vertex1_end.data()));
        case CCDMethod::TIGHT_CCD:
            return bsc_tightbound::Intersect_EE_robust(
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
                Vec3d(edge1_vertex1_end.data()));
        case CCDMethod::FLOAT_MIN_SEPARATION:
        case CCDMethod::EXACT_RATIONAL_MIN_SEPARATION:
        case CCDMethod::EXACT_DOUBLE_MIN_SEPARATION:
            return edgeEdgeMSCCD(
                // Edge 1 at t=0
                edge0_vertex0_start, edge0_vertex1_start,
                // Edge 2 at t=0
                edge1_vertex0_start, edge1_vertex1_start,
                // Edge 1 at t=1
                edge0_vertex0_end, edge0_vertex1_end,
                // Edge 2 at t=1
                edge1_vertex0_end, edge1_vertex1_end,
                /*minimum_distance=*/DEFAULT_MIN_DISTANCE, method);
        case CCDMethod::INTERVAL:
        return edgeEdgeInterval(// Edge 1 at t=0
                edge0_vertex0_start, edge0_vertex1_start,
                // Edge 2 at t=0
                edge1_vertex0_start, edge1_vertex1_start,
                // Edge 1 at t=1
                edge0_vertex0_end, edge0_vertex1_end,
                // Edge 2 at t=1
                edge1_vertex0_end, edge1_vertex1_end);

        default:
            throw "Invalid CCDMethod";
        }
    } catch (const char* err) {
        // Conservative answer upon failure.
        std::cerr << "Edge-edge CCD failed because \"" << err << "\" for "
                  << method_names[method] << std::endl;
        return true;
    } catch (...) {
        // Conservative answer upon failure.
        std::cerr << "Edge-edge CCD failed for unknown reason when using "
                  << method_names[method] << std::endl;
        return true;
    }
}

// Detect collisions between a vertex and a triangular face.
bool vertexFaceMSCCD(
    const Eigen::Vector3d& vertex_start,
    const Eigen::Vector3d& face_vertex0_start,
    const Eigen::Vector3d& face_vertex1_start,
    const Eigen::Vector3d& face_vertex2_start,
    const Eigen::Vector3d& vertex_end,
    const Eigen::Vector3d& face_vertex0_end,
    const Eigen::Vector3d& face_vertex1_end,
    const Eigen::Vector3d& face_vertex2_end,
    const double min_distance,
    const CCDMethod method)
{
    double toi; // Computed by some methods but never returned
    try {
        switch (method) {
        case CCDMethod::FLOAT_MIN_SEPARATION: {
            bool hit = msccd::root_finder::vertexFaceMSCCD(
                // Point at t=0
                vertex_start,
                // Triangle at t = 0
                face_vertex0_start, face_vertex1_start, face_vertex2_start,
                // Point at t=1
                vertex_end,
                // Triangle at t = 1
                face_vertex0_end, face_vertex1_end, face_vertex2_end,
                min_distance, toi);
            if (hit && (toi < 0 || toi > 1)) {
                std::cout << toi << std::endl;
                throw "toi out of range";
            }
            return hit;
        }
        case CCDMethod::EXACT_RATIONAL_MIN_SEPARATION:
            return ccd::vertexFaceCCD(
                // Point at t=0
                vertex_start,
                // Triangle at t = 0
                face_vertex0_start, face_vertex1_start, face_vertex2_start,
                // Point at t=1
                vertex_end,
                // Triangle at t = 1
                face_vertex0_end, face_vertex1_end, face_vertex2_end,
                min_distance);
        case CCDMethod::EXACT_DOUBLE_MIN_SEPARATION:
            return doubleccd::vertexFaceCCD(
                // Point at t=0
                vertex_start,
                // Triangle at t = 0
                face_vertex0_start, face_vertex1_start, face_vertex2_start,
                // Point at t=1
                vertex_end,
                // Triangle at t = 1
                face_vertex0_end, face_vertex1_end, face_vertex2_end,
                min_distance);
        default:
            throw "Invalid Minimum Separation CCDMethod";
        }
    } catch (const char* err) {
        // Conservative answer upon failure.
        std::cerr << "Vertex-face CCD failed because \"" << err << "\" for "
                  << method_names[method] << std::endl;
        return true;
    } catch (...) {
        // Conservative answer upon failure.
        std::cerr << "Vertex-face CCD failed for unknown reason when using "
                  << method_names[method] << std::endl;
        return true;
    }
}

// Detect collisions between two edges as they move.
bool edgeEdgeMSCCD(
    const Eigen::Vector3d& edge0_vertex0_start,
    const Eigen::Vector3d& edge0_vertex1_start,
    const Eigen::Vector3d& edge1_vertex0_start,
    const Eigen::Vector3d& edge1_vertex1_start,
    const Eigen::Vector3d& edge0_vertex0_end,
    const Eigen::Vector3d& edge0_vertex1_end,
    const Eigen::Vector3d& edge1_vertex0_end,
    const Eigen::Vector3d& edge1_vertex1_end,
    const double min_distance,
    const CCDMethod method)
{
    double toi; // Computed by some methods but never returned
    try {
        switch (method) {
        case CCDMethod::FLOAT_MIN_SEPARATION: {
            bool hit = msccd::root_finder::edgeEdgeMSCCD(
                // Edge 1 at t=0
                edge0_vertex0_start, edge0_vertex1_start,
                // Edge 2 at t=0
                edge1_vertex0_start, edge1_vertex1_start,
                // Edge 1 at t=1
                edge0_vertex0_end, edge0_vertex1_end,
                // Edge 2 at t=1
                edge1_vertex0_end, edge1_vertex1_end, min_distance, toi);
            if (hit && (toi < 0 || toi > 1)) {
                std::cout << toi << std::endl;
                throw "toi out of range";
            }
            return hit;
        }
        case CCDMethod::EXACT_RATIONAL_MIN_SEPARATION:
            return ccd::edgeEdgeCCD(
                // Edge 1 at t=0
                edge0_vertex0_start, edge0_vertex1_start,
                // Edge 2 at t=0
                edge1_vertex0_start, edge1_vertex1_start,
                // Edge 1 at t=1
                edge0_vertex0_end, edge0_vertex1_end,
                // Edge 2 at t=1
                edge1_vertex0_end, edge1_vertex1_end, min_distance);
        case CCDMethod::EXACT_DOUBLE_MIN_SEPARATION:
            return doubleccd::edgeEdgeCCD(
                // Edge 1 at t=0
                edge0_vertex0_start, edge0_vertex1_start,
                // Edge 2 at t=0
                edge1_vertex0_start, edge1_vertex1_start,
                // Edge 1 at t=1
                edge0_vertex0_end, edge0_vertex1_end,
                // Edge 2 at t=1
                edge1_vertex0_end, edge1_vertex1_end, min_distance);
        default:
            throw "Invalid Minimum Separation CCDMethod";
        }
    } catch (const char* err) {
        // Conservative answer upon failure.
        std::cerr << "Edge-edge CCD failed because \"" << err << "\" for "
                  << method_names[method] << std::endl;
        return true;
    } catch (...) {
        // Conservative answer upon failure.
        std::cerr << "Edge-edge CCD failed for unknown reason when using "
                  << method_names[method] << std::endl;
        return true;
    }
}
bool edgeEdgeInterval(
    const Eigen::Vector3d& edge0_vertex0_start,
    const Eigen::Vector3d& edge0_vertex1_start,
    const Eigen::Vector3d& edge1_vertex0_start,
    const Eigen::Vector3d& edge1_vertex1_start,
    const Eigen::Vector3d& edge0_vertex0_end,
    const Eigen::Vector3d& edge0_vertex1_end,
    const Eigen::Vector3d& edge1_vertex0_end,
    const Eigen::Vector3d& edge1_vertex1_end)
{
    double toi;// time of intersection
    return intervalccd::edgeEdgeCCD(
        edge0_vertex0_start,
        edge0_vertex1_start,
        edge1_vertex0_start,
        edge1_vertex1_start,
        edge0_vertex0_end,
        edge0_vertex1_end,
        edge1_vertex0_end,
        edge1_vertex1_end,toi);
    
}

} // namespace ccd

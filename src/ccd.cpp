// Eigen wrappers for different CCD methods
#include "ccd.hpp"

#include <iostream>

// Etienne Vouga's CCD using a root finder in floating points
#ifdef CCD_WRAPPER_WITH_FPRF
#include <CTCD.h>
#endif
// Root parity method of Brochu et al. [2012]
#ifdef CCD_WRAPPER_WITH_RP
#include <rootparitycollisiontest.h>
#endif
// Teseo's reimplementation of Brochu et al. [2012] using rationals
#ifdef CCD_WRAPPER_WITH_RRP
#include <ECCD.hpp>
#endif
// Bernstein sign classification method of Tang et al. [2014]
#ifdef CCD_WRAPPER_WITH_BSC
#include <bsc.h>
#endif
// TightCCD method of Wang et al. [2015]
#ifdef CCD_WRAPPER_WITH_TIGHT_CCD
#include <bsc_tightbound.h>
#endif
// SafeCCD
#ifdef ENABLE_SAFE_CCD
#include <SAFE_CCD.h>
#endif
// Minimum separation root parity
#ifdef CCD_WRAPPER_WITH_FIXEDRP
// Rational root parity with minimum separation and fixes
#include <CCD/ccd.hpp>
// Root parity with minimum separation and fixes
#include <doubleCCD/doubleccd.hpp>
#endif
// Minimum separation root finder of Harmon et al. [2011]
#ifdef CCD_WRAPPER_WITH_MSRF
#include <minimum_separation_root_finder.hpp>
#endif
// Interval based CCD of [Redon et al. 2002]
// Interval based CCD of [Redon et al. 2002] solved using [Snyder 1992]
#ifdef CCD_WRAPPER_WITH_INTERVAL
#include <interval_ccd/interval_ccd.hpp>
#endif
// Custom inclusion based CCD of [Wang et al. 2020]
#ifdef CCD_WRAPPER_WITH_TIGHT_INCLUSION
#include <tight_inclusion/inclusion_ccd.hpp>
#endif

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
    const CCDMethod method,
    const double tolerance,
    const long max_iter,
    const std::array<double, 3>& err)
{
    double toi; // Computed by some methods but never returned
    try {
        switch (method) {
        case CCDMethod::FLOATING_POINT_ROOT_FINDER:
#ifdef CCD_WRAPPER_WITH_FPRF
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
#else
            throw "CCD method is not enabled";
#endif
        case CCDMethod::MIN_SEPARATION_ROOT_FINDER:
#ifdef CCD_WRAPPER_WITH_MSRF
            return vertexFaceMSCCD(
                // Point at t=0
                vertex_start,
                // Triangle at t = 0
                face_vertex0_start, face_vertex1_start, face_vertex2_start,
                // Point at t=1
                vertex_end,
                // Triangle at t = 1
                face_vertex0_end, face_vertex1_end, face_vertex2_end,
                /*minimum_distance=*/DEFAULT_MIN_DISTANCE, method, tolerance,
                max_iter, err);
#else
            throw "CCD method is not enabled";
#endif
        case CCDMethod::ROOT_PARITY:
#ifdef CCD_WRAPPER_WITH_RP
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
#else
            throw "CCD method is not enabled";
#endif
        case CCDMethod::RATIONAL_ROOT_PARITY:
#ifdef CCD_WRAPPER_WITH_RRP
            return eccd::vertexFaceCCD(
                // Point at t=0
                vertex_start,
                // Triangle at t = 0
                face_vertex0_start, face_vertex1_start, face_vertex2_start,
                // Point at t=1
                vertex_end,
                // Triangle at t = 1
                face_vertex0_end, face_vertex1_end, face_vertex2_end);
#else
            throw "CCD method is not enabled";
#endif
        case CCDMethod::FIXED_ROOT_PARITY:
#ifdef CCD_WRAPPER_WITH_FIXEDRP
            return doubleccd::vertexFaceCCD(
                // Point at t=0
                vertex_start,
                // Triangle at t = 0
                face_vertex0_start, face_vertex1_start, face_vertex2_start,
                // Point at t=1
                vertex_end,
                // Triangle at t = 1
                face_vertex0_end, face_vertex1_end, face_vertex2_end);
#else
            throw "CCD method is not enabled";
#endif
        case CCDMethod::RATIONAL_FIXED_ROOT_PARITY:
#ifdef CCD_WRAPPER_WITH_FIXEDRP
            return ccd::vertexFaceCCD(
                // Point at t=0
                vertex_start,
                // Triangle at t = 0
                face_vertex0_start, face_vertex1_start, face_vertex2_start,
                // Point at t=1
                vertex_end,
                // Triangle at t = 1
                face_vertex0_end, face_vertex1_end, face_vertex2_end);
#else
            throw "CCD method is not enabled";
#endif
        case CCDMethod::TIGHT_INCLUSION:
            // Call the MSCCD function for these to remove duplicate code
            return vertexFaceMSCCD(
                // Point at t=0
                vertex_start,
                // Triangle at t = 0
                face_vertex0_start, face_vertex1_start, face_vertex2_start,
                // Point at t=1
                vertex_end,
                // Triangle at t = 1
                face_vertex0_end, face_vertex1_end, face_vertex2_end,
                /*minimum_distance=*/0, method, tolerance, max_iter, err);
        case CCDMethod::BSC:
#ifdef CCD_WRAPPER_WITH_BSC
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
#else
            throw "CCD method is not enabled";
#endif
        case CCDMethod::TIGHT_CCD:
#ifdef CCD_WRAPPER_WITH_TIGHT_CCD
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
#else
            throw "CCD method is not enabled";
#endif
        case CCDMethod::SAFE_CCD:
#ifdef CCD_WRAPPER_WITH_SAFE_CCD
        {
            double b = safeccd::calculate_B(
                vertex_start.data(), face_vertex0_start.data(),
                face_vertex1_start.data(), face_vertex2_start.data(),
                vertex_end.data(), face_vertex0_end.data(),
                face_vertex1_end.data(), face_vertex2_end.data(), false);
            safeccd::SAFE_CCD<double> safe;
            safe.Set_Coefficients(b);
            double t, u[3], v[3];
            double vs[3], ve[3], f0s[3], f0e[3], f1s[3], f1e[3], f2s[3], f2e[3];
            for (int i = 0; i < 3; i++) {
                vs[i] = vertex_start[i];
                ve[i] = vertex_end[i];
                f0s[i] = face_vertex0_start[i];
                f0e[i] = face_vertex0_end[i];
                f1s[i] = face_vertex1_start[i];
                f1e[i] = face_vertex1_end[i];
                f2s[i] = face_vertex2_start[i];
                f2e[i] = face_vertex2_end[i];
            }
            return safe.Vertex_Triangle_CCD(
                vs, ve, f0s, f0e, f1s, f1e, f2s, f2e, t, u, v);
        }
#else
            throw "CCD method is not enabled";
#endif
        case CCDMethod::UNIVARIATE_INTERVAL_ROOT_FINDER:
#ifdef CCD_WRAPPER_WITH_INTERVAL
            return intervalccd::vertexFaceCCD_Redon(
                // Point at t=0
                vertex_start,
                // Triangle at t = 0
                face_vertex0_start, face_vertex1_start, face_vertex2_start,
                // Point at t=1
                vertex_end,
                // Triangle at t = 1
                face_vertex0_end, face_vertex1_end, face_vertex2_end,
                // Time of impact
                toi, tolerance);
#else
            throw "CCD method is not enabled";
#endif
        case CCDMethod::MULTIVARIATE_INTERVAL_ROOT_FINDER:
#ifdef CCD_WRAPPER_WITH_INTERVAL
            return intervalccd::vertexFaceCCD_Interval(
                // Point at t=0
                vertex_start,
                // Triangle at t = 0
                face_vertex0_start, face_vertex1_start, face_vertex2_start,
                // Point at t=1
                vertex_end,
                // Triangle at t = 1
                face_vertex0_end, face_vertex1_end, face_vertex2_end,
                // Time of impact
                toi,tolerance);
#else
            throw "CCD method is not enabled";
#endif

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
    const CCDMethod method,
    const double tolerance,
    const long max_iter,
    const std::array<double, 3>& err)
{
    double toi; // Computed by some methods but never returned
    try {
        switch (method) {
        case CCDMethod::FLOATING_POINT_ROOT_FINDER:
#ifdef CCD_WRAPPER_WITH_FPRF
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
#else
            throw "CCD method is not enabled";
#endif
        case CCDMethod::MIN_SEPARATION_ROOT_FINDER:
#ifdef CCD_WRAPPER_WITH_MSRF
            return edgeEdgeMSCCD(
                // Edge 1 at t=0
                edge0_vertex0_start, edge0_vertex1_start,
                // Edge 2 at t=0
                edge1_vertex0_start, edge1_vertex1_start,
                // Edge 1 at t=1
                edge0_vertex0_end, edge0_vertex1_end,
                // Edge 2 at t=1
                edge1_vertex0_end, edge1_vertex1_end,
                /*minimum_distance=*/DEFAULT_MIN_DISTANCE, method, tolerance,
                max_iter, err);
#else
            throw "CCD method is not enabled";
#endif
        case CCDMethod::ROOT_PARITY:
#ifdef CCD_WRAPPER_WITH_RP
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
#else
            throw "CCD method is not enabled";
#endif
        case CCDMethod::RATIONAL_ROOT_PARITY:
#ifdef CCD_WRAPPER_WITH_RRP
            return eccd::edgeEdgeCCD(
                // Edge 1 at t=0
                edge0_vertex0_start, edge0_vertex1_start,
                // Edge 2 at t=0
                edge1_vertex0_start, edge1_vertex1_start,
                // Edge 1 at t=1
                edge0_vertex0_end, edge0_vertex1_end,
                // Edge 2 at t=1
                edge1_vertex0_end, edge1_vertex1_end);
#else
            throw "CCD method is not enabled";
#endif
case CCDMethod::FIXED_ROOT_PARITY:
#ifdef CCD_WRAPPER_WITH_FIXEDRP
            return doubleccd::edgeEdgeCCD(
                // Edge 1 at t=0
                edge0_vertex0_start, edge0_vertex1_start,
                // Edge 2 at t=0
                edge1_vertex0_start, edge1_vertex1_start,
                // Edge 1 at t=1
                edge0_vertex0_end, edge0_vertex1_end,
                // Edge 2 at t=1
                edge1_vertex0_end, edge1_vertex1_end);
#else
            throw "CCD method is not enabled";
#endif
        case CCDMethod::RATIONAL_FIXED_ROOT_PARITY:
#ifdef CCD_WRAPPER_WITH_FIXEDRP
            return ccd::edgeEdgeCCD(
                // Edge 1 at t=0
                edge0_vertex0_start, edge0_vertex1_start,
                // Edge 2 at t=0
                edge1_vertex0_start, edge1_vertex1_start,
                // Edge 1 at t=1
                edge0_vertex0_end, edge0_vertex1_end,
                // Edge 2 at t=1
                edge1_vertex0_end, edge1_vertex1_end);
#else
            throw "CCD method is not enabled";
#endif
        case CCDMethod::TIGHT_INCLUSION:
            // Call the MSCCD function for these to remove duplicate code
            return edgeEdgeMSCCD(
                // Edge 1 at t=0
                edge0_vertex0_start, edge0_vertex1_start,
                // Edge 2 at t=0
                edge1_vertex0_start, edge1_vertex1_start,
                // Edge 1 at t=1
                edge0_vertex0_end, edge0_vertex1_end,
                // Edge 2 at t=1
                edge1_vertex0_end, edge1_vertex1_end,
                /*minimum_distance=*/0, method, tolerance, max_iter, err);
        case CCDMethod::BSC:
#ifdef CCD_WRAPPER_WITH_BSC
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
#else
            throw "CCD method is not enabled";
#endif
        case CCDMethod::TIGHT_CCD:
#ifdef CCD_WRAPPER_WITH_TIGHT_CCD
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
#else
            throw "CCD method is not enabled";
#endif
        case CCDMethod::SAFE_CCD:
#ifdef CCD_WRAPPER_WITH_SAFE_CCD
        {
            double b = safeccd::calculate_B(
                edge0_vertex0_start.data(), edge0_vertex1_start.data(),
                edge1_vertex0_start.data(), edge1_vertex1_start.data(),
                edge0_vertex0_end.data(), edge0_vertex1_end.data(),
                edge1_vertex0_end.data(), edge1_vertex1_end.data(), true);
            safeccd::SAFE_CCD<double> safe;
            safe.Set_Coefficients(b);
            double t, u[3], v[3];
            double vs[3], ve[3], f0s[3], f0e[3], f1s[3], f1e[3], f2s[3], f2e[3];
            for (int i = 0; i < 3; i++) {
                vs[i] = edge0_vertex0_start[i];
                ve[i] = edge0_vertex0_end[i];
                f0s[i] = edge0_vertex1_start[i];
                f0e[i] = edge0_vertex1_end[i];
                f1s[i] = edge1_vertex0_start[i];
                f1e[i] = edge1_vertex0_end[i];
                f2s[i] = edge1_vertex1_start[i];
                f2e[i] = edge1_vertex1_end[i];
            }
            return safe.Edge_Edge_CCD(
                vs, ve, f0s, f0e, f1s, f1e, f2s, f2e, t, u, v);
        }
#else
            throw "CCD method is not enabled";
#endif
        case CCDMethod::UNIVARIATE_INTERVAL_ROOT_FINDER:
#ifdef CCD_WRAPPER_WITH_INTERVAL
            return intervalccd::edgeEdgeCCD_Redon(
                // Edge 1 at t=0
                edge0_vertex0_start, edge0_vertex1_start,
                // Edge 2 at t=0
                edge1_vertex0_start, edge1_vertex1_start,
                // Edge 1 at t=1
                edge0_vertex0_end, edge0_vertex1_end,
                // Edge 2 at t=1
                edge1_vertex0_end, edge1_vertex1_end,
                // Time of impact
                toi,tolerance);
#else
            throw "CCD method is not enabled";
#endif
        case CCDMethod::MULTIVARIATE_INTERVAL_ROOT_FINDER:
#ifdef CCD_WRAPPER_WITH_INTERVAL
            return intervalccd::edgeEdgeCCD_Interval(
                // Edge 1 at t=0
                edge0_vertex0_start, edge0_vertex1_start,
                // Edge 2 at t=0
                edge1_vertex0_start, edge1_vertex1_start,
                // Edge 1 at t=1
                edge0_vertex0_end, edge0_vertex1_end,
                // Edge 2 at t=1
                edge1_vertex0_end, edge1_vertex1_end,
                // Time of impact
                toi,tolerance);
#else
            throw "CCD method is not enabled";
#endif

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
    const CCDMethod method,
    const double tolerance,
    const long max_iter,
    const std::array<double, 3>& err)
{
    double toi; // Computed by some methods but never returned
    try {
        switch (method) {
        case CCDMethod::MIN_SEPARATION_ROOT_FINDER:
#ifdef CCD_WRAPPER_WITH_MSRF
        {
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
#else
            throw "CCD method is not enabled";
#endif
        
        case CCDMethod::TIGHT_INCLUSION:
#ifdef CCD_WRAPPER_WITH_TIGHT_INCLUSION
        {
            double output_tolerance;
            const double t_max = 1.0;
            // 0: normal ccd method which only checks t = [0,1]
            // 1: ccd with max_itr and t=[0, t_max]
            const int CCD_TYPE = 1;
            return inclusion_ccd::vertexFaceCCD_double(
                // Point at t=0
                vertex_start,
                // Triangle at t = 0
                face_vertex0_start, face_vertex1_start, face_vertex2_start,
                // Point at t=1
                vertex_end,
                // Triangle at t = 1
                face_vertex0_end, face_vertex1_end, face_vertex2_end,
                err,              // rounding error
                min_distance,     // minimum separation distance
                toi,              // time of impact
                tolerance,        // delta
                t_max,            // Maximum time to check
                max_iter,         // Maximum number of iterations
                output_tolerance, // delta_actual
                CCD_TYPE);
        }
#else
            throw "CCD method is not enabled";
#endif
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
    const CCDMethod method,
    const double tolerance,
    const long max_iter,
    const std::array<double, 3>& err)
{
    double toi; // Computed by some methods but never returned
    try {
        switch (method) {
        case CCDMethod::MIN_SEPARATION_ROOT_FINDER:
#ifdef CCD_WRAPPER_WITH_MSRF
        {
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
#else
            throw "CCD method is not enabled";
#endif
        
        case CCDMethod::TIGHT_INCLUSION:
#ifdef CCD_WRAPPER_WITH_TIGHT_INCLUSION
        {
            double output_tolerance;
            const double t_max = 1.0;
            // 0: normal ccd method which only checks t = [0,1]
            // 1: ccd with max_itr and t=[0, t_max]
            const int CCD_TYPE = 1;
            return inclusion_ccd::edgeEdgeCCD_double(
                // Edge 1 at t=0
                edge0_vertex0_start, edge0_vertex1_start,
                // Edge 2 at t=0
                edge1_vertex0_start, edge1_vertex1_start,
                // Edge 1 at t=1
                edge0_vertex0_end, edge0_vertex1_end,
                // Edge 2 at t=1
                edge1_vertex0_end, edge1_vertex1_end,
                err,              // rounding error
                min_distance,     // minimum separation distance
                toi,              // time of impact
                tolerance,        // delta
                t_max,            // Maximum time to check
                max_iter,         // Maximum number of iterations
                output_tolerance, // delta_actual
                CCD_TYPE);
        }
#else
            throw "CCD method is not enabled";
#endif
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
bool edgeEdgeCCD_OURS(
    const Eigen::Vector3d& edge0_vertex0_start,
    const Eigen::Vector3d& edge0_vertex1_start,
    const Eigen::Vector3d& edge1_vertex0_start,
    const Eigen::Vector3d& edge1_vertex1_start,
    const Eigen::Vector3d& edge0_vertex0_end,
    const Eigen::Vector3d& edge0_vertex1_end,
    const Eigen::Vector3d& edge1_vertex0_end,
    const Eigen::Vector3d& edge1_vertex1_end,
    const std::array<double, 3>& err,
    const double ms, // TODO maybe add an assertion to check if ms is too big?
    double& toi,
    const double tolerance,
    const double pre_check_t,
    const int max_itr,
    double& output_tolerance,
    const int CCD_TYPE)
{
    return inclusion_ccd::edgeEdgeCCD_double(
        edge0_vertex0_start, edge0_vertex1_start, edge1_vertex0_start,
        edge1_vertex1_start, edge0_vertex0_end, edge0_vertex1_end,
        edge1_vertex0_end, edge1_vertex1_end, err, ms, toi, tolerance,
        pre_check_t, max_itr, output_tolerance, CCD_TYPE);
}

bool vertexFaceCCD_OURS(
    const Eigen::Vector3d& edge0_vertex0_start,
    const Eigen::Vector3d& edge0_vertex1_start,
    const Eigen::Vector3d& edge1_vertex0_start,
    const Eigen::Vector3d& edge1_vertex1_start,
    const Eigen::Vector3d& edge0_vertex0_end,
    const Eigen::Vector3d& edge0_vertex1_end,
    const Eigen::Vector3d& edge1_vertex0_end,
    const Eigen::Vector3d& edge1_vertex1_end,
    const std::array<double, 3>& err,
    const double ms, // TODO maybe add an assertion to check if ms is too big?
    double& toi,
    const double tolerance,
    const double pre_check_t,
    const int max_itr,
    double& output_tolerance,
    const int CCD_TYPE)
{
    return inclusion_ccd::vertexFaceCCD_double(
        edge0_vertex0_start, edge0_vertex1_start, edge1_vertex0_start,
        edge1_vertex1_start, edge0_vertex0_end, edge0_vertex1_end,
        edge1_vertex0_end, edge1_vertex1_end, err, ms, toi, tolerance,
        pre_check_t, max_itr, output_tolerance, CCD_TYPE);
}
} // namespace ccd

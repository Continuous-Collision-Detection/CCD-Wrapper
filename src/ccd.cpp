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
// SafeCCD
#include <SAFE_CCD.h>
// Rational root parity with minimum separation and fixes
#include <CCD/ccd.hpp>
// Root parity with minimum separation and fixes
#include <doubleCCD/doubleccd.hpp>
// Interval based CCD of [Redon et al. 2002]
// Interval based CCD of [Redon et al. 2002] solved using [Snyder 1992]
// Custom inclusion based CCD of [Wang et al. 2020]
#include <interval_ccd/interval_ccd.hpp>
// Minimum separation root finder of Harmon et al. [2011]
#include <minimum_separation_root_finder.hpp>

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
    const std::array<double, 3>& err)
{
    double toi; // Computed by some methods but never returned
    try {
        switch (method) {
        case CCDMethod::FLOATING_POINT_ROOT_FINDER:
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
        case CCDMethod::MIN_SEPARATION_ROOT_FINDER:
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
        case CCDMethod::MIN_SEPARATION_ROOT_PARITY:
        case CCDMethod::RATIONAL_MIN_SEPARATION_ROOT_PARITY:
        case CCDMethod::TIGHT_INTERVALS:
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
                /*minimum_distance=*/0, method);
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
        case CCDMethod::SAFE_CCD: {
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
        case CCDMethod::UNIVARIATE_INTERVAL_ROOT_FINDER:
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
                toi);
        case CCDMethod::MULTIVARIATE_INTERVAL_ROOT_FINDER:
            return intervalccd::vertexFaceCCD(
                // Point at t=0
                vertex_start,
                // Triangle at t = 0
                face_vertex0_start, face_vertex1_start, face_vertex2_start,
                // Point at t=1
                vertex_end,
                // Triangle at t = 1
                face_vertex0_end, face_vertex1_end, face_vertex2_end,
                // Time of impact
                toi);

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
    const std::array<double, 3>& err)
{
    double toi; // Computed by some methods but never returned
    try {
        switch (method) {
        case CCDMethod::FLOATING_POINT_ROOT_FINDER:
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
        case CCDMethod::MIN_SEPARATION_ROOT_FINDER:
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
        case CCDMethod::MIN_SEPARATION_ROOT_PARITY:
        case CCDMethod::RATIONAL_MIN_SEPARATION_ROOT_PARITY:
        case CCDMethod::TIGHT_INTERVALS:
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
                /*minimum_distance=*/0, method);
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
        case CCDMethod::SAFE_CCD: {
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
        case CCDMethod::UNIVARIATE_INTERVAL_ROOT_FINDER:
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
                toi);
        case CCDMethod::MULTIVARIATE_INTERVAL_ROOT_FINDER:
            return intervalccd::edgeEdgeCCD(
                // Edge 1 at t=0
                edge0_vertex0_start, edge0_vertex1_start,
                // Edge 2 at t=0
                edge1_vertex0_start, edge1_vertex1_start,
                // Edge 1 at t=1
                edge0_vertex0_end, edge0_vertex1_end,
                // Edge 2 at t=1
                edge1_vertex0_end, edge1_vertex1_end,
                // Time of impact
                toi);

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
    return intervalccd::edgeEdgeCCD_double(
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
    return intervalccd::vertexFaceCCD_double(
        edge0_vertex0_start, edge0_vertex1_start, edge1_vertex0_start,
        edge1_vertex1_start, edge0_vertex0_end, edge0_vertex1_end,
        edge1_vertex0_end, edge1_vertex1_end, err, ms, toi, tolerance,
        pre_check_t, max_itr, output_tolerance, CCD_TYPE);
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
    const std::array<double, 3>& err)
{
    double toi; // Computed by some methods but never returned
    try {
        switch (method) {
        case CCDMethod::MIN_SEPARATION_ROOT_FINDER: {
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
        case CCDMethod::MIN_SEPARATION_ROOT_PARITY:
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
        case CCDMethod::RATIONAL_MIN_SEPARATION_ROOT_PARITY:
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
        case CCDMethod::TIGHT_INTERVALS:
            double output_tolerance;
            return intervalccd::vertexFaceCCD_double(
                // Point at t=0
                vertex_start,
                // Triangle at t = 0
                face_vertex0_start, face_vertex1_start, face_vertex2_start,
                // Point at t=1
                vertex_end,
                // Triangle at t = 1
                face_vertex0_end, face_vertex1_end, face_vertex2_end,
                // Calculate the rounding error automatically
                { -1.0, -1.0, -1.0 },
                // Minimum seperation distance
                min_distance,
                // Time of impact
                toi,
                /*tolerance=*/1e-6,
                /*t_max=*/1,
                /*max_itr=*/-1, output_tolerance,
                /*CCD_TYPE=*/1);
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
    const std::array<double, 3>& err)
{
    double toi; // Computed by some methods but never returned
    try {
        switch (method) {
        case CCDMethod::MIN_SEPARATION_ROOT_FINDER: {
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
        case CCDMethod::MIN_SEPARATION_ROOT_PARITY:
            return doubleccd::edgeEdgeCCD(
                // Edge 1 at t=0
                edge0_vertex0_start, edge0_vertex1_start,
                // Edge 2 at t=0
                edge1_vertex0_start, edge1_vertex1_start,
                // Edge 1 at t=1
                edge0_vertex0_end, edge0_vertex1_end,
                // Edge 2 at t=1
                edge1_vertex0_end, edge1_vertex1_end, min_distance);
        case CCDMethod::RATIONAL_MIN_SEPARATION_ROOT_PARITY:
            return ccd::edgeEdgeCCD(
                // Edge 1 at t=0
                edge0_vertex0_start, edge0_vertex1_start,
                // Edge 2 at t=0
                edge1_vertex0_start, edge1_vertex1_start,
                // Edge 1 at t=1
                edge0_vertex0_end, edge0_vertex1_end,
                // Edge 2 at t=1
                edge1_vertex0_end, edge1_vertex1_end, min_distance);
        case CCDMethod::TIGHT_INTERVALS:
            double output_tolerance;
            return intervalccd::edgeEdgeCCD_double(
                // Edge 1 at t=0
                edge0_vertex0_start, edge0_vertex1_start,
                // Edge 2 at t=0
                edge1_vertex0_start, edge1_vertex1_start,
                // Edge 1 at t=1
                edge0_vertex0_end, edge0_vertex1_end,
                // Edge 2 at t=1
                edge1_vertex0_end, edge1_vertex1_end,
                // Calculate the rounding error automatically
                { -1.0, -1.0, -1.0 },
                // Minimum seperation distance
                min_distance,
                // Time of impact
                toi,
                /*tolerance=*/1e-6,
                /*t_max=*/1,
                /*max_itr=*/-1, output_tolerance,
                /*CCD_TYPE=*/1);
        default:
            assert(!is_minimum_separation_method(method));
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

} // namespace ccd

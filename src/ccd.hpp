/// @brief Eigen wrappers for different CCD methods

#pragma once

#include <Eigen/Core>

namespace ccd {

/// Methods of continuous collision detection.
enum CCDMethod {
    /// Etienne Vouga's CCD using a root finder in floating points
    FLOATING_POINT_ROOT_FINDER = 0,
    /// Floating-point root-finder minimum separation CCD of [Lu et al. 2018]
    MIN_SEPARATION_ROOT_FINDER,
    /// Root parity method of [Brochu et al. 2012]
    ROOT_PARITY,
    /// Teseo's reimplementation of [Brochu et al. 2012] using rationals
    RATIONAL_ROOT_PARITY,
    /// Root parity with minimum separation and fixes
    MIN_SEPARATION_ROOT_PARITY,
    /// Rational root parity with minimum separation and fixes
    RATIONAL_MIN_SEPARATION_ROOT_PARITY,
    /// Bernstein sign classification method of [Tang et al. 2014]
    BSC,
    /// TightCCD method of [Wang et al. 2015]
    TIGHT_CCD,
    /// Interval based CCD of [Redon et al. 2002]
    UNIVARIATE_INTERVAL_ROOT_FINDER,
    /// Interval based CCD of [Redon et al. 2002] solved using [Snyder 1992]
    MULTIVARIATE_INTERVAL_ROOT_FINDER,
    /// Custom inclusion based CCD of [Wang et al. 2020]
    TIGHT_INTERVALS,
    /// WARNING: Not a method! Counts the number of methods.
    NUM_CCD_METHODS
};

static const char* method_names[CCDMethod::NUM_CCD_METHODS] = {
    "FloatingPointRootFinder",
    "MinSeperationRootFinder",
    "RootParity",
    "RationalRootParity",
    "MinSeparationRootParity",
    "RationalMinSeparationRootParity",
    "BSC",
    "TightCCD",
    "UnivariateIntervalRootFinder",
    "MultivariateIntervalRootFinder",
    "TightIntervals",
};

/// Minimum separation distance used when looking for 0 distance collisions.
static const double DEFAULT_MIN_DISTANCE = 1e-8;

/**
 * @brief Detect collisions between a vertex and a triangular face.
 *
 * Looks for collisions between a point and triangle as they move linearily
 * with constant velocity. Returns true if the vertex and face collide.
 *
 * @param[in]  vertex_start        Start position of the vertex.
 * @param[in]  face_vertex0_start  Start position of the first vertex of the
 *                                 face.
 * @param[in]  face_vertex1_start  Start position of the second vertex of the
 *                                 face.
 * @param[in]  face_vertex2_start  Start position of the third vertex of the
 *                                 face.
 * @param[in]  vertex_end          End position of the vertex.
 * @param[in]  face_vertex0_end    End position of the first vertex of the
 *                                 face.
 * @param[in]  face_vertex1_end    End position of the second vertex of the
 *                                 face.
 * @param[in]  face_vertex2_end    End position of the third vertex of the
 *                                 face.
 * @param[in]  method              Method of exact CCD.
 *
 * @returns  True if the vertex and face collide.
 */
bool vertexFaceCCD(
    const Eigen::Vector3d& vertex_start,
    const Eigen::Vector3d& face_vertex0_start,
    const Eigen::Vector3d& face_vertex1_start,
    const Eigen::Vector3d& face_vertex2_start,
    const Eigen::Vector3d& vertex_end,
    const Eigen::Vector3d& face_vertex0_end,
    const Eigen::Vector3d& face_vertex1_end,
    const Eigen::Vector3d& face_vertex2_end,
    const CCDMethod method);

/**
 * @brief Detect collisions between two edges as they move.
 *
 * Looks for collisions between edges as they move linearly with constant
 * velocity. Returns true if the edges collide.
 *
 * @param[in]  edge0_vertex0_start  Start position of the first edge's first
 *                                  vertex.
 * @param[in]  edge0_vertex1_start  Start position of the first edge's second
 *                                  vertex.
 * @param[in]  edge1_vertex0_start  Start position of the second edge's first
 *                                  vertex.
 * @param[in]  edge1_vertex1_start  Start position of the second edge's second
 *                                  vertex.
 * @param[in]  edge0_vertex0_end    End position of the first edge's first
 *                                  vertex.
 * @param[in]  edge0_vertex1_end    End position of the first edge's second
 *                                  vertex.
 * @param[in]  edge1_vertex0_end    End position of the second edge's first
 *                                  vertex.
 * @param[in]  edge1_vertex1_end    End position of the second edge's second
 *                                  vertex.
 * @param[in]  method               Method of exact CCD.
 *
 * @returns True if the edges collide.
 */
bool edgeEdgeCCD(
    const Eigen::Vector3d& edge0_vertex0_start,
    const Eigen::Vector3d& edge0_vertex1_start,
    const Eigen::Vector3d& edge1_vertex0_start,
    const Eigen::Vector3d& edge1_vertex1_start,
    const Eigen::Vector3d& edge0_vertex0_end,
    const Eigen::Vector3d& edge0_vertex1_end,
    const Eigen::Vector3d& edge1_vertex0_end,
    const Eigen::Vector3d& edge1_vertex1_end,
    const CCDMethod method);

/**
 * @brief Detect proximity collisions between a vertex and a triangular face.
 *
 * Looks for collisions between a point and triangle as they move linearily
 * with constant velocity. Returns true if the vertex and face collide.
 *
 * @param[in]  vertex_start        Start position of the vertex.
 * @param[in]  face_vertex0_start  Start position of the first vertex of the
 *                                 face.
 * @param[in]  face_vertex1_start  Start position of the second vertex of the
 *                                 face.
 * @param[in]  face_vertex2_start  Start position of the third vertex of the
 *                                 face.
 * @param[in]  vertex_end          End position of the vertex.
 * @param[in]  face_vertex0_end    End position of the first vertex of the
 *                                 face.
 * @param[in]  face_vertex1_end    End position of the second vertex of the
 *                                 face.
 * @param[in]  face_vertex2_end    End position of the third vertex of the
 *                                 face.
 * @param[in]  method              Method of minimum separation CCD.
 *
 * @returns  True if the vertex and face collide.
 */
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
    const CCDMethod method);

/**
 * @brief Detect proximity collisions between two edges as they move.
 *
 * Looks for collisions between edges as they move linearly with constant
 * velocity. Returns true if the edges collide.
 *
 * @param[in]  edge0_vertex0_start  Start position of the first edge's first
 *                                  vertex.
 * @param[in]  edge0_vertex1_start  Start position of the first edge's second
 *                                  vertex.
 * @param[in]  edge1_vertex0_start  Start position of the second edge's first
 *                                  vertex.
 * @param[in]  edge1_vertex1_start  Start position of the second edge's second
 *                                  vertex.
 * @param[in]  edge0_vertex0_end    End position of the first edge's first
 *                                  vertex.
 * @param[in]  edge0_vertex1_end    End position of the first edge's second
 *                                  vertex.
 * @param[in]  edge1_vertex0_end    End position of the second edge's first
 *                                  vertex.
 * @param[in]  edge1_vertex1_end    End position of the second edge's second
 *                                  vertex.
 * @param[in]  method               Method of minimum separation CCD.
 *
 * @returns True if the edges collide.
 */
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
    const CCDMethod method);

inline bool is_minimum_separation_method(const CCDMethod& method)
{
    switch (method) {
    case CCDMethod::MIN_SEPARATION_ROOT_FINDER:
    case CCDMethod::MIN_SEPARATION_ROOT_PARITY:
    case CCDMethod::RATIONAL_MIN_SEPARATION_ROOT_PARITY:
    case CCDMethod::TIGHT_INTERVALS:
        return true;
    default:
        return false;
    }
}

inline bool is_conservative_method(const CCDMethod& method)
{
    switch (method) {
    // MIN_SEPARATION_ROOT_FINDER is conservative because minimum seperation
    // distance of zero does not work well.
    case CCDMethod::MIN_SEPARATION_ROOT_FINDER:
    case CCDMethod::TIGHT_CCD:
    case CCDMethod::UNIVARIATE_INTERVAL_ROOT_FINDER:
    case CCDMethod::MULTIVARIATE_INTERVAL_ROOT_FINDER:
    case CCDMethod::TIGHT_INTERVALS:
        return true;
    default:
        return false;
    }
}

inline bool is_time_of_impact_computed(const CCDMethod& method)
{
    switch (method) {
    case CCDMethod::FLOATING_POINT_ROOT_FINDER:
    case CCDMethod::MIN_SEPARATION_ROOT_FINDER:
    case CCDMethod::UNIVARIATE_INTERVAL_ROOT_FINDER:
    case CCDMethod::MULTIVARIATE_INTERVAL_ROOT_FINDER:
    case CCDMethod::TIGHT_INTERVALS:
        return true;
    default:
        return false;
    }
}

} // namespace ccd

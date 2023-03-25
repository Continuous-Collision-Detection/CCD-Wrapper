#pragma once

namespace ccd {

class CCDMethod {
    /// @brief Detect collisions between a point and a triangle.
    ///
    /// Looks for collisions between a point and triangle as they move linearily
    /// with constant velocity. Returns true if the point and triangle collide.
    ///
    /// @param[in] p_t0 Start position of the point.
    /// @param[in] t0_t0 Start position of the first vertex of the triangle.
    /// @param[in] t1_t0 Start position of the second vertex of the triangle.
    /// @param[in] t2_t0 Start position of the third vertex of the triangle.
    /// @param[in] p_t1 End position of the point.
    /// @param[in] t0_t0 End position of the first vertex of the triangle.
    /// @param[in] t1_t0 End position of the second vertex of the triangle.
    /// @param[in] t2_t0 End position of the third vertex of the triangle.
    ///
    /// @returns  True if the point and triangle collide.
    virtual bool point_triangle_ccd(
        const Eigen::Vector3d& p_t0,
        const Eigen::Vector3d& t0_t0,
        const Eigen::Vector3d& t1_t0,
        const Eigen::Vector3d& t2_t0,
        const Eigen::Vector3d& p_t1,
        const Eigen::Vector3d& t0_t0,
        const Eigen::Vector3d& t1_t0,
        const Eigen::Vector3d& t2_t0) const = 0;

    /// This version computes the time of impact (ToI).
    /// By default this is done through bisection, but should be overridden if
    /// the method can directly compute the ToI.
    virtual bool point_triangle_ccd(
        const Eigen::Vector3d& p_t0,
        const Eigen::Vector3d& t0_t0,
        const Eigen::Vector3d& t1_t0,
        const Eigen::Vector3d& t2_t0,
        const Eigen::Vector3d& p_t1,
        const Eigen::Vector3d& t0_t0,
        const Eigen::Vector3d& t1_t0,
        const Eigen::Vector3d& t2_t0,
        double& toi) const;

    /// @brief Detect collisions between two edges as they move.
    ///
    /// Looks for collisions between edges as they move linearly with constant
    /// velocity. Returns true if the edges collide.
    ///
    /// @param[in] ea0_t0 Start position of the first edge's first vertex.
    /// @param[in] ea1_t0 Start position of the first edge's second vertex.
    /// @param[in] eb0_t0 Start position of the second edge's first vertex.
    /// @param[in] eb1_t0 Start position of the second edge's second vertex.
    /// @param[in] ea0_t1 End position of the first edge's first vertex.
    /// @param[in] ea1_t1 End position of the first edge's second vertex.
    /// @param[in] eb0_t1 End position of the second edge's first vertex.
    /// @param[in] eb1_t1 End position of the second edge's second vertex.
    ///
    /// @returns True if the edges collide.
    virtual bool edgeEdgeCCD(
        const Eigen::Vector3d& ea0_t0,
        const Eigen::Vector3d& ea1_t0,
        const Eigen::Vector3d& eb0_t0,
        const Eigen::Vector3d& eb1_t0,
        const Eigen::Vector3d& ea0_t1,
        const Eigen::Vector3d& ea1_t1,
        const Eigen::Vector3d& eb0_t1,
        const Eigen::Vector3d& eb1_t1) const = 0;

    /// This version computes the time of impact (ToI).
    /// By default this is done through bisection, but should be overridden if
    /// the method can directly compute the ToI.
    virtual bool edgeEdgeCCD(
        const Eigen::Vector3d& ea0_t0,
        const Eigen::Vector3d& ea1_t0,
        const Eigen::Vector3d& eb0_t0,
        const Eigen::Vector3d& eb1_t0,
        const Eigen::Vector3d& ea0_t1,
        const Eigen::Vector3d& ea1_t1,
        const Eigen::Vector3d& eb0_t1,
        const Eigen::Vector3d& eb1_t1,
        double& toi) const;

    virtual bool supportes_minimum_separation() const { return false; }

    static std::shared_ptr<CCDMethod>
    get_method(const std::string& method_name);
};

} // namespace ccd

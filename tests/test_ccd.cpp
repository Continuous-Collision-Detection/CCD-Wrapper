#include <catch2/catch.hpp>

#include <ccd.hpp>

static const double EPSILON = std::numeric_limits<float>::epsilon();

#ifdef EXPORT_CCD_QUERIES
#include "dump_queries.hpp"

static int vf_counter = 0;
static int ee_counter = 0;
#endif

TEST_CASE(
    "Test Point-Triangle Continuous Collision Detection",
    "[ccd][point-triangle]")
{
    using namespace ccd;
    CCDMethod method = CCDMethod(GENERATE(range(0, int(NUM_CCD_METHODS))));

    if (!is_method_enabled(method)) {
        return;
    }

    // point
#ifdef WIN32
    double v0z = 0.0;
#else
    double v0z = GENERATE(0.0, -1.0);
#endif
    Eigen::Vector3d v0(0, 1, v0z);
    // triangle = (v1, v2, v3)
    Eigen::Vector3d v1(-1, 0, 1);
    Eigen::Vector3d v2(1, 0, 1);
    Eigen::Vector3d v3(0, 0, -1);

    // displacements
    double u0y
        = -GENERATE(-1.0, 0.0, 0.5 - EPSILON, 0.5, 0.5 + EPSILON, 1.0, 2.0);
#ifdef WIN32
    double u0z = EPSILON;
#else
    double u0z = GENERATE(-EPSILON, 0.0, EPSILON);
#endif
    Eigen::Vector3d u0(0, u0y, u0z);
    double u1y
        = GENERATE(-1.0, 0.0, 0.5 - EPSILON, 0.5, 0.5 + EPSILON, 1.0, 2.0);
    Eigen::Vector3d u1(0, u1y, 0);

    SECTION("Clockwise triangle")
    {
        // already in clockwise order
    }
    SECTION("Counter-clockwise triangle") { std::swap(v1, v2); }

    bool expected_hit = ((-u0y + u1y >= 1) && (v0z + u0z >= v3.z()));

    bool hit = vertexFaceCCD(
        v0, v1, v2, v3, v0 + u0, v1 + u1, v2 + u1, v3 + u1, method);

#ifdef EXPORT_CCD_QUERIES
    if (method
        == CCDMethod::FLOATING_POINT_ROOT_FINDER) { // Only export one query
        dump_vf_query(
            fmt::format("vertex_face_{:07d}", vf_counter++), v0, v1, v2, v3,
            v0 + u0, v1 + u1, v2 + u1, v3 + u1, expected_hit);
    }
#endif

    CAPTURE(v0z, u0y, u1y, u0z, EPSILON, method_names[method]);
    // Conservative methods can produce false positives, so only check if the
    // hit value is negative.
    if (!is_conservative_method(method) || !hit) {
        CHECK(hit == expected_hit);
    }
}

TEST_CASE("Test Edge-Edge Continuous Collision Detection", "[ccd][edge-edge]")
{
    using namespace ccd;
    CCDMethod method = CCDMethod(GENERATE(range(0, int(NUM_CCD_METHODS))));

    if (!is_method_enabled(method)) {
        return;
    }

    // e0 = (v0, v1)
    Eigen::Vector3d v0(-1, -1, 0);
    Eigen::Vector3d v1(1, -1, 0);
    // e2 = (v2, v3)
    double e1x = GENERATE(
        -1 - EPSILON, -1, -1 + EPSILON, -0.5, 0, 0.5, 1 - EPSILON, 1,
        1 + EPSILON);
    Eigen::Vector3d v2(e1x, 1, -1);
    Eigen::Vector3d v3(e1x, 1, 1);

    // displacements
    double y_displacement
        = GENERATE(-1.0, 0.0, 1 - EPSILON, 1.0, 1 + EPSILON, 2.0);
    Eigen::Vector3d u0(0, y_displacement, 0);
    Eigen::Vector3d u1(0, -y_displacement, 0);

    bool expected_hit = y_displacement >= 1.0 && e1x >= -1 && e1x <= 1;

    bool hit = edgeEdgeCCD(
        v0, v1, v2, v3, v0 + u0, v1 + u0, v2 + u1, v3 + u1, method);

#ifdef EXPORT_CCD_QUERIES
    if (method
        == CCDMethod::FLOATING_POINT_ROOT_FINDER) { // Only export one query
        dump_ee_query(
            fmt::format("edge_edge_{:07d}", ee_counter++), v0, v1, v2, v3,
            v0 + u0, v1 + u0, v2 + u1, v3 + u1, expected_hit);
        ee_counter++;
    }
#endif

    CAPTURE(y_displacement, e1x, method_names[method]);
    // Conservative methods can produce false positives, so only check if the
    // hit value is negative.
    if (!is_conservative_method(method) || !hit) {
        CHECK(hit == expected_hit);
    }
}

TEST_CASE("Test Fixed Edge Edge-Edge Case", "[ccd][edge-edge]")
{
    using namespace ccd;
    CCDMethod method = CCDMethod(GENERATE(range(0, int(NUM_CCD_METHODS))));

    if (!is_method_enabled(method)) {
        return;
    }

    // e0 = (v0, v1)
    Eigen::Vector3d v0(-1, -1, 0);
    Eigen::Vector3d v1(1, -1, 0);
    // e2 = (v2, v3)
    double e1x = GENERATE(
        -1 - EPSILON, -1, -1 + EPSILON, -0.5, 0, 0.5, 1 - EPSILON, 1,
        1 + EPSILON);
    Eigen::Vector3d v2(e1x, 1, -1);
    Eigen::Vector3d v3(e1x, 1, 1);

    // displacements
    double y_displacement
        = 2 * GENERATE(-1.0, 0.0, 1 - EPSILON, 1.0, 1 + EPSILON, 2.0);
    Eigen::Vector3d u0(0, y_displacement, 0);
    Eigen::Vector3d u1(0, 0, 0);

    bool expected_hit = y_displacement >= 2.0 && e1x >= -1 && e1x <= 1;

    bool hit = edgeEdgeCCD(
        v0, v1, v2, v3, v0 + u0, v1 + u0, v2 + u1, v3 + u1, method);

#ifdef EXPORT_CCD_QUERIES
    if (method
        == CCDMethod::FLOATING_POINT_ROOT_FINDER) { // Only export one query
        dump_ee_query(
            fmt::format("edge_edge_{:07d}", ee_counter++), v0, v1, v2, v3,
            v0 + u0, v1 + u0, v2 + u1, v3 + u1, expected_hit);
        ee_counter++;
    }
#endif

    CAPTURE(y_displacement, method_names[method]);
    // Conservative methods can produce false positives, so only check if the
    // hit value is negative.
    if (!is_conservative_method(method) || !hit) {
        CHECK(hit == expected_hit);
    }
}

TEST_CASE("Zhongshi test case", "[ccd][point-triangle]")
{
    using namespace ccd;
    CCDMethod method = CCDMethod(GENERATE(range(0, int(NUM_CCD_METHODS))));

    if (!is_method_enabled(method)) {
        return;
    }

#ifdef WIN32
    double qy = EPSILON;
#else
    double qy = GENERATE(-EPSILON, 0, EPSILON);
#endif

    Eigen::Vector3d q;
    q << 0, qy, 0;

    Eigen::Vector3d b0;
    b0 << 0, 0, 0;
    Eigen::Vector3d b1;
    b1 << 0, 1, 0;
    Eigen::Vector3d b2;
    b2 << 1, 0, 0;

    Eigen::Vector3d t0;
    t0 << 0, 0, 1;
    Eigen::Vector3d t1;
    t1 << 0, 1, 1;
    Eigen::Vector3d t2;
    t2 << 1, 0, 1;

    Eigen::Vector3d q1;
    q1 << 0, qy, 0;

    bool expected_hit = q.y() >= 0;

    bool hit = vertexFaceCCD(q, b0, b1, b2, q1, t0, t1, t2, method);

#ifdef EXPORT_CCD_QUERIES
    if (method
        == CCDMethod::FLOATING_POINT_ROOT_FINDER) { // Only export one query
        dump_vf_query(
            fmt::format("vertex_face_{:07d}", vf_counter++), q, b0, b1, b2, q1,
            t0, t1, t2, expected_hit);
    }
#endif

    CAPTURE(qy, method_names[method]);
    // Conservative methods can produce false positives, so only check if the
    // hit value is negative.
    if (!is_conservative_method(method) || !hit) {
        CHECK(hit == expected_hit);
    }
}

TEST_CASE("Bolun test case", "[ccd][point-triangle]")
{
    using namespace ccd;
    CCDMethod method = CCDMethod(GENERATE(range(0, int(NUM_CCD_METHODS))));

    if (!is_method_enabled(method)) {
        return;
    }

#ifndef NDEBUG
    // BSC has an assertion that causes this test to fail.
    if (method == CCDMethod::FIXED_ROOT_PARITY) {
        FAIL();
    }
#endif

    Eigen::Vector3d x0(0.1, 0.1, 0.1), x1(0, 0, 1), x2(1, 0, 1), x3(0, 1, 1),
        x0b(0.1, 0.1, 0.1), x1b(0, 0, 0), x2b(0, 1, 0), x3b(1, 0, 0);

    bool expected_hit = true;

    bool hit = vertexFaceCCD(x0, x1, x2, x3, x0b, x1b, x2b, x3b, method);

#ifdef EXPORT_CCD_QUERIES
    if (method
        == CCDMethod::FLOATING_POINT_ROOT_FINDER) { // Only export one query
        dump_vf_query(
            fmt::format("vertex_face_{:07d}", vf_counter++), x0, x1, x2, x3,
            x0b, x1b, x2b, x3b, expected_hit);
    }
#endif

    CAPTURE(method_names[method]);
    // Conservative methods can produce false positives, so only check if the
    // hit value is negative.
    if (!is_conservative_method(method) || !hit) {
        CHECK(hit == expected_hit);
    }
}

#if defined(CCD_WRAPPER_WITH_BSC) && defined(CCD_WRAPPER_WITH_RRP)
TEST_CASE("BSC False Negative", "[ccd][point-triangle][bsc][!shouldfail]")
{
    Eigen::Vector3d v0_t0, v1_t0, v2_t0, v3_t0, v0_t1, v1_t1, v2_t1, v3_t1;
    SECTION("Case 1")
    {
        v0_t0 = Eigen::Vector3d(1.0, 0.5, 1.0);
        v1_t0 = Eigen::Vector3d(2.220446049250313E-16, 0.5718259037844386, 1.0);
        v2_t0 = Eigen::Vector3d(1.0, 0.5718259037844384, 1.0);
        v3_t0 = Eigen::Vector3d(1.0, 1.5718259037844384, 1.0);
        v0_t1 = Eigen::Vector3d(1.0, 0.5, 1.0);
        v1_t1 = Eigen::Vector3d(2.220446049250313E-16, 0.2776264037844385, 1.0);
        v2_t1 = Eigen::Vector3d(1.0, 0.2776264037844385, 1.0);
        v3_t1 = Eigen::Vector3d(1.0, 1.2776264037844385, 1.0);
    }
    SECTION("Case 2")
    {
        v0_t0 = Eigen::Vector3d(1.0, 0.5, 1.0);
        v1_t0 = Eigen::Vector3d(0, 0.57, 1.0);
        v2_t0 = Eigen::Vector3d(1.0, 0.57, 1.0);
        v3_t0 = Eigen::Vector3d(1.0, 1.57, 1.0);
        v0_t1 = Eigen::Vector3d(1.0, 0.5, 1.0);
        v1_t1 = Eigen::Vector3d(0, 0.28, 1.0);
        v2_t1 = Eigen::Vector3d(1.0, 0.28, 1.0);
        v3_t1 = Eigen::Vector3d(1.0, 1.28, 1.0);
    }

    bool expected_hit = vertexFaceCCD(
        v0_t0, v1_t0, v2_t0, v3_t0, v0_t1, v1_t1, v2_t1, v3_t1,
        ccd::CCDMethod::RATIONAL_ROOT_PARITY);

    bool hit = vertexFaceCCD(
        v0_t0, v1_t0, v2_t0, v3_t0, v0_t1, v1_t1, v2_t1, v3_t1,
        ccd::CCDMethod::BSC);

    CHECK(hit == expected_hit);
}
#endif

TEST_CASE("Teseo test case", "[!mayfail][ccd][edge-edge]")
{
    using namespace ccd;
    CCDMethod method = CCDMethod(GENERATE(range(0, int(NUM_CCD_METHODS))));

    if (!is_method_enabled(method)) {
        return;
    }

    CAPTURE(method_names[method]);
#ifndef NDEBUG
    // BSC has an assertion that causes this test to fail.
    if (method == CCDMethod::BSC || method == CCDMethod::TIGHT_CCD) {
        FAIL();
    }
#endif

    const Eigen::Vector3d a0s(-30022200, 2362580, 165247);
    const Eigen::Vector3d a1s(-32347850, 8312380, -1151003);
    const Eigen::Vector3d a0e(-28995600, 345838, 638580);
    const Eigen::Vector3d a1e(-31716930, 6104858, -713340);
    const Eigen::Vector3d b0(-30319900, 3148750, 0);
    const Eigen::Vector3d b1(-28548800, 900349, 0);

    bool expected_hit = true;

    bool hit = edgeEdgeCCD(a0s, a1s, b0, b1, a0e, a1e, b0, b1, method);

#ifdef EXPORT_CCD_QUERIES
    if (method
        == CCDMethod::FLOATING_POINT_ROOT_FINDER) { // Only export one query
        dump_vf_query(
            fmt::format("edge_edge_{:07d}", ee_counter++), x0, x1, x2, x3, x0b,
            x1b, x2b, x3b, expected_hit);
    }
#endif

    CAPTURE(method_names[method]);
    // Conservative methods can produce false positives, so only check if the
    // hit value is negative.
    if (!is_conservative_method(method) || !hit) {
        CHECK(hit == expected_hit);
    }
}

TEST_CASE("Teseo test case 2", "[!mayfail][ccd][edge-edge][teseo2]")
{
    using namespace ccd;
    CCDMethod method = CCDMethod(GENERATE(range(0, int(NUM_CCD_METHODS))));

    if (!is_method_enabled(method)) {
        return;
    }

#ifndef NDEBUG
    // BSC has an assertion that causes this test to fail.
    if (method == CCDMethod::FIXED_ROOT_PARITY) {
        FAIL();
    }
#endif

    const Eigen::Vector3d a0s(0, 0, 1);
    const Eigen::Vector3d a1s(0, 1, 1);
    Eigen::Vector3d a0e(1, 1, 0);
    Eigen::Vector3d a1e(0, 0, 0);
    const Eigen::Vector3d b0(0.1, 0.2, 2);
    const Eigen::Vector3d b1(0.1, 0.2, -1);

    double t = GENERATE(0.5, 0.8, 0.88, 0.9, 1.0);
    a0e = (a0e - a0s) * t + a0s;
    a1e = (a1e - a1s) * t + a1s;

    bool expected_hit = true;

    bool hit = edgeEdgeCCD(a0s, a1s, b0, b1, a0e, a1e, b0, b1, method);

#ifdef EXPORT_CCD_QUERIES
    if (method
        == CCDMethod::FLOATING_POINT_ROOT_FINDER) { // Only export one query
        dump_vf_query(
            fmt::format("edge_edge_{:07d}", ee_counter++), x0, x1, x2, x3, x0b,
            x1b, x2b, x3b, expected_hit);
    }
#endif

    CAPTURE(method_names[method]);
    // Conservative methods can produce false positives, so only check if the
    // hit value is negative.
    if (!is_conservative_method(method) || !hit) {
        CHECK(hit == expected_hit);
    }
}

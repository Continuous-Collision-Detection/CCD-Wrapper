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

    // point
    double v0z = GENERATE(0.0, -1.0);
    Eigen::Vector3d v0(0, 1, v0z);
    // triangle = (v1, v2, v3)
    Eigen::Vector3d v1(-1, 0, 1);
    Eigen::Vector3d v2(1, 0, 1);
    Eigen::Vector3d v3(0, 0, -1);

    // displacements
    double u0y
        = -GENERATE(-1.0, 0.0, 0.5 - EPSILON, 0.5, 0.5 + EPSILON, 1.0, 2.0);
    double u0z = GENERATE(-EPSILON, 0.0, EPSILON);
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
    if (method == CCDMethod::FLOAT) { // Only export one query
        dump_vf_query(
            fmt::format("vertex_face_{:07d}", vf_counter++), v0, v1, v2, v3,
            v0 + u0, v1 + u1, v2 + u1, v3 + u1, expected_hit);
    }
#endif

    CAPTURE(v0z, u0y, u1y, u0z, EPSILON, method_names[method]);
    // TightCCD can produce false positives, so only check if the hit value is
    // negative.
    if (method != CCDMethod::TIGHT_CCD || !hit) {
        CHECK(hit == expected_hit);
    }
}

TEST_CASE("Test Edge-Edge Continuous Collision Detection", "[ccd][edge-edge]")
{
    using namespace ccd;
    CCDMethod method = CCDMethod(GENERATE(range(0, int(NUM_CCD_METHODS))));

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
    if (method == CCDMethod::FLOAT) { // Only export one query
        dump_ee_query(
            fmt::format("edge_edge_{:07d}", ee_counter++), v0, v1, v2, v3,
            v0 + u0, v1 + u0, v2 + u1, v3 + u1, expected_hit);
        ee_counter++;
    }
#endif

    CAPTURE(y_displacement, e1x, method_names[method]);
    // TightCCD can produce false positives, so only check if the hit value is
    // negative.
    if (method != CCDMethod::TIGHT_CCD || !hit) {
        CHECK(hit == expected_hit);
    }
}

TEST_CASE("Zhongshi test case", "[ccd][point-triangle]")
{
    using namespace ccd;
    CCDMethod method = CCDMethod(GENERATE(range(0, int(NUM_CCD_METHODS))));

    double qy = GENERATE(-EPSILON, 0, EPSILON);

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
    if (method == CCDMethod::FLOAT) { // Only export one query
        dump_vf_query(
            fmt::format("vertex_face_{:07d}", vf_counter++), q, b0, b1, b2, q1,
            t0, t1, t2, expected_hit);
    }
#endif

    CAPTURE(qy, method_names[method]);
    // TightCCD can produce false positives, so only check if the hit value is
    // negative.
    if (method != CCDMethod::TIGHT_CCD || !hit) {
        CHECK(hit == expected_hit);
    }
}

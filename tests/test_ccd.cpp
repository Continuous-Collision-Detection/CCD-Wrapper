#include <catch2/catch.hpp>

#ifdef EXPORT_CCD_QUERIES
#include <fmt/format.h>
#include <highfive/H5Easy.hpp>
#endif

#include <ccd.hpp>

static const double EPSILON = std::numeric_limits<float>::epsilon();

#ifdef EXPORT_CCD_QUERIES
static int vf_counter = 0;
static int ee_counter = 0;
#endif

TEST_CASE(
    "Test Point-Triangle Continuous Collision Detection",
    "[ccd][point-triangle]")
{
    using namespace ccd;
    CCDMethod method = GENERATE(
        CCDMethod::FLOAT, CCDMethod::ROOT_PARITY,
        CCDMethod::RATIONAL_ROOT_PARITY, CCDMethod::BSC, CCDMethod::TIGHT_CCD);

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
        HighFive::File file(
            fmt::format("vertex-face-collisions.hdf5"),
            HighFive::File::OpenOrCreate);
        Eigen::Matrix<double, 8, 3> points;
        points.row(0) = v0;
        points.row(1) = v1;
        points.row(2) = v2;
        points.row(3) = v3;
        points.row(4) = v0 + u0;
        points.row(5) = v1 + u1;
        points.row(6) = v2 + u1;
        points.row(7) = v3 + u1;
        H5Easy::dump(
            file, fmt::format("/vertex_face_{:07d}/points", vf_counter), points,
            H5Easy::DumpMode::Overwrite);
        H5Easy::dump(
            file, fmt::format("/vertex_face_{:07d}/result", vf_counter),
            (unsigned char)expected_hit, H5Easy::DumpMode::Overwrite);
        vf_counter++;
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
    CCDMethod method = GENERATE(
        CCDMethod::FLOAT, CCDMethod::ROOT_PARITY,
        CCDMethod::RATIONAL_ROOT_PARITY, CCDMethod::BSC, CCDMethod::TIGHT_CCD);

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
        HighFive::File file(
            fmt::format("edge-edge-collisions.hdf5"),
            HighFive::File::OpenOrCreate);
        Eigen::Matrix<double, 8, 3> points;
        points.row(0) = v0;
        points.row(1) = v1;
        points.row(2) = v2;
        points.row(3) = v3;
        points.row(4) = v0 + u0;
        points.row(5) = v1 + u0;
        points.row(6) = v2 + u1;
        points.row(7) = v3 + u1;
        H5Easy::dump(
            file, fmt::format("/edge_edge_{:07d}/points", ee_counter), points,
            H5Easy::DumpMode::Overwrite);
        H5Easy::dump(
            file, fmt::format("/edge_edge_{:07d}/result", ee_counter),
            (unsigned char)expected_hit, H5Easy::DumpMode::Overwrite);
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

TEST_CASE("Zhongshi test case", "[ccd][point-triangle][!mayfail]")
{
    using namespace ccd;
    CCDMethod method = GENERATE(
        CCDMethod::FLOAT, CCDMethod::ROOT_PARITY,
        CCDMethod::RATIONAL_ROOT_PARITY, CCDMethod::BSC, CCDMethod::TIGHT_CCD);

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
        HighFive::File file(
            fmt::format("vertex-face-collisions.hdf5"),
            HighFive::File::OpenOrCreate);
        Eigen::Matrix<double, 8, 3> points;
        points.row(0) = q;
        points.row(1) = b0;
        points.row(2) = b1;
        points.row(3) = b2;
        points.row(4) = q1;
        points.row(5) = t0;
        points.row(6) = t1;
        points.row(7) = t2;
        H5Easy::dump(
            file, fmt::format("/vertex_face_{:07d}/points", vf_counter), points,
            H5Easy::DumpMode::Overwrite);
        H5Easy::dump(
            file, fmt::format("/vertex_face_{:07d}/result", vf_counter),
            (unsigned char)expected_hit, H5Easy::DumpMode::Overwrite);
        vf_counter++;
    }
#endif

    CAPTURE(qy, method_names[method]);
    // TightCCD can produce false positives, so only check if the hit value is
    // negative.
    if (method != CCDMethod::TIGHT_CCD || !hit) {
        CHECK(hit == expected_hit);
    }
}

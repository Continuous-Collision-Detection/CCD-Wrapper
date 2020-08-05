// Round the queries to conform with the minimum seperation root parity input
// requirements.

#include <string>

#include <Eigen/Core>
#include <fmt/format.h>
#include <highfive/H5Easy.hpp>

#include <doubleCCD/double_subfunctions.h>

#include <ccd.hpp>

using namespace ccd;

int parse_args(
    int argc, char* argv[], std::string& filename, bool& is_edge_edge)
{
    const std::string usage
        = fmt::format("usage: {} /path/to/data/file.hdf5 {{vf|ee}}", argv[0]);
    if (argc < 2) {
        std::cerr << usage << std::endl
                  << "error: missing path to data" << std::endl;
        return 1;
    }
    if (argc < 3) {
        std::cerr << usage << std::endl
                  << "error: missing type of collisions" << std::endl;
        return 2;
    } else if (strcmp(argv[2], "vf") != 0 && strcmp(argv[2], "ee") != 0) {
        std::cerr << usage << std::endl
                  << "invalid type of collisions: " << argv[2] << std::endl;
        return 2;
    }

    filename = std::string(argv[1]);
    is_edge_edge = strcmp(argv[2], "ee") == 0;
    return 0;
}

int main(int argc, char* argv[])
{
    std::string filename;
    bool is_edge_edge;
    int code = parse_args(argc, argv, filename, is_edge_edge);
    if (code) {
        return code;
    }

    H5Easy::File file(filename, HighFive::File::OpenOrCreate);

    Eigen::MatrixXd all_V = H5Easy::load<Eigen::MatrixXd>(file, "/points");
    assert(all_V.rows() % 8 == 0 && all_V.cols() == 3);
    size_t num_queries = all_V.rows() / 8;

    Eigen::MatrixXd all_rounded_V(all_V.rows(), all_V.cols());
    Eigen::VectorXd rounding_errors(all_V.rows());
    Eigen::Matrix<unsigned char, Eigen::Dynamic, 1> expected_results(
        all_V.rows());

    for (size_t i = 0; i < num_queries; i++) {
        Eigen::Matrix<double, 8, 3> V = all_V.middleRows<8>(8 * i);

        if (is_edge_edge) {
            doubleccd::ee_pair input_ee_pair(
                V.row(0), V.row(1), V.row(2), V.row(3), V.row(4), V.row(5),
                V.row(6), V.row(7));

            doubleccd::ee_pair rounded_ee_pair;
            rounding_errors[i]
                = doubleccd::shift_edge_edge(input_ee_pair, rounded_ee_pair);

            V.row(0) = rounded_ee_pair.a0;
            V.row(1) = rounded_ee_pair.a1;
            V.row(2) = rounded_ee_pair.b0;
            V.row(3) = rounded_ee_pair.b1;
            V.row(4) = rounded_ee_pair.a0b;
            V.row(5) = rounded_ee_pair.a1b;
            V.row(6) = rounded_ee_pair.b0b;
            V.row(7) = rounded_ee_pair.b1b;

            expected_results[i] = edgeEdgeCCD(
                V.row(0), V.row(1), V.row(2), V.row(3), V.row(4), V.row(5),
                V.row(6), V.row(7), CCDMethod::RATIONAL_ROOT_PARITY);
        } else {
            doubleccd::vf_pair input_vf_pair(
                V.row(0), V.row(1), V.row(2), V.row(3), V.row(4), V.row(5),
                V.row(6), V.row(7));

            doubleccd::vf_pair rounded_vf_pair;
            rounding_errors[i]
                = doubleccd::shift_vertex_face(input_vf_pair, rounded_vf_pair);

            V.row(0) = rounded_vf_pair.x0;
            V.row(1) = rounded_vf_pair.x1;
            V.row(2) = rounded_vf_pair.x2;
            V.row(3) = rounded_vf_pair.x3;
            V.row(4) = rounded_vf_pair.x0b;
            V.row(5) = rounded_vf_pair.x1b;
            V.row(6) = rounded_vf_pair.x2b;
            V.row(7) = rounded_vf_pair.x3b;

            expected_results[i] = vertexFaceCCD(
                V.row(0), V.row(1), V.row(2), V.row(3), V.row(4), V.row(5),
                V.row(6), V.row(7), CCDMethod::RATIONAL_ROOT_PARITY);
        }

        all_rounded_V.middleRows<8>(8 * i) = V;
    }

    H5Easy::dump(file, "/rounded/points", all_rounded_V);
    H5Easy::dump(file, "/rounded/result", expected_results);
    H5Easy::dump(file, "/rounded/error", rounding_errors);
}

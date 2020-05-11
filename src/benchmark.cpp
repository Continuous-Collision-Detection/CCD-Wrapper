// Time the different CCD methods

#include <string>

#include <Eigen/Core>
#include <boost/filesystem.hpp>
#include <fmt/format.h>
#include <highfive/H5Easy.hpp>
#include <igl/Timer.h>
#include <nlohmann/json.hpp>

#include <ccd.hpp>
#include <utils/get_rss.hpp>

using namespace ccd;

int parse_args(
    int argc,
    char* argv[],
    std::string& data_dir,
    bool& is_edge_edge,
    CCDMethod& method)
{
    const std::string usage = fmt::format(
        "usage: {} /path/to/data/ {{vf|ee}} {{0-{:d}}}", argv[0],
        NUM_CCD_METHODS - 1);
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
    if (argc < 4) {
        std::cerr << usage << std::endl
                  << "error: missing method of collision detection"
                  << std::endl;
        return 3;
    }
    int method_number = atoi(argv[3]);
    if (method_number < 0 || method_number >= NUM_CCD_METHODS) {
        std::cerr << usage << std::endl
                  << "error: invalid method of collision detection: " << argv[3]
                  << std::endl;
        std::cerr << "options:" << std::endl;
        for (int i = 0; i < NUM_CCD_METHODS; i++) {
            std::cout << i << ": " << method_names[i] << std::endl;
        }
        return 3;
    }

    data_dir = std::string(argv[1]);
    is_edge_edge = strcmp(argv[2], "ee") == 0;
    method = CCDMethod(method_number);
    return 0;
}

int main(int argc, char* argv[])
{
    std::string data_dir;
    bool is_edge_edge;
    CCDMethod method;
    bool use_shifted = true;
    int code = parse_args(argc, argv, data_dir, is_edge_edge, method);
    if (code) {
        return code;
    }

    igl::Timer timer;

    int num_queries = 0;
    double timing = 0.0;
    int false_positives = 0;
    int false_negatives = 0;

    for (auto& entry : boost::filesystem::directory_iterator(data_dir)) {
        if (boost::filesystem::extension(entry.path()) != ".hdf5"
            && boost::filesystem::extension(entry.path()) != ".h5") {
            continue;
        }
        H5Easy::File file(entry.path().string());

        const auto query_names = file.getGroup("/").listObjectNames();

        for (size_t i = 0; i < query_names.size(); i++) {
            std::string query_name = fmt::format(
                "{}{}", query_names[i], use_shifted ? "/shifted" : "");
            Eigen::Matrix<double, 8, 3> V
                = H5Easy::load<Eigen::Matrix<double, 8, 3>>(
                    file, fmt::format("{}/points", query_name));
            bool expected_results = H5Easy::load<unsigned char>(
                file, fmt::format("{}/result", query_name));

            // Time the methods
            bool result;
            timer.start();
            if (is_edge_edge) {
                result = edgeEdgeCCD(
                    V.row(0), V.row(1), V.row(2), V.row(3), V.row(4), V.row(5),
                    V.row(6), V.row(7), method);
            } else {
                result = vertexFaceCCD(
                    V.row(0), V.row(1), V.row(2), V.row(3), V.row(4), V.row(5),
                    V.row(6), V.row(7), method);
            }
            timer.stop();
            timing += timer.getElapsedTimeInMicroSec();

            // Count the inaccuracies
            if (result != expected_results) {
                if (result) {
                    false_positives++;
                } else {
                    false_negatives++;
                }
                if (method == CCDMethod::EXACT_RATIONAL_MIN_SEPARATION
                    || method == CCDMethod::EXACT_DOUBLE_MIN_SEPARATION
                    || method == CCDMethod::RATIONAL_ROOT_PARITY) {
                    std::cout
                        << fmt::format(
                               "file={} query_name={} method={} {}",
                               basename(entry.path()), method_names[method],
                               query_names[i],
                               result ? "false_positives" : "false_negatives")
                        << std::endl;
                }
            }
            std::cout << ++num_queries << "\r" << std::flush;
        }
    }

    std::string benchmark_file
        = (boost::filesystem::path(data_dir) / "benchmark.json").string();
    std::ifstream input(benchmark_file);
    nlohmann::json benchmark = nlohmann::json::parse(input);
    input.close();

    benchmark["collision_type"] = is_edge_edge ? "ee" : "vf";
    benchmark["num_queries"] = num_queries;
    benchmark[method_names[method]] = {
        { "avg_query_time", timing / num_queries },
        { "num_false_positives", false_positives },
        { "num_false_negatives", false_negatives },
    };

    std::ofstream(benchmark_file) << benchmark.dump(4);
}

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

int parse_args(
    int argc,
    char* argv[],
    std::string& data_dir,
    bool& is_edge_edge,
    ccd::CCDMethod& method)
{
    const std::string usage = fmt::format(
        "usage: {} /path/to/data/ {{vf|ee}} {{0-{:d}}}", argv[0],
        ccd::NUM_CCD_METHODS - 1);
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
    if (method_number < 0 || method_number >= ccd::NUM_CCD_METHODS) {
        std::cerr << usage << std::endl
                  << "error: invalid method of collision detection: " << argv[3]
                  << std::endl;
        std::cerr << "options:" << std::endl;
        for (int i = 0; i < ccd::NUM_CCD_METHODS; i++) {
            std::cout << i << ": " << ccd::method_names[i] << std::endl;
        }
        return 3;
    }

    data_dir = std::string(argv[1]);
    is_edge_edge = strcmp(argv[2], "ee") == 0;
    method = ccd::CCDMethod(method_number);
    return 0;
}

int main(int argc, char* argv[])
{
    std::string data_dir;
    bool is_edge_edge;
    ccd::CCDMethod method;
    int code = parse_args(argc, argv, data_dir, is_edge_edge, method);
    if (code) {
        return code;
    }

    igl::Timer timer;

    int num_queries = 0;
    double timing = 0.0;
    int false_positives = 0;
    int false_negatives = 0;

    int i = 0;
    for (auto& entry : boost::filesystem::directory_iterator(data_dir)) {
        if (boost::filesystem::extension(entry.path()) != ".hdf5"
            && boost::filesystem::extension(entry.path()) != ".h5") {
            continue;
        }
        H5Easy::File file(entry.path().string());

        HighFive::Group root = file.getGroup("/");
        num_queries += root.getNumberObjects();

        const auto query_names = root.listObjectNames();
        for (const auto query_name : query_names) {
            Eigen::Matrix<double, 8, 3> V
                = H5Easy::load<Eigen::Matrix<double, 8, 3>>(
                    file, fmt::format("{}/points", query_name));

            // Time the methods
            bool result;
            timer.start();
            if (is_edge_edge) {
                result = ccd::edgeEdgeCCD(
                    V.row(0), V.row(1), V.row(2), V.row(3), V.row(4), V.row(5),
                    V.row(6), V.row(7), ccd::CCDMethod(method));
            } else {
                result = ccd::vertexFaceCCD(
                    V.row(0), V.row(1), V.row(2), V.row(3), V.row(4), V.row(5),
                    V.row(6), V.row(7), ccd::CCDMethod(method));
            }
            timer.stop();
            timing += timer.getElapsedTimeInMicroSec();

            // Count the inaccuracies
            bool expected_result = H5Easy::load<unsigned char>(
                file, fmt::format("{}/result", query_name));
            if (result != expected_result) {
                if (result) {
                    false_positives++;
                } else {
                    false_negatives++;
                }
            }
            std::cout << i++ << "\r" << std::flush;
        }
    }

    std::string benchmark_file
        = (boost::filesystem::path(data_dir) / "benchmark.json").string();
    std::ifstream input(benchmark_file);
    nlohmann::json benchmark;
    if (input.good()) {
        benchmark = nlohmann::json::parse(input);
    }
    input.close();
    assert(
        !benchmark.contains("num_queries")
        || benchmark["num_queries"].get<int>() == num_queries);

    benchmark["collision_type"] = is_edge_edge ? "ee" : "vf";
    benchmark["num_queries"] = num_queries;
    benchmark[ccd::method_names[method]] = {
        { "avg_query_time", timing / num_queries },
        { "peak_memory", getPeakRSS() },
        { "num_false_positives", false_positives },
        { "num_false_negatives", false_negatives },
    };

    std::ofstream(benchmark_file) << benchmark.dump(4);
}

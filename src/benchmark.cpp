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

    long peak_memory = 0;

    Eigen::Matrix<double, 8, 3> V;
    for (auto& entry : boost::filesystem::directory_iterator(data_dir)) {
        if (boost::filesystem::extension(entry.path()) != ".hdf5"
            && boost::filesystem::extension(entry.path()) != ".h5") {
            continue;
        }
        Eigen::Matrix<double, Eigen::Dynamic, 3> all_V;
        Eigen::Matrix<bool, Eigen::Dynamic, 1> expected_results;

        // Load all the data into memory so we can get an accurate measure of
        // memory usage.
        H5Easy::File file(entry.path().string());
        const auto query_names = file.getGroup("/").listObjectNames();
        all_V.resize(8 * query_names.size(), 3);
        expected_results.resize(query_names.size());
        for (int i = 0; i < query_names.size(); i++) {
            std::string dir = fmt::format(
                "{}/{}", query_names[i], use_shifted ? "shifted/" : "");
            all_V.middleRows<8>(8 * i)
                = H5Easy::load<Eigen::Matrix<double, 8, 3>>(
                    file, fmt::format("{}points", dir));
            expected_results[i] = bool(H5Easy::load<unsigned char>(
                file, fmt::format("{}result", dir)));
        }

        // Measure jsut the memory of loading the queries
        long load_memory = getPeakRSS();

        for (int i = 0; i < expected_results.size(); i++) {
            V = all_V.middleRows<8>(8 * i);

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
            if (result != expected_results[i]) {
                if (result) {
                    false_positives++;
                } else {
                    false_negatives++;
                }
                if (method == CCDMethod::EXACT_RATIONAL_MIN_DISTANCE
                    || method == CCDMethod::EXACT_DOUBLE_MIN_DISTANCE
                    || method == CCDMethod::RATIONAL_ROOT_PARITY) {
                    std::cout
                        << fmt::format(
                               "method={} query_name={} {}",
                               method_names[method], query_names[i],
                               result ? "false_positives" : "false_negatives")
                        << std::endl;
                }
            }
            std::cout << num_queries++ << "\r" << std::flush;
        }

        peak_memory = std::max(peak_memory, long(getPeakRSS()) - load_memory);
    }

    assert(peak_memory >= 0);

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
    benchmark[method_names[method]] = {
        { "avg_query_time", timing / num_queries },
        { "peak_memory", peak_memory },
        { "num_false_positives", false_positives },
        { "num_false_negatives", false_negatives },
    };

    std::ofstream(benchmark_file) << benchmark.dump(4);
}

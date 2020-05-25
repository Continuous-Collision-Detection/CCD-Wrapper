// Time the different CCD methods

#include <filesystem>
#include <fstream>
#include <regex>
#include <string>

#include <CLI/CLI.hpp>
#include <Eigen/Core>
#include <fmt/format.h>
#include <highfive/H5Easy.hpp>
#include <igl/Timer.h>
#include <nlohmann/json.hpp>

#include <ccd.hpp>
#include <utils/get_rss.hpp>

using namespace ccd;

struct Args {
    std::string data_dir;
    bool is_edge_edge;
    CCDMethod method;
    double min_distance = DEFAULT_MIN_DISTANCE;
};

Args parse_args(int argc, char* argv[])
{
    Args args;

    CLI::App app { "CCD Wrapper Benchmark" };

    app.add_option("data_directory", args.data_dir, "/path/to/data/")
        ->required();

    std::string col_type;
    app.add_set("collision_type", col_type, { "vf", "ee" }, "type of collision")
        ->required();

    std::stringstream method_options;
    method_options << "CCD method\noptions:" << std::endl;
    for (int i = 0; i < NUM_CCD_METHODS; i++) {
        method_options << i << ": " << method_names[i] << std::endl;
    }
    app.add_option("CCD_method", args.method, method_options.str())->required();

    app.add_option("min_distance,-d", args.min_distance, "minimum distance");

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        exit(app.exit(e));
    }

    args.is_edge_edge = col_type == "ee";

    if (args.method < 0 || args.method >= NUM_CCD_METHODS) {
        exit(app.exit(CLI::Error(
            "",
            fmt::format(
                "invalid method of collision detection: {:d}", args.method))));
    }

    return args;
}

int main(int argc, char* argv[])
{
    Args args = parse_args(argc, argv);

    bool use_msccd = isMinSeparationMethod(args.method);

    igl::Timer timer;

    int num_queries = 0;
    double timing = 0.0;
    int false_positives = 0;
    int false_negatives = 0;

    for (auto& entry : std::filesystem::directory_iterator(args.data_dir)) {
        if (!std::regex_match(
                entry.path().string(), std::regex("(.*)-packed.hdf5"))) {
            continue;
        }
        H5Easy::File file(entry.path().string());

        Eigen::MatrixXd all_V
            = H5Easy::load<Eigen::MatrixXd>(file, "/rounded/points");
        assert(V.rows() % 8 == 0 && V.cols() == 3);
        Eigen::Matrix<unsigned char, Eigen::Dynamic, 1> expected_results
            = H5Easy::load<Eigen::Matrix<unsigned char, Eigen::Dynamic, 1>>(
                file, "/rounded/result");
        assert(V.rows() / 8 == expected_results.rows());

        for (size_t i = 0; i < expected_results.rows(); i++) {
            Eigen::Matrix<double, 8, 3> V = all_V.middleRows<8>(8 * i);
            bool expected_result = bool(expected_results(i));

            // Time the methods
            bool result;
            timer.start();
            if (use_msccd) {
                if (args.is_edge_edge) {
                    result = edgeEdgeMSCCD(
                        V.row(0), V.row(1), V.row(2), V.row(3), V.row(4),
                        V.row(5), V.row(6), V.row(7), args.min_distance,
                        args.method);
                } else {
                    result = vertexFaceMSCCD(
                        V.row(0), V.row(1), V.row(2), V.row(3), V.row(4),
                        V.row(5), V.row(6), V.row(7), args.min_distance,
                        args.method);
                }
            } else {
                if (args.is_edge_edge) {
                    result = edgeEdgeCCD(
                        V.row(0), V.row(1), V.row(2), V.row(3), V.row(4),
                        V.row(5), V.row(6), V.row(7), args.method);
                } else {
                    result = vertexFaceCCD(
                        V.row(0), V.row(1), V.row(2), V.row(3), V.row(4),
                        V.row(5), V.row(6), V.row(7), args.method);
                }
            }
            timer.stop();
            timing += timer.getElapsedTimeInMicroSec();

            // Count the inaccuracies
            if (result != expected_result) {
                if (result) {
                    false_positives++;
                } else {
                    false_negatives++;
                    if (args.method == CCDMethod::EXACT_RATIONAL_MIN_SEPARATION
                        || args.method
                            == CCDMethod::EXACT_DOUBLE_MIN_SEPARATION) {
                        std::cerr << fmt::format(
                                         "file={} index={:d} method={} "
                                         "false_negative",
                                         entry.path().string(), 8 * i,
                                         method_names[args.method])
                                  << std::endl;
                    }
                }
                if (args.method == CCDMethod::RATIONAL_ROOT_PARITY) {
                    std::cerr
                        << fmt::format(
                               "file={} index={:d} method={} {}",
                               entry.path().string(), 8 * i,
                               method_names[args.method],
                               result ? "false_positive" : "false_negative")
                        << std::endl;
                }
            }
            std::cout << ++num_queries << "\r" << std::flush;
        }
    }

    nlohmann::json benchmark;
    benchmark["collision_type"] = args.is_edge_edge ? "ee" : "vf";
    benchmark["num_queries"] = num_queries;
    std::string method_name = method_names[args.method];

    if (use_msccd) {
        std::string str_min_distane = fmt::format("{:g}", args.min_distance);
        benchmark[method_name]
            = { { str_min_distane,
                  {
                      { "avg_query_time", timing / num_queries },
                      { "num_false_positives", false_positives },
                      { "num_false_negatives", false_negatives },
                  } } };
    } else {
        benchmark[method_name] = {
            { "avg_query_time", timing / num_queries },
            { "num_false_positives", false_positives },
            { "num_false_negatives", false_negatives },
        };
    }

    std::string fname
        = (std::filesystem::path(args.data_dir) / "benchmark.json").string();
    {
        std::ifstream file(fname);
        if (file.good()) {
            nlohmann::json full_benchmark = nlohmann::json::parse(file);
            full_benchmark.merge_patch(benchmark);
            benchmark = full_benchmark;
        }
    }

    std::ofstream(fname) << benchmark.dump(4);
}

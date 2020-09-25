// Time the different CCD methods

#include <CLI/CLI.hpp>
#include <Eigen/Core>
#include <boost/filesystem.hpp>
#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include <ccd.hpp>
#include <utils/read_rational_csv.hpp>
#include <utils/timer.hpp>

using namespace ccd;

std::string root_path(CCD_WRAPPER_SAMPLE_QUERIES_DIR);
std::vector<std::string> simulation_folders
    = { { "chain", "cow-heads", "golf-ball", "mat-twist" } };
std::vector<std::string> handcrafted_folders
    = { { "erleben-sliding-spike", "erleben-spike-wedge",
          "erleben-sliding-wedge", "erleben-wedge-crack", "erleben-spike-crack",
          "erleben-wedges", "erleben-cube-cliff-edges", "erleben-spike-hole",
          "erleben-cube-internal-edges", "erleben-spikes", "unit-tests" } };
std::vector<std::string> fnames = { { "data_0_0.csv", "data_0_1.csv" } };

/*
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

void run_benchmark(int argc, char* argv[])
{
    Args args = parse_args(argc, argv);

    bool use_msccd = is_minimum_separation_method(args.method);
    std::cout << "method " << args.method << " out of " << NUM_CCD_METHODS
              << std::endl;

    int num_queries = 0;
    double timing = 0.0;
    int false_positives = 0;
    int false_negatives = 0;

    Timer timer;
    for (auto& entry : std::filesystem::directory_iterator(args.data_dir)) {
        if (entry.path().extension() != ".csv") {
            continue;
        }

        std::vector<bool> expected_results;
        Eigen::MatrixXd all_V
            = read_rational_csv(entry.path().string(), expected_results);
        assert(all_V.rows() % 8 == 0 && all_V.cols() == 3);
        assert(all_V.rows() / 8 == expected_results.rows());

        for (size_t i = 0; i < expected_results.size(); i++) {
            Eigen::Matrix<double, 8, 3> V = all_V.middleRows<8>(8 * i);
            bool expected_result = expected_results[i];

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
                    if (args.method
                            == CCDMethod::RATIONAL_MIN_SEPARATION_ROOT_PARITY
                        || args.method
                            == CCDMethod::MIN_SEPARATION_ROOT_PARITY) {
                        std::cerr << fmt::format(
                                         "file={} index={:d} method={} "
                                         "false_negative",
                                         entry.path().string(), 8 * i,
                                         method_names[args.method])
                                  << std::endl;
                    }
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
    std::cout << "false positives, " << false_positives << std::endl;
    std::cout << "false negatives, " << false_negatives << std::endl;
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
*/

void run_rational_data_single_method(
    const bool is_edge_edge,
    const bool is_simulation_data,
    const double minimum_seperation,
    const double tolerance,
    const CCDMethod method,
    const long max_itr = 1e6)
{
    std::cout << "\n******* Running method: " << method_names[method]
              << std::endl;
    bool use_msccd = is_minimum_separation_method(method);
    std::string sub_folder;
    Eigen::MatrixXd all_V;
    std::vector<bool> results;
    Timer timer;

    int total_number = -1;
    double new_timing = 0.0;
    int total_positives = 0;
    int new_false_positives = 0;
    int new_false_negatives = 0;

    int nbr_larger_tol = 0;
    int nbr_diff_tol = 0;
    double max_tol = 0;
    double sum_tol = 0;

    if (is_edge_edge) {
        sub_folder = "/edge-edge/";
    } else {
        sub_folder = "/vertex-face/";
    }
    int max_fnbr = is_simulation_data ? 4 : 11;
    const auto folders
        = is_simulation_data ? simulation_folders : handcrafted_folders;
    for (int fnbr = 0; fnbr < max_fnbr; fnbr++) {
        for (int ff = 0; ff < 2; ff++) {
            all_V = read_rational_csv(
                root_path + folders[fnbr] + sub_folder + fnames[ff], results);
            assert(all_V.rows() % 8 == 0 && all_V.cols() == 3);
            int v_size = all_V.rows() / 8;
            for (int i = 0; i < v_size; i++) {
                total_number += 1;
                Eigen::Matrix<double, 8, 3> V = all_V.middleRows<8>(8 * i);
                bool expected_result
                    = results[i * 8]; // args[k].result;// from mathematica

                bool new_result;

                const std::array<double, 3> err = { { -1, -1, -1 } };

                double toi;
                const double t_max = 1;

                double output_tolerance = tolerance;

                timer.start();
                if (use_msccd) {
                    if (is_edge_edge) {
                        new_result = edgeEdgeMSCCD(
                            V.row(0), V.row(1), V.row(2), V.row(3), V.row(4),
                            V.row(5), V.row(6), V.row(7), minimum_seperation,
                            method, tolerance);
                    } else {
                        new_result = vertexFaceMSCCD(
                            V.row(0), V.row(1), V.row(2), V.row(3), V.row(4),
                            V.row(5), V.row(6), V.row(7), minimum_seperation,
                            method, tolerance);
                    }
                } else {
                    if (is_edge_edge) {
                        new_result = edgeEdgeCCD(
                            V.row(0), V.row(1), V.row(2), V.row(3), V.row(4),
                            V.row(5), V.row(6), V.row(7), method, tolerance);
                    } else {
                        new_result = vertexFaceCCD(
                            V.row(0), V.row(1), V.row(2), V.row(3), V.row(4),
                            V.row(5), V.row(6), V.row(7), method, tolerance);
                    }
                }
                timer.stop();
                new_timing += timer.getElapsedTimeInMicroSec();
                std::cout << total_number << "\r" << std::flush;

                if (expected_result) {
                    total_positives++;
                }
                if (new_result != expected_result) {
                    if (new_result) {
                        new_false_positives++;
                    } else {
                        new_false_negatives++;
                        if (method == CCDMethod::TIGHT_INCLUSION) {
                            std::cout << "false negative, "
                                      << root_path + folders[fnbr] + sub_folder
                                    + fnames[ff]
                                      << ", " << i << std::endl;
                            for (int j = 0; j < 8; j++) {
                                std::cout << "v" << j << " " << V(j, 0) << ", "
                                          << V(j, 1) << ", " << V(j, 2)
                                          << std::endl;
                                if (j == 3)
                                    std::cout << std::endl;
                            }

                            std::cout << "is edge? " << is_edge_edge
                                      << std::endl
                                      << std::endl;

                            exit(0);
                        }
                    }
                }
            }
        }
    }

    std::cout << "total number, " << total_number + 1 << std::endl;
    std::cout << "total positives, " << total_positives << std::endl;
    std::cout << "is_edge_edge? , " << is_edge_edge << std::endl;
    std::cout << "new_false_positives, " << new_false_positives << std::endl;
    std::cout << "new_false_negatives, " << new_false_negatives << std::endl;
    std::cout << "average time, " << new_timing / double(total_number + 1)
              << std::endl
              << std::endl;
    std::cout << "total time, " << new_timing << std::endl << std::endl;
    std::cout << "** finished " << method_names[method] << std::endl;
}

void run_one_method_over_all_data(const CCDMethod method)
{
    double minimum_seperation = 0;
    double tolerance = 1e-6;
    long max_itr = 1e6;
    bool is_edge_edge;
    bool is_simu_data;

    is_edge_edge = true;
    is_simu_data = false;
    std::cout << "****Running handcrafted dataset, edge-edge: " << std::endl;
    run_rational_data_single_method(
        is_edge_edge, is_simu_data, minimum_seperation, tolerance, method,
        max_itr);
    is_edge_edge = false;
    is_simu_data = false;
    std::cout << "****Running handcrafted dataset, vertex-face: " << std::endl;
    run_rational_data_single_method(
        is_edge_edge, is_simu_data, minimum_seperation, tolerance, method,
        max_itr);
    is_edge_edge = true;
    is_simu_data = true;
    std::cout << "****Running simulation dataset, edge-edge: " << std::endl;
    run_rational_data_single_method(
        is_edge_edge, is_simu_data, minimum_seperation, tolerance, method,
        max_itr);
    is_edge_edge = false;
    is_simu_data = true;
    std::cout << "****Running simulation dataset, vertex-face: " << std::endl;
    run_rational_data_single_method(
        is_edge_edge, is_simu_data, minimum_seperation, tolerance, method,
        max_itr);
}

void run_all_methods()
{
    for (int i = 0; i < int(NUM_CCD_METHODS); i++) {
        CCDMethod method = CCDMethod(i);
        if (is_method_enabled(method)) {
            run_one_method_over_all_data(method);
        }
    }
}

int main(int argc, char* argv[]) { run_all_methods(); }

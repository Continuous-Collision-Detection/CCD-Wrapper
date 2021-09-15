// Time the different CCD methods

#include <CLI/CLI.hpp>
#include <Eigen/Core>
#include <fmt/color.h>
#include <fmt/format.h>
#include <ghc/fs_std.hpp> // filesystem
#include <nlohmann/json.hpp>

#include <ccd.hpp>
#include <utils/read_rational_csv.hpp>
#include <utils/timer.hpp>

using namespace ccd;

std::vector<std::string> simulation_folders
    = { { "chain", "cow-heads", "golf-ball", "mat-twist" } };
std::vector<std::string> handcrafted_folders
    = { { "erleben-sliding-spike", "erleben-spike-wedge",
          "erleben-sliding-wedge", "erleben-wedge-crack", "erleben-spike-crack",
          "erleben-wedges", "erleben-cube-cliff-edges", "erleben-spike-hole",
          "erleben-cube-internal-edges", "erleben-spikes", "unit-tests" } };

struct Args {
    fs::path data_dir = CCD_WRAPPER_SAMPLE_QUERIES_DIR;
    std::vector<CCDMethod> methods;
    double minimum_separation = 0;
    double tight_inclusion_tolerance = 1e-6;
    long tight_inclusion_max_iter = 1e6;
    bool run_ee_dataset = true;
    bool run_vf_dataset = true;
    bool run_simulation_dataset = true;
    bool run_handcrafted_dataset = true;
};

Args parse_args(int argc, char* argv[])
{
    Args args;

    CLI::App app { "CCD Benchmark" };

    app.add_option("--data,--queries", args.data_dir, "/path/to/data/")
        ->check(CLI::ExistingDirectory)
        ->default_val(args.data_dir);

    // std::string col_type;
    // app.add_set(
    //     "collision_type", col_type, { "vf", "ee" }, "type of collision")
    //     ->required();

    // Initialize args.methods;
    args.methods.reserve(int(NUM_CCD_METHODS));
    for (int i = 0; i < int(NUM_CCD_METHODS); i++) {
        args.methods.push_back(CCDMethod(i));
    }

    std::stringstream method_options;
    method_options << "CCD methods to benchmark\noptions:" << std::endl;
    for (int i = 0; i < NUM_CCD_METHODS; i++) {
        method_options << i << ": " << method_names[i] << std::endl;
    }

    app.add_option("-m,--methods", args.methods, method_options.str())
        ->check(CLI::Range(0, NUM_CCD_METHODS - 1))
        ->default_val(args.methods);

    app.add_option(
           "-d,--minimum-separation", args.minimum_separation,
           "minimum separation distance")
        ->default_val(args.minimum_separation);

    app.add_option(
           "--delta,--ti-tolerance", args.tight_inclusion_tolerance,
           "Tight Inclusion tolerance (δ)")
        ->default_val(args.tight_inclusion_tolerance);

    app.add_option(
           "--mi,--ti-max-iter", args.tight_inclusion_max_iter,
           "Tight Inclusion maximum iterations (mᵢ)")
        ->default_val(args.tight_inclusion_max_iter);

    app.add_flag(
        "!--no-ee", args.run_ee_dataset, "do not run the edge-edge dataset");
    app.add_flag(
        "!--no-vf", args.run_vf_dataset, "do not run the vertex-face dataset");

    app.add_flag(
        "!--no-simulation", args.run_simulation_dataset,
        "do not run the simulation dataset");
    app.add_flag(
        "!--no-handcrafted", args.run_handcrafted_dataset,
        "do not run the handcrafted dataset");

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        exit(app.exit(e));
    }

    return args;
}

/*
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
    for (auto& entry : fs::directory_iterator(args.data_dir)) {
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
    const Args& args,
    const CCDMethod method,
    const bool is_edge_edge,
    const bool is_simulation_data)
{
    bool use_msccd = is_minimum_separation_method(method);
    Eigen::MatrixXd all_V;
    std::vector<bool> results;
    Timer timer;

    int total_number = -1;
    double total_time = 0.0;
    int total_positives = 0;
    int num_false_positives = 0;
    int num_false_negatives = 0;

    std::string sub_folder = is_edge_edge ? "edge-edge" : "vertex-face";

    const std::vector<std::string>& scene_names
        = is_simulation_data ? simulation_folders : handcrafted_folders;

    for (const auto& scene_name : scene_names) {
        fs::path scene_path = args.data_dir / scene_name / sub_folder;
        if (!fs::exists(scene_path)) {
            std::cout << "Missing: " << scene_path.string() << std::endl;
            continue;
        }

        for (const auto& entry : fs::directory_iterator(scene_path)) {
            if (entry.path().extension() != ".csv") {
                continue;
            }

            // std::cout << "reading data from " << entry.path().string()
            //           << std::endl;
            all_V = read_rational_csv(entry.path().string(), results);
            assert(all_V.rows() % 8 == 0 && all_V.cols() == 3);

            int v_size = all_V.rows() / 8;
            for (int i = 0; i < v_size; i++) {
                total_number += 1;
                Eigen::Matrix<double, 8, 3> V = all_V.middleRows<8>(8 * i);
                bool expected_result = results[i * 8];

                bool result;
                timer.start();
                if (use_msccd) {
                    if (is_edge_edge) {
                        result = edgeEdgeMSCCD(
                            V.row(0), V.row(1), V.row(2), V.row(3), V.row(4),
                            V.row(5), V.row(6), V.row(7),
                            args.minimum_separation, method,
                            args.tight_inclusion_tolerance,
                            args.tight_inclusion_max_iter);
                    } else {
                        result = vertexFaceMSCCD(
                            V.row(0), V.row(1), V.row(2), V.row(3), V.row(4),
                            V.row(5), V.row(6), V.row(7),
                            args.minimum_separation, method,
                            args.tight_inclusion_tolerance,
                            args.tight_inclusion_max_iter);
                    }
                } else {
                    if (is_edge_edge) {
                        result = edgeEdgeCCD(
                            V.row(0), V.row(1), V.row(2), V.row(3), V.row(4),
                            V.row(5), V.row(6), V.row(7), method,
                            args.tight_inclusion_tolerance,
                            args.tight_inclusion_max_iter);
                    } else {
                        result = vertexFaceCCD(
                            V.row(0), V.row(1), V.row(2), V.row(3), V.row(4),
                            V.row(5), V.row(6), V.row(7), method,
                            args.tight_inclusion_tolerance,
                            args.tight_inclusion_max_iter);
                    }
                }
                timer.stop();
                total_time += timer.getElapsedTimeInMicroSec();
#ifndef CCD_WRAPPER_IS_CI_BUILD
                std::cout << total_number << "\r" << std::flush;
#endif

                if (expected_result) {
                    total_positives++;
                }
                if (result != expected_result) {
                    if (result) {
                        num_false_positives++;
                    } else {
                        num_false_negatives++;
                        if (method == CCDMethod::TIGHT_INCLUSION) {
                            fmt::print(
                                "false negative, {:s}, {:d}\nis edge-edge? {}",
                                entry.path().string(), i, is_edge_edge);
                            exit(1);
                        }
                    }
                }
            }
        }
    }

    fmt::print(
        "total # of queries: {:d}\n"
        "total positives: {:d}\n"
        "is_edge_edge?: {}\n"
        "# of false positives: {:d}\n"
        "# of false negatives: {:d}\n"
        "average time: {:g}μs\n\n",
        total_number + 1, total_positives, is_edge_edge, num_false_positives,
        num_false_negatives, total_time / double(total_number + 1));
}

void run_one_method_over_all_data(const Args& args, const CCDMethod method)
{
    if (args.run_handcrafted_dataset) {
        fmt::print(fmt::emphasis::bold, "Running handcrafted dataset:\n");
        if (args.run_vf_dataset) {
            std::cout << "Vertex-Face:" << std::endl;
            run_rational_data_single_method(
                args, method, /*is_edge_edge=*/false, /*is_simu_data=*/false);
        }
        if (args.run_ee_dataset) {
            std::cout << "Edge-Edge:" << std::endl;
            run_rational_data_single_method(
                args, method, /*is_edge_edge=*/true, /*is_simu_data=*/false);
        }
    }
    if (args.run_simulation_dataset) {
        fmt::print(fmt::emphasis::bold, "Running simulation dataset:\n");
        if (args.run_vf_dataset) {
            std::cout << "Vertex-Face:" << std::endl;
            run_rational_data_single_method(
                args, method, /*is_edge_edge=*/false, /*is_simu_data=*/true);
        }
        if (args.run_ee_dataset) {
            std::cout << "Edge-Edge:" << std::endl;
            run_rational_data_single_method(
                args, method, /*is_edge_edge=*/true, /*is_simu_data=*/true);
        }
    }
}

void run_all_methods(const Args& args)
{
    for (CCDMethod method : args.methods) {
        if (is_method_enabled(method)) {
            fmt::print(
                fmt::emphasis::bold | fmt::emphasis::underline,
                "Benchmarking {}\n", method_names[method]);
            run_one_method_over_all_data(args, method);
            fmt::print("finished {}\n", method_names[method]);
        } else {
            std::cerr << "CCD method " << method_names[method]
                      << " requested, but it is disabled" << std::endl;
        }
        std::cout << std::endl;
    }
}

int main(int argc, char* argv[])
{
    Args args = parse_args(argc, argv);
    run_all_methods(args);
}

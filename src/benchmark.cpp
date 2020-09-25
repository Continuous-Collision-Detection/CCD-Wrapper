// Time the different CCD methods

#include <filesystem>
#include <fstream>
#include <regex>
#include <string>

#include <CLI/CLI.hpp>
#include <Eigen/Core>
#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include <ccd.hpp>

#include <utils/get_rss.hpp>
#include <utils/rational.hpp>
#include <utils/timer.hpp>

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

// void run_benchmark(int argc, char* argv[])
// {
//     Args args = parse_args(argc, argv);
//
//     bool use_msccd = is_minimum_separation_method(args.method);
//     std::cout << "method, " << args.method << " out of " << NUM_CCD_METHODS
//               << std::endl;
//     Timer timer;
//
//     int num_queries = 0;
//     double timing = 0.0;
//     int false_positives = 0;
//     int false_negatives = 0;
//
//     for (auto& entry : std::filesystem::directory_iterator(args.data_dir)) {
//         if (entry.path().extension() != ".hdf5"
//             && entry.path().extension() != ".h5") {
//             continue;
//         }
//         H5Easy::File file(entry.path().string());
//
//         Eigen::MatrixXd all_V
//             = H5Easy::load<Eigen::MatrixXd>(file, "/rounded/points");
//         assert(all_V.rows() % 8 == 0 && all_V.cols() == 3);
//         Eigen::Matrix<unsigned char, Eigen::Dynamic, 1> expected_results
//             = H5Easy::load<Eigen::Matrix<unsigned char, Eigen::Dynamic, 1>>(
//                 file, "/rounded/result");
//         assert(all_V.rows() / 8 == expected_results.rows());
//
//         for (size_t i = 0; i < expected_results.rows(); i++) {
//             Eigen::Matrix<double, 8, 3> V = all_V.middleRows<8>(8 * i);
//             bool expected_result = bool(expected_results(i));
//
//             // Time the methods
//             bool result;
//             timer.start();
//             if (use_msccd) {
//                 if (args.is_edge_edge) {
//                     result = edgeEdgeMSCCD(
//                         V.row(0), V.row(1), V.row(2), V.row(3), V.row(4),
//                         V.row(5), V.row(6), V.row(7), args.min_distance,
//                         args.method);
//                 } else {
//                     result = vertexFaceMSCCD(
//                         V.row(0), V.row(1), V.row(2), V.row(3), V.row(4),
//                         V.row(5), V.row(6), V.row(7), args.min_distance,
//                         args.method);
//                 }
//             } else {
//                 if (args.is_edge_edge) {
//                     result = edgeEdgeCCD(
//                         V.row(0), V.row(1), V.row(2), V.row(3), V.row(4),
//                         V.row(5), V.row(6), V.row(7), args.method);
//                 } else {
//                     result = vertexFaceCCD(
//                         V.row(0), V.row(1), V.row(2), V.row(3), V.row(4),
//                         V.row(5), V.row(6), V.row(7), args.method);
//                 }
//             }
//             timer.stop();
//             timing += timer.getElapsedTimeInMicroSec();
//
//             // Count the inaccuracies
//             if (result != expected_result) {
//                 if (result) {
//                     false_positives++;
//                 } else {
//                     false_negatives++;
//                     if (args.method
//                             == CCDMethod::RATIONAL_MIN_SEPARATION_ROOT_PARITY
//                         || args.method
//                             == CCDMethod::MIN_SEPARATION_ROOT_PARITY) {
//                         std::cerr << fmt::format(
//                                          "file={} index={:d} method={} "
//                                          "false_negative",
//                                          entry.path().string(), 8 * i,
//                                          method_names[args.method])
//                                   << std::endl;
//                     }
//                 }
//                 if (args.method == CCDMethod::RATIONAL_ROOT_PARITY) {
//                     std::cerr
//                         << fmt::format(
//                                "file={} index={:d} method={} {}",
//                                entry.path().string(), 8 * i,
//                                method_names[args.method],
//                                result ? "false_positive" : "false_negative")
//                         << std::endl;
//                 }
//             }
//             std::cout << ++num_queries << "\r" << std::flush;
//         }
//     }
//
//     nlohmann::json benchmark;
//     benchmark["collision_type"] = args.is_edge_edge ? "ee" : "vf";
//     benchmark["num_queries"] = num_queries;
//     std::string method_name = method_names[args.method];
//
//     if (use_msccd) {
//         std::string str_min_distane = fmt::format("{:g}", args.min_distance);
//         benchmark[method_name]
//             = { { str_min_distane,
//                   {
//                       { "avg_query_time", timing / num_queries },
//                       { "num_false_positives", false_positives },
//                       { "num_false_negatives", false_negatives },
//                   } } };
//     } else {
//         benchmark[method_name] = {
//             { "avg_query_time", timing / num_queries },
//             { "num_false_positives", false_positives },
//             { "num_false_negatives", false_negatives },
//         };
//     }
//     std::cout << "false positives, " << false_positives << std::endl;
//     std::cout << "false negatives, " << false_negatives << std::endl;
//     std::string fname
//         = (std::filesystem::path(args.data_dir) / "benchmark.json").string();
//     {
//         std::ifstream file(fname);
//         if (file.good()) {
//             nlohmann::json full_benchmark = nlohmann::json::parse(file);
//             full_benchmark.merge_patch(benchmark);
//             benchmark = full_benchmark;
//         }
//     }
//
//     std::ofstream(fname) << benchmark.dump(4);
// }

std::string root_path = "/home/bolun1/interval/data/";
std::vector<std::string> simulation_folders
    = { { "chain", "cow-heads", "golf-ball", "mat-twist" } };
std::vector<std::string> handcrafted_folders
    = { { "erleben-sliding-spike", "erleben-spike-wedge",
          "erleben-sliding-wedge", "erleben-wedge-crack", "erleben-spike-crack",
          "erleben-wedges", "erleben-cube-cliff-edges", "erleben-spike-hole",
          "erleben-cube-internal-edges", "erleben-spikes", "unit-tests" } };
std::vector<std::string> fnames = { { "data_0_0.csv", "data_0_1.csv" } };

struct write_format {
    std::string file;
    int nbr;
    bool is_edge_edge;
    bool result;
    bool ground_truth;
    double time;
    int method;
};

Eigen::MatrixXd
read_rational_CSV(const std::string inputFileName, std::vector<bool>& results)
{
    // be careful, there are n lines which means there are n/8 queries, but has
    // n results, which means results are duplicated
    results.clear();
    std::vector<std::array<double, 3>> vs;
    vs.clear();
    std::ifstream infile;
    infile.open(inputFileName);
    std::array<double, 3> v;
    if (!infile.is_open()) {
        std::cout << "Path Wrong!!!!" << std::endl;
        std::cout << "path, " << inputFileName << std::endl;
        return Eigen::MatrixXd(1, 1);
    }

    int l = 0;
    while (infile) // there is input overload classfile
    {
        l++;
        std::string s;
        if (!getline(infile, s))
            break;
        if (s[0] != '#') {
            std::istringstream ss(s);
            std::array<std::string, 7> record; // the first six are one vetex,
                                               // the seventh is the result
            int c = 0;
            while (ss) {
                std::string line;
                if (!getline(ss, line, ','))
                    break;
                try {
                    record[c] = line;
                    c++;
                } catch (const std::invalid_argument e) {
                    std::cout << "NaN found in file " << inputFileName
                              << " line " << l << std::endl;
                    e.what();
                }
            }
            Rational rt;
            double x = rt.get_double(record[0], record[1]),
                   y = rt.get_double(record[2], record[3]),
                   z = rt.get_double(record[4], record[5]);
            v[0] = x;
            v[1] = y;
            v[2] = z;
            vs.push_back(v);
            if (record[6] != "1" && record[6] != "0") {
                std::cout
                    << "ERROR:result position should be 1 or 0, but it is "
                    << record[6] << std::endl;
            }
            results.push_back(std::stoi(record[6]));
        }
    }
    Eigen::MatrixXd all_v(vs.size(), 3);
    for (int i = 0; i < vs.size(); i++) {
        all_v(i, 0) = vs[i][0];
        all_v(i, 1) = vs[i][1];
        all_v(i, 2) = vs[i][2];
    }
    if (!infile.eof()) {
        std::cerr << "Could not read file " << inputFileName << "\n";
    }

    return all_v;
}

void run_rational_data_single_method(
    const bool is_edge_edge,
    const bool is_simulation_data,
    const double minimum_seperation,
    const double tolerance,
    const CCDMethod method,
    const int max_itr = 1e6)
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

    // std::vector<write_format> queryinfo;
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
            all_V = read_rational_CSV(
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
                const int CCD_TYPE
                    = 1; // 0, normal ccd method which only checks t = [0,1]; 1,
                         // ccd with max_itr and t=[0, t_max]
                timer.start();
                // if (is_edge_edge)
                // {
                //     new_result = edgeEdgeCCD_double(
                //         V.row(0), V.row(1), V.row(2), V.row(3), V.row(4),
                //         V.row(5), V.row(6), V.row(7),
                //         err, minimum_seperation, toi, tolerance, t_max,
                //         max_itr, output_tolerance, CCD_TYPE);
                //     //std::cout<<"edge edge check"<<std::endl;
                // }
                // else
                // {
                //     new_result = vertexFaceCCD_double(
                //         V.row(0), V.row(1), V.row(2), V.row(3), V.row(4),
                //         V.row(5), V.row(6), V.row(7),
                //         err, minimum_seperation, toi, tolerance, t_max,
                //         max_itr, output_tolerance, CCD_TYPE);
                // }

                if (use_msccd) {
                    if (is_edge_edge) {
                        new_result = edgeEdgeMSCCD(
                            V.row(0), V.row(1), V.row(2), V.row(3), V.row(4),
                            V.row(5), V.row(6), V.row(7), minimum_seperation,
                            method);
                    } else {
                        new_result = vertexFaceMSCCD(
                            V.row(0), V.row(1), V.row(2), V.row(3), V.row(4),
                            V.row(5), V.row(6), V.row(7), minimum_seperation,
                            method);
                    }
                } else {
                    if (is_edge_edge) {
                        new_result = edgeEdgeCCD(
                            V.row(0), V.row(1), V.row(2), V.row(3), V.row(4),
                            V.row(5), V.row(6), V.row(7), method);
                    } else {
                        new_result = vertexFaceCCD(
                            V.row(0), V.row(1), V.row(2), V.row(3), V.row(4),
                            V.row(5), V.row(6), V.row(7), method);
                    }
                }
                timer.stop();
                new_timing += timer.getElapsedTimeInMicroSec();
                // sum_tol += output_tolerance;
                // max_tol = std::max(output_tolerance, max_tol);
                // if (output_tolerance > tolerance)
                // {
                //     nbr_larger_tol++;
                // }
                // if (output_tolerance != tolerance)
                // {
                //     nbr_diff_tol++;
                // }
                std::cout << total_number << "\r" << std::flush;

                // write_format qif;
                // qif.method=0;
                // qif.nbr=i;
                // qif.result=new_result;
                // qif.ground_truth=expected_result;
                // qif.is_edge_edge=is_edge_edge;
                // qif.time=timer.getElapsedTimeInMicroSec();
                // qif.file=inputFileName;

                // queryinfo.push_back(qif);

                if (expected_result)
                    total_positives++;
                if (new_result != expected_result) {
                    // std::cout<<"\nresult don't match, groud,
                    // "<<expected_result<<" ours, "<<new_result<<std::endl;

                    if (new_result)
                        new_false_positives++;

                    if (new_result == 0) {
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
    // std::cout << "percentage of early return, " << double(nbr_diff_tol) /
    // total_number << std::endl; std::cout << "percentage of early and larger
    // tol return, " << double(nbr_larger_tol) / total_number << std::endl;
    // std::cout << "max tol, " << max_tol << std::endl;
    // std::cout << "average tol, " << sum_tol / total_number << std::endl
    //           << std::endl;
    //    intervalccd::print_time_1();
    // intervalccd::print_time_2();
    std::cout << "total time, " << new_timing << std::endl << std::endl;
    /// home/bolun1/interval/
    // write_summary(folder+"method"+std::to_string(method)+"_is_edge_edge_"+std::to_string(is_edge_edge)
    // +"_"+std::to_string(total_number+1)+tail+".csv",method,total_number+1,total_positives,is_edge_edge,new_false_positives,
    // new_false_negatives,new_timing/double(total_number+1));
    // write_results_csv(folder+"method"+std::to_string(method)+"_is_edge_edge_"+std::to_string(is_edge_edge)
    // +"_"+std::to_string(total_number+1)+"_queries"+tail+".csv",queryinfo);
    // if(method==CCDMethod::OURS){
    //     write_iteration_info(folder+"method"+std::to_string(method)+"_is_edge_edge_"+std::to_string(is_edge_edge)
    //     +"_"+std::to_string(total_number+1)+"_itration"+tail+".csv",double(nbr_diff_tol)/total_number,max_tol,sum_tol/total_number);
    // }
    std::cout << "** finished " << method_names[method] << std::endl;
}

void run_one_method_over_all_data(const CCDMethod method)
{
    double minimum_seperation = 0;
    double tolerance = 1e-6;
    int max_itr = 1e6;
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

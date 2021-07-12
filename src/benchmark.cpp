// Time the different CCD methods

#include <CLI/CLI.hpp>
#include <Eigen/Core>
#include <boost/filesystem.hpp>
#include <fmt/color.h>
#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include <ccd.hpp>
#include <utils/read_rational_csv.hpp>
#include <utils/timer.hpp>
#include <utils/rational.hpp>
#include <tight_inclusion/inclusion_ccd.hpp>
// #include <tight_inclusion/interval_ccd.hpp>
#include <CTCD.h>
using namespace ccd;

std::vector<std::string> simulation_folders
    = { { "chain", "cow-heads", "golf-ball", "mat-twist" } };
std::vector<std::string> handcrafted_folders
    = { { "erleben-sliding-spike", "erleben-spike-wedge",
          "erleben-sliding-wedge", "erleben-wedge-crack", "erleben-spike-crack",
          "erleben-wedges", "erleben-cube-cliff-edges", "erleben-spike-hole",
          "erleben-cube-internal-edges", "erleben-spikes", "unit-tests" } };

struct Args {
    boost::filesystem::path data_dir = CCD_WRAPPER_SAMPLE_QUERIES_DIR;
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

    // Initialize args.methods;
    args.methods.reserve(int(NUM_CCD_METHODS));
    for (int i = 0; i < int(NUM_CCD_METHODS); i++) {
        args.methods.push_back(CCDMethod(i));
    }

    CLI::App app { "CCD Benchmark" };

    std::string data_dir_str;
    app.add_option("--data,--queries", args.data_dir, "/path/to/data/")
        ->check(CLI::ExistingDirectory);

    // std::string col_type;
    // app.add_set(
    //     "collision_type", col_type, { "vf", "ee" }, "type of collision")
    //     ->required();

    std::stringstream method_options;
    method_options << "CCD methods to benchmark\noptions:" << std::endl;
    for (int i = 0; i < NUM_CCD_METHODS; i++) {
        method_options << i << ": " << method_names[i] << std::endl;
    }
    app.add_option("-m,--methods", args.methods, method_options.str())
        ->check(CLI::Range(0, NUM_CCD_METHODS - 1));

    app.add_option(
        "-d,--minimum-separation", args.minimum_separation,
        "minimum separation distance");

    app.add_option(
        "--delta,--ti-tolerance", args.tight_inclusion_tolerance,
        "Tight Inclusion tolerance (δ)");

    app.add_option(
        "--mi,--ti-max-iter", args.tight_inclusion_max_iter,
        "Tight Inclusion maximum iterations (mᵢ)");

    bool no_ee = false;
    app.add_flag("--no-ee", no_ee, "do not run the edge-edge dataset");
    bool no_vf = false;
    app.add_flag("--no-vf", no_vf, "do not run the vertex-face dataset");

    bool no_simulation = false;
    app.add_flag(
        "--no-simulation", no_simulation, "do not run the simulation dataset");
    bool no_handcrafted = false;
    app.add_flag(
        "--no-handcrafted", no_handcrafted,
        "do not run the handcrafted dataset");

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        exit(app.exit(e));
    }

    // args.data_dir = boost::filesystem::path(data_dir_str);

    args.run_ee_dataset = !no_ee;
    args.run_vf_dataset = !no_vf;
    args.run_simulation_dataset = !no_simulation;
    args.run_handcrafted_dataset = !no_handcrafted;

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
    for (auto& entry : boost::filesystem::directory_iterator(args.data_dir)) {
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



void write_summary(
    const std::string file,
    const int method,
    const int total_number,
    const int positives,
    const bool is_edge_edge,
    const int fp,
    const int fn,
    const double average_time,
    const double time_lower,
    const double time_upper)
{
    std::ofstream fout;
    fout.open(file);
    fout << "method,total_nbr,positives,is_edge_edge,fp,fn,average_time, time_lower, time_upper"
         << std::endl;
    fout << method << "," << total_number << "," << positives << ","
         << is_edge_edge << "," << fp << "," << fn << ',' << average_time<<","<<time_lower<<","<<time_upper
         << std::endl;
    // fout<<"method, "<<method<<std::endl;
    // fout<<"total nbr, "<<total_number<<std::endl;
    // fout<<"positives, "<<positives<<std::endl;
    // fout<<"is_edge_edge, "<<is_edge_edge<<std::endl;
    // fout<<"fp, "<<fp<<std::endl;
    // fout<<"fn, "<<fn<<std::endl;
    // fout<<"average time, "<<average_time<<std::endl;
    fout.close();
}
struct write_format {
    std::string file;
    int nbr;
    bool is_edge_edge;
    bool result;
    bool ground_truth;
    double time;
    int method;
};
void write_results_csv(
    const std::string& file, const std::vector<write_format>& qs)
{

    std::ofstream fout;
    fout.open(file);
    fout << "file,nbr,method,is_edge_edge,result,ground_truth,time"
         << std::endl;
    for (int i = 0; i < qs.size(); i++) {
        fout << qs[i].file << "," << qs[i].nbr << "," << qs[i].method << ","
             << qs[i].is_edge_edge << "," << qs[i].result << ","
             << qs[i].ground_truth << "," << qs[i].time << std::endl;
    }
    fout.close();
}
void write_iteration_info(
    const std::string file,
    const double ratio,
    const double max_tol,
    const double avg_tol)
{
    std::ofstream fout;
    fout.open(file);
    fout << "ratio,max_tol,avg_tol" << std::endl;
    fout << ratio << "," << max_tol << "," << avg_tol << std::endl;

    fout.close();
}
void write_csv(const std::string& file, const std::vector<std::string> titles,const std::vector<double> data) {
	std::ofstream fout;
	fout.open(file);
	for (int i = 0; i < titles.size()-1; i++) {
	fout << titles[i] << ",";
		}
	fout << titles.back() << std::endl;
        for (int i = 0; i < data.size() - 1; i++) {
		fout << data[i] << ",";
			}
	fout << data.back() << std::endl;
	fout.close();
}
//#############################
// this is a value used to control if write info for per query
bool WRITE_QUERY_INFO=false;
// this is to control if write iteration info
bool WRITE_ITERATION_INFO=false;
// this is to print all the result and timing
bool WRITE_ALL_TIMING=false;
bool WRITE_STATISTIC=true;
bool DEBUG_FLAG=false;
std::ofstream pout;
//#############################
void run_rational_data_single_method(
    const Args& args,
    const CCDMethod method,
    const bool is_edge_edge,
    const bool is_simulation_data, const std::string folder="", const std::string tail="")
{
    bool use_msccd = is_minimum_separation_method(method);
    Eigen::MatrixXd all_V;
    std::vector<bool> results;
    Timer timer;
    std::vector<write_format> queryinfo;
    int total_number = -1;
    double total_time = 0.0;
    int total_positives = 0;
    int num_false_positives = 0;
    int num_false_negatives = 0;
    double time_lower=1e100;
    double time_upper=-1;
    std::string sub_folder = is_edge_edge ? "/edge-edge/" : "/vertex-face/";
    int nbr_larger_tol = 0;
    int nbr_diff_tol = 0;
    double max_tol = 0;
    double sum_tol = 0;
    long  queue_size_avg=0;
    long  queue_size_max=0;
    long  current_queue_size=0;
    long long queue_size_total=0;
    const std::vector<std::string>& scene_names
        = is_simulation_data ? simulation_folders : handcrafted_folders;
    if(WRITE_ALL_TIMING){
        pout.open(folder+"edge_"+std::to_string(is_edge_edge)+"simu_"+std::to_string(is_simulation_data)+tail+"_all_info.csv");
        pout<<"result,timing\n";
    }
    
    for (const auto& scene_name : scene_names) {
        boost::filesystem::path scene_path(
            args.data_dir / scene_name / sub_folder);
        if (!boost::filesystem::exists(scene_path)) {
            std::cout << "Missing: " << scene_path.string() << std::endl;
            continue;
        }
        std::string dbgname="erleben-cube-cliff-edges";
        std::string fname="edge-edge-0004.csv";
        int dbgnbr=3;
        if(DEBUG_FLAG && scene_name!=dbgname){
            continue;
        }
        for (const auto& entry :
             boost::filesystem::directory_iterator(scene_path)) {
            if (entry.path().extension() != ".csv") {
                continue;
            }
            std::string flagname="/home/bolun1/interval/fixed_data/rounded/"+dbgname+sub_folder+fname;
            if(DEBUG_FLAG && entry.path().string()!=flagname){
                continue;
            }   
            // std::cout << "reading data from " << entry.path().string()
            //           << std::endl;
            all_V = read_rational_csv(entry.path().string(), results);
            assert(all_V.rows() % 8 == 0 && all_V.cols() == 3);

            int v_size = all_V.rows() / 8;
            for (int i = 0; i < v_size; i++) {
                if (DEBUG_FLAG && i!=dbgnbr){
                    continue;
                }
                total_number += 1;
                Eigen::Matrix<double, 8, 3> V = all_V.middleRows<8>(8 * i);
                bool expected_result = results[i * 8];

                bool result;
                timer.start();
                long round_nbr=0;
                for(int ri=0;ri<1;ri++){

                    round_nbr+=1;
                    if (method==CCDMethod::TIGHT_INCLUSION && WRITE_ITERATION_INFO){
                    const std::array<double, 3> err = { { -1, -1, -1 } };
                    double toi;
                    const double t_max = 1;

                    double output_tolerance = args.tight_inclusion_tolerance;
                    if (is_edge_edge){
                        result=edgeEdgeCCD_OURS( V.row(0), V.row(1), V.row(2), V.row(3), V.row(4),
                            V.row(5), V.row(6), V.row(7), err, args.minimum_separation,
                            toi, args.tight_inclusion_tolerance,
                            t_max, args.tight_inclusion_max_iter, output_tolerance,
                            1);
                    }
                    else{
                         result=vertexFaceCCD_OURS( V.row(0), V.row(1), V.row(2), V.row(3), V.row(4),
                            V.row(5), V.row(6), V.row(7), err, args.minimum_separation,
                            toi, args.tight_inclusion_tolerance,
                            t_max, args.tight_inclusion_max_iter, output_tolerance,
                            1);
                    }
                    sum_tol += output_tolerance;
                    max_tol = std::max(output_tolerance, max_tol);
                    if (output_tolerance > args.tight_inclusion_tolerance) {
                        nbr_larger_tol++;
                    }
                    if (output_tolerance != args.tight_inclusion_tolerance) {
                        nbr_diff_tol++;
                    }


                }

                else{



                if(method==CCDMethod::FLOATING_POINT_ROOT_FINDER && args.minimum_separation>0){
                    double toi;
                    if (is_edge_edge) {
                        result = CTCD::edgeEdgeCTCD(
                            V.row(0), V.row(1), V.row(2), V.row(3), V.row(4),
                            V.row(5), V.row(6), V.row(7),
                            args.minimum_separation, toi);
                    } else {
                        result = CTCD::vertexFaceCTCD(
                            V.row(0), V.row(1), V.row(2), V.row(3), V.row(4),
                            V.row(5), V.row(6), V.row(7),
                            args.minimum_separation, toi);
                    }
                    
                }
                else{
                    if (use_msccd) {
                    // if(method==CCDMethod::TIGHT_INCLUSION){
                    //     if (is_edge_edge) {
                    //         result = edgeEdgeMSCCD(
                    //             V.row(0), V.row(1), V.row(2), V.row(3), V.row(4),
                    //             V.row(5), V.row(6), V.row(7),
                    //             args.minimum_separation, method,
                    //             args.tight_inclusion_tolerance,
                    //             args.tight_inclusion_max_iter);
                    //     } else {
                    //         result = vertexFaceMSCCD(
                    //             V.row(0), V.row(1), V.row(2), V.row(3), V.row(4),
                    //             V.row(5), V.row(6), V.row(7),
                    //             args.minimum_separation, method,
                    //             args.tight_inclusion_tolerance,
                    //             args.tight_inclusion_max_iter);
                    //     }
                    // }
                    
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
                }
                }
                double tt=timer.getElapsedTimeInMicroSec();
                if(tt>10) break;
                }// ri

                
                
                timer.stop();
                double this_time=timer.getElapsedTimeInMicroSec()/round_nbr;
                total_time += this_time;
#ifndef CCD_WRAPPER_IS_CI_BUILD
                std::cout << total_number << "\r" << std::flush;
#endif
                current_queue_size=inclusion_ccd::return_queue_size();
                if(current_queue_size>queue_size_max){
                    queue_size_max=current_queue_size;
                }
                //queue_size_total+=current_queue_size;
                if(DEBUG_FLAG){
                    std::cout<<"result, "<<result<<std::endl;
                    std::cout<<V<<std::endl;
                }
                if(WRITE_ALL_TIMING){
                    pout<<result<<","<<this_time<<"\n";
                }
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
                            //exit(1);
                        }
                    }
                }
        if(time_upper<this_time){
            time_upper=this_time;
            std::cout<<"upper get updated, "<<time_upper<<std::endl;
        }
        if(time_lower>this_time){
            time_lower=this_time;
        }
        write_format qif;
        qif.method = int(method);
        qif.nbr = i;
        qif.result = result;
        qif.ground_truth = expected_result;
        qif.is_edge_edge = is_edge_edge;
        qif.time = this_time;
        qif.file = entry.path().string();
        queryinfo.push_back(qif);
            }
        }
    }
        if(WRITE_ALL_TIMING){
            pout.close();
            std::string fff="file got wrote ";
            std::string print_info=fff+
            "edge_"+std::to_string(is_edge_edge)+"simu_"+std::to_string(is_simulation_data)+tail+
            "_all_info.csv";
            std::cout<<print_info<<std::endl;
        }
        if(!DEBUG_FLAG&&WRITE_STATISTIC){
        write_summary(
            folder + "method" + std::to_string(int(method)) + "_is_edge_edge_"
            + std::to_string(is_edge_edge) + "_"
            + std::to_string(total_number + 1) + tail + ".csv",
        method, total_number + 1, total_positives, is_edge_edge,
        num_false_positives, num_false_negatives,
        total_time / double(total_number + 1),time_lower,time_upper);
        }
        
        if(WRITE_QUERY_INFO){
         write_results_csv(
        folder + "method" + std::to_string(method) + "_is_edge_edge_"
            + std::to_string(is_edge_edge) + "_"
            + std::to_string(total_number + 1) + "_queries" + tail + ".csv",
        queryinfo);
        }
        if (WRITE_ITERATION_INFO) {
        write_iteration_info(
            folder + "method" + std::to_string(method) + "_is_edge_edge_"
                + std::to_string(is_edge_edge) + "_"
                + std::to_string(total_number + 1) + "_itration" + tail
                + ".csv",
            double(nbr_diff_tol) / total_number, max_tol,
            sum_tol / total_number);
    }
    if(1){
        std::cout<<"start writting queue info"<<std::endl;
        queue_size_avg=queue_size_total/(total_number + 1);
        std::vector<std::string> titles={{"max","avg"}};
        std::vector<double> queue_info={{double(queue_size_max),double(queue_size_avg)}};
        
        write_csv(folder + "method" + std::to_string(method) + "_is_edge_edge_"
                + std::to_string(is_edge_edge) + "_"
                + std::to_string(total_number + 1) + "_queue_info" + tail
                + ".csv",titles,queue_info);
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

void run_one_method_over_all_data(const Args& args, const CCDMethod method,
const std::string folder="", const std::string tail="")
{
    if (args.run_handcrafted_dataset) {
        fmt::print(fmt::emphasis::bold, "Running handcrafted dataset:\n");
        if (args.run_vf_dataset) {
            std::cout << "Vertex-Face:" << std::endl;
            run_rational_data_single_method(
                args, method, /*is_edge_edge=*/false, /*is_simu_data=*/false,folder,tail);
        }
        if (args.run_ee_dataset) {
            std::cout << "Edge-Edge:" << std::endl;
            run_rational_data_single_method(
                args, method, /*is_edge_edge=*/true, /*is_simu_data=*/false,folder,tail);
        }
    }
    if (args.run_simulation_dataset) {
        fmt::print(fmt::emphasis::bold, "Running simulation dataset:\n");
        if (args.run_vf_dataset) {
            std::cout << "Vertex-Face:" << std::endl;
            run_rational_data_single_method(
                args, method, /*is_edge_edge=*/false, /*is_simu_data=*/true,folder,tail);
        }
        if (args.run_ee_dataset) {
            std::cout << "Edge-Edge:" << std::endl;
            run_rational_data_single_method(
                args, method, /*is_edge_edge=*/true, /*is_simu_data=*/true,folder,tail);
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
void run_three_methods_diff_delta(int select)
{

    std::string folder = "/home/bolun1/interval/data1207/diff_delta/";// this is the output folder
    std::string tail = "";
    double ms = 0;
    std::vector<std::string> tolerance;
    bool ms_flag = true;
    int max_itr = 1e6;
    // tolerance.push_back("1");
    Args arg;
    arg.data_dir="/home/zachary/Development/ccd/ccd-queries/";
    arg.minimum_separation=0;
    for (int i = 0; i < 9; i++) {
        tolerance.push_back("1e-" + std::to_string(i));
    }

    for (int i = 0; i < tolerance.size(); i++) { // temporarly changed to run the last data
        arg.tight_inclusion_tolerance= std::stod(tolerance[i]);
        tail="_"+tolerance[i];
        if (select==0){
            arg.methods.clear();
            std::cout<<"running method "<<CCDMethod::TIGHT_INCLUSION<<std::endl;
        arg.methods.push_back(CCDMethod::TIGHT_INCLUSION);
        run_one_method_over_all_data(arg, CCDMethod::TIGHT_INCLUSION,folder,tail);
        
        }
        if (select==1){
            std::cout<<"running method "<<CCDMethod::MULTIVARIATE_INTERVAL_ROOT_FINDER<<std::endl;
            arg.methods.clear();
        arg.methods.push_back(CCDMethod::MULTIVARIATE_INTERVAL_ROOT_FINDER);
        run_one_method_over_all_data(arg, CCDMethod::MULTIVARIATE_INTERVAL_ROOT_FINDER,folder,tail);
        }
        if (select==2)
        {
            std::cout<<"running method "<<CCDMethod::UNIVARIATE_INTERVAL_ROOT_FINDER<<std::endl;
            arg.methods.clear();
            arg.methods.push_back(CCDMethod::UNIVARIATE_INTERVAL_ROOT_FINDER);
        run_one_method_over_all_data(arg, CCDMethod::UNIVARIATE_INTERVAL_ROOT_FINDER,folder,tail);
        }
    }
}

void run_all_methods_for_handcrafted_table(){
    std::string folder = "/home/bolun1/interval/data1207/all_methods/";// this is the output folder
    std::string tail = "";
    Args arg;
    arg.data_dir="/home/bolun1/interval/fixed_data/ccd-queries/";
    arg.methods.clear();
    arg.methods.push_back(CCDMethod::FLOATING_POINT_ROOT_FINDER);
    arg.methods.push_back(CCDMethod::MIN_SEPARATION_ROOT_FINDER);
    arg.methods.push_back(CCDMethod::ROOT_PARITY);
    arg.methods.push_back(CCDMethod::RATIONAL_ROOT_PARITY);
    arg.methods.push_back(CCDMethod::BSC);
    arg.methods.push_back(CCDMethod::TIGHT_CCD);
    arg.methods.push_back(CCDMethod::UNIVARIATE_INTERVAL_ROOT_FINDER);
    arg.methods.push_back(CCDMethod::MULTIVARIATE_INTERVAL_ROOT_FINDER);
    arg.methods.push_back(CCDMethod::TIGHT_INCLUSION);
    arg.minimum_separation=0;
    arg.tight_inclusion_tolerance=1e-6;
    arg.tight_inclusion_max_iter = 1e6;
    arg.run_ee_dataset = true;
    arg.run_vf_dataset = true;
    arg.run_simulation_dataset = false; // not running simulation
    arg.run_handcrafted_dataset = true;
    for (CCDMethod method : arg.methods) {
        if (is_method_enabled(method)) {
            fmt::print(
                fmt::emphasis::bold | fmt::emphasis::underline,
                "Benchmarking {}\n", method_names[method]);
            run_one_method_over_all_data(arg, method,folder,tail);
            fmt::print("finished {}\n", method_names[method]);
        } else {
            std::cerr << "CCD method " << method_names[method]
                      << " requested, but it is disabled" << std::endl;
        }
        std::cout << std::endl;
    }
    // run_one_method_over_all_data(arg, CCDMethod::TIGHT_INCLUSION,folder,tail);
}
void run_ours_for_handcrafted_table(){
    std::string folder = "/home/bolun1/interval/data1207/ours_queryinfo/";// this is the output folder
    std::string tail = "";

    

    // tolerance.push_back("1");
    Args arg;
    arg.data_dir="/home/bolun1/interval/fixed_data/ccd-queries/";
    
    arg.methods.clear();
    arg.methods.push_back(CCDMethod::TIGHT_INCLUSION);// only run ours

    arg.minimum_separation=0;
    arg.tight_inclusion_tolerance=1e-6;
    arg.tight_inclusion_max_iter = 1e6;
    arg.run_ee_dataset = true;
    arg.run_vf_dataset = true;
    arg.run_simulation_dataset = false; // not running simulation
    arg.run_handcrafted_dataset = true;

    WRITE_QUERY_INFO=true;// write query info

    for (CCDMethod method : arg.methods) {
        if (is_method_enabled(method)) {
            fmt::print(
                fmt::emphasis::bold | fmt::emphasis::underline,
                "Benchmarking {}\n", method_names[method]);
            run_one_method_over_all_data(arg, method,folder,tail);
            fmt::print("finished {}\n", method_names[method]);
        } else {
            std::cerr << "CCD method " << method_names[method]
                      << " requested, but it is disabled" << std::endl;
        }
        std::cout << std::endl;
    }
    // run_one_method_over_all_data(arg, CCDMethod::TIGHT_INCLUSION,folder,tail);
}

void run_ours_diff_tol_for_simulation_data(){
    std::string folder = "/home/bolun1/interval/data1207/diff_delta_new/";// this is the output folder
    std::string tail = "";


    // tolerance.push_back("1");
    Args arg;
    arg.data_dir="/home/bolun1/interval/fixed_data/ccd-queries/";
    
    arg.methods.clear();
    arg.methods.push_back(CCDMethod::TIGHT_INCLUSION);// only run ours

    arg.minimum_separation=0;
    // arg.tight_inclusion_tolerance=1e-6;
    arg.tight_inclusion_max_iter = 1e6;
    arg.run_ee_dataset = true;
    arg.run_vf_dataset = true;
    arg.run_simulation_dataset = true; // running simulation
    arg.run_handcrafted_dataset = false;
    WRITE_ALL_TIMING=true;
    // WRITE_QUERY_INFO=true;// write query info
    
    std::vector<std::string> tolerance; // different tolerance
    for (int i = 0; i < 9; i++) {
        tolerance.push_back("1e-" + std::to_string(i));
    }

    // now debugging
    for (int i = 0; i < tolerance.size(); i++) { // temporarly changed to run the last data
        arg.tight_inclusion_tolerance= std::stod(tolerance[i]);
        tail="_"+tolerance[i];
       
           
        run_one_method_over_all_data(arg, CCDMethod::TIGHT_INCLUSION,folder,tail);
        
        

    }
    
}
void run_ms_methods_for_all_data(){
    std::string folder = "/home/bolun1/interval/data1207/ms_table/";// this is the output folder
    std::string tail = "";


    // tolerance.push_back("1");
    Args arg;
    arg.data_dir="/home/bolun1/interval/fixed_data/ccd-queries/";
    
    arg.methods.clear();
    arg.methods.push_back(CCDMethod::TIGHT_INCLUSION);
    arg.methods.push_back(CCDMethod::MIN_SEPARATION_ROOT_FINDER);
    arg.methods.push_back(CCDMethod::FLOATING_POINT_ROOT_FINDER);

    // arg.minimum_separation=0;
    arg.tight_inclusion_tolerance=1e-6;
    arg.tight_inclusion_max_iter = 1e6;
    arg.run_ee_dataset = true;
    arg.run_vf_dataset = true;
    arg.run_simulation_dataset = true; 
    arg.run_handcrafted_dataset = true;

    

    std::vector<std::string> ms; // different ms
 
    ms.push_back("1e-2" );
    ms.push_back("1e-8" );
    ms.push_back("1e-16" );
    ms.push_back("1e-30" );
    ms.push_back("1e-100" );
   
    for (int i = 0; i < ms.size(); i++) { // temporarly changed to run the last data
        arg.minimum_separation= std::stod(ms[i]);
        tail="_"+ms[i];
        
        run_one_method_over_all_data(arg, CCDMethod::TIGHT_INCLUSION,folder,tail);
        run_one_method_over_all_data(arg, CCDMethod::MIN_SEPARATION_ROOT_FINDER,folder,tail);
        run_one_method_over_all_data(arg, CCDMethod::FLOATING_POINT_ROOT_FINDER,folder,tail);
        
      
        
    }
}

// start should be 0, 20, 30, 40, 50
void run_our_method_diff_ms_for_simulation_data( int start){
    std::string folder = "/home/bolun1/interval/data1207/our_ms_new/";// this is the output folder
    std::string tail = "";


    // tolerance.push_back("1");
    Args arg;
    arg.data_dir="/home/bolun1/interval/fixed_data/ccd-queries/";
    
    arg.methods.clear();
    arg.methods.push_back(CCDMethod::TIGHT_INCLUSION);


    // arg.minimum_separation=0;
    arg.tight_inclusion_tolerance=1e-6;
    arg.tight_inclusion_max_iter = 1e6;
    arg.run_ee_dataset = true;
    arg.run_vf_dataset = true;
    arg.run_simulation_dataset = true; 
    arg.run_handcrafted_dataset = false;
    WRITE_ALL_TIMING=true;
    // WRITE_QUERY_INFO=true;// write query info
    
    

    std::vector<std::string> ms; // different ms
    
    for (int i = start; i < start+20; i++) {
        ms.push_back("1e-" + std::to_string(i));
    }
   
    for (int i = 0; i < ms.size(); i++) { // temporarly changed to run the last data
        arg.minimum_separation= std::stod(ms[i]);
        tail="_"+ms[i];

        run_one_method_over_all_data(arg, CCDMethod::TIGHT_INCLUSION,folder,tail);
      
    }
    
}
void run_ours_handcrafted_without_mi(){
    std::string folder = "/home/bolun1/interval/data1207/ours_disable_mi/";// this is the output folder
    std::string tail = "";

    

    // tolerance.push_back("1");
    Args arg;
    arg.data_dir="/home/bolun1/interval/fixed_data/ccd-queries/";
    
    arg.methods.clear();
    arg.methods.push_back(CCDMethod::TIGHT_INCLUSION);// only run ours

    arg.minimum_separation=0;
    arg.tight_inclusion_tolerance=1e-6;
    arg.tight_inclusion_max_iter = -1;// disable mi
    arg.run_ee_dataset = true;
    arg.run_vf_dataset = true;
    arg.run_simulation_dataset = false; // not running simulation
    arg.run_handcrafted_dataset = true;

    WRITE_QUERY_INFO=false;// write query info
    WRITE_ITERATION_INFO=true;//write iteration info
    for (CCDMethod method : arg.methods) {
        if (is_method_enabled(method)) {
            fmt::print(
                fmt::emphasis::bold | fmt::emphasis::underline,
                "Benchmarking {}\n", method_names[method]);
            run_one_method_over_all_data(arg, method,folder,tail);
            fmt::print("finished {}\n", method_names[method]);
        } else {
            std::cerr << "CCD method " << method_names[method]
                      << " requested, but it is disabled" << std::endl;
        }
        std::cout << std::endl;
    }
}
void run_ours_handcrafted_iteration_info(){
    std::string folder = "/home/bolun1/interval/data1207/ours_itr/";// this is the output folder
    std::string tail = "";

    

    // tolerance.push_back("1");
    Args arg;
    arg.data_dir="/home/bolun1/interval/fixed_data/ccd-queries/";
    
    arg.methods.clear();
    arg.methods.push_back(CCDMethod::TIGHT_INCLUSION);// only run ours

    arg.minimum_separation=0;
    arg.tight_inclusion_tolerance=1e-6;
    arg.tight_inclusion_max_iter = 1e6;// disable mi
    arg.run_ee_dataset = true;
    arg.run_vf_dataset = true;
    arg.run_simulation_dataset = false; // not running simulation
    arg.run_handcrafted_dataset = true;

    WRITE_QUERY_INFO=false;// write query info
    WRITE_ITERATION_INFO=true;//write iteration info
    for (CCDMethod method : arg.methods) {
        if (is_method_enabled(method)) {
            fmt::print(
                fmt::emphasis::bold | fmt::emphasis::underline,
                "Benchmarking {}\n", method_names[method]);
            run_one_method_over_all_data(arg, method,folder,tail);
            fmt::print("finished {}\n", method_names[method]);
        } else {
            std::cerr << "CCD method " << method_names[method]
                      << " requested, but it is disabled" << std::endl;
        }
        std::cout << std::endl;
    }
}

void run_rp_for_all_data(const int which){
    std::string folder = "/home/bolun1/interval/data_rp/";// this is the output folder
    std::string tail = "";

    

    // tolerance.push_back("1");
    Args arg;
    arg.data_dir="/home/bolun1/interval/fixed_data/sampled_rounded_queries/";
    
    arg.methods.clear();
    if(which==0){
    arg.methods.push_back(CCDMethod::FIXED_ROOT_PARITY);
    }
    if(which==1){
    arg.methods.push_back(CCDMethod::ROOT_PARITY);
    }
    if(which==2){
    arg.methods.push_back(CCDMethod::RATIONAL_ROOT_PARITY);
    }
    if(which==3){
    arg.methods.push_back(CCDMethod::TIGHT_INCLUSION);
    }
    
    arg.minimum_separation=0;
    arg.tight_inclusion_tolerance=1e-6;
    arg.tight_inclusion_max_iter = 1e6;
    arg.run_ee_dataset = true;
    arg.run_vf_dataset = true;
    arg.run_simulation_dataset = true; // not running simulation
    arg.run_handcrafted_dataset = true;

    WRITE_QUERY_INFO=false;// write query info

    for (CCDMethod method : arg.methods) {
        if (is_method_enabled(method)) {
            fmt::print(
                fmt::emphasis::bold | fmt::emphasis::underline,
                "Benchmarking {}\n", method_names[method]);
            run_one_method_over_all_data(arg, method,folder,tail);
            fmt::print("finished {}\n", method_names[method]);
        } else {
            std::cerr << "CCD method " << method_names[method]
                      << " requested, but it is disabled" << std::endl;
        }
        std::cout << std::endl;
    }
    // run_one_method_over_all_data(arg, CCDMethod::TIGHT_INCLUSION,folder,tail);
}
void run_debug(const int which){
    std::string folder = "/home/bolun1/interval/data_rp/";// this is the output folder
    std::string tail = "";

    

    // tolerance.push_back("1");
    Args arg;
    arg.data_dir="/home/bolun1/interval/fixed_data/sampled_rounded_queries/";
    
    arg.methods.clear();
    if(which==0){
    arg.methods.push_back(CCDMethod::FIXED_ROOT_PARITY);
    }
    if(which==1){
    arg.methods.push_back(CCDMethod::ROOT_PARITY);
    }
    if(which==2){
    arg.methods.push_back(CCDMethod::RATIONAL_ROOT_PARITY);
    }
    if(which==3){
    arg.methods.push_back(CCDMethod::TIGHT_INCLUSION);
    }
    
    arg.minimum_separation=0;
    arg.tight_inclusion_tolerance=1e-6;
    arg.tight_inclusion_max_iter = 1e6;
    arg.run_ee_dataset = true;
    arg.run_vf_dataset = false;
    arg.run_simulation_dataset = false; // not running simulation
    arg.run_handcrafted_dataset = true;

    WRITE_QUERY_INFO=false;// write query info

    for (CCDMethod method : arg.methods) {
        if (is_method_enabled(method)) {
            fmt::print(
                fmt::emphasis::bold | fmt::emphasis::underline,
                "Benchmarking {}\n", method_names[method]);
            run_one_method_over_all_data(arg, method,folder,tail);
            fmt::print("finished {}\n", method_names[method]);
        } else {
            std::cerr << "CCD method " << method_names[method]
                      << " requested, but it is disabled" << std::endl;
        }
        std::cout << std::endl;
    }
    // run_one_method_over_all_data(arg, CCDMethod::TIGHT_INCLUSION,folder,tail);
}
void write_csv_double_2_float_data_result(const Eigen::MatrixXd& data,const std::vector<bool> & results,
const std::string & filename){
    std::ofstream fout;
    if(data.rows()!=results.size()){
        std::cout<<"data size don't match"<<std::endl;
    }
     fout.open(filename);
    for(unsigned int i=0;i<data.rows();i++){       
        // convert to float
        
        float d0=data(i,0);
        float d1=data(i,1);
        float d2=data(i,2);
        std::array<double,3> floats={{d0,d1,d2}};
        std::vector<std::string> fs;
        for(int j=0;j<3;j++){
            Rational r(floats[j]);
            std::string nu=r.get_numerator_str();
            std::string de=r.get_denominator_str();
            fs.push_back(nu);
            fs.push_back(de);
        }
        if(fs.size()!=6){
            std::cout<<"wrong size in write_csv_double_2_float_data_result()"<<std::endl;
        }
        fout<<fs[0]<<","<<fs[1]<<","<<fs[2]<<","<<fs[3]
        <<","<<fs[4]<<","<<fs[5]<<","<<results[i]<<std::endl;
    }
    fout.close();
}
void read_csv_2_float_write_csv(
    boost::filesystem::path dir,
    const bool is_edge_edge,
    const bool is_simulation_data, const std::string outfolder)
{

    // std::vector<std::array<std::string,6>> all_Vstring;
    std::vector<bool> results;
    Eigen::MatrixXd all_V;
    std::string sub_folder = is_edge_edge ? "/edge-edge/" : "/vertex-face/";
    const std::vector<std::string>& scene_names
        = is_simulation_data ? simulation_folders : handcrafted_folders;
    
    for (const auto& scene_name : scene_names) {
        boost::filesystem::path scene_path(
            dir / scene_name / sub_folder);
        if (!boost::filesystem::exists(scene_path)) {
            std::cout << "Missing: " << scene_path.string() << std::endl;
            continue;
        }
        for (const auto& entry :
             boost::filesystem::directory_iterator(scene_path)) {
            if (entry.path().extension() != ".csv") {
                continue;
            }
            //all_Vstring= read_rational_CSV_get_string(entry.path().string(), results);
            all_V = read_rational_csv(entry.path().string(), results);
            if(all_V.rows()%8!=0){
                std::cout<<"wrong size of vertices"<<std::endl;
            }
            boost::filesystem::path pathObj(entry.path().string());
            std::string filename=pathObj.filename().string();
            
            boost::filesystem::path outdir_convert=outfolder;
            boost::filesystem::path out_path(
            outdir_convert / scene_name / sub_folder);
            std::string outfile=out_path.string()+filename;
            //std::cout<<"path correct? "<<outfile<<std::endl; exit(0);
            write_csv_double_2_float_data_result(all_V, results, outfile);
            std::cout<<"finish "<<outfile<<std::endl;
            
        }
    }
}
void compare_two_rational_csv(
    boost::filesystem::path dir,
    const bool is_edge_edge,
    const bool is_simulation_data, const std::string outfolder)
{

    // std::vector<std::array<std::string,6>> all_Vstring;
    std::vector<bool> results;
    Eigen::MatrixXd all_V, all_V1;
    std::string sub_folder = is_edge_edge ? "/edge-edge/" : "/vertex-face/";
    const std::vector<std::string>& scene_names
        = is_simulation_data ? simulation_folders : handcrafted_folders;
    
    for (const auto& scene_name : scene_names) {
        boost::filesystem::path scene_path(
            dir / scene_name / sub_folder);
        if (!boost::filesystem::exists(scene_path)) {
            std::cout << "Missing: " << scene_path.string() << std::endl;
            continue;
        }
        for (const auto& entry :
             boost::filesystem::directory_iterator(scene_path)) {
            if (entry.path().extension() != ".csv") {
                continue;
            }
             boost::filesystem::path pathObj(entry.path().string());
            std::string filename=pathObj.filename().string();
            
            boost::filesystem::path outdir_convert=outfolder;
            boost::filesystem::path out_path(
            outdir_convert / scene_name / sub_folder);
            std::string outfile=out_path.string()+filename;
            //all_Vstring= read_rational_CSV_get_string(entry.path().string(), results);
            all_V = read_rational_csv(entry.path().string(), results);
            all_V1 = read_rational_csv(outfile, results);

            if(all_V.rows()!=all_V1.rows()){
                std::cout<<"wrong in size"<<std::endl;
                exit(0);
            }
            for(int i=0;i<all_V.rows();i++){
                double dis=(all_V.row(i)-all_V1.row(i)).norm();
                if(dis>0){
                    std::cout<<"good! dis="<<dis<<std::endl;
                }
            }

            if(all_V.rows()%8!=0){
                std::cout<<"wrong size of vertices"<<std::endl;
                exit(0);
            }
           
            //std::cout<<"path correct? "<<outfile<<std::endl; exit(0);
            
        }
    }
}
void round_all_the_csv_to_float(){
boost::filesystem::path dir="/home/bolun1/interval/fixed_data/ccd-queries/";
std::string outdir="/home/bolun1/interval/fixed_data/float_without_gt/";
// read_csv_2_float_write_csv(dir,0,0,outdir);
// read_csv_2_float_write_csv(dir,1,0,outdir);
// read_csv_2_float_write_csv(dir,0,1,outdir);
// read_csv_2_float_write_csv(dir,1,1,outdir);
    //compare_two_rational_csv(dir,1,0,outdir);
}
void run_ours_float_for_all_data(){
    std::string folder = "/home/bolun1/interval/data0706/";// this is the output folder
    std::string tail = "";

    

    // tolerance.push_back("1");
    Args arg;
    arg.data_dir="/home/bolun1/interval/fixed_data/float_without_gt/";
    
    arg.methods.clear();
    arg.methods.push_back(CCDMethod::TIGHT_INCLUSION);// only run ours

    arg.minimum_separation=0;
    arg.tight_inclusion_tolerance=1e-6;
    arg.tight_inclusion_max_iter = 1e6;
    arg.run_ee_dataset = true;
    arg.run_vf_dataset = true;
    arg.run_simulation_dataset = true; // not running simulation
    arg.run_handcrafted_dataset = true;

    WRITE_QUERY_INFO=false;// write query info

    for (CCDMethod method : arg.methods) {
        if (is_method_enabled(method)) {
            fmt::print(
                fmt::emphasis::bold | fmt::emphasis::underline,
                "Benchmarking {}\n", method_names[method]);
            run_one_method_over_all_data(arg, method,folder,tail);
            fmt::print("finished {}\n", method_names[method]);
        } else {
            std::cerr << "CCD method " << method_names[method]
                      << " requested, but it is disabled" << std::endl;
        }
        std::cout << std::endl;
    }
    // run_one_method_over_all_data(arg, CCDMethod::TIGHT_INCLUSION,folder,tail);
}


int main(int argc, char* argv[])
{

    //round_all_the_csv_to_float();
    //std::cout<<"round all data to float"<<std::endl;
    run_ours_float_for_all_data();
    exit(0);

    std::string func_name=argv[1];
    if(func_name=="table1"){
        std::cout<<"running run_all_methods_for_handcrafted_table"<<std::endl;
        run_all_methods_for_handcrafted_table();
    }
    if(func_name=="fig7"){
        std::cout<<"running run_ours_for_handcrafted_table"<<std::endl;
        run_ours_for_handcrafted_table();
    }
    if(func_name=="fig8"){
        std::cout<<"running run_ours_diff_tol_for_simulation_data"<<std::endl;
        run_ours_diff_tol_for_simulation_data();
    }
    if(func_name=="table2"){
        std::cout<<"running run_ms_methods_for_all_data"<<std::endl;
        run_ms_methods_for_all_data();
    }
    if(func_name=="fig11"){
        std::string ss=argv[2];
        int start=std::stoi(ss);
        std::cout<<"running run_our_method_diff_ms_for_simulation_data, "<<start<<std::endl;
        run_our_method_diff_ms_for_simulation_data(start);
    }
    if(func_name=="itr"){
        std::cout<<"running run_ours_handcrafted_iteration_info, "<<std::endl;
        run_ours_handcrafted_iteration_info();
    }
    if(func_name=="dis_itr"){
        std::cout<<"running run_ours_handcrafted_without_mi, "<<std::endl;
        run_ours_handcrafted_without_mi();
    }
    if(func_name=="rp_ours"){
        std::cout<<"running run_rp_for_all_data, ours "<<std::endl;
        run_rp_for_all_data(0);
    }
    if(func_name=="rp_original"){
        std::cout<<"running run_rp_for_all_data, original "<<std::endl;
        run_rp_for_all_data(1);
    }
    if(func_name=="rp_rational"){
        std::cout<<"running run_rp_for_all_data, rational "<<std::endl;
        run_rp_for_all_data(2);
    }
    if(func_name=="ti"){
        std::cout<<"running run_rp_for_all_data, ti "<<std::endl;
        run_rp_for_all_data(3);
    }
    if(func_name=="dbg"){
        run_debug(0);
    }
}

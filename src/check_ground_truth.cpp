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
#include <iomanip>
// #include <tight_inclusion/interval_ccd.hpp>
#include <CTCD.h>
#include <tbb/mutex.h>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <tbb/enumerable_thread_specific.h>
using namespace ccd;

std::vector<std::string> simulation_folders
    = { { "chain", "cow-heads", "golf-ball", "mat-twist" } };
std::vector<std::string> handcrafted_folders
    = { { "erleben-sliding-spike", "erleben-spike-wedge",
          "erleben-sliding-wedge", "erleben-wedge-crack", "erleben-spike-crack",
          "erleben-wedges", "erleben-cube-cliff-edges", "erleben-spike-hole",
          "erleben-cube-internal-edges", "erleben-spikes", "unit-tests" } };

struct Args {
    boost::filesystem::path data_dir = "CCD_WRAPPER_SAMPLE_QUERIES_DIR";
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
// void write_csv(const std::string& file, const std::vector<std::string> titles,const std::vector<double> data) {
// 	std::ofstream fout;
// 	fout.open(file);
// 	for (int i = 0; i < titles.size()-1; i++) {
// 	fout << titles[i] << ",";
// 		}
// 	fout << titles.back() << std::endl;
//         for (int i = 0; i < data.size() - 1; i++) {
// 		fout << data[i] << ",";
// 			}
// 	fout << data.back() << std::endl;
// 	fout.close();
// }

// TODO
template<typename T>
void write_csv(const std::string& file, const std::vector<std::string> titles,const std::vector<T> data, bool large_info) {
	std::cout<<"inside write"<<std::endl;
    std::ofstream fout;
	fout.open(file);
	
    if(large_info){
        fout<<"data"<<std::endl;
        for(int i=0;i<data.size();i++){
            fout<<data[i]<<std::endl;
        }
        
    }
    else{
        for (int i = 0; i < titles.size()-1; i++) {
	        fout << titles[i] << ",";
		}
	    fout << titles.back() << std::endl;
        for (int i = 0; i < data.size() - 1; i++) {
		    fout << data[i] << ",";
			}
	    fout << data.back() << std::endl;
    }

        
	fout.close();
}

void run_CPU_parallel_TICCD(const std::vector<Eigen::Matrix<double, 8, 3>>& queries, const bool is_edge_edge,
std::vector<bool>& results, double& time_all){
const std::array<double, 3> err = { { -1, -1, -1 } };
double minimum_separation=0;
double tight_inclusion_tolerance=1e-6;
double tight_inclusion_max_iter=1e6;
const double t_max = 1;
double output_tolerance = tight_inclusion_tolerance;
double toi;
bool result;
Timer timer;
results.resize(queries.size());
timer.start();
tbb::parallel_for( tbb::blocked_range<int>(0,queries.size()),
                       [&](tbb::blocked_range<int> r)
                        {
                           for (int i=r.begin(); i<r.end(); ++i)
                           {
                               Eigen::Matrix<double, 8, 3> V=queries[i];

                            if (is_edge_edge){
                                results[i]=edgeEdgeCCD_OURS( V.row(0), V.row(1), V.row(2), V.row(3), V.row(4),
                                    V.row(5), V.row(6), V.row(7), err, minimum_separation,
                                    toi, tight_inclusion_tolerance,
                                    t_max, tight_inclusion_max_iter, output_tolerance,
                                    1);
                            }
                            else{
                                 results[i]=vertexFaceCCD_OURS( V.row(0), V.row(1), V.row(2), V.row(3), V.row(4),
                                    V.row(5), V.row(6), V.row(7), err, minimum_separation,
                                    toi, tight_inclusion_tolerance,
                                    t_max, tight_inclusion_max_iter, output_tolerance,
                                    1);
                            }
                           } 
                        });
timer.stop();
time_all=timer.getElapsedTimeInMicroSec();
return;
}


//#############################
// this is a value used to control if write info for per query
bool WRITE_QUERY_INFO=false;
// this is to control if write iteration info
bool WRITE_ITERATION_INFO=false;
// this is to print all the result and timing

bool WRITE_STATISTIC=true;

bool WRITE_PROBLEMATIC=true;

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
    std::vector<long> queue_sizes;
    std::vector<double> tois;
    int fpcounter=0;
    std::vector<Eigen::Matrix<double, 8, 3>> all_queries;
    std::vector<bool> all_results;
    std::vector<bool> all_expects;
    
    long long queue_size_total=0;
    const std::vector<std::string>& scene_names
        = is_simulation_data ? simulation_folders : handcrafted_folders;


    for (const auto& scene_name : scene_names) {
        boost::filesystem::path scene_path(
            args.data_dir / scene_name / sub_folder);
        if (!boost::filesystem::exists(scene_path)) {
            std::cout << "Missing: " << scene_path.string() << std::endl;
            continue;
        }
      
        for (const auto& entry :
             boost::filesystem::directory_iterator(scene_path)) {
            if (entry.path().extension() != ".csv") {
                continue;
            }
           
            all_V = read_rational_csv(entry.path().string(), results);
            assert(all_V.rows() % 8 == 0 && all_V.cols() == 3);

            int v_size = all_V.rows() / 8;
            for (int i = 0; i < v_size; i++) {
                total_number += 1;
                Eigen::Matrix<double, 8, 3> V = all_V.middleRows<8>(8 * i);
                bool expected_result = results[i * 8];
                all_queries.push_back(V);
                all_expects.push_back(expected_result);
            }
            
                
              
                
            }
        }
        if(total_number + 1!=all_queries.size()){
            std::cout<<"size wrong with queries"<<std::endl;
            exit(1);
        }
        std::cout<<"data loaded"<<std::endl;
        
        run_CPU_parallel_TICCD(all_queries,is_edge_edge,all_results,total_time);
        for(int i=0;i<all_queries.size();i++){
            if (all_expects[i]) {
                    total_positives++;
            }
            if (all_results[i] != all_expects[i]) {
                if (all_results[i]) {
                    num_false_positives++;
                    
                } else {
                    num_false_negatives++;
                }
            }
        }

        
        if(WRITE_STATISTIC){
        write_summary(
            folder + "method_tbb"   + "_is_edge_edge_"
            + std::to_string(is_edge_edge) + "_"
            + std::to_string(total_number + 1) + tail + ".csv",
        1024, total_number + 1, total_positives, is_edge_edge,
        num_false_positives, num_false_negatives,
        total_time / double(total_number + 1),0,0);
        }
        
    
    if(1){
        // std::cout<<"start writting queue info"<<std::endl;
        // queue_size_avg=queue_size_total/(total_number + 1);
        // std::cout<<"check pt"<<std::endl;
        // //std::vector<std::string> titles={{"max","avg"}};
        // std::cout<<"max avg "<<queue_size_max<<" "<<queue_size_avg<<std::endl;
        // // write_queue_sizes(folder + "method" + std::to_string(method) + "_is_edge_edge_"
        // //         + std::to_string(is_edge_edge) + "_"
        // //         + std::to_string(total_number + 1) + "_queue_info" + tail
        // //         + ".csv",queue_sizes);
        // std::vector<std::string> titles;
        
        // write_csv(folder + "method" + std::to_string(method) + "_is_edge_edge_"
        //         + std::to_string(is_edge_edge) + "_"
        //         + std::to_string(total_number + 1) + "_queue_info" + tail
        //         + ".csv",titles,queue_sizes,true);
        // write_csv(folder + "method" + std::to_string(method) + "_is_edge_edge_"
        //         + std::to_string(is_edge_edge) + "_"
        //         + std::to_string(total_number + 1) + "_tois" + tail
        //         + ".csv",titles,tois,true);
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

void run_tbb_for_all_data(){

    std::string folder = "/home/bolun1/interval/data0915/";// this is the output folder
    std::string tail = "";
    Args arg;
    arg.data_dir="/home/bolun1/interval/fixed_data/float_with_gt/";
    
    arg.methods.clear();
  
    arg.methods.push_back(CCDMethod::TIGHT_INCLUSION);
    
    
    arg.minimum_separation=0;
    arg.tight_inclusion_tolerance=1e-6;
    arg.tight_inclusion_max_iter = 1e6;
    arg.run_ee_dataset = true;
    arg.run_vf_dataset = true;
    arg.run_simulation_dataset = true; // not running simulation
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
int main(int argc, char* argv[])
{
    run_tbb_for_all_data();
}

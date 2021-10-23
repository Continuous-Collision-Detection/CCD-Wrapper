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
#include <utils/io.h>
#include <tight_inclusion/inclusion_ccd.hpp>
#include <iomanip>
// #include <tight_inclusion/interval_ccd.hpp>
#include <CTCD.h>
#include <tbb/mutex.h>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <tbb/task_scheduler_init.h>
#include <tbb/enumerable_thread_specific.h>
using namespace ccd;


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
Eigen::Matrix<double, 8, 3> substract_ccd(const std::vector<std::array<double, 3>> &data, int nbr)
{
    Eigen::Matrix<double, 8, 3> result;
    int start = nbr * 8;
    for (int i = 0; i < 8; i++)
    {
        result(i,0) = data[i + start][0];
        result(i,1) = data[i + start][1];
        result(i,2) = data[i + start][2];
    }
    return result;
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
/*
positive id 119959
positive id 219064
positive id 220848
positive id 359287
positive id 361226
positive id 375453
positive id 377605
positive id 668033
positive id 672149
positive id 960843
positive id 963066
*/
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
    const bool is_edge_edge,
    const bool is_simulation_data, const std::string folder="", const std::string tail="")
{
    //Eigen::MatrixXd all_V;
    std::vector<std::array<double,3>> all_V_vec;
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
    std::string sub_name = is_edge_edge ? "edge-edge" : "vertex-face";
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
        std::vector<std::string> bases = file_path_base();
    long long queue_size_total=0;
    const std::vector<std::string>& scene_names
        = is_simulation_data ? simulation_folders : handcrafted_folders;
    int tmp_count=0;

    for (const auto& scene_name : scene_names) {
        std::string scene_path = args.data_dir + scene_name + sub_folder;
        bool skip_folder = false;
      
        for (const auto &entry : bases)
        {
            if (skip_folder)
            {
                break;
            }
            std::string filename = scene_path + sub_name + "-" + entry + ".csv";
            std::string filename_noext =  filename.substr(0, filename.find_last_of("."));
            
            std::string vertexFilename = std::string(filename_noext + "_vertex.bin");
            std::ifstream vinfile (vertexFilename, std::ios::in | std::ios::binary);

            std::string resultsFilename = std::string(filename_noext + "_result.bin");
            std::ifstream rinfile (resultsFilename, std::ios::in | std::ios::binary);
            
            if (vinfile && rinfile)
            {
                read_rational_binary(vertexFilename, all_V_vec );
                read_rational_binary(resultsFilename, results );
            }
            else 
                all_V_vec = read_rational_csv_bin(filename, results);
            if (all_V_vec.size() == 0)
            {
                std::cout << "data size " << all_V_vec.size() << std::endl;
                std::cout << filename << std::endl;
            }

            if (all_V_vec.size() == 0)
            {
                skip_folder = true;
                continue;
            }
           
            int v_size = all_V_vec.size() / 8;
            for (int i = 0; i < v_size; i++) {
                total_number += 1;
                Eigen::Matrix<double, 8, 3> V = substract_ccd(all_V_vec, i);
                bool expected_result = results[i * 8];
                
                // if(expected_result){// to find slow ones
                //     tmp_count+=1;
                //     std::cout<<"positive id "<<total_number<<std::endl;
                //     if(tmp_count>10){
                //         exit(0);
                //     }
                // }
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
#ifdef DUPLICATE_ONE_QUERIE
        Eigen::Matrix<double, 8, 3> duplicated_V=all_queries[219064];
        int duplicate_size=10000;
        all_queries.resize(duplicate_size);
        for(int i=0;i<duplicate_size;i++){
            all_queries[i]=duplicated_V;
        }
        std::cout<<"duplicating testing "<<std::endl;

#endif
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
            + std::to_string(all_queries.size()) + tail + ".csv",
        1024, all_queries.size(), total_positives, is_edge_edge,
        num_false_positives, num_false_negatives,
        total_time / double(all_queries.size()),0,0);
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
        all_queries.size(), total_positives, is_edge_edge, num_false_positives,
        num_false_negatives, total_time / double(all_queries.size()));
}

void run_one_method_over_all_data(const Args& args,
const std::string folder="", const std::string tail="")
{
    if (args.run_handcrafted_dataset) {
        fmt::print(fmt::emphasis::bold, "Running handcrafted dataset:\n");
        if (args.run_vf_dataset) {
            std::cout << "Vertex-Face:" << std::endl;
            run_rational_data_single_method(
                args, /*is_edge_edge=*/false, /*is_simu_data=*/false,folder,tail);
        }
        if (args.run_ee_dataset) {
            std::cout << "Edge-Edge:" << std::endl;
            run_rational_data_single_method(
                args, /*is_edge_edge=*/true, /*is_simu_data=*/false,folder,tail);
        }
    }
    if (args.run_simulation_dataset) {
        fmt::print(fmt::emphasis::bold, "Running simulation dataset:\n");
        if (args.run_vf_dataset) {
            std::cout << "Vertex-Face:" << std::endl;
            run_rational_data_single_method(
                args, /*is_edge_edge=*/false, /*is_simu_data=*/true,folder,tail);
        }
        if (args.run_ee_dataset) {
            std::cout << "Edge-Edge:" << std::endl;
            run_rational_data_single_method(
                args, /*is_edge_edge=*/true, /*is_simu_data=*/true,folder,tail);
        }
    }
}

void run_tbb_for_all_data(){

    std::string folder = "/home/bolun1/interval/data0915/";// this is the output folder
    std::string tail = "";
    Args arg;
    arg.data_dir="/home/bolun1/interval/fixed_data/polluted/float_with_gt/";
    arg.minimum_separation=0;
    arg.tight_inclusion_tolerance=1e-6;
    arg.tight_inclusion_max_iter = 1e6;
    arg.run_ee_dataset = false;
    arg.run_vf_dataset = true;
    arg.run_simulation_dataset = true; // not running simulation
    arg.run_handcrafted_dataset = false;


    run_one_method_over_all_data(arg,folder,tail);
}
int main(int argc, char* argv[])
{
    int parallel = 0;
    if (argc == 1)
    {
        parallel = 1;
    }
    else
    {
        parallel = std::stoi(argv[1]);
    }
    if (parallel <= 0)
    {
        std::cout << "wrong parallel nbr = " << parallel << std::endl;
        return 0;
    }
    std::cout<<"default threads "<<tbb::task_scheduler_init::default_num_threads()<<std::endl;
    tbb::task_scheduler_init init(parallel);
    //tbb::task_scheduler_init init;  // Automatic number of threads
  //tbb::task_scheduler_init init(tbb::task_scheduler_init::default_num_threads());  // Explicit number of threads
    run_tbb_for_all_data();
}

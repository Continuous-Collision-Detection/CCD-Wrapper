// Time the different CCD methods

// #include <filesystem>
// #include <fstream>
// #include <regex>
// #include <string>

// #include <CLI/CLI.hpp>
// #include <Eigen/Core>
// #include <fmt/format.h>
// #include <highfive/H5Easy.hpp>
// #include <igl/Timer.h>
// #include <nlohmann/json.hpp>

// #include <ccd.hpp>
// #include <utils/get_rss.hpp>
// #include<interval_ccd/interval_ccd.hpp>
// #include<interval_ccd/interval_root_finder.hpp>
// #include<fstream>
// using namespace ccd;

// struct Args {
//     std::string data_dir;
//     bool is_edge_edge;
//     CCDMethod method;
//     double min_distance = DEFAULT_MIN_DISTANCE;
// };

// Args parse_args(int argc, char* argv[])
// {
//     Args args;

//     CLI::App app { "CCD Wrapper Benchmark" };

//     app.add_option("data_directory", args.data_dir, "/path/to/data/")
//         ->required();

//     std::string col_type;
//     app.add_set("collision_type", col_type, { "vf", "ee" }, "type of collision")
//         ->required();

//     std::stringstream method_options;
//     method_options << "CCD method\noptions:" << std::endl;
//     for (int i = 0; i < NUM_CCD_METHODS; i++) {
//         method_options << i << ": " << method_names[i] << std::endl;
//     }
//     app.add_option("CCD_method", args.method, method_options.str())->required();

//     app.add_option("min_distance,-d", args.min_distance, "minimum distance");

//     try {
//         app.parse(argc, argv);
//     } catch (const CLI::ParseError& e) {
//         exit(app.exit(e));
//     }

//     args.is_edge_edge = col_type == "ee";

//     if (args.method < 0 || args.method >= NUM_CCD_METHODS) {
//         exit(app.exit(CLI::Error(
//             "",
//             fmt::format(
//                 "invalid method of collision detection: {:d}", args.method))));
//     }

//     return args;
// }

// int main(int argc, char* argv[])
// {
//     //Args args = parse_args(argc, argv);
//     Args args;// arguments are like: executable, vf, dir1, di2, dir3,...
//     args.min_distance=DEFAULT_MIN_DISTANCE;
//     std::string argv1=argv[1];
//     if(argv1=="vf"){
//         args.is_edge_edge=false;
//     }
//     else{
//         args.is_edge_edge=true;
//     }

    
//     std::cout<<"is edge edge? "<<args.is_edge_edge<<std::endl;
//     //std::cout<<"argv[1] ,"<<argv[1]<<","<<std::endl;

//     std::string mtd=argv[2];
//     args.method=CCDMethod::INTERVAL;
//     bool use_msccd = isMinSeparationMethod(args.method);
//     std::vector<std::string> dirs;
//     for(int i=2;i<argc;i++){
//         dirs.push_back(argv[i]);
//         std::cout<<"dir: "<<argv[i]<<std::endl;;
//     }


//     std::cout<<"method, "<<args.method<<" out of "<< NUM_CCD_METHODS<<std::endl;
//     igl::Timer timer;

//     int num_queries = 0;
//     double timing = 0.0, new_timing=0.0;
//     int false_positives = 0;
//     int false_negatives = 0;
//     int total_positives=0;
//     int new_false_positives=0;
//     int new_false_negatives=0;
//     std::ofstream fout;
//     fout.open("/home/bolun1/interval/histo_vf_-3.csv");
//     for(int ndir=0;ndir<dirs.size();ndir++){
//     for (auto& entry : std::filesystem::directory_iterator(dirs[ndir])) {
//         if (entry.path().extension() != ".hdf5"
//             && entry.path().extension() != ".h5") {
//             continue;
//         }
//         H5Easy::File file(entry.path().string());

//         Eigen::MatrixXd all_V
//             = H5Easy::load<Eigen::MatrixXd>(file, "/rounded/points");
//         assert(all_V.rows() % 8 == 0 && all_V.cols() == 3);
//         Eigen::Matrix<unsigned char, Eigen::Dynamic, 1> expected_results
//             = H5Easy::load<Eigen::Matrix<unsigned char, Eigen::Dynamic, 1>>(
//                 file, "/rounded/result");
//         assert(all_V.rows() / 8 == expected_results.rows());

//         for (size_t i = 0; i < expected_results.rows(); i++) {
//             Eigen::Matrix<double, 8, 3> V = all_V.middleRows<8>(8 * i);
//             bool expected_result = bool(expected_results(i));
//             //std::cout<<"we are running i,"<<i<<std::endl;
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
//             fout<<timer.getElapsedTimeInMicroSec()<<", "<<result<<", ";
//             bool new_result;
//             timer.start();
//             if (args.is_edge_edge) {
//                     new_result = edgeEdgeCCD_new(
//                         V.row(0), V.row(1), V.row(2), V.row(3), V.row(4),
//                         V.row(5), V.row(6), V.row(7));
//                 } else {
//                      new_result = vertexFaceCCD_new(
//                         V.row(0), V.row(1), V.row(2), V.row(3), V.row(4),
//                         V.row(5), V.row(6), V.row(7));
//                     // result = vertexFaceCCD(
//                     //     V.row(0), V.row(1), V.row(2), V.row(3), V.row(4),
//                     //     V.row(5), V.row(6), V.row(7), args.method);
//             }
//             timer.stop();
//             new_timing+=timer.getElapsedTimeInMicroSec();
//             fout<<timer.getElapsedTimeInMicroSec()<<", "<<new_result<<std::endl;
//             if(result==true) total_positives++;
//             if(result!=new_result){
//                  std::cout<<"the ith don't match, i "<<i<<"rst, "<<result<<" , "<<new_result<<std::endl;
//                  if(new_result==0){
//                      std::cout<<dirs[ndir]<<", "<<i<<std::endl;
//                  }
//                 // std::cout<<"ori, new, "<<result<<" , "<<new_result<<std::endl;
//                 // std::cout<<"the input,\n"<<V<<std::endl;
//             }
//             // Count the inaccuracies
//             if(new_result!=expected_result){
//                 if(new_result)
//                 new_false_positives++;
//                 else{
//                     new_false_negatives++;
//                 }
//             }

//             if (result != expected_result) {
//                 if (result) {
//                     false_positives++;
//                 } else {
//                     false_negatives++;
//                     if (args.method == CCDMethod::EXACT_RATIONAL_MIN_SEPARATION
//                         || args.method
//                             == CCDMethod::EXACT_DOUBLE_MIN_SEPARATION) {
//                         std::cerr << fmt::format(
//                             "file={} index={:d} method={} false_negative",
//                             entry.path().string(), 8 * i,
//                             method_names[args.method])
//                                   << std::endl;
//                     }
//                 }
//                 if (args.method == CCDMethod::RATIONAL_ROOT_PARITY) {
//                     std::cerr << fmt::format(
//                         "file={} index={:d} method={} {}",
//                         entry.path().string(), 8 * i, method_names[args.method],
//                         result ? "false_positive" : "false_negative")
//                               << std::endl;
//                 }
//             }
//             std::cout << ++num_queries << "\r" << std::flush;
//         }
//     }
//     }
//     fout.close();
//     nlohmann::json benchmark;
//     benchmark["collision_type"] = args.is_edge_edge ? "ee" : "vf";
//     benchmark["num_queries"] = num_queries;
//     std::string method_name = method_names[args.method];

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
//     std::cout<<"false positives, "<<false_positives<<std::endl;
//     std::cout<<"false negatives, "<<false_negatives<<std::endl;
//     std::cout<<"total positives, "<<total_positives<<std::endl;
//     std::cout<<"avg qry time, " <<timing/num_queries<<std::endl;
//     std::cout<<"nmbr of qrys,"<<num_queries<<std::endl;
//     std::cout<<"newfalse positives, "<<new_false_positives<<std::endl;
//     std::cout<<"newfalse negatives, "<<new_false_negatives<<std::endl;
//     std::cout<<"new method avg qry time, " <<(new_timing-intervalccd::print_time_rational())/num_queries<<std::endl;
//     std::cout<<"new method total time,"<<new_timing-intervalccd::print_time_rational()<<std::endl;
//     intervalccd::print_time_1();
//     intervalccd::print_time_2();
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

//     std::ofstream(fname) << benchmark.dump(4);
// }
int main(){
    return 0;
}
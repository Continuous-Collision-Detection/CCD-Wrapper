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
#include<interval_ccd/interval_ccd.hpp>
#include<interval_ccd/interval_root_finder.hpp>
#include<fstream>
using namespace ccd;

struct Args {
    std::string data_dir;
    bool is_edge_edge;
    CCDMethod method;
    double min_distance = DEFAULT_MIN_DISTANCE;
    int nbr=0;
    bool result=false;
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

void read_CSV(const std::string inputFileName, std::vector<Args>& args) {


	args.clear();
	std::ifstream infile;
	infile.open(inputFileName);
    Args argi;
	if (!infile.is_open())
	{
		std::cout << "Path Wrong!!!!" << std::endl;
        return;
	}

	int l = 0;
	while (infile) // there is input overload classfile
	{
		l++;
		std::string s;
		if (!getline(infile, s)) break;
        if(l==1) continue;
		if (s[0] != '#') {
			std::istringstream ss(s);
			std::array<std::string,5> record;
			int c = 0;
			while (ss) {
				std::string line;
				if (!getline(ss, line, ','))
					break;
				try {
					record[c] = line;
					c++;
				}
				catch (const std::invalid_argument e) {
					std::cout << "NaN found in file " << inputFileName << " line " << l
						<< std::endl;
					e.what();
				}
			}
            argi.data_dir=record[1];
            argi.is_edge_edge=!bool(std::stoi(record[4]));
            argi.nbr=std::stoi(record[2])/8;
            if(record[3]=="False"){
                argi.result=0;
            }
            else{
                argi.result=1;
            }
            argi.method=CCDMethod::INTERVAL;
            
			args.push_back(argi);
		}
	}
	if (!infile.eof()) {
		std::cerr << "Could not read file " << inputFileName << "\n";
	}
	
	return;
}

// this reads the file. the nbr is the query id (from 0) in this path
void read_CSV_new(const std::string inputFileName, std::vector<Args>& args) {
   

	args.clear();
	std::ifstream infile;
	infile.open(inputFileName);
    Args argi;
	if (!infile.is_open())
	{
        
		std::cout << "Path Wrong!!!!" << std::endl;
        std::cout<<"file, "<<inputFileName<<std::endl;
        return;
	}

	int l = 0;
	while (infile) // there is input overload classfile
	{
		l++;
		std::string s;
		if (!getline(infile, s)) break;
        if(l==1) continue;
		if (s[0] != '#') {
			std::istringstream ss(s);
			std::array<std::string,5> record;
			int c = 0;
			while (ss) {
				std::string line;
				if (!getline(ss, line, ','))
					break;
				try {
					record[c] = line;
					c++;
				}
				catch (const std::invalid_argument e) {
					std::cout << "NaN found in file " << inputFileName << " line " << l
						<< std::endl;
					e.what();
				}
			}
            argi.data_dir=record[1];
            argi.is_edge_edge=!bool(std::stoi(record[4]));
            argi.nbr=std::stoi(record[2])/8;
            if(record[3]=="False"){
                argi.result=0;
            }
            else{
                argi.result=1;
            }
            argi.method=CCDMethod::INTERVAL;
            
			args.push_back(argi);
		}
	}
	if (!infile.eof()) {
		std::cerr << "Could not read file " << inputFileName << "\n";
	}
	
	return;
}


Eigen::MatrixXd read_rational_CSV(const std::string inputFileName) {

    std::vector<std::array<double,3>> vs;
    vs.clear();
	std::ifstream infile;
	infile.open(inputFileName);
    std::array<double,3> v;
	if (!infile.is_open())
	{
		std::cout << "Path Wrong!!!!" << std::endl;
        return Eigen::MatrixXd(1,1);
	}

	int l = 0;
	while (infile) // there is input overload classfile
	{
		l++;
		std::string s;
		if (!getline(infile, s)) break;
		if (s[0] != '#') {
			std::istringstream ss(s);
			std::array<std::string,6> record;
			int c = 0;
			while (ss) {
				std::string line;
				if (!getline(ss, line, ','))
					break;
				try {
					record[c] = line;
					c++;

				}
				catch (const std::invalid_argument e) {
					std::cout << "NaN found in file " << inputFileName << " line " << l
						<< std::endl;
					e.what();
				}
			}
            intervalccd::Rational rt;
            double x=rt.get_double(record[0],record[1]),y=rt.get_double(record[2],record[3]),z=rt.get_double(record[4],record[5]);
            v[0]=x;v[1]=y;v[2]=z;
            vs.push_back(v);
		}
	}
    Eigen::MatrixXd all_v(vs.size(),3);
    for(int i=0;i<vs.size();i++){
        all_v(i,0)=vs[i][0];
        all_v(i,1)=vs[i][1];
        all_v(i,2)=vs[i][2];
    }
	if (!infile.eof()) {
		std::cerr << "Could not read file " << inputFileName << "\n";
	}
	
	return all_v;
}
Eigen::MatrixXd read_rational_CSV(const std::string inputFileName, std::vector<bool> &results) {
    // be careful, there are n lines which means there are n/8 queries, but has n results, which means
    // results are duplicated
    results.clear();
    std::vector<std::array<double,3>> vs;
    vs.clear();
	std::ifstream infile;
	infile.open(inputFileName);
    std::array<double,3> v;
	if (!infile.is_open())
	{
		std::cout << "Path Wrong!!!!" << std::endl;
        return Eigen::MatrixXd(1,1);
	}

	int l = 0;
	while (infile) // there is input overload classfile
	{
		l++;
		std::string s;
		if (!getline(infile, s)) break;
		if (s[0] != '#') {
			std::istringstream ss(s);
			std::array<std::string,7> record;// the first six are one vetex, the seventh is the result
			int c = 0;
			while (ss) {
				std::string line;
				if (!getline(ss, line, ','))
					break;
				try {
					record[c] = line;
					c++;

				}
				catch (const std::invalid_argument e) {
					std::cout << "NaN found in file " << inputFileName << " line " << l
						<< std::endl;
					e.what();
				}
			}
            intervalccd::Rational rt;
            double x=rt.get_double(record[0],record[1]),y=rt.get_double(record[2],record[3]),z=rt.get_double(record[4],record[5]);
            v[0]=x;v[1]=y;v[2]=z;
            vs.push_back(v);
            if(record[6]!="1"&&record[6]!="0"){
                std::cout<<"ERROR:result position should be 1 or 0, but it is "<<record[6]<<std::endl;
            }
            results.push_back(std::stoi(record[6]));
		}
	}
    Eigen::MatrixXd all_v(vs.size(),3);
    for(int i=0;i<vs.size();i++){
        all_v(i,0)=vs[i][0];
        all_v(i,1)=vs[i][1];
        all_v(i,2)=vs[i][2];
    }
	if (!infile.eof()) {
		std::cerr << "Could not read file " << inputFileName << "\n";
	}
	
	return all_v;
}
std::vector<std::array<std::string,6>> read_rational_CSV_get_string(const std::string inputFileName) {

    std::vector<std::array<std::string,6>> vstring;
    vstring.clear();
	std::ifstream infile;
	infile.open(inputFileName);
    
	if (!infile.is_open())
	{
		std::cout << "Path Wrong!!!!" << std::endl;
        exit(0);
	}

	int l = 0;
	while (infile) // there is input overload classfile
	{
		l++;
		std::string s;
		if (!getline(infile, s)) break;
		if (s[0] != '#') {
			std::istringstream ss(s);
			std::array<std::string,6> record;
			int c = 0;
			while (ss) {
				std::string line;
				if (!getline(ss, line, ','))
					break;
				try {
					record[c] = line;
					c++;

				}
				catch (const std::invalid_argument e) {
					std::cout << "NaN found in file " << inputFileName << " line " << l
						<< std::endl;
					e.what();
				}
			}
            vstring.push_back(record);
		}
	}
    // Eigen::MatrixXd all_v(vs.size(),3);
    // for(int i=0;i<vs.size();i++){
    //     all_v(i,0)=vs[i][0];
    //     all_v(i,1)=vs[i][1];
    //     all_v(i,2)=vs[i][2];
    // }
	if (!infile.eof()) {
		std::cerr << "Could not read file " << inputFileName << "\n";
	}
	
	return vstring;
}

void run_single_query(int argc, char* argv[]){
    //Args args = parse_args(argc, argv);
    Args args;// arguments are like: executable, filename, query id, is_vf, result
    args.min_distance=DEFAULT_MIN_DISTANCE;
    std::string argv3=argv[3];
    if(argv3=="0"){// argv3 is is_vf
        args.is_edge_edge=true;
    }
    else{
        args.is_edge_edge=false;
    }
    std::string filename=argv[1];
    std::string istr=argv[2];
    std::string rstr=argv[4];
    //std::cout<<"is edge edge? "<<args.is_edge_edge<<std::endl;
    //std::cout<<"argv[1] ,"<<argv[1]<<","<<std::endl;

    
    args.method=CCDMethod::INTERVAL;
    bool use_msccd = isMinSeparationMethod(args.method);
    
    


    //std::cout<<"method, "<<args.method<<" out of "<< NUM_CCD_METHODS<<std::endl;
    igl::Timer timer;

    int num_queries = 0;
    double timing = 0.0, new_timing=0.0;
    int false_positives = 0;
    int false_negatives = 0;
    int total_positives=0;
    int new_false_positives=0;
    int new_false_negatives=0;
    //std::ofstream fout;
    //fout.open("/home/bolun1/interval/histo_vf_-3.csv");
    //std::cout<<"name, "<<filename<<std::endl;
    for (auto& entry : std::filesystem::directory_iterator(filename)) {
        
        if (entry.path().extension() != ".hdf5"
            && entry.path().extension() != ".h5") {
            continue;
        }
        H5Easy::File file(entry.path().string());

        Eigen::MatrixXd all_V
            = H5Easy::load<Eigen::MatrixXd>(file, "/points");
        assert(all_V.rows() % 8 == 0 && all_V.cols() == 3);
        
        long i=std::stoi(istr);
        
            Eigen::Matrix<double, 8, 3> V = all_V.middleRows<8>(8 * i);
            bool expected_result = std::stoi(rstr);
            //std::cout<<"we are running i,"<<i<<std::endl;
            // Time the methods
            bool result=false;
            timer.start();
            if (use_msccd) {
                
            } else {
                
            }
            timer.stop();
            timing += timer.getElapsedTimeInMicroSec();
            //fout<<timer.getElapsedTimeInMicroSec()<<", "<<result<<", ";
            bool new_result;
            timer.start();
            // if (args.is_edge_edge) {
            //         new_result = edgeEdgeCCD_new(
            //             V.row(0), V.row(1), V.row(4),
            //             V.row(5),  V.row(2), V.row(3), V.row(6), V.row(7));
            //     } else {
            //          new_result = vertexFaceCCD_new(
            //             V.row(0), V.row(1), V.row(4),
            //             V.row(5),  V.row(2), V.row(3), V.row(6), V.row(7));
            //         // result = vertexFaceCCD(
            //         //     V.row(0), V.row(1), V.row(2), V.row(3), V.row(4),
            //         //     V.row(5), V.row(6), V.row(7), args.method);
            // }
            timer.stop();
            new_timing+=timer.getElapsedTimeInMicroSec();
            //fout<<timer.getElapsedTimeInMicroSec()<<", "<<new_result<<std::endl;
            // if(result==true) total_positives++;
            // if(result!=new_result){
            //      std::cout<<"the ith don't match, i "<<i<<"rst, "<<result<<" , "<<new_result<<std::endl;
                 
            //     // std::cout<<"ori, new, "<<result<<" , "<<new_result<<std::endl;
            //     // std::cout<<"the input,\n"<<V<<std::endl;
            // }
            if(new_result!=expected_result){
                std::cout<<"result don't match, groud, "<<expected_result<<" ours, "<<new_result<<std::endl;
               
                if(new_result==0){
                    std::cout<<"false nega, "<<filename<<", "<<i<<std::endl;
                    for(int j=0;j<8;j++){
                        std::cout<<"v"<<j<<" "<<V(j,0)<<", "<<V(j,1)<<", "<<V(j,2)<<std::endl;
                    }


                }
            }
            // Count the inaccuracies
            if(new_result!=expected_result){
                if(new_result)
                new_false_positives++;
                else{
                    new_false_negatives++;
                }
            }

            if (result != expected_result) {
                if (result) {
                    false_positives++;
                } else {
                    false_negatives++;
                    if (args.method == CCDMethod::EXACT_RATIONAL_MIN_SEPARATION
                        || args.method
                            == CCDMethod::EXACT_DOUBLE_MIN_SEPARATION) {
                        std::cerr << fmt::format(
                            "file={} index={:d} method={} false_negative",
                            entry.path().string(), 8 * i,
                            method_names[args.method])
                                  << std::endl;
                    }
                }
                if (args.method == CCDMethod::RATIONAL_ROOT_PARITY) {
                    std::cerr << fmt::format(
                        "file={} index={:d} method={} {}",
                        entry.path().string(), 8 * i, method_names[args.method],
                        result ? "false_positive" : "false_negative")
                              << std::endl;
                }
            }
            std::cout << ++num_queries << "\r" << std::flush;
        
    }
    
}

void write_csv(const Eigen::Matrix<std::string,Eigen::Dynamic,Eigen::Dynamic >& data,
const std::string & filename){
    std::ofstream fout;
    int filenumber=0;
    //fout.open(filename+"small_"+std::to_string(filenumber)+".csv");
    //std::cout<<"query rows, "<<data.rows()<<std::endl;
    for(unsigned int i=0;i<data.rows();i++){
        if(i%100000==0) {
            fout.open(filename+"small_"+std::to_string(filenumber)+".csv");
            //std::cout<<"file number, "<<filenumber<<std::endl;
            filenumber++;
        }
        fout<<data(i,0)<<","<<data(i,1)<<","<<data(i,2)<<","<<data(i,3)<<","<<data(i,4)<<","<<data(i,5)<<std::endl;
        if((i+1)%100000==0) 
        {
            fout.close();
            //std::cout<<"closed"<<std::endl;
            }
    }
    fout.close();
}

// this is to write small file
void write_csv_data_result(const std::vector<std::array<std::string,6>>& data,const std::vector<bool> & results,
const std::string & filename){
    std::ofstream fout;
    if(data.size()!=results.size()){
        std::cout<<"data size don't match"<<std::endl;
    }
     fout.open(filename);
    for(unsigned int i=0;i<data.size();i++){       
           
        fout<<data[i][0]<<","<<data[i][1]<<","<<data[i][2]<<","<<data[i][3]
        <<","<<data[i][4]<<","<<data[i][5]<<","<<results[i]<<std::endl;
    }
    fout.close();
}

// arguments should be: excutive, ee, file0, file1, ...
int get_rational_vertices(int argc, char* argv[])
{
    bool debug_flag=0;
    std::vector<std::string> dirs;
    
    int rows=0;
    if(debug_flag){
        dirs.clear();
        dirs.push_back("/home/zachary/Development/ccd-queries/erleben-cube-cliff-edges/vertex-face/");
    }
    else{
        for(int i=2;i<argc;i++){
        dirs.push_back(argv[i]);
        std::cout<<"dir: "<<argv[i]<<std::endl;
        }
    }


    for(int nm=0;nm<dirs.size();nm++){
        int filenumber=0;
    std::string filedir=dirs[nm];
    int h5_nbr=-1;
    for (auto& entry : std::filesystem::directory_iterator(filedir)) {
        if (entry.path().extension() != ".hdf5"
            && entry.path().extension() != ".h5") {
            continue;
        }
        H5Easy::File file(entry.path().string(), HighFive::File::ReadWrite);

        h5_nbr++;

    Eigen::MatrixXd all_V = H5Easy::load<Eigen::MatrixXd>(file, "/points");
    assert(all_V.rows() % 8 == 0 && all_V.cols() == 3);
    size_t num_queries = all_V.rows() / 8;

    Eigen::Matrix<std::string,Eigen::Dynamic,Eigen::Dynamic > All_Rational_V(all_V.rows(), all_V.cols()*2);
    Eigen::Matrix<std::string, 8, 6> Rational_V;
    //Eigen::MatrixXd Rational_V(all_V.rows(), all_V.cols()*2);

    if(debug_flag){
        // just for debug
        num_queries=1;
    }

    for (size_t i = 0; i < num_queries; i++) {
        Eigen::Matrix<double, 8, 3> V = all_V.middleRows<8>(8 * i);
        //std::cout<<"V\n"<<V<<std::endl<<std::endl;
        for(int j=0;j<8;j++){
            
            for(int k=0;k<3;k++){
                intervalccd::Rational r(V(j,k)),new_r;
                Rational_V(j,2*k)=r.get_numerator_str();
                Rational_V(j,2*k+1)=r.get_denominator_str();
                //std::cout<<r.get_numerator_str()<<" "<<r.get_denominator_str()<<" ";
                

                double test_number=
                new_r.get_double(r.get_numerator_str(),r.get_denominator_str());
                //double(r.get_numerator())/double(r.get_denominator());
                if(test_number!=V(j,k)){
                    std::cout<<"\nnumber not match, number, "<<V(j,k)<<", test number, "<<test_number
                    <<", numerator, "<<r.get_numerator()<<", denominator, "<<r.get_denominator()<<std::endl;
                    
                    if(r.get_denominator()==0){
                        std::cout<<"the j and k are, "<<j<<" , "<<k<<std::endl;
                        std::cout<<"numerator string, "<<r.get_numerator_str()<<std::endl;
                        std::cout<<"denominator string, "<<r.get_denominator_str()<<std::endl;
                        std::cout<<"dir, "<<filedir<<", i, "<<i<<std::endl;

                    }
                }
                // if(r.get_denominator()!=0){
                //     std::cout<<"denominator not 0"<<std::endl;
                // }

            }
        }

        All_Rational_V.middleRows<8>(8 * i) = Rational_V;
    }
    Eigen::Vector3d tst(1,1,1);
    Eigen::Matrix<std::string, Eigen::Dynamic,Eigen::Dynamic> tmpstr(2,1);
    for(int itr=0;itr<2;itr++){
        for(int itr1=0;itr1<1;itr1++){
            tmpstr(itr,itr1)="sss";
        }
    }
    // std::cout<<"initialized strings"<<std::endl;
    // // std::cout<<"path, "<<entry.path().string()<<std::endl;
    // // std::cout<<All_Rational_V<<std::endl;
    // H5Easy::dump(file, "/strings_new3", tmpstr);
    rows+=All_Rational_V.rows();
    write_csv(All_Rational_V,filedir+"Rational_"+std::to_string(h5_nbr)+"_");
    filenumber++;
    std::cout<<"finshed, "<<entry.path().string()<<std::endl;
    }
    std::cout<<"there are "<<rows<<" rows in "<<dirs[nm]<<std::endl;
   
    
    }
    return 0;
     
}
void write_final_version_data_set(const std::vector<std::string>& files){

    std::string old_name="empty";

    std::vector<std::array<std::string,6>> all_V;

    bool is_edge_edge=false;

    for(int fnbr=0;fnbr<files.size();fnbr++){
        const std::string inputFileName=files[fnbr];
        //std::cout<<"starting file, "<<inputFileName<<std::endl;
        std::vector<Args> args;
        read_CSV_new(inputFileName, args);
        std::cout<<"start writing files in inputFileName"<<std::endl;
        //std::cout<<"read big file success"<<std::endl;
    for(int k=0;k<args.size();k++){
        
        if(is_edge_edge!=args[k].is_edge_edge){
            is_edge_edge=args[k].is_edge_edge;
            //std::cout<<"switch method, is_edge_edge, "<<is_edge_edge<<std::endl;
        }



        
        //for (auto& entry : std::filesystem::directory_iterator(args[k].data_dir)) {
           
            // when old_name!= args[k].data_dir,load new vertices all_V
            if(old_name!=args[k].data_dir){
                
                // all_V has n rows, meaning have n/8 queries
                all_V= read_rational_CSV_get_string(args[k].data_dir);// args[k] get updated first, then update all_V
                if(all_V.size()%8!=0){
                    std::cout<<"ERROR, all_V should have 8*query_number rows"<<std::endl;
                }
                std::vector<bool> ccd_results;
                for(int j=0;j<all_V.size()/8;j++){
                    for(int l=0;l<8;l++){// duplicate the results
                        ccd_results.push_back(args[k+j].result);
                    }
                        
                }
                
                old_name=args[k].data_dir;
                //std::cout<<"checking file, "<<old_name<<std::endl;
                write_csv_data_result(all_V,ccd_results,args[k].data_dir+"_final.csv");
                ccd_results.clear();

            }
            

    }

    }
    std::cout<<"successfully finish writing files in inputFileName"<<std::endl;
    
    
}
struct write_format{
    std::string file;
    int nbr;
    bool is_edge_edge;
    bool result;
    bool ground_truth;
    double time;
    int method;
};
void write_results_csv(const std::string& file,const std::vector<write_format>& qs){
    
    std::ofstream fout;
    fout.open(file);
    fout<<"file,nbr,method,is_edge_edge,result,ground_truth,time"<<std::endl;
    for(int i=0;i<qs.size();i++){
        fout<<qs[i].file<<","<<qs[i].nbr<<","<<qs[i].method<<","<<qs[i].is_edge_edge<<","<<qs[i].result<<","
        <<qs[i].ground_truth<<","<<qs[i].time<<std::endl;
    }
    fout.close();
}
void write_summary(const std::string file, const int method, const int total_number, const int positives,
const bool is_edge_edge, const int fp, const int fn, const double average_time){
    std::ofstream fout;
    fout.open(file);
    fout<<"method,total_nbr,positives,is_edge_edge,fp,fn,average_time"<<std::endl;
    fout<<method<<","<<total_number<<","<<positives<<","<<is_edge_edge<<","<<fp<<","<<fn<<','<<average_time<<std::endl;
    // fout<<"method, "<<method<<std::endl;
    // fout<<"total nbr, "<<total_number<<std::endl;
    // fout<<"positives, "<<positives<<std::endl;
    // fout<<"is_edge_edge, "<<is_edge_edge<<std::endl;
    // fout<<"fp, "<<fp<<std::endl;
    // fout<<"fn, "<<fn<<std::endl;
    // fout<<"average time, "<<average_time<<std::endl;
    fout.close();


}
void write_iteration_info(const std::string file, const double ratio, const double max_tol,const double avg_tol){
    std::ofstream fout;
    fout.open(file);
    fout<<"ratio,max_tol,avg_tol"<<std::endl;
    fout<<ratio<<","<<max_tol<<","<<avg_tol<<std::endl;
    
    fout.close();
}

//#define TEMP_CHECK
void run_rational_data_single_method(bool use_msccd,double minimum_seperation,const double tolerance,CCDMethod method,
const std::vector<std::string>& files ,const std::string folder,const std::string tail="", const int max_itr=1e6){
    
    bool fix_for_debug=0;
    

    igl::Timer timer;

    int num_queries = 0;
    double new_timing=0.0;
    int total_positives=0;
    int new_false_positives=0;
    int new_false_negatives=0;
    std::string old_name="empty";

    int total_number=-1;
    Eigen::MatrixXd all_V;
    std::vector<bool> results;
    long i;
    bool is_edge_edge=false;
    std::vector<write_format> queryinfo;
    int nbr_larger_tol=0;
    int nbr_diff_tol=0;
    double max_tol=0;
    double sum_tol=0;
    for(int fnbr=0;fnbr<files.size();fnbr++){
        const std::string inputFileName=files[fnbr];
        //std::cout<<"starting file, "<<inputFileName<<std::endl;
        std::vector<Args> args;
        read_CSV_new(inputFileName, args);
        //std::cout<<"read big file success"<<std::endl;
    for(int k=0;k<args.size();k++){
        if(fix_for_debug){
            if (args[k].data_dir
        !="/home/zachary/Development/ccd-queries/erleben-cube-cliff-edges/edge-edge/Rational_small_9.csv"){
            continue;
        }
        }
        
        i=args[k].nbr;
        if(is_edge_edge!=args[k].is_edge_edge){
            is_edge_edge=args[k].is_edge_edge;
            //std::cout<<"switch method, is_edge_edge, "<<is_edge_edge<<std::endl;
        }


        total_number+=1;
        if(total_number%10000==0){
            std::cout<<"finished "<<total_number<<std::endl;
            
        }
        #ifdef TEMP_CHECK
        if(total_number<7644){
                continue;
            }
            if(total_number>7644){
                exit(0);
            }
        #endif
        //for (auto& entry : std::filesystem::directory_iterator(args[k].data_dir)) {
           
            // when old_name!= args[k].data_dir,load new vertices all_V
            if(old_name!=args[k].data_dir){
                results.clear();
                //TODO here we changed the result source
                all_V= read_rational_CSV(args[k].data_dir+"_final.csv",results);
               
                assert(all_V.rows() % 8 == 0 && all_V.cols() == 3);
                
                old_name=args[k].data_dir;
                //std::cout<<"checking file, "<<old_name<<std::endl;
            }
            

            Eigen::Matrix<double, 8, 3> V = all_V.middleRows<8>(8 * i);
            
//std::cout<<"V\n"<<V<<std::endl;
            timer.start();
            ///////////////////////////////////////////////////////
            // int pid=13649+1;
            // if(total_number<pid){
            //     continue;
            // }
            // if(total_number==pid){
            //     std::cout<<"trying "<<total_number<<std::endl;
            //     std::cout<<V<<std::endl;
            // }
            // if(total_number>pid){
            //     std::cout<<"over"<<std::endl;
            // }
            ///////////////////////////////////////////////////////
            bool new_result;

            if(method!=CCDMethod::OURS){
            if (use_msccd) {
                if (is_edge_edge) {
                    new_result = edgeEdgeMSCCD(
                        V.row(0), V.row(1), V.row(2), V.row(3), V.row(4),
                        V.row(5), V.row(6), V.row(7), minimum_seperation,
                        method,tolerance);
                } else {
                    new_result = vertexFaceMSCCD(
                        V.row(0), V.row(1), V.row(2), V.row(3), V.row(4),
                        V.row(5), V.row(6), V.row(7), minimum_seperation,
                        method,tolerance);
                }
            } else {
                if (is_edge_edge) {
                    new_result = edgeEdgeCCD(
                        V.row(0), V.row(1), V.row(2), V.row(3), V.row(4),
                        V.row(5), V.row(6), V.row(7), method,tolerance);
                        //std::cout<<"edge edge check"<<std::endl;
                } else {
                    new_result = vertexFaceCCD(
                        V.row(0), V.row(1), V.row(2), V.row(3), V.row(4),
                        V.row(5), V.row(6), V.row(7), method,tolerance);
                }
            }
        }else{// our method

            const std::array<double, 3> err={{-1,-1,-1}};
            
            double toi;
            const double t_max = 1;
             
            double output_tolerance=tolerance;
            const int CCD_TYPE=1;// 0, normal ccd method which only checks t = [0,1]; 1, ccd with max_itr and t=[0, t_max]
            if (is_edge_edge) {
                new_result = edgeEdgeCCD_OURS(
                    V.row(0), V.row(1), V.row(2), V.row(3), V.row(4),
                    V.row(5), V.row(6), V.row(7),
                    err,minimum_seperation,toi,tolerance,t_max,max_itr,output_tolerance,CCD_TYPE);
                    //std::cout<<"edge edge check"<<std::endl;
            } else {
                new_result = vertexFaceCCD_OURS(
                    V.row(0), V.row(1), V.row(2), V.row(3), V.row(4),
                    V.row(5), V.row(6), V.row(7),
                    err,minimum_seperation,toi,tolerance,t_max,max_itr,output_tolerance,CCD_TYPE);
            }
            
            sum_tol+=output_tolerance;
            max_tol=std::max(output_tolerance,max_tol);
            if(output_tolerance>tolerance){
                nbr_larger_tol++;
            }
            if(output_tolerance!=tolerance){
                nbr_diff_tol++;
            }

        }

        #ifdef TEMP_CHECK
	    std::cout<<"is edge edge? "<<is_edge_edge<<std::endl;
	    std::cout<<"V\n"<<V<<std::endl;
            intervalccd::print_time_1();
    intervalccd::print_time_2();
    #endif
            timer.stop();
            new_timing += timer.getElapsedTimeInMicroSec();
            // if(new_timing>1000000){
            //     std::cout<<"file,"<<inputFileName<<", i, "<<i<<std::endl;
            //     std::cout<<"time, "<<new_timing<<std::endl;
            // }
            std::cout << total_number << "\r" << std::flush;
            
            // exit(0);
            
            //TODO here we changed the result source
            bool expected_result = results[i*8];//args[k].result;// from mathematica
            write_format qif;
            qif.method=method;
            qif.nbr=i;
            qif.result=new_result;
            qif.ground_truth=expected_result;
            qif.is_edge_edge=is_edge_edge;
            qif.time=timer.getElapsedTimeInMicroSec();
            qif.file=inputFileName;
            
            queryinfo.push_back(qif);

            if(expected_result) total_positives++;
            if(new_result!=expected_result){
                //std::cout<<"\nresult don't match, groud, "<<expected_result<<" ours, "<<new_result<<std::endl;

                    
                if(new_result) new_false_positives++;
                    
                if(new_result==0){
                    std::cout<<"false nega, "<<args[k].data_dir<<", "<<i<<std::endl;
                    for(int j=0;j<8;j++){
                        std::cout<<"v"<<j<<" "<<V(j,0)<<", "<<V(j,1)<<", "<<V(j,2)<<std::endl;
                        if(j==3) std::cout<<std::endl;
                    }
                    new_false_negatives++;
                    
                    if(method==CCDMethod::OURS){
                        std::cout<<"false nega number, "<<new_false_negatives<<std::endl;
                        std::cout<<"is edge? "<<args[k].is_edge_edge<<std::endl<<std::endl;
                        std::cout<<"number over all, "<<total_number<<std::endl;
                        exit(0);
                    }
                    

                }
                
            }
          
    }
    }
    std::cout<<"total number, "<<total_number+1<<std::endl;
    std::cout<<"total positives, "<<total_positives<<std::endl;
    std::cout<<"ccd method, "<<method<<std::endl;
    std::cout<<"is_edge_edge? , "<<is_edge_edge<<std::endl;
    std::cout<<"new_false_positives, "<<new_false_positives<<std::endl;
    std::cout<<"new_false_negatives, "<<new_false_negatives<<std::endl;
    std::cout<<"average time, "<<new_timing/double(total_number+1)<<std::endl<<std::endl;
    std::cout<<"percentage of early return, "<<double(nbr_diff_tol)/total_number<<std::endl;
    std::cout<<"percentage of early and larger tol return, "<<double(nbr_larger_tol)/total_number<<std::endl;
    std::cout<<"max tol, "<<max_tol<<std::endl;
    std::cout<<"average tol, "<<sum_tol/total_number<<std::endl<<std::endl;
//    intervalccd::print_time_1();
    // intervalccd::print_time_2();
    std::cout<<"total time, "<<new_timing<<std::endl<<std::endl;
    ///home/bolun1/interval/
    write_summary(folder+"method"+std::to_string(method)+"_is_edge_edge_"+std::to_string(is_edge_edge)
    +"_"+std::to_string(total_number+1)+tail+".csv",method,total_number+1,total_positives,is_edge_edge,new_false_positives,
    new_false_negatives,new_timing/double(total_number+1));
    write_results_csv(folder+"method"+std::to_string(method)+"_is_edge_edge_"+std::to_string(is_edge_edge)
    +"_"+std::to_string(total_number+1)+"_queries"+tail+".csv",queryinfo);
    if(method==CCDMethod::OURS){
        write_iteration_info(folder+"method"+std::to_string(method)+"_is_edge_edge_"+std::to_string(is_edge_edge)
        +"_"+std::to_string(total_number+1)+"_itration"+tail+".csv",double(nbr_diff_tol)/total_number,max_tol,sum_tol/total_number);
    }
    
}
void run_all(){
    std::string file_folder="/home/bolun1/interval/data0824/";
    std::vector<std::string> files;
    files.push_back(file_folder+"truthsm_hand_crafted_new.csv");
    files.push_back(file_folder+"truthsm_simulation_vf_new.csv");

}
void select_one_method_to_run(const std::string folder, const int& mtd, const bool ms_flag, 
const double ms, const double tolerance,const std::string tail, bool run_only_simu, const int max_itr){// method
    int method=mtd;
    
    std::string folder1="/home/bolun1/interval/data0824/";
    std::vector<std::string> files;
    if(!run_only_simu){
         files.push_back(folder1+"truthsm_hand_crafted_new.csv");
       run_rational_data_single_method(ms_flag,ms,tolerance,(CCDMethod)method,files,folder,tail,max_itr);
        files.clear();
    files.push_back(folder1+"truthsm_hand_crafted_vf_new.csv");
    run_rational_data_single_method(ms_flag,ms,tolerance,(CCDMethod)method,files,folder,tail,max_itr);
        
    }
   files.clear();
        files.push_back(folder1+"truthsm_simulation_new.csv");
       run_rational_data_single_method(ms_flag,ms,tolerance,(CCDMethod)method,files,folder,tail,max_itr);
    
    // // exit(0);
    files.clear();
    files.push_back(folder1+"truthsm_simulation_vf_new.csv");
    run_rational_data_single_method(ms_flag,ms,tolerance,(CCDMethod)method,files,folder,tail,max_itr);
}

void run_select(const int select){
    int method; bool ms_flag; double ms=0;double tolerance=1e-6;int max_itr=1e6;
    std::string tail="";
    std::string folder="/home/bolun1/interval/data0824/";
    method=select;
    if(select==0||select==1||select==2||select==3||select==4||select==8||select==10||select==11){
        ms_flag=false;
        
    }
    else{
        if(select==5||select==6||select==7||select==9)ms_flag=true;
        else{std::cout<<"wrong method name"<<std::endl;exit(0);}
    }
    
    select_one_method_to_run(folder,method,ms_flag,ms,tolerance,tail,false, max_itr);
}

void run_ms_methods_diff_eps(int start){
    
    std::string folder="/home/bolun1/interval/data0824/diff_eps/";
    std::string tail="";
    double tolerance=1e-6;
    std::vector<std::string> ms;
    int max_itr=1e6;
    bool ms_flag=true;
    // ms.push_back("1");
    for(int i=start;i<start+10;i++){
        ms.push_back("1e-"+std::to_string(2*i));
    }

    for(int i=0;i<ms.size();i++){
        select_one_method_to_run(folder,9,ms_flag,std::stod(ms[i]),tolerance,ms[i],false,max_itr);// only simulation
        // select_one_method_to_run(folder,5,ms_flag,std::stod(ms[i]),tolerance,ms[i],false,max_itr);
    }
    
}
void run_our_methods_diff_delta(int start){
    
    std::string folder="/home/bolun1/interval/data0824/diff_delta/";
    std::string tail = "";
    double ms=0;
    std::vector<std::string> tolerance;
    bool ms_flag=true;
    int max_itr=1e6;
    // tolerance.push_back("1");
    for(int i=0;i<9;i++){
        tolerance.push_back("1e-"+std::to_string(i));
    }

    for(int i=0;i<tolerance.size();i++){// temporarly changed to run the last data
        select_one_method_to_run(folder,9,ms_flag,ms,std::stod(tolerance[i]),tolerance[i],true,max_itr);
    }
    
}
void run_our_methods_diff_max_itr(){
    
    std::string folder="/home/bolun1/interval/data0824/diff_itr/";
    std::string tail = "";
    double ms=0;
    std::vector<std::string> max_itr;
    bool ms_flag=true;
    double tol=1e-6;
    // max_itr.push_back("1");
    for(int i=2;i<8;i++){
        max_itr.push_back("1e"+std::to_string(i));
    }

    for(int i=0;i<max_itr.size();i++){// temporarly changed to run the last data
        int itr=std::stod(max_itr[i]);
        std::cout<<"should run "<<max_itr[i]<<std::endl;
        std::cout<<"running max itration = "<<itr<<std::endl;
        select_one_method_to_run(folder,9,ms_flag,ms,tol,max_itr[i],true,itr);
    }
    
}

int main(int argc, char* argv[])
{   
    std::string select_content=argv[1];
    if(select_content=="0"){
        int select=std::stoi(argv[2]);
        std::cout<<"running method "<<select<<std::endl;
        run_select(select);
    }
    if(select_content=="1"){
        std::cout<<"running different eps, input start number should be 0, 10, 20, 30,..."<<std::endl;
        int start=std::stoi(argv[2]);
        run_ms_methods_diff_eps(start);
    }
    if(select_content=="2"){
        std::cout<<"running different delta"<<std::endl;
        // int start=std::stoi(argv[2]);
        run_our_methods_diff_delta(0);
    }
    if(select_content=="3"){
        std::cout<<"running different max_itr"<<std::endl;
        // int start=std::stoi(argv[2]);
        run_our_methods_diff_max_itr();
    }
    if(select_content=="1024"){
    
    std::string folder1="/home/bolun1/interval/data0824/";
    std::vector<std::string> files;

    files.clear();
    files.push_back(folder1+"truthsm_hand_crafted_new.csv");
    write_final_version_data_set(files);

    files.clear();
    files.push_back(folder1+"truthsm_hand_crafted_vf_new.csv");
    write_final_version_data_set(files);
        
    files.clear();
    files.push_back(folder1+"truthsm_simulation_new.csv");
    write_final_version_data_set(files);
    
    // // exit(0);
    files.clear();
    files.push_back(folder1+"truthsm_simulation_vf_new.csv");
    write_final_version_data_set(files);
    }
    
    
    // run_all();
    // const std::string inputFileName="/home/bolun1/interval/new_truths.csv";
    // std::vector<Args> args;
    // read_CSV(inputFileName, args);
    
    // std::cout<<"args13653, "<<args[13653].data_dir<<", "<<args[13653].nbr<<", is collision,"<<args[13653].result
    // <<" is edge edge, "<<args[13653].is_edge_edge<<std::endl;
    // run_csv_file();
    // run_rational_csv_file();
    //  run_rational_data_single_method(false,0,CCDMethod::RATIONAL_ROOT_PARITY);

    {
// // std::string file="/home/bolun1/interval/two_roots_vf.csv";
//     // bool is_ee=false;
//     // std::string file="/home/bolun1/interval/two_roots_ee.csv";
//     // bool is_ee=true;
//     std::string file="/home/bolun1/interval/cop_vf.csv";
//     bool is_ee=false;
//     run_single_rational_csv_file(true,0, CCDMethod::OURS, is_ee, file);
//     run_single_rational_csv_file(true,0, CCDMethod::EXACT_DOUBLE_MIN_SEPARATION, is_ee, file);
//     run_single_rational_csv_file(false,0, CCDMethod::RATIONAL_ROOT_PARITY, is_ee, file);
//     std::cout<<"start BSC "<<std::endl;
//     run_single_rational_csv_file(false,0, CCDMethod::BSC, is_ee, file);
//     std::cout<<"start TIGHT_CCD "<<std::endl;
//     run_single_rational_csv_file(false,0, CCDMethod::TIGHT_CCD, is_ee, file);
//     std::cout<<"start FLOAT "<<std::endl;
//     run_single_rational_csv_file(false,0, CCDMethod::FLOAT, is_ee, file);
    }
    
    //write hdf5 file into rational string csv files
    // get_rational_vertices(argc,argv);
    return 0;
    
}

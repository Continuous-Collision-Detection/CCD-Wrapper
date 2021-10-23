#include "io.h"


std::vector<std::string> simulation_folders = {{"chain", "cow-heads", "golf-ball", "mat-twist"}};
std::vector<std::string> handcrafted_folders = {{"erleben-sliding-spike", "erleben-spike-wedge",
                                                 "erleben-sliding-wedge", "erleben-wedge-crack", "erleben-spike-crack",
                                                 "erleben-wedges", "erleben-cube-cliff-edges", "erleben-spike-hole",
                                                 "erleben-cube-internal-edges", "erleben-spikes", "unit-tests"}};

std::vector<std::string> file_path_base()
{
    // path is in the form of "chain/edge-edge/"
    std::vector<std::string> result;
    result.reserve(9999);
    for (int i = 1; i < 10000; i++)
    {
        std::string base;
        if (i < 10)
        {
            base = "000" + std::to_string(i);
        }
        if (i >= 10 && i < 100)
        {
            base = "00" + std::to_string(i);
        }
        if (i >= 100 && i < 1000)
        {
            base = "0" + std::to_string(i);
        }
        if (i >= 1000 && i < 10000)
        {
            base = std::to_string(i);
        }
        result.push_back(base);
    }
    return result;
}


// using namespace std;

void read_rational_binary(
   const std::string& inputFileName, std::vector<std::array<double, 3>>& vf
)
{

    vf.clear();
    std::ifstream infile (inputFileName, std::ios::in | std::ios::binary);
    std::streampos fileSize;

    infile.seekg(0, std::ios::end);
    fileSize = infile.tellg();
    infile.seekg(0, std::ios::beg);
    // std::cout << "fileSize: " << fileSize << std::endl;
    
    
    

    // read the data:
    std::vector<double> vs;
    vs.resize(fileSize/sizeof(double));
    infile.read(reinterpret_cast<char*>(vs.data()), fileSize);
    // vs.insert(vs.begin(),
    //            std::istream_iterator<double>(infile),
    //            std::istream_iterator<double>());
    // std::cout << "1st val: " << vs[0] << std::endl;

    
    for (size_t i=0; i < vs.size(); i+=3)
    {
        // std::cout << "vs[i]: " << vs[i] << std::endl;
        std::array<double,3> a = {vs[i],vs[i+1], vs[i+2]};
        vf.push_back(a);
    }
    // infile.close();

    // while (!infile.eof())
    // {
    //     infile.read(reinterpret_cast<char*>(&v[0]), sizeof(double)*3);
    //     vs.push_back(v);
    //     // std::cout << "vs.size: " << vs.size() << std::endl;
    // }
}

void read_rational_binary(
   const std::string& inputFileName, std::vector<bool>& results
)
{
    results.clear();
    // std::vector<std::array<double, 3>> vs;
    // vs.clear();
    std::ifstream infile (inputFileName, std::ios::in | std::ios::binary);
    // infile.open(inputFileName);
    // std::array<double, 3> v;
    // std::cout << inputFileName << std::endl;

    std::vector<int> tmp;
    // int r;

    std::streampos fileSize;
    infile.seekg(0, std::ios::end);
    fileSize = infile.tellg();
    infile.seekg(0, std::ios::beg);
    // std::cout << "fileSize: " << fileSize << std::endl;

    // tmp.resize(fileSize / sizeof(int));
    tmp.resize( fileSize / sizeof(int) );
    infile.read(reinterpret_cast<char*>(tmp.data()), fileSize);
    // tmp.insert(tmp.begin(),
    //            std::istream_iterator<int>(infile),
    //            std::istream_iterator<int>());

    for (auto t : tmp)
    {
      results.push_back(t != 0);
    }
    // infile.close();
    // while (!infile.eof())
    // {
    //     infile.read(reinterpret_cast<char*>(r), sizeof(int));
    //     results.push_back(r != 0);
    // }
} 

void toBinary(
    std::string filename,
    std::vector<std::array<double, 3>>& all_V
)
{
    std::ofstream myFile (filename, std::ios::out | std::ios::binary);
    // Prefer container.data() over &container[0]
    std::vector<double> vs;
    for (auto v : all_V)
        {
            vs.push_back(v[0]);
            vs.push_back(v[1]);
            vs.push_back(v[2]);
        }
    myFile.write (reinterpret_cast<char*>(vs.data()), vs.size()*sizeof(double));
    myFile.close();
}

void toBinary(
    std::string filename,
    std::vector<int>& results
)
{
    std::ofstream myFile (filename, std::ofstream::out | std::ofstream::binary);
    // Prefer container.data() over &container[0]
    myFile.write (reinterpret_cast<char*>(results.data()), results.size()*sizeof(int));
    myFile.close();
}


std::vector<std::array<double,3>> 
read_rational_csv_bin(const std::string& filename, std::vector<bool>& results)
{
    Eigen::MatrixXd V=ccd::read_rational_csv(filename, results);
    std::vector<std::array<double,3>>  all_V(V.rows());
    for(int i=0;i<V.rows();i++){
        all_V[i][0]=V(i,0);
        all_V[i][1]=V(i,1);
        all_V[i][2]=V(i,2);
    } 
    // std::cout<< "results: " << results.size() << std::endl;

    // need to store contiguosly for bitstream
    std::vector<int> resultsAsInt;
    for (auto t : results)
        resultsAsInt.push_back(int(t));
    // std::cout << "size of results: " << resultsAsInt.size() << std::endl;

    std::string filename_noext =  filename.substr(0, filename.find_last_of("."));
    toBinary(std::string(filename_noext + "_vertex.bin"), all_V);
    toBinary(std::string(filename_noext + "_result.bin"), resultsAsInt);

    return all_V;
}



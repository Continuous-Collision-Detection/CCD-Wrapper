// Time the different CCD methods

#include <string>

#include <Eigen/Core>
#include <boost/filesystem.hpp>
#include <fmt/format.h>
#include <highfive/H5Easy.hpp>

#include <ccd.hpp>

using namespace ccd;

int parse_args(
    int argc, char* argv[], std::string& data_dir, bool& is_edge_edge)
{
    const std::string usage
        = fmt::format("usage: {} /path/to/data/ {{vf|ee}}", argv[0]);
    if (argc < 2) {
        std::cerr << usage << std::endl
                  << "error: missing path to data" << std::endl;
        return 1;
    }
    if (argc < 3) {
        std::cerr << usage << std::endl
                  << "error: missing type of collisions" << std::endl;
        return 2;
    } else if (strcmp(argv[2], "vf") != 0 && strcmp(argv[2], "ee") != 0) {
        std::cerr << usage << std::endl
                  << "invalid type of collisions: " << argv[2] << std::endl;
        return 2;
    }

    data_dir = std::string(argv[1]);
    is_edge_edge = strcmp(argv[2], "ee") == 0;
    return 0;
}

int main(int argc, char* argv[])
{
    std::string data_dir;
    bool is_edge_edge;
    int code = parse_args(argc, argv, data_dir, is_edge_edge);
    if (code) {
        return code;
    }

    for (auto& entry : boost::filesystem::directory_iterator(data_dir)) {
        if (boost::filesystem::extension(entry.path()) != ".hdf5"
            && boost::filesystem::extension(entry.path()) != ".h5") {
            continue;
        }
        std::cout << entry.path().string() << std::endl;
        H5Easy::File file(entry.path().string(), HighFive::File::OpenOrCreate);

        const auto query_names = file.getGroup("/").listObjectNames();

        for (size_t i = 0; i < query_names.size(); i++) {
            const auto& query_name = query_names[i];
            Eigen::Matrix<double, 8, 3> V
                = H5Easy::load<Eigen::Matrix<double, 8, 3>>(
                    file, fmt::format("{}/points", query_name));

            bool result;
            if (is_edge_edge) {
                result = edgeEdgeCCD(
                    V.row(0), V.row(1), V.row(2), V.row(3), V.row(4), V.row(5),
                    V.row(6), V.row(7), CCDMethod::RATIONAL_ROOT_PARITY);
            } else {
                result = vertexFaceCCD(
                    V.row(0), V.row(1), V.row(2), V.row(3), V.row(4), V.row(5),
                    V.row(6), V.row(7), CCDMethod::RATIONAL_ROOT_PARITY);
            }

            H5Easy::dump(
                file, fmt::format("{}/result", query_name),
                (unsigned char)result, H5Easy::DumpMode::Overwrite);

            std::cout << i << "\r" << std::flush;
        }
        std::cout << std::endl;
    }
}

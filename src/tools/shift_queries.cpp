// Time the different CCD methods

#include <string>

#include <Eigen/Core>
#include <boost/filesystem.hpp>
#include <fmt/format.h>
#include <highfive/H5Easy.hpp>

#include <doubleCCD/double_subfunctions.h>

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

            double shift_err;
            bool result;
            if (is_edge_edge) {
                doubleccd::ee_pair input_ee_pair(
                    V.row(0), V.row(1), V.row(2), V.row(3), V.row(4), V.row(5),
                    V.row(6), V.row(7));

                doubleccd::ee_pair shifted_ee_pair;
                double shift_err = doubleccd::shift_edge_edge(
                    input_ee_pair, shifted_ee_pair);

                V.row(0) = shifted_ee_pair.a0;
                V.row(1) = shifted_ee_pair.a1;
                V.row(2) = shifted_ee_pair.b0;
                V.row(3) = shifted_ee_pair.b1;
                V.row(4) = shifted_ee_pair.a0b;
                V.row(5) = shifted_ee_pair.a1b;
                V.row(6) = shifted_ee_pair.b0b;
                V.row(7) = shifted_ee_pair.b1b;

                result = edgeEdgeCCD(
                    V.row(0), V.row(1), V.row(2), V.row(3), V.row(4), V.row(5),
                    V.row(6), V.row(7), CCDMethod::RATIONAL_ROOT_PARITY);
            } else {
                doubleccd::vf_pair input_vf_pair(
                    V.row(0), V.row(1), V.row(2), V.row(3), V.row(4), V.row(5),
                    V.row(6), V.row(7));

                doubleccd::vf_pair shifted_vf_pair;
                shift_err = doubleccd::shift_vertex_face(
                    input_vf_pair, shifted_vf_pair);

                V.row(0) = shifted_vf_pair.x0;
                V.row(1) = shifted_vf_pair.x1;
                V.row(2) = shifted_vf_pair.x2;
                V.row(3) = shifted_vf_pair.x3;
                V.row(4) = shifted_vf_pair.x0b;
                V.row(5) = shifted_vf_pair.x1b;
                V.row(6) = shifted_vf_pair.x2b;
                V.row(7) = shifted_vf_pair.x3b;

                result = vertexFaceCCD(
                    V.row(0), V.row(1), V.row(2), V.row(3), V.row(4), V.row(5),
                    V.row(6), V.row(7), CCDMethod::RATIONAL_ROOT_PARITY);
            }

            HighFive::Group shifted_query
                = file.getGroup(query_name).createGroup("shifted");
            HighFive::DataSet dataset = shifted_query.createDataSet<double>(
                "error", HighFive::DataSpace::From(shift_err));
            dataset.write(shift_err);
            dataset = shifted_query.createDataSet<unsigned char>(
                "result", HighFive::DataSpace::From((unsigned char)result));
            dataset.write((unsigned char)result);

            dataset = shifted_query.createDataSet<double>(
                "points", H5Easy::DataSpace(H5Easy::detail::eigen::shape(V)));
            Eigen::Ref<
                const Eigen::Array<double, 8, 3, Eigen::RowMajor>, 0,
                Eigen::InnerStride<1>>
                row_major(V);
            dataset.write_raw(V.data());

            std::cout << i << "\r" << std::flush;
        }
        std::cout << std::endl;
    }
}

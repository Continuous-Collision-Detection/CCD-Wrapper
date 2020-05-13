// Time the different CCD methods

#include <string>

#include <Eigen/Core>
#include <boost/filesystem.hpp>
#include <fmt/format.h>
#include <highfive/H5Easy.hpp>
#include <igl/Timer.h>
#include <nlohmann/json.hpp>

#include <ccd.hpp>
#include <utils/get_rss.hpp>

using namespace ccd;

int parse_args(
    int argc, char* argv[], std::string& data_dir, bool& is_edge_edge)
{
    const std::string usage = fmt::format(
        "usage: {} /path/to/data/ {{vf|ee}} {{0-{:d}}}", argv[0],
        NUM_CCD_METHODS - 1);
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
    CCDMethod method = CCDMethod::EXACT_DOUBLE_MIN_SEPARATION;
    bool use_shifted = true;
    int code = parse_args(argc, argv, data_dir, is_edge_edge);
    if (code) {
        return code;
    }

    igl::Timer timer;

    Eigen::ArrayXd min_distances
        = Eigen::VectorXd::LinSpaced(100, 2, -100).unaryExpr([](double x) {
              return pow(10, x);
          });
    Eigen::ArrayXd timings = Eigen::VectorXd::Zero(min_distances.size());

    size_t num_queries = 0;
    for (auto& entry : boost::filesystem::directory_iterator(data_dir)) {
        if (boost::filesystem::extension(entry.path()) != ".hdf5"
            && boost::filesystem::extension(entry.path()) != ".h5") {
            continue;
        }
        std::cout << entry.path().string() << std::endl;
        H5Easy::File file(entry.path().string());

        const auto query_names = file.getGroup("/").listObjectNames();

        for (size_t i = 0; i < query_names.size(); i++) {
            std::string query_name = fmt::format(
                "{}{}", query_names[i], use_shifted ? "/shifted" : "");
            Eigen::Matrix<double, 8, 3> V
                = H5Easy::load<Eigen::Matrix<double, 8, 3>>(
                    file, fmt::format("{}/points", query_name));
            bool expected_results = H5Easy::load<unsigned char>(
                file, fmt::format("{}/result", query_name));

            for (size_t di = 0; di < min_distances.size(); di++) {
                // Time the methods
                double d = min_distances[di];
                bool result;
                timer.start();
                if (is_edge_edge) {
                    result = edgeEdgeMSCCD(
                        V.row(0), V.row(1), V.row(2), V.row(3), V.row(4),
                        V.row(5), V.row(6), V.row(7), d, method);
                } else {
                    result = vertexFaceMSCCD(
                        V.row(0), V.row(1), V.row(2), V.row(3), V.row(4),
                        V.row(5), V.row(6), V.row(7), d, method);
                }
                timer.stop();
                timings[di] += timer.getElapsedTimeInMicroSec();

                // Count the inaccuracies
                if (result == false && expected_results == true) {
                    std::cout << fmt::format(
                                     "file={} query_name={} d={:g} {}",
                                     basename(entry.path()), query_names[i], d,
                                     "false_negatives")
                              << std::endl;
                }
            }

            std::cout << ++num_queries << "\r" << std::flush;
        }
    }

    timings /= num_queries;

    nlohmann::json timings_json;
    timings_json["collision_type"] = is_edge_edge ? "ee" : "vf";
    timings_json["min_distances"] = std::vector<double>(
        min_distances.data(), min_distances.data() + min_distances.size());
    timings_json["timings"]
        = std::vector<double>(timings.data(), timings.data() + timings.size());

    std::string benchmark_file
        = (boost::filesystem::path(data_dir) / "min_distance_timings.json")
              .string();
    std::ofstream(benchmark_file) << timings_json.dump(4);
}

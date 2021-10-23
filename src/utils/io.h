#pragma once

#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <iterator>
#include "read_rational_csv.hpp"

using namespace ccd;

extern std::vector<std::string> simulation_folders, handcrafted_folders;

struct Args
{
    std::string data_dir;
    double minimum_separation = 0;
    double tight_inclusion_tolerance = 1e-6;
    long tight_inclusion_max_iter = 1e6;
    bool run_ee_dataset = true;
    bool run_vf_dataset = true;
    bool run_simulation_dataset = true;
    bool run_handcrafted_dataset = true;
};

std::vector<std::string> file_path_base();

std::vector<std::array<double,3>> 
read_rational_csv_bin(const std::string& filename, std::vector<bool>& results);

void read_rational_binary(
   const std::string& inputFileName, std::vector<bool>& results);

void read_rational_binary(
   const std::string& inputFileName, std::vector<std::array<double, 3>>& vs);
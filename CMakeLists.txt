# Detects whether this is a top-level project
get_directory_property(HAS_PARENT PARENT_DIRECTORY)
if(HAS_PARENT)
    set(CCD_WRAPPER_TOPLEVEL_PROJECT OFF)
else()
    set(CCD_WRAPPER_TOPLEVEL_PROJECT ON)
endif()

# Check required CMake version
set(REQUIRED_CMAKE_VERSION "3.14.0")
if(CCD_WRAPPER_TOPLEVEL_PROJECT)
    cmake_minimum_required(VERSION ${REQUIRED_CMAKE_VERSION})
else()
    # Don't use cmake_minimum_required here to avoid implicitly overriding parent policies
    if(${CMAKE_VERSION} VERSION_LESS ${REQUIRED_CMAKE_VERSION})
        message(FATAL_ERROR "CMake required version to build CCD Wrapper is ${REQUIRED_CMAKE_VERSION}")
    endif()
endif()

# Include user-provided default options if available. We do that before the main
# `project()` so that we can define the C/C++ compilers from the option file.
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/CCDWrapperOptions.cmake)
    message(STATUS "Using local options file: ${CMAKE_CURRENT_SOURCE_DIR}/CCDWrapperOptions.cmake")
    include(${CMAKE_CURRENT_SOURCE_DIR}/CCDWrapperOptions.cmake)
endif()

################################################################################

project(CCDWrapper
        DESCRIPTION "A library for various continuous collision detection algorithms."
        LANGUAGES CXX)

# project-options
option(CCD_WRAPPER_WITH_UNIT_TESTS "Build unit tests using Catch2"        ${CCD_WRAPPER_TOPLEVEL_PROJECT})
option(CCD_WRAPPER_WITH_BENCHMARK  "Build exectuable for timing methods"  ${CCD_WRAPPER_TOPLEVEL_PROJECT})

# methods
option(CCD_WRAPPER_WITH_FPRF            "Enable floating-point root finder method"                      ON)
option(CCD_WRAPPER_WITH_MSRF            "Enable minimum separation root-finding method"                 ON)
option(CCD_WRAPPER_WITH_RP              "Enable root parity method"                                     ON)
option(CCD_WRAPPER_WITH_RRP             "Enable rational root parity method"                            ON)
option(CCD_WRAPPER_WITH_FPRP            "Enable floating-point root parity method"                     OFF)
mark_as_advanced(CCD_WRAPPER_WITH_FPRP) # This is a private method
option(CCD_WRAPPER_WITH_RFRP            "Enable rational fixed root parity method"                     OFF)
mark_as_advanced(CCD_WRAPPER_WITH_RFRP) # This is a private method
option(CCD_WRAPPER_WITH_BSC             "Enable Bernstein sign classification method"                   ON)
option(CCD_WRAPPER_WITH_TIGHT_CCD       "Enable TightCCD method"                                        ON)
option(CCD_WRAPPER_WITH_INTERVAL        "Enable interval-based methods"                                 ON)
option(CCD_WRAPPER_WITH_TIGHT_INCLUSION "Enable Tight Inclusion method"                                 ON)

option(CCD_WRAPPER_IS_CI_BUILD "Is this being built on GitHub Actions" OFF)
mark_as_advanced(CCD_WRAPPER_IS_CI_BUILD) # Do not change this value

# Set default minimum C++ standard
if(CCD_WRAPPER_TOPLEVEL_PROJECT)
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

### Configuration
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/ccd_wrapper/")
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/recipes/")
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/find/")

# Color output
include(ccd_wrapper_use_colors)

################################################################################
# CCD Wrapper Library
################################################################################

add_library(ccd_wrapper src/ccd.cpp)
add_library(ccd_wrapper::ccd_wrapper ALIAS ccd_wrapper)

target_include_directories(ccd_wrapper PUBLIC src)

################################################################################
# Optional Definitions
################################################################################

# For MSVC, do not use the min and max macros.
target_compile_definitions(ccd_wrapper PUBLIC NOMINMAX)

################################################################################
# Dependencies
################################################################################

# Extra warnings
include(ccd_wrapper_warnings)
target_link_libraries(ccd_wrapper PRIVATE ccd_wrapper::warnings)

# Eigen
include(eigen)
target_link_libraries(ccd_wrapper PUBLIC Eigen3::Eigen)

# Etienne Vouga's CTCD Library for the floating point root finding algorithm
if(CCD_WRAPPER_WITH_FPRF)
    include(floating_point_root_finder)
    target_link_libraries(ccd_wrapper PUBLIC ccd_wrapper::floating_point_root_finder)
    # target_compile_definitions(ccd_wrapper PUBLIC CCD_WRAPPER_WITH_FPRF=true)
endif()
target_compile_definitions(ccd_wrapper PUBLIC
    CCD_WRAPPER_WITH_FPRF=$<BOOL:${CCD_WRAPPER_WITH_FPRF}>)

# Exact-CCD from Brochu et al. [2012] and Tang et al. [2014]
if(CCD_WRAPPER_WITH_RP OR CCD_WRAPPER_WITH_BSC)
    include(root_parity_and_bernstein_sign_classification)
    target_link_libraries(ccd_wrapper PUBLIC ccd_wrapper::root_parity_and_bernstein_sign_classification)
endif()
target_compile_definitions(ccd_wrapper PUBLIC
    CCD_WRAPPER_WITH_RP=$<BOOL:${CCD_WRAPPER_WITH_RP}>)
target_compile_definitions(ccd_wrapper PUBLIC
    CCD_WRAPPER_WITH_BSC=$<BOOL:${CCD_WRAPPER_WITH_BSC}>)

# Rational implmentation of Brochu et al. [2012]
if(CCD_WRAPPER_WITH_RRP)
    include(rational_root_parity)
    target_link_libraries(ccd_wrapper PUBLIC ccd_wrapper::rational_root_parity)
endif()
target_compile_definitions(ccd_wrapper PUBLIC
    CCD_WRAPPER_WITH_RRP=$<BOOL:${CCD_WRAPPER_WITH_RRP}>)

# SafeCCD
# if(CCD_WRAPPER_WITH_SAFE_CCD)
#   target_link_libraries(ccd_wrapper PUBLIC SafeCCD)
# endif()
target_compile_definitions(ccd_wrapper PUBLIC CCD_WRAPPER_WITH_SAFE_CCD=false)

# TightCCD implmentation of Wang et al. [2015]
if(CCD_WRAPPER_WITH_TIGHT_CCD)
    include(tight_ccd)
    target_link_libraries(ccd_wrapper PUBLIC ccd_wrapper::tight_ccd)
endif()
target_compile_definitions(ccd_wrapper PUBLIC
    CCD_WRAPPER_WITH_TIGHT_CCD=$<BOOL:${CCD_WRAPPER_WITH_TIGHT_CCD}>)

# Minimum separation root finder of [Harmon et al. 2011]
if(CCD_WRAPPER_WITH_MSRF)
    include(minimum_separation_root_finder)
    target_link_libraries(ccd_wrapper PUBLIC ccd_wrapper::minimum_separation_root_finder)
endif()
target_compile_definitions(ccd_wrapper PUBLIC
    CCD_WRAPPER_WITH_MSRF=$<BOOL:${CCD_WRAPPER_WITH_MSRF}>)

# Floating-point fixed root parity
if(CCD_WRAPPER_WITH_FPRP)
    include(fixed_root_parity)
    target_link_libraries(ccd_wrapper PUBLIC ccd_wrapper::floating_point_root_parity)
endif()
target_compile_definitions(ccd_wrapper PUBLIC
    CCD_WRAPPER_WITH_FPRP=$<BOOL:${CCD_WRAPPER_WITH_FPRP}>)

# Rational fixed root parity
if(CCD_WRAPPER_WITH_RFRP)
    include(fixed_root_parity)
    target_link_libraries(ccd_wrapper PUBLIC ccd_wrapper::rational_fixed_root_parity)
endif()
target_compile_definitions(ccd_wrapper PUBLIC
    CCD_WRAPPER_WITH_RFRP=$<BOOL:${CCD_WRAPPER_WITH_RFRP}>)

# Interval-based methods
if(CCD_WRAPPER_WITH_INTERVAL)
    include(interval_based_ccd)
    target_link_libraries(ccd_wrapper PUBLIC ccd_wrapper::interval_based_ccd)
endif()
target_compile_definitions(ccd_wrapper PUBLIC
    CCD_WRAPPER_WITH_INTERVAL=$<BOOL:${CCD_WRAPPER_WITH_INTERVAL}>)

# Custom inclusion based CCD of [Wang et al. 2020]
if(CCD_WRAPPER_WITH_TIGHT_INCLUSION)
    include(tight_inclusion)
    target_link_libraries(ccd_wrapper PUBLIC tight_inclusion::tight_inclusion)
endif()
target_compile_definitions(ccd_wrapper PUBLIC
    CCD_WRAPPER_WITH_TIGHT_INCLUSION=$<BOOL:${CCD_WRAPPER_WITH_TIGHT_INCLUSION}>)

################################################################################
# Compiler options
################################################################################

# Use C++11
target_compile_features(ccd_wrapper PUBLIC cxx_std_11)

################################################################################
# Tests
################################################################################

if(CCD_WRAPPER_WITH_UNIT_TESTS)
    include(CTest)
    enable_testing()

    # Include Catch2 and provide function `catch_discover_tests` to register tests.
    include(catch2)
    FetchContent_GetProperties(catch2)
    include("${catch2_SOURCE_DIR}/contrib/Catch.cmake")

    add_subdirectory(tests)
endif()

################################################################################
# Benchmark
################################################################################

if(CCD_WRAPPER_WITH_BENCHMARK)
    add_executable(ccd_benchmark
        src/benchmark.cpp
        src/utils/read_rational_csv.cpp
    )
    target_include_directories(ccd_benchmark PUBLIC src)

    target_link_libraries(ccd_benchmark PUBLIC ccd_wrapper::ccd_wrapper)

    include(filesystem)
    target_link_libraries(ccd_benchmark PUBLIC ghc::filesystem)

    include(fmt)
    target_link_libraries(ccd_benchmark PUBLIC fmt::fmt)

    include(json)
    target_link_libraries(ccd_benchmark PUBLIC nlohmann::json)

    include(cli11)
    target_link_libraries(ccd_benchmark PUBLIC CLI11::CLI11)

    # GMP for reading rational query csv files
    find_package(GMP)
    IF(NOT ${GMP_FOUND})
        MESSAGE(FATAL_ERROR "GMP not found! Needed by CCD Benchmark for reading rational query csv files.")
    ENDIF()
    target_link_libraries(ccd_benchmark PUBLIC gmp::gmp)

    # Download Sample Queries
    include(sample_queries)
    target_compile_definitions(ccd_benchmark PUBLIC
        CCD_WRAPPER_SAMPLE_QUERIES_DIR="${CCD_WRAPPER_SAMPLE_QUERIES_DIR}")

    target_compile_features(ccd_benchmark PUBLIC cxx_std_11)

    if(CCD_WRAPPER_IS_CI_BUILD)
        target_compile_definitions(ccd_benchmark PRIVATE CCD_WRAPPER_IS_CI_BUILD)
    endif()
endif()

cmake_minimum_required(VERSION 3.8)
project(CCDWrapper)

################################################################################

# project-options
option(CCD_WRAPPER_WITH_UNIT_TESTS "Build unit tests using Catch2"                 ON)
option(CCD_WRAPPER_WITH_BENCHMARK  "Build exectuable for timing methods"          OFF)
option(CCD_WRAPPER_WITH_FPRF       "Enable floating-point root finder method"      ON)
option(CCD_WRAPPER_WITH_MSRF       "Enable minimum separation root-finding method" ON)
option(CCD_WRAPPER_WITH_RP         "Enable root parity method"                     ON)
option(CCD_WRAPPER_WITH_RRP        "Enable rational root parity method"            ON)
option(CCD_WRAPPER_WITH_MSRP       "Enable minimum separation root parity method" OFF)
mark_as_advanced(CCD_WRAPPER_WITH_MSRP) # This is a private method
option(CCD_WRAPPER_WITH_BSC        "Enable Bernstein sign classification method"   ON)
option(CCD_WRAPPER_WITH_TIGHT_CCD  "Enable TightCCD method"                        ON)
option(CCD_WRAPPER_WITH_INTERVAL   "Enable interval-based methods"                ON)
option(CCD_WRAPPER_WITH_TIGHT_INCLUSION "Enable Tight Inclusion method"            ON)

# libigl
option(LIBIGL_USE_STATIC_LIBRARY     "Use libigl as static library" OFF)
option(LIBIGL_WITH_ANTTWEAKBAR       "Use AntTweakBar"              OFF)
option(LIBIGL_WITH_CGAL              "Use CGAL"                     OFF)
option(LIBIGL_WITH_COMISO            "Use CoMiso"                   OFF)
option(LIBIGL_WITH_CORK              "Use Cork"                     OFF)
option(LIBIGL_WITH_EMBREE            "Use Embree"                   OFF)
option(LIBIGL_WITH_LIM               "Use LIM"                      OFF)
option(LIBIGL_WITH_MATLAB            "Use Matlab"                   OFF)
option(LIBIGL_WITH_MOSEK             "Use MOSEK"                    OFF)
option(LIBIGL_WITH_OPENGL            "Use OpenGL"                   OFF)
option(LIBIGL_WITH_OPENGL_GLFW       "Use GLFW"                     OFF)
option(LIBIGL_WITH_OPENGL_GLFW_IMGUI "Use ImGui"                    OFF)
option(LIBIGL_WITH_PNG               "Use PNG"                      OFF)
option(LIBIGL_WITH_PYTHON            "Use Python"                   OFF)
option(LIBIGL_WITH_TETGEN            "Use Tetgen"                   OFF)
option(LIBIGL_WITH_TRIANGLE          "Use Triangle"                 OFF)
option(LIBIGL_WITH_VIEWER            "Use OpenGL viewer"            OFF)
option(LIBIGL_WITH_XML               "Use XML"                      OFF)
option(LIBIGL_WITH_PREDICATES        "Use exact predicates"          ON)

################################################################################

### Configuration
set(CCD_WRAPPER_EXTERNAL "${CMAKE_CURRENT_SOURCE_DIR}/external")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(Warnings)
include(UseColors)
include(${PROJECT_NAME}Dependencies)

################################################################################
# Library
################################################################################

add_library(${PROJECT_NAME} src/ccd.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC src)

################################################################################
# Required Libraries
################################################################################

# Eigen
target_link_libraries(${PROJECT_NAME} PUBLIC Eigen3::Eigen)

# Etienne Vouga's CTCD Library for the floating point root finding algorithm
if(CCD_WRAPPER_WITH_FPRF)
  target_link_libraries(${PROJECT_NAME} PUBLIC FloatingPointRootFinder)
  target_compile_definitions(${PROJECT_NAME} PUBLIC CCD_WRAPPER_WITH_FPRF)
endif()

# Exact-CCD from Brochu et al. [2012] and Tang et al. [2014]
if(CCD_WRAPPER_WITH_RP OR CCD_WRAPPER_WITH_BSC)
  target_link_libraries(${PROJECT_NAME} PUBLIC RootParity_and_BernsteinSignClassification)
  target_compile_definitions(${PROJECT_NAME} PUBLIC CCD_WRAPPER_WITH_RP)
  target_compile_definitions(${PROJECT_NAME} PUBLIC CCD_WRAPPER_WITH_BSC)
endif()

# Rational implmentation of Brochu et al. [2012]
if(CCD_WRAPPER_WITH_RRP)
  target_link_libraries(${PROJECT_NAME} PUBLIC RationalRootParity)
  target_compile_definitions(${PROJECT_NAME} PUBLIC CCD_WRAPPER_WITH_RRP)
endif()

# TightCCD implmentation of Wang et al. [2015]
if(CCD_WRAPPER_WITH_TIGHT_CCD)
  target_link_libraries(${PROJECT_NAME} PUBLIC TightCCD)
  target_compile_definitions(${PROJECT_NAME} PUBLIC CCD_WRAPPER_WITH_TIGHT_CCD)
endif()

# SafeCCD
if(CCD_WRAPPER_WITH_SAFE_CCD)
  target_link_libraries(${PROJECT_NAME} PUBLIC SafeCCD)
  target_compile_definitions(${PROJECT_NAME} PUBLIC CCD_WRAPPER_WITH_SAFE_CCD)
endif()

# Minimum separation root finder of [Harmon et al. 2011]
if(CCD_WRAPPER_WITH_MSRF)
  target_link_libraries(${PROJECT_NAME} PUBLIC MinimumSeparationRootFinder)
  target_compile_definitions(${PROJECT_NAME} PUBLIC CCD_WRAPPER_WITH_MSRF)
endif()

# Root parity with minimum separation
if(CCD_WRAPPER_WITH_MSRP)
  target_link_libraries(${PROJECT_NAME} PUBLIC MSRootParity)
  target_link_libraries(${PROJECT_NAME} PUBLIC RationalMSRootParity)
  target_compile_definitions(${PROJECT_NAME} PUBLIC CCD_WRAPPER_WITH_MSRP)
endif()

# Interval-based methods
if(CCD_WRAPPER_WITH_INTERVAL)
  target_link_libraries(${PROJECT_NAME} PUBLIC IntervalBased)
  target_compile_definitions(${PROJECT_NAME} PUBLIC CCD_WRAPPER_WITH_INTERVAL)
endif()

# Custom inclusion based CCD of [Wang et al. 2020]
if(CCD_WRAPPER_WITH_TIGHT_INCLUSION)
  target_link_libraries(${PROJECT_NAME} PUBLIC TightInclusion)
  target_compile_definitions(${PROJECT_NAME} PUBLIC CCD_WRAPPER_WITH_TIGHT_INCLUSION)
endif()

# Extra warnings
target_link_libraries(${PROJECT_NAME} PRIVATE warnings::all)

################################################################################
# Compiler options
################################################################################

# Use C++17
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

# Fix min and max on Windows
target_compile_definitions(${PROJECT_NAME} PUBLIC NOMINMAX)

################################################################################
# Tests
################################################################################

if(CCD_WRAPPER_WITH_UNIT_TESTS)
  # Catch2
  if(NOT TARGET Catch2::Catch2)
      ccd_wrapper_download_catch2()
      add_subdirectory(${CCD_WRAPPER_EXTERNAL}/Catch2 catch2)
      list(APPEND CMAKE_MODULE_PATH ${CCD_WRAPPER_EXTERNAL}/Catch2/contrib)
  endif()

  include(CTest)
  enable_testing()
  add_subdirectory(tests)
endif()

################################################################################
# Benchmark
################################################################################

if(CCD_WRAPPER_WITH_BENCHMARK)
  add_executable(${PROJECT_NAME}_benchmark
    src/benchmark.cpp
    src/utils/get_rss.cpp
  )
  target_link_libraries(${PROJECT_NAME}_benchmark PUBLIC ${PROJECT_NAME})
  target_link_libraries(${PROJECT_NAME}_benchmark PUBLIC igl::core)
  target_link_libraries(${PROJECT_NAME}_benchmark PUBLIC HighFive::HighFive)
  target_link_libraries(${PROJECT_NAME}_benchmark PUBLIC fmt::fmt)
  target_link_libraries(${PROJECT_NAME}_benchmark PUBLIC nlohmann_json::nlohmann_json)
  target_link_libraries(${PROJECT_NAME}_benchmark PUBLIC CLI11::CLI11)
  target_compile_features(${PROJECT_NAME}_benchmark PUBLIC cxx_std_14)
  target_include_directories(${PROJECT_NAME}_benchmark PUBLIC ${GMP_INCLUDE_DIRS} src)
  target_link_libraries(${PROJECT_NAME}_benchmark PUBLIC ${GMP_LIBRARIES})

  if(CCD_WRAPPER_WITH_MSRP)
    add_executable(round_queries src/tools/round_queries.cpp)
    target_link_libraries(round_queries PUBLIC ${PROJECT_NAME})
    target_link_libraries(round_queries PUBLIC HighFive::HighFive)
    target_link_libraries(round_queries PUBLIC fmt::fmt)
  endif()

  # add_executable(${PROJECT_NAME}_check_ground_truth src/check_ground_truth.cpp)
  # target_link_libraries(${PROJECT_NAME}_check_ground_truth PUBLIC ${PROJECT_NAME})
  # target_link_libraries(${PROJECT_NAME}_check_ground_truth PUBLIC igl::core)
  # target_link_libraries(${PROJECT_NAME}_check_ground_truth PUBLIC HighFive::HighFive)
  # target_link_libraries(${PROJECT_NAME}_check_ground_truth PUBLIC fmt::fmt)
  # target_link_libraries(${PROJECT_NAME}_check_ground_truth PUBLIC nlohmann_json::nlohmann_json)
  # target_link_libraries(${PROJECT_NAME}_check_ground_truth PUBLIC CLI11::CLI11)
  # target_compile_features(${PROJECT_NAME}_check_ground_truth PUBLIC cxx_std_14)
endif()

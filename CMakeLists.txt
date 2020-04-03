cmake_minimum_required(VERSION 3.8)
project(CCDWrapper)

################################################################################

# project-options
option(CCD_WRAPPER_WITH_UNIT_TESTS   "Build unit tests using Catch2"         ON)
option(CCD_WRAPPER_WITH_BENCHMARK    "Build exectuable for timing methods"  OFF)

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
option(LIBIGL_WITH_PREDICATES        "Use exact predicates"         OFF)

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

add_library(${PROJECT_NAME}
  src/ccd.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC src)

################################################################################
# Required Libraries
################################################################################

# Eigen
target_link_libraries(${PROJECT_NAME} PUBLIC Eigen3::Eigen)

# Etienne Vouga's CTCD Library for the floating point root finding algorithm
target_link_libraries(${PROJECT_NAME} PUBLIC EVCTCD)

# Exact-CCD from Brochu et al. [2012] and Tang et al. [2014]
target_link_libraries(${PROJECT_NAME} PUBLIC exact-ccd::exact-ccd)

# Rational implmentation of Brochu et al. [2012]
target_link_libraries(${PROJECT_NAME} PUBLIC RationalCCD)

# TightCCD implmentation of Wang et al. [2015]
target_link_libraries(${PROJECT_NAME} PUBLIC TightCCD)

# Extra warnings
target_link_libraries(${PROJECT_NAME} PRIVATE warnings::all)

################################################################################
# Compiler options
################################################################################

# Use C++11
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_11)

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
  )
  target_link_libraries(${PROJECT_NAME}_benchmark PUBLIC ${PROJECT_NAME})
  target_link_libraries(${PROJECT_NAME}_benchmark PUBLIC igl::core)
  target_link_libraries(${PROJECT_NAME}_benchmark PUBLIC HighFive::HighFive)

  # Boost
  set(Boost_USE_MULTITHREADED TRUE)
  find_package(Boost COMPONENTS filesystem REQUIRED)
  target_link_libraries(${PROJECT_NAME}_benchmark PUBLIC Boost::filesystem)
endif()

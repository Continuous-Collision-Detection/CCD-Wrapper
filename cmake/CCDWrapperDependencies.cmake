# Prepare dependencies
#
# For each third-party library, if the appropriate target doesn't exist yet,
# download it via external project, and add_subdirectory to build it alongside
# this project.


# Download and update 3rd_party libraries
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
list(REMOVE_DUPLICATES CMAKE_MODULE_PATH)
include(${PROJECT_NAME}DownloadExternal)

################################################################################
# Required libraries
################################################################################

# Eigen
if(NOT TARGET Eigen3::Eigen)
  ccd_wrapper_download_eigen()
  add_library(${PROJECT_NAME}_eigen INTERFACE)
  target_include_directories(${PROJECT_NAME}_eigen SYSTEM INTERFACE
    $<BUILD_INTERFACE:${CCD_WRAPPER_EXTERNAL}/eigen>
    $<INSTALL_INTERFACE:include>
  )
  set_property(TARGET ${PROJECT_NAME}_eigen PROPERTY EXPORT_NAME Eigen3::Eigen)
  add_library(Eigen3::Eigen ALIAS ${PROJECT_NAME}_eigen)
endif()

# Etienne Vouga's CTCD Library
if(CCD_WRAPPER_WITH_FPRF AND NOT TARGET FloatingPointRootFinder)
  ccd_wrapper_download_floating_point_root_finder()

  file(GLOB FLOATING_POINT_ROOT_FINDER_FILES "${CCD_WRAPPER_EXTERNAL}/Floating-Point-Root-Finder/src/*.cpp")
  add_library(FloatingPointRootFinder ${FLOATING_POINT_ROOT_FINDER_FILES})
  target_include_directories(
    FloatingPointRootFinder PUBLIC "${CCD_WRAPPER_EXTERNAL}/Floating-Point-Root-Finder/include")
  target_link_libraries(FloatingPointRootFinder PUBLIC Eigen3::Eigen)

  # Turn off floating point contraction for CCD robustness
  target_compile_options(FloatingPointRootFinder PUBLIC "-ffp-contract=off")
endif()

# Brochu et al. [2012] and Tang et al. [2014]
if((CCD_WRAPPER_WITH_RP OR CCD_WRAPPER_WITH_BSC) AND NOT TARGET RootParity_and_BernsteinSignClassification)
  ccd_wrapper_download_root_parity_and_bernstein_sign_classification()
  add_subdirectory(${CCD_WRAPPER_EXTERNAL}/RP-and-BSC EXCLUDE_FROM_ALL)
  add_library(RootParity_and_BernsteinSignClassification ALIAS exact-ccd)
endif()

# Rational implmentation of Brochu et al. [2012]
if(CCD_WRAPPER_WITH_RRP AND NOT TARGET RationalRootParity)
  ccd_wrapper_download_rational_root_parity()
  add_subdirectory(${CCD_WRAPPER_EXTERNAL}/Rational-Root-Parity EXCLUDE_FROM_ALL)
  add_library(RationalRootParity ALIAS RationalCCD)
endif()

# Root Parity with Minimum Separation
if(CCD_WRAPPER_WITH_MSRP AND NOT TARGET MSRootParity)
  ccd_wrapper_download_minimum_separation_root_parity()
  set(CCD_WITH_UNIT_TESTS OFF CACHE BOOL "" FORCE)
  add_subdirectory(${CCD_WRAPPER_EXTERNAL}/Minimum-Separation-Root-Parity)
  add_library(MSRootParity ALIAS CCD_double)
  add_library(RationalMSRootParity ALIAS CCD_rational)
endif()

# TightCCD implmentation of Wang et al. [2015]
if(CCD_WRAPPER_WITH_TIGHT_CCD AND NOT TARGET TightCCD)
  ccd_wrapper_download_tight_ccd()
  add_subdirectory(${CCD_WRAPPER_EXTERNAL}/TightCCD EXCLUDE_FROM_ALL)
endif()

# SafeCCD
if(CCD_WRAPPER_WITH_SAFE_CCD AND NOT TARGET SafeCCD)
  add_subdirectory(${CCD_WRAPPER_EXTERNAL}/SafeCCD)
endif()

# Minimum separation root finder of [Harmon et al. 2011]
if(CCD_WRAPPER_WITH_MSRF AND NOT TARGET MinimumSeparationRootFinder)
  ccd_wrapper_download_minimum_separation_root_finder()
  add_subdirectory(${CCD_WRAPPER_EXTERNAL}/Minimum-Separation-Root-Finder EXCLUDE_FROM_ALL)
  add_library(MinimumSeparationRootFinder ALIAS MSRF_CCD)
endif()

# Interval-based CCD
if(CCD_WRAPPER_WITH_INTERVAL AND NOT TARGET IntervalBased)
  ccd_wrapper_download_interval_based()
  add_subdirectory(${CCD_WRAPPER_EXTERNAL}/Interval-Based)
  add_library(IntervalBased ALIAS INTERVAL_CCD)
endif()

# Tight Inclusion
if(CCD_WRAPPER_WITH_TIGHT_INCLUSION AND NOT TARGET TightInclusion)
  ccd_wrapper_download_tight_inclusion()
  add_subdirectory(${CCD_WRAPPER_EXTERNAL}/Tight-Inclusion)
  add_library(TightInclusion ALIAS tight_inclusion)
endif()

if(CCD_WRAPPER_WITH_BENCHMARK)
  # String formatting
  if(NOT TARGET fmt::fmt)
    ccd_wrapper_download_fmt()
    add_subdirectory(${CCD_WRAPPER_EXTERNAL}/fmt)
  endif()

  # json
  if(NOT TARGET nlohmann_json::nlohmann_json)
    ccd_wrapper_download_json()
    option(JSON_BuildTests "" OFF)
    option(JSON_MultipleHeaders "" ON)
    add_subdirectory(${CCD_WRAPPER_EXTERNAL}/json)
  endif()

  if(NOT TARGET CLI11::CLI11)
    ccd_wrapper_download_cli11()
    add_subdirectory(${CCD_WRAPPER_EXTERNAL}/CLI11)
  endif()
endif()

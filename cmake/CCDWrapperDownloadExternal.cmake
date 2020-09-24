include(DownloadProject)

# With CMake 3.8 and above, we can hide warnings about git being in a
# detached head by passing an extra GIT_CONFIG option
if(NOT (${CMAKE_VERSION} VERSION_LESS "3.8.0"))
    set(CCD_WRAPPER_EXTRA_OPTIONS "GIT_CONFIG advice.detachedHead=false")
else()
    set(CCD_WRAPPER_EXTRA_OPTIONS "")
endif()

function(ccd_wrapper_download_project name)
    download_project(
        PROJ         ${name}
        SOURCE_DIR   ${CCD_WRAPPER_EXTERNAL}/${name}
        DOWNLOAD_DIR ${CCD_WRAPPER_EXTERNAL}/.cache/${name}
        QUIET
        ${CCD_WRAPPER_EXTRA_OPTIONS}
        ${ARGN}
    )
endfunction()

################################################################################

# Eigen
function(ccd_wrapper_download_eigen)
  ccd_wrapper_download_project(eigen
    URL https://gitlab.com/libeigen/eigen/-/archive/3.3.7/eigen-3.3.7.zip
  )
endfunction()

# Etienne Vouga's CTCD Library
function(ccd_wrapper_download_floating_point_root_finder)
  ccd_wrapper_download_project(Floating-Point-Root-Finder
    GIT_REPOSITORY https://github.com/Continuous-Collision-Detection/Floating-Point-Root-Finder.git
    GIT_TAG        e5fe5c9767207df5047e375fb20180a665ae186f
  )
endfunction()

# Root Parity [Brochu et al. 2012] and BSC [Tang et al. 2014]
function(ccd_wrapper_download_root_parity_and_bernstein_sign_classification)
  ccd_wrapper_download_project(Root-Parity-and-Bernstein-Sign-Classification
    GIT_REPOSITORY https://github.com/Continuous-Collision-Detection/Root-Parity-and-Bernstein-Sign-Classification.git
    GIT_TAG        ac8a63b2aeaf83f9f3ab90a16c7a1860f037bc69
  )
endfunction()

# Rational Root Parity CCD (rational version of [Brochu et al. 2012])
function(ccd_wrapper_download_rational_root_parity)
  ccd_wrapper_download_project(Rational-Root-Parity
    GIT_REPOSITORY https://github.com/Continuous-Collision-Detection/Rational-Root-Parity.git
    GIT_TAG        4125e192ea7e4df9cd6cf13a7dac1d6016ce7c97
  )
endfunction()

# Root Parity with Minimum Separation
function(ccd_wrapper_download_minimum_separation_root_parity)
   ccd_wrapper_download_project(Minimum-Separation-Root-Parity
    GIT_REPOSITORY git@github.com:geometryprocessing/CCD.git
    GIT_TAG        ed282878600787ea6d2b15fcef0763d42eb0a438
  )
endfunction()

# TightCCD implmentation of [Wang et al. 2015]
function(ccd_wrapper_download_tight_ccd)
  ccd_wrapper_download_project(TightCCD
    GIT_REPOSITORY https://github.com/Continuous-Collision-Detection/TightCCD.git
    GIT_TAG        366384bac36ed1549d265e90c180fdd1beab42c6
  )
endfunction()

# Minimum separation root finder of [Harmon et al. 2011]
function(ccd_wrapper_download_minimum_separation_root_finder)
  ccd_wrapper_download_project(Minimum-Separation-Root-Finder
    GIT_REPOSITORY https://github.com/Continuous-Collision-Detection/Minimum-Separation-Root-Finder.git
    GIT_TAG        7f2a75d50139444c8e412a8d9b54f1c20d9e8848
  )
endfunction()

# Interval-based methods
function(ccd_wrapper_download_interval_based)
   ccd_wrapper_download_project(Interval-Based
    GIT_REPOSITORY https://github.com/Continuous-Collision-Detection/Interval-Based.git
    GIT_TAG        99ad4623ecef7677775a937965cb173cbdce6b9d
  )
endfunction()

# Tight Inclusion
function(ccd_wrapper_download_tight_inclusion)
   ccd_wrapper_download_project(Tight-Inclusion
    GIT_REPOSITORY https://github.com/Continuous-Collision-Detection/Tight-Inclusion.git
    GIT_TAG        3cc653be5aa50dd16d7e2503687bbffd243e32a9
  )
endfunction()

# Catch2 for testing
function(ccd_wrapper_download_catch2)
  ccd_wrapper_download_project(Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v2.11.1
  )
endfunction()

# libigl for timing
function(ccd_wrapper_download_libigl)
   ccd_wrapper_download_project(libigl
   GIT_REPOSITORY https://github.com/libigl/libigl.git
   GIT_TAG        aea868bd1fc64f71afecd2c51e51507a99d8e3e5
  )
endfunction()

# HighFive - Header-only C++ HDF5 interface
function(ccd_wrapper_download_high_five)
  ccd_wrapper_download_project(HighFive
    GIT_REPOSITORY https://github.com/BlueBrain/HighFive.git
    GIT_TAG        v2.2.1
  )
endfunction()

# A modern string formatting library
function(ccd_wrapper_download_fmt)
  ccd_wrapper_download_project(fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG        6.2.0
  )
endfunction()

# JSON for Modern C++
function(ccd_wrapper_download_json)
  ccd_wrapper_download_project(json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.7.3
  )
endfunction()

# Command line parser
function(ccd_wrapper_download_cli11)
  ccd_wrapper_download_project(CLI11
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
    GIT_TAG        v1.9.0
  )
endfunction()

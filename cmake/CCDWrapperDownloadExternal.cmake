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
function(ccd_wrapper_download_evctcd)
  ccd_wrapper_download_project(EVCTCD
    GIT_REPOSITORY https://github.com/evouga/collisiondetection.git
    GIT_TAG        e5fe5c9767207df5047e375fb20180a665ae186f
  )
endfunction()

# exact-ccd (clone of [Brochu et al. 2012] and [Tang et al. 2014])
function(ccd_wrapper_download_exact_ccd)
  ccd_wrapper_download_project(exact-ccd
    GIT_REPOSITORY https://github.com/zfergus/exact-ccd.git
    GIT_TAG        cf0bd9b3de14d9ae735d9b8db3b6a6e94e4f3dfe
  )
endfunction()

# Rational CCD (rational version of [Brochu et al. 2012])
function(ccd_wrapper_download_rational_ccd)
  ccd_wrapper_download_project(rational_ccd
    GIT_REPOSITORY https://github.com/teseoch/Exact-CDD.git
    GIT_TAG        63fdb92b917c8d7349c9403656f1497df6fa6ce5
  )
endfunction()

# TightCCD implmentation of [Wang et al. 2015]
function(ccd_wrapper_download_tight_ccd)
  ccd_wrapper_download_project(TightCCD
    GIT_REPOSITORY https://github.com/zfergus/TightCCD.git
    GIT_TAG        1da1fd01fffabd2611707b0b3a1096928c0da98b
  )
endfunction()

# Tight Intervals and Root Parity with Minimum Separation
function(ccd_wrapper_download_tight_msccd)
   ccd_wrapper_download_project(tight_msccd
    GIT_REPOSITORY git@github.com:geometryprocessing/CCD.git
    GIT_TAG        e6da650ee7fa3fd32593caf79386829bc6b25aae
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

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
        URL           http://bitbucket.org/eigen/eigen/get/3.3.7.tar.bz2
        URL_MD5       05b1f7511c93980c385ebe11bd3c93fa
    )
endfunction()

# Etienne Vouga's CTCD Library
function(ccd_wrapper_download_evctcd)
  ccd_wrapper_download_project(EVCTCD
    GIT_REPOSITORY https://github.com/evouga/collisiondetection.git
    GIT_TAG        e5fe5c9767207df5047e375fb20180a665ae186f
  )
endfunction()

# exact-ccd (clone of Brochu et al. [2012] and Tang et al. [2014])
function(ccd_wrapper_download_exact_ccd)
  ccd_wrapper_download_project(exact-ccd
    GIT_REPOSITORY https://github.com/jiangzhongshi/exact-ccd.git
    GIT_TAG        eb91cfb81f1e074412927496c7b3eca1778a0ce1
  )
endfunction()

# Rational CCD (rational version of Brochu et al. [2012])
function(ccd_wrapper_download_rational_ccd)
  ccd_wrapper_download_project(rational_ccd
    GIT_REPOSITORY https://github.com/teseoch/Exact-CDD.git
    GIT_TAG        94b05129efa2c33348622b2890bd464cc95f65a7
  )
endfunction()

# TightCCD implmentation of Wang et al. [2015]
function(ccd_wrapper_download_tight_ccd)
  ccd_wrapper_download_project(TightCCD
    GIT_REPOSITORY https://github.com/zfergus/TightCCD.git
    GIT_TAG        1da1fd01fffabd2611707b0b3a1096928c0da98b
  )
endfunction()

# Catch2 for testing
function(ccd_wrapper_download_catch2)
  ccd_wrapper_download_project(Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v2.11.1
  )
endfunction()

# Continuous Collision Detection Wrapper

<a href="https://travis-ci.com/zfergus/ccd-wrapper"><img src="https://travis-ci.com/zfergus/ccd-wrapper.svg?branch=master" title="Build Status" alt="Build Status"></img></a>
<a href="https://opensource.org/licenses/MIT"><img src="https://img.shields.io/github/license/zfergus/ccd.svg?color=blue"></img></a>

Wrappers for various continuous collision detection algorithms.

## Adding CCD-Wrapper to you CMake Project

You can simply add CCD-Wrapper using `DownloadProject.cmake` with these lines:

```cmake
function(${PROJECT_NAME}_download_ccd_wrapper)
  ${PROJECT_NAME}_download_project(ccd-wrapper
    GIT_REPOSITORY https://github.com/zfergus/ccd-wrapper.git
    GIT_TAG        master
  )
endfunction()

if(NOT TARGET CCDWrapper::CCDWrapper)
  ${PROJECT_NAME}_download_ccd_wrapper()
  set(CCD_WRAPPER_WITH_UNIT_TESTS OFF CACHE BOOL " " FORCE)
  # Set Eigen directory environment variable (needed for EVCTCD)
  set(ENV{EIGEN3_INCLUDE_DIR} "${PROJECT_NAME_EXTERNAL}/path/to/eigen/")
  add_subdirectory(${PROJECT_NAME_EXTERNAL}/ccd-wrapper)
  add_library(CCDWrapper::CCDWrapper ALIAS CCDWrapper)
endif()
```

# Continuous Collision Detection Wrapper

[![Build status](https://github.com/Continuous-Collision-Detection/CCD-Wrapper/workflows/Build/badge.svg)](https://github.com/Continuous-Collision-Detection/CCD-Wrapper/actions?query=workflow%3ABuild+branch%3Amaster+event%3Apush)
<a href="https://opensource.org/licenses/MIT"><img src="https://img.shields.io/github/license/Continuous-Collision-Detection/CCD-Wrapper.svg?color=blue"></img></a>

Wrappers for various continuous collision detection algorithms.

## Adding CCD-Wrapper to you CMake Project

You can simply add CCD-Wrapper using `DownloadProject.cmake` with these lines:

```cmake
function(${PROJECT_NAME}_download_ccd_wrapper)
  ${PROJECT_NAME}_download_project(ccd-wrapper
    GIT_REPOSITORY https://github.com/Continuous-Collision-Detection/CCD-Wrapper.git
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

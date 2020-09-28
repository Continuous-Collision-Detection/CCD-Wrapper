# Continuous Collision Detection Wrapper and Benchmark

[![Build status](https://github.com/Continuous-Collision-Detection/CCD-Wrapper/workflows/Build/badge.svg)](https://github.com/Continuous-Collision-Detection/CCD-Wrapper/actions?query=workflow%3ABuild+branch%3Amaster+event%3Apush)
<a href="https://opensource.org/licenses/MIT"><img src="https://img.shields.io/github/license/Continuous-Collision-Detection/CCD-Wrapper.svg?color=blue"></img></a>

A wrapper and benchmark for various continuous collision detection algorithms.

## Build

To build the library and executable benchmark on Linux or macOS run:
```sh
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```

## Dependencies

The system level dependencies vary depending on which methods are enabled. To compile all methods and the benchmark, the following are required:

* [Boost](https://www.boost.org/): for filesystem and intervals (when interval-based methods are enabled)
* [GMP](https://gmplib.org/): for rational numbers arithmetic (used when loading benchmark data)

Eigen and other dependencies will be downloaded through CMake.


## Add CCD-Wrapper to your CMake Project

You can add CCD-Wrapper using `DownloadProject.cmake` with these lines:

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
  add_subdirectory(${PROJECT_NAME_EXTERNAL}/ccd-wrapper)
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC CCDWrapper)
```

## Running the Benchmark

:warning: All benchmark results are generated on Linux using GCC.

To run the benchmark run `CCDWrapper_benchmark`.

For complete list of benchmark options run `CCDWrapper_benchmark --help`.

By default the benchmark runs on a small subset of CCD queries automatically downloaded to `external/Sample-Queries`.
The full dataset can be found [here](https://archive.nyu.edu/handle/2451/61518). Use `CCDWrapper_benchmark --data </path/to/data>` to tell the benchmark where to find the root directory of the dataset. Currently, the dataset directories are hardcoded (e.g., `chain`, `cow-heads`, `golf-ball`, and `mat-twist` for the simulation dataset).

## More Methods?

Do you know of a CCD method we are missing? We would love to add it to our wrapper and benchmark. Please submit an issue on GihHub to let us know.

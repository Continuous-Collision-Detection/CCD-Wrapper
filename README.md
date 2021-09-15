# Continuous Collision Detection Wrapper and Benchmark

[![Build status](https://github.com/Continuous-Collision-Detection/CCD-Wrapper/workflows/Build/badge.svg)](https://github.com/Continuous-Collision-Detection/CCD-Wrapper/actions?query=workflow%3ABuild+branch%3Amaster+event%3Apush)
[![Benchmark](https://github.com/Continuous-Collision-Detection/CCD-Wrapper/workflows/Benchmark/badge.svg)](https://github.com/Continuous-Collision-Detection/CCD-Wrapper/actions?query=workflow%3ABenchmark+branch%3Amaster+event%3Apush)
[![Nightly](https://github.com/Continuous-Collision-Detection/CCD-Wrapper/workflows/Nightly/badge.svg)](https://github.com/Continuous-Collision-Detection/CCD-Wrapper/actions?query=workflow%3ANightly+branch%3Amaster)
<a href="https://opensource.org/licenses/MIT"><img src="https://img.shields.io/github/license/Continuous-Collision-Detection/CCD-Wrapper.svg?color=blue"></img></a>

A wrapper and benchmark for various continuous collision detection algorithms.

## Build

To build the library and executable benchmark on Linux or macOS run:
```sh
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j4
```

## Dependencies

The system level dependencies vary depending on which methods are enabled. To compile all methods and the benchmark, the following are required:

* [Boost](https://www.boost.org/): for interval arithmetic (when interval-based methods are enabled)
* [GMP](https://gmplib.org/): for rational numbers arithmetic (used when loading benchmark data)

Eigen and other dependencies will be downloaded through CMake.

## Add CCD-Wrapper to your CMake Project

The easiest way to add the CCD-Wrapper to an existing CMake project is to download it through CMake.
CMake provides functionality for doing this called [FetchContent](https://cmake.org/cmake/help/latest/module/FetchContent.html) (requires CMake ≥ 3.14).
We use this same process to download all external dependencies.
For example,

```CMake
include(FetchContent)
FetchContent_Declare(
    ccd_wrapper
    GIT_REPOSITORY https://github.com/Continuous-Collision-Detection/CCD-Wrapper
    GIT_TAG ${CCD_WRAPPER_GIT_TAG}
    GIT_SHALLOW FALSE
)
FetchContent_MakeAvailable(ccd_wrapper)
```

where `CCD_WRAPPER_GIT_TAG` is set to the version of the wrapper you want to use. This will download and add the CCD wrapper to CMake. The CCD wrapper library can then be linked against using

```CMake
# Link against the CCD Wrapper
target_link_libraries(${MY_LIB_NAME} PUBLIC ccd_wrapper::ccd_wrapper)
```

where `MY_LIB_NAME` is the name of your library (or executable).

## Running the Benchmark

To run the benchmark run `ccd_benchmark`.

For a complete list of benchmark options run `ccd_benchmark --help`.

By default the benchmark runs on a small subset of CCD queries automatically downloaded to `sample-ccd-queries`.
The full dataset can be found [here](https://archive.nyu.edu/handle/2451/61518). Use `ccd_benchmark --data </path/to/data>` to tell the benchmark where to find the root directory of the dataset. Currently, the dataset directories are hardcoded (e.g., `chain`, `cow-heads`, `golf-ball`, and `mat-twist` for the simulation dataset).

## Visualize Benchmark Queries

We provide a visualization tool in `visualization/visualCCD.py` for CCD dataset of the paper "A Large Scale Benchmark and an Inclusion-Based Algorithm for Continuous Collision Detection" (https://archive.nyu.edu/handle/2451/61518).

See [`visualization/readme.txt`](./visualization/readme.txt) for more information.

## More Methods?

Do you know of a CCD method we are missing? We would love to add it to our wrapper and benchmark. Please submit an issue on GitHub to let us know.

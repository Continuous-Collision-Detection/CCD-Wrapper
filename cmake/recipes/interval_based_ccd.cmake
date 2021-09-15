# Interval-based CCD
if(TARGET ccd_wrapper::interval_based_ccd)
    return()
endif()

message(STATUS "Third-party: creating target 'ccd_wrapper::interval_based_ccd'")

include(FetchContent)
FetchContent_Declare(
    interval_based_ccd
    GIT_REPOSITORY https://github.com/Continuous-Collision-Detection/Interval-Based.git
    GIT_TAG 4237dfde4e3f0a757ac738d052423957e3c5d16b
    GIT_SHALLOW FALSE
)

FetchContent_MakeAvailable(interval_based_ccd)

add_library(ccd_wrapper::interval_based_ccd ALIAS INTERVAL_CCD)

# TightCCD implmentation of Wang et al. [2015]
if(TARGET ccd_wrapper::tight_ccd)
    return()
endif()

message(STATUS "Third-party: creating target 'ccd_wrapper::tight_ccd'")

include(FetchContent)
FetchContent_Declare(
    tight_ccd
    GIT_REPOSITORY https://github.com/Continuous-Collision-Detection/TightCCD.git
    GIT_TAG 366384bac36ed1549d265e90c180fdd1beab42c6
    GIT_SHALLOW FALSE
)

FetchContent_MakeAvailable(tight_ccd)

add_library(ccd_wrapper::tight_ccd ALIAS TightCCD)

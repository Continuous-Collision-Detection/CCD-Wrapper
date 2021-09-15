# Minimum separation root finder of [Harmon et al. 2011]
if(TARGET ccd_wrapper::minimum_separation_root_finder)
    return()
endif()

message(STATUS "Third-party: creating target 'ccd_wrapper::minimum_separation_root_finder'")

include(FetchContent)
FetchContent_Declare(
    minimum_separation_root_finder
    GIT_REPOSITORY https://github.com/Continuous-Collision-Detection/Minimum-Separation-Root-Finder.git
    GIT_TAG 1aeb9ac0a404b43980bc82912fac836b720b609b
    GIT_SHALLOW FALSE
)

FetchContent_MakeAvailable(minimum_separation_root_finder)

add_library(ccd_wrapper::minimum_separation_root_finder ALIAS MSRF_CCD)

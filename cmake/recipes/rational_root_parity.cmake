# Rational Root Parity CCD (rational version of [Brochu et al. 2012])
if(TARGET ccd_wrapper::rational_root_parity)
    return()
endif()

message(STATUS "Third-party: creating target 'ccd_wrapper::rational_root_parity'")

include(FetchContent)
FetchContent_Declare(
    rational_root_parity
    GIT_REPOSITORY https://github.com/Continuous-Collision-Detection/Rational-Root-Parity.git
    GIT_TAG 812f8cc6bdd6768b4a70291f411581c3544bb227
    GIT_SHALLOW FALSE
)

FetchContent_MakeAvailable(rational_root_parity)

add_library(ccd_wrapper::rational_root_parity ALIAS RationalCCD)

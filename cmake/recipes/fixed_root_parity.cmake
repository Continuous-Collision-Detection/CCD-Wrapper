# New Root Parity
if((NOT CCD_WRAPPER_WITH_FPRP OR TARGET ccd_wrapper::floating_point_root_parity)
    AND (NOT CCD_WRAPPER_WITH_RFRP OR TARGET ccd_wrapper::rational_fixed_root_parity))
    return()
endif()

if(CCD_WRAPPER_WITH_FPRP)
    message(STATUS "Third-party: creating target 'ccd_wrapper::floating_point_root_parity'")
endif()

if(CCD_WRAPPER_WITH_RFRP)
    message(STATUS "Third-party: creating target 'ccd_wrapper::rational_fixed_root_parity'")
endif()

# WARNING: Queries should be rounded manually
message(STATUS "Make sure to round queries to Floating-Point Root Parity CCD")
option(CCD_ROUND_INPUTS "Round the inputs before collision detection" OFF)

set(CCD_BUILD_RATIONAL ${CCD_WRAPPER_WITH_RFRP} CACHE BOOL "Build Rational Version" FORCE)

include(FetchContent)
FetchContent_Declare(
    fixed_root_parity
    GIT_REPOSITORY git@github.com:Continuous-Collision-Detection/ExactRootParityCCD.git
    GIT_TAG ded6440dd454c57042fd54ced2f805c4b4b64137
    GIT_SHALLOW FALSE
)

FetchContent_MakeAvailable(fixed_root_parity)

add_library(ccd_wrapper_fixed_root_parity INTERFACE)

if(CCD_WRAPPER_WITH_FPRP)
    add_library(ccd_wrapper::floating_point_root_parity ALIAS ccd_double)
endif()

if(CCD_WRAPPER_WITH_RFRP)
    add_library(ccd_wrapper::rational_fixed_root_parity ALIAS ccd_rational)
endif()

# Brochu et al. [2012] and Tang et al. [2014]
if(TARGET ccd_wrapper::root_parity_and_bernstein_sign_classification)
    return()
endif()

message(STATUS "Third-party: creating target 'ccd_wrapper::root_parity_and_bernstein_sign_classification'")

include(FetchContent)
FetchContent_Declare(
    rp_and_bsc
    GIT_REPOSITORY https://github.com/Continuous-Collision-Detection/Root-Parity-and-Bernstein-Sign-Classification.git
    GIT_TAG 1686f79ca4b3c95979e2f6a96504d4562e44d3ce
    GIT_SHALLOW FALSE
)

FetchContent_MakeAvailable(rp_and_bsc)

add_library(ccd_wrapper::root_parity_and_bernstein_sign_classification ALIAS exact-ccd)

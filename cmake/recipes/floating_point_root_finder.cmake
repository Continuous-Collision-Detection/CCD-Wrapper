# Etienne Vouga's CCD Library
if(TARGET ccd_wrapper::floating_point_root_finder)
    return()
endif()

message(STATUS "Third-party: creating target 'ccd_wrapper::floating_point_root_finder'")

include(FetchContent)
FetchContent_Declare(
    floating_point_root_finder
    GIT_REPOSITORY https://github.com/Continuous-Collision-Detection/Floating-Point-Root-Finder.git
    GIT_TAG e5fe5c9767207df5047e375fb20180a665ae186f
    GIT_SHALLOW FALSE
)

FetchContent_GetProperties(floating_point_root_finder)
if(NOT floating_point_root_finder_POPULATED)
    FetchContent_Populate(floating_point_root_finder)
endif()

add_library(ccd_wrapper_floating_point_root_finder
    "${floating_point_root_finder_SOURCE_DIR}/src/CTCD.cpp"
)
add_library(ccd_wrapper::floating_point_root_finder ALIAS ccd_wrapper_floating_point_root_finder)

target_include_directories(ccd_wrapper_floating_point_root_finder PUBLIC "${floating_point_root_finder_SOURCE_DIR}/include")

include(eigen)
target_link_libraries(ccd_wrapper_floating_point_root_finder PUBLIC Eigen3::Eigen)

# Turn off floating point contraction for CCD robustness
target_compile_options(ccd_wrapper_floating_point_root_finder PRIVATE "-ffp-contract=off")

# Prepare dependencies
#
# For each third-party library, if the appropriate target doesn't exist yet,
# download it via external project, and add_subdirectory to build it alongside
# this project.


# Download and update 3rd_party libraries
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
list(REMOVE_DUPLICATES CMAKE_MODULE_PATH)
include(${PROJECT_NAME}DownloadExternal)

################################################################################
# Required libraries
################################################################################

# Eigen
if(NOT TARGET Eigen3::Eigen)
  ccd_wrapper_download_eigen()
  add_library(${PROJECT_NAME}_eigen INTERFACE)
  target_include_directories(${PROJECT_NAME}_eigen SYSTEM INTERFACE
    $<BUILD_INTERFACE:${CCD_WRAPPER_EXTERNAL}/eigen>
    $<INSTALL_INTERFACE:include>
  )
  set_property(TARGET ${PROJECT_NAME}_eigen PROPERTY EXPORT_NAME Eigen3::Eigen)
  add_library(Eigen3::Eigen ALIAS ${PROJECT_NAME}_eigen)
  # Set Eigen directory environment variable (needed for EVCTCD)
  set(ENV{EIGEN3_INCLUDE_DIR} "${CCD_WRAPPER_EXTERNAL}/eigen/")
endif()

# Etienne Vouga's CTCD Library
if(NOT TARGET EVCTCD)
  ccd_wrapper_download_evctcd()
  add_subdirectory(${CCD_WRAPPER_EXTERNAL}/EVCTCD)
  # These includes are PRIVATE for some reason
  target_include_directories(collisiondetection PUBLIC "${CCD_WRAPPER_EXTERNAL}/EVCTCD/include")
  # Turn of floating point contraction for CCD robustness
  target_compile_options(collisiondetection PUBLIC "-ffp-contract=off")
  # Rename for convenience
  add_library(EVCTCD ALIAS collisiondetection)
endif()

# Brochu et al. [2012] and Tang et al. [2014]
if(NOT TARGET exact-ccd::exact-ccd)
  ccd_wrapper_download_exact_ccd()
  add_subdirectory(${CCD_WRAPPER_EXTERNAL}/exact-ccd EXCLUDE_FROM_ALL)
  add_library(exact-ccd::exact-ccd ALIAS exact-ccd)
endif()

# Rational implmentation of Brochu et al. [2012]
if(NOT TARGET RationalCCD)
  ccd_wrapper_download_rational_ccd()
  add_subdirectory(${CCD_WRAPPER_EXTERNAL}/rational_ccd)
endif()

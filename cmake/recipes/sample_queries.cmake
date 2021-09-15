# Sampled CCD Queries

# Sample queries folder path can be overriden by the user
set(CCD_WRAPPER_SAMPLE_QUERIES_DIR "${CMAKE_SOURCE_DIR}/sample-ccd-queries"
    CACHE PATH "Where should CCD Wrapper download and look for Sampled CCD Queries?"
)
message(STATUS "Using sample queries directory: ${CCD_WRAPPER_SAMPLE_QUERIES_DIR}")

include(FetchContent)
FetchContent_Declare(
    sample_ccd_queries
    SOURCE_DIR ${CCD_WRAPPER_SAMPLE_QUERIES_DIR}
    GIT_REPOSITORY https://github.com/Continuous-Collision-Detection/Sample-Queries.git
    GIT_TAG 4d6cce33477d8d5c666c31c8ea23e1aea97be371
)

FetchContent_GetProperties(sample_ccd_queries)
if(NOT sample_ccd_queries_POPULATED)
    FetchContent_Populate(sample_ccd_queries)
endif()

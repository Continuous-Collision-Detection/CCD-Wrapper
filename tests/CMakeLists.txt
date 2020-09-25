cmake_minimum_required(VERSION 3.1)
################################################################################

add_executable(${PROJECT_NAME}_tests
    main.cpp
    test_ccd.cpp
)

################################################################################
# Required Libraries
################################################################################

target_link_libraries(${PROJECT_NAME}_tests PUBLIC ${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME}_tests PUBLIC Catch2::Catch2 warnings::all)

if(CCD_WRAPPER_WITH_BENCHMARK)
  target_link_libraries(${PROJECT_NAME}_tests PUBLIC fmt::fmt)
  # target_compile_definitions(${PROJECT_NAME}_tests PRIVATE EXPORT_CCD_QUERIES)
endif()

################################################################################
# Add Tests
################################################################################

foreach(source IN ITEMS ${test_sources})
   source_group("tests" FILES "${source}")
endforeach()

# Register tests
set(PARSE_CATCH_TESTS_ADD_TO_CONFIGURE_DEPENDS ON)
include(Catch)
catch_discover_tests(${PROJECT_NAME}_tests)

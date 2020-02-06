cmake_minimum_required(VERSION 3.1)
################################################################################

add_executable(${PROJECT_NAME}_tests
    main.cpp
    test_ccd.cpp
)

if(${PROJECT_NAME}_WITH_COMPARISONS)
  target_compile_definitions(${PROJECT_NAME}_tests PUBLIC TEST_COMPARISONS)
endif()

################################################################################
# Required Libraries
################################################################################

target_link_libraries(${PROJECT_NAME}_tests PUBLIC ${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME}_tests PUBLIC Catch2::Catch2 warnings::all)

if(${PROJECT_NAME}_WITH_COMPARISONS)
  target_link_libraries(${PROJECT_NAME}_tests PUBLIC ${PROJECT_NAME}_comparisons)
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
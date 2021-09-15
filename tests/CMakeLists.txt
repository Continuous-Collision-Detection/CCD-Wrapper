################################################################################
# Tests
################################################################################

add_executable(ccd_wrapper_tests
    main.cpp
    test_ccd.cpp
)

################################################################################
# Required Libraries
################################################################################

target_link_libraries(ccd_wrapper_tests PUBLIC ccd_wrapper::ccd_wrapper)

include(ccd_wrapper_warnings)
target_link_libraries(ccd_wrapper_tests PRIVATE ccd_wrapper::warnings)

include(catch2)
target_link_libraries(ccd_wrapper_tests PUBLIC Catch2::Catch2)

if(CCD_WRAPPER_WITH_BENCHMARK)
    include(fmt)
    target_link_libraries(ccd_wrapper_tests PUBLIC fmt::fmt)
    # target_compile_definitions(ccd_wrapper_tests PRIVATE EXPORT_CCD_QUERIES)
endif()

################################################################################
# Add Tests
################################################################################

foreach(source IN ITEMS ${test_sources})
   source_group("tests" FILES "${source}")
endforeach()

# Register tests
set(PARSE_CATCH_TESTS_ADD_TO_CONFIGURE_DEPENDS ON)
# include(Catch)
catch_discover_tests(ccd_wrapper_tests)

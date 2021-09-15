# Tight Inclusion
if(TARGET tight_inclusion::tight_inclusion)
    return()
endif()

message(STATUS "Third-party: creating target 'tight_inclusion::tight_inclusion'")

OPTION(TIGHT_INCLUSION_WITH_GMP              "Enable rational based predicates, for debug"             OFF)
OPTION(TIGHT_INCLUSION_WITH_TESTS            "Enable test functions"                                   OFF)
OPTION(TIGHT_INCLUSION_WITH_TIMER            "Enable profiling timers, for debug"                      OFF)
OPTION(TIGHT_INCLUSION_WITH_NO_ZERO_TOI      "Enable refinement if CCD produces a zero ToI"            OFF)
OPTION(TIGHT_INCLUSION_WITH_DOUBLE_PRECISION "Enable double precision floating point numbers as input"  ON)
OPTION(TIGHT_INCLUSION_LIMIT_QUEUE_SIZE      "Enable limitation of maximal queue size"                 OFF)

include(FetchContent)
FetchContent_Declare(
    tight_inclusion
    GIT_REPOSITORY https://github.com/Continuous-Collision-Detection/Tight-Inclusion.git
    GIT_TAG f1c66b40086377ac81b1283b88652ad855838325
    GIT_SHALLOW FALSE
)

FetchContent_MakeAvailable(tight_inclusion)

add_library(tight_inclusion::tight_inclusion ALIAS tight_inclusion)

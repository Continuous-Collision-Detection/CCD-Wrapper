# Tight Inclusion
if(TARGET tight_inclusion::tight_inclusion)
    return()
endif()

message(STATUS "Third-party: creating target 'tight_inclusion::tight_inclusion'")

OPTION(TIGHT_INCLUSION_WITH_GMP              "Enable rational based predicates, for debug"             OFF)
OPTION(TIGHT_INCLUSION_WITH_TESTS            "Enable test functions"                                   OFF)
OPTION(TIGHT_INCLUSION_WITH_TIMER            "Enable profiling timers, for debug"                      OFF)
OPTION(TIGHT_INCLUSION_WITH_DOUBLE_PRECISION "Enable double precision floating point numbers as input"  ON)
OPTION(TIGHT_INCLUSION_LIMIT_QUEUE_SIZE      "Enable limitation of maximal queue size"                 OFF)

include(FetchContent)
FetchContent_Declare(
    tight_inclusion
    GIT_REPOSITORY https://github.com/Continuous-Collision-Detection/Tight-Inclusion.git
    GIT_TAG 1c66674b5222f80fc6a1bb48119f9d39e912a4bc
    GIT_SHALLOW FALSE
)

FetchContent_MakeAvailable(tight_inclusion)

include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.4.0
)
FetchContent_MakeAvailable(Catch2)

add_executable(LoP01Tests
    test_location.cpp
    test_scene.cpp
    test_game.cpp
)

target_link_libraries(LoP01Tests PRIVATE LoP01Lib Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(LoP01Tests)

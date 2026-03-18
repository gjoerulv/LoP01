cmake_minimum_required(VERSION 3.20)

project(ProjectAshvale VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
  raylib
  GIT_REPOSITORY https://github.com/raysan5/raylib.git
  GIT_TAG 5.5
)

FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
)

FetchContent_MakeAvailable(raylib nlohmann_json)

add_library(ashvale_logic
  src/core/GameClock.cpp
  src/core/SaveGame.cpp
  src/data/ContentRepository.cpp
  src/gameplay/battle/Battle.cpp
  src/gameplay/GameSession.cpp
  src/gameplay/location/LocationScene.cpp
  src/gameplay/battle/BattleFactory.cpp
)

target_include_directories(ashvale_logic PUBLIC src)
target_link_libraries(ashvale_logic PUBLIC nlohmann_json::nlohmann_json)

add_library(ashvale_rendering
  src/rendering/HudRenderer.cpp
  src/rendering/DebugOverlay.cpp
  src/rendering/TitleRenderer.cpp
  src/rendering/OverworldRenderer.cpp
  src/rendering/LocationRenderer.cpp
  src/rendering/BattleRenderer.cpp
)

target_include_directories(ashvale_rendering PUBLIC src)
target_link_libraries(ashvale_rendering PUBLIC raylib)

add_executable(ashvale_slice
  src/app/App.cpp
  src/app/main.cpp
  src/app/input/InputTranslator.cpp
  src/app/OverworldController.cpp
  src/app/LocationController.cpp
  src/app/BattleController.cpp
  src/app/mappers/HudModelMapper.cpp
  src/app/mappers/OverworldModelMapper.cpp
  src/app/mappers/LocationModelMapper.cpp
  src/app/mappers/BattleModelMapper.cpp
  src/app/BattleEventTextFormatter.cpp
)

target_link_libraries(ashvale_slice PRIVATE ashvale_logic ashvale_rendering)

if (EXISTS "${CMAKE_SOURCE_DIR}/content")
  add_custom_command(TARGET ashvale_slice POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/content"
            "$<TARGET_FILE_DIR:ashvale_slice>/content"
    COMMENT "Copying content folder next to ashvale_slice executable"
  )
endif()

if (WIN32)
  target_link_libraries(ashvale_slice PRIVATE winmm)
endif()

include(CTest)

if (BUILD_TESTING)
  FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.7.1
  )
  FetchContent_MakeAvailable(Catch2)

  add_executable(ashvale_logic_tests
    src/app/BattleEventTextFormatter.cpp
    tests/BattleTests.cpp
    tests/BattleEventTextFormatterTests.cpp
    tests/GameClockTests.cpp
    tests/GameSessionTests.cpp
    tests/LocationSceneTests.cpp
    tests/SaveGameTests.cpp
  )

  target_link_libraries(ashvale_logic_tests PRIVATE ashvale_logic Catch2::Catch2WithMain)

  include(Catch)
  catch_discover_tests(ashvale_logic_tests)
endif()
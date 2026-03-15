cmake_minimum_required(VERSION 3.20)
project(LoP01 VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Game library (logic shared with tests) ───────────────────────────────────
set(GAME_SOURCES
    src/Game.cpp
    src/Location.cpp
    src/Player.cpp
    src/Scene.cpp
)

add_library(LoP01Lib STATIC ${GAME_SOURCES})
target_include_directories(LoP01Lib PUBLIC src)

# ── Game executable ──────────────────────────────────────────────────────────
add_executable(LoP01 src/main.cpp)
target_link_libraries(LoP01 PRIVATE LoP01Lib)

# ── Tests ────────────────────────────────────────────────────────────────────
enable_testing()
add_subdirectory(tests)

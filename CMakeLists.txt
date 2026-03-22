cmake_minimum_required(VERSION 3.16)

project(MonopolyAi LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_CLANG_TIDY clang-tidy)

file(GLOB_RECURSE ALL_SOURCE_FILES src/*.cc src/*.h src/*.hpp tests/*.cc tests/*.h tests/*.hpp)
add_custom_target(auto-format ALL
    COMMAND clang-format -i ${ALL_SOURCE_FILES}
)

find_package(Armadillo REQUIRED)

if(Armadillo_FOUND AND NOT TARGET Armadillo::Armadillo)
  add_library(Armadillo::Armadillo INTERFACE IMPORTED)
  set_target_properties(
    Armadillo::Armadillo
    PROPERTIES
      INTERFACE_LINK_LIBRARIES "${ARMADILLO_LIBRARIES}"
      INTERFACE_INCLUDE_DIRECTORIES "${ARMADILLO_INCLUDE_DIRS}"
  )
endif()

find_package(ensmallen REQUIRED)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.4.0
)
FetchContent_MakeAvailable(Catch2)

find_package(PkgConfig REQUIRED)
pkg_check_modules(MLPACK REQUIRED mlpack)

file(GLOB_RECURSE GAME_CORE_SOURCES
    "src/ai/*.h"
    "src/ai/*.cc"
    "src/core/*.h"
    "src/core/*.cc"
    "src/gui/*.h"
    "src/gui/*.cc"
)

add_library(GameCore ${GAME_CORE_SOURCES})

file(GLOB_RECURSE TRAINER_SOURCES
    "src/trainer/*.h"
    "src/trainer/*.cc"
)

add_library(Trainer ${TRAINER_SOURCES})

target_link_libraries(GameCore PUBLIC
    ${MLPACK_LIBRARIES}
    Armadillo::Armadillo
    Qt6::Widgets
)

target_include_directories(GameCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${MLPACK_INCLUDE_DIRS}
)

target_link_libraries(Trainer PUBLIC
    GameCore
    ${MLPACK_LIBRARIES}
    Armadillo::Armadillo
)

target_compile_options(GameCore PUBLIC ${MLPACK_CFLAGS_OTHER})

add_executable(MonopolyGame src/Main.cc)
target_link_libraries(MonopolyGame PRIVATE GameCore)

add_compile_definitions(MLPACK_ENABLE_ANN_SERIALIZATION)
add_executable(MonopolyTrain src/trainer/Main.cc)
target_link_libraries(MonopolyTrain PRIVATE Trainer GameCore)

enable_testing()
add_subdirectory(tests)
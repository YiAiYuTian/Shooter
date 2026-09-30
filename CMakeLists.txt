cmake_minimum_required(VERSION 3.21)
project(shooter
  VERSION 0.1.1
  DESCRIPTION "A small platform shooting game"
  LANGUAGES C CXX
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL3 REQUIRED CONFIG)
find_package(SDL3_ttf REQUIRED CONFIG)
find_package(SDL3_image REQUIRED CONFIG)
find_package(spdlog REQUIRED CONFIG)
find_package(glm REQUIRED CONFIG)
find_package(box2d CONFIG QUIET)
if(NOT box2d_FOUND)
  include(FetchContent)
  FetchContent_Declare(box2d
    URL https://github.com/erincatto/Box2D/archive/refs/tags/v2.4.1.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
  set(BOX2D_BUILD_TESTBED OFF CACHE BOOL "" FORCE)
  set(BOX2D_BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(box2d)
  add_library(box2d::box2d ALIAS box2d)
endif()

add_executable(${PROJECT_NAME} WIN32 main.cpp)

target_sources(${PROJECT_NAME} PRIVATE 
  src/core/app.cpp
  src/core/logger.cpp
  src/core/resource_manager.cpp
  src/event/event_manager.cpp
  src/event/input_manager.cpp
  src/render/render_manager.cpp
  src/game/animation/animation.cpp
  src/game/utils.cpp
  src/game/scenes/game_world.cpp
  src/game/scenes/menu_scene.cpp
  src/game/scenes/game_scene.cpp
  src/game/physics/physics.cpp
  src/game/objects/character.cpp
  src/game/objects/player.cpp
  src/game/objects/zombie.cpp
  src/game/objects/ai.cpp
  src/game/objects/platform.cpp
  src/game/objects/bullet.cpp
  src/game/objects/explosion.cpp
  src/game/ui/button.cpp
  src/game/ui/text.cpp
  src/game/ui/progress_bar.cpp
)

target_compile_options(${PROJECT_NAME} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/utf-8>)

target_link_libraries(${PROJECT_NAME} PRIVATE 
  SDL3::SDL3 
  SDL3_image::SDL3_image 
  SDL3_ttf::SDL3_ttf 
  spdlog::spdlog 
  glm::glm
  box2d::box2d
)

if(WIN32)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:${PROJECT_NAME}> $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>
    COMMAND_EXPAND_LISTS)
endif()

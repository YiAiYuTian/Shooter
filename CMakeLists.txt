cmake_minimum_required(VERSION 3.20)
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

add_executable(${PROJECT_NAME} WIN32 main.cpp)

target_sources(${PROJECT_NAME} PRIVATE 
  src/core/app.cpp
  src/core/logger.cpp
  src/event/event_manager.cpp
  src/event/input_manager.cpp
)

target_compile_options(${PROJECT_NAME} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/utf-8>)

target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3 SDL3_ttf::SDL3_ttf spdlog::spdlog)

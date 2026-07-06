cmake_minimum_required(VERSION 3.25)

project(pixel_town_ten_day_plan VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(pixel_town_core INTERFACE)
target_include_directories(pixel_town_core INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/src")

add_library(pixel_town_io
  src/io/resource_diagnostics.cpp
  src/io/startup_log.cpp
)
target_include_directories(pixel_town_io PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(USE_EXTERNAL_GLFW OFF CACHE STRING "" FORCE)
add_subdirectory(third_party/raylib EXCLUDE_FROM_ALL)

add_executable(pixel_town src/app/main.cpp)
target_link_libraries(pixel_town PRIVATE pixel_town_core pixel_town_io raylib)
target_compile_definitions(pixel_town PRIVATE PIXEL_TOWN_VERSION="${PROJECT_VERSION}")

enable_testing()

add_executable(pixel_town_tests
  tests/display_config_test.cpp
  tests/resource_diagnostics_test.cpp
)
target_link_libraries(pixel_town_tests PRIVATE pixel_town_core pixel_town_io)
target_include_directories(pixel_town_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/third_party/doctest")

add_test(NAME pixel_town_tests COMMAND pixel_town_tests)

if(MSVC)
  target_compile_options(pixel_town PRIVATE /W4 /permissive-)
  target_compile_options(pixel_town_io PRIVATE /W4 /permissive-)
  target_compile_options(pixel_town_tests PRIVATE /W4 /permissive-)
else()
  target_compile_options(pixel_town PRIVATE -Wall -Wextra -Wpedantic)
  target_compile_options(pixel_town_io PRIVATE -Wall -Wextra -Wpedantic)
  target_compile_options(pixel_town_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

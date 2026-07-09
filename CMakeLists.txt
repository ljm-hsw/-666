cmake_minimum_required(VERSION 3.25)

project(pixel_town_ten_day_plan VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(pixel_town_core
  src/core/game_session.cpp
  src/core/story_text.cpp
  src/core/tavern_rules.cpp
)
target_include_directories(pixel_town_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")

add_library(pixel_town_io
  src/io/app_settings.cpp
  src/io/demo_preset.cpp
  src/io/resource_diagnostics.cpp
  src/io/save_game.cpp
  src/io/startup_log.cpp
)
target_include_directories(pixel_town_io PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(pixel_town_io PUBLIC pixel_town_core)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(USE_EXTERNAL_GLFW OFF CACHE STRING "" FORCE)
add_subdirectory(third_party/raylib EXCLUDE_FROM_ALL)

add_library(pixel_town_locations
  src/locations/restaurant.cpp
  src/locations/convenience_store.cpp
  src/locations/library_data.cpp
  src/locations/library_rules.cpp
)
target_include_directories(pixel_town_locations PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(pixel_town_locations PUBLIC pixel_town_core)

add_library(pixel_town_location_scene
  src/locations/library_npc.cpp
  src/locations/library_scene.cpp
)
target_include_directories(pixel_town_location_scene PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(pixel_town_location_scene PUBLIC pixel_town_locations raylib)

add_library(pixel_town_app_logic
  src/app/location_result_adapter.cpp
  src/app/location_runtime.cpp
  src/app/restaurant_ui_model.cpp
  src/app/ui_primitives.cpp
)
target_include_directories(pixel_town_app_logic PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(pixel_town_app_logic PUBLIC pixel_town_core pixel_town_location_scene)

add_executable(pixel_town
  src/app/game_flow.cpp
  src/app/main.cpp
  src/app/visual_prototype.cpp
  src/locations/library_ui.cpp
)
target_link_libraries(pixel_town PRIVATE pixel_town_io pixel_town_app_logic)
target_compile_definitions(pixel_town PRIVATE PIXEL_TOWN_VERSION="${PROJECT_VERSION}")

enable_testing()

add_executable(pixel_town_tests
  tests/display_config_test.cpp
  tests/tavern_rules_test.cpp
  tests/demo_preset_test.cpp
  tests/game_session_test.cpp
  tests/interaction_runtime_test.cpp
  tests/location_result_adapter_test.cpp
  tests/restaurant_ui_model_test.cpp
  tests/app_settings_test.cpp
  tests/convenience_store_test.cpp
  tests/resource_diagnostics_test.cpp
  tests/save_game_test.cpp
  tests/restaurant_test.cpp
  tests/story_text_test.cpp
  tests/test_library.cpp
)
target_link_libraries(pixel_town_tests PRIVATE pixel_town_io pixel_town_app_logic)
target_include_directories(pixel_town_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/third_party/doctest")

add_test(NAME pixel_town_tests COMMAND pixel_town_tests)
set_tests_properties(pixel_town_tests PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

if(MSVC)
  target_compile_options(pixel_town PRIVATE /W4 /permissive-)
  target_compile_options(pixel_town_core PRIVATE /W4 /permissive-)
  target_compile_options(pixel_town_io PRIVATE /W4 /permissive-)
  target_compile_options(pixel_town_locations PRIVATE /W4 /permissive-)
  target_compile_options(pixel_town_location_scene PRIVATE /W4 /permissive-)
  target_compile_options(pixel_town_app_logic PRIVATE /W4 /permissive-)
  target_compile_options(pixel_town_tests PRIVATE /W4 /permissive-)
else()
  target_compile_options(pixel_town PRIVATE -Wall -Wextra -Wpedantic)
  target_compile_options(pixel_town_core PRIVATE -Wall -Wextra -Wpedantic)
  target_compile_options(pixel_town_io PRIVATE -Wall -Wextra -Wpedantic)
  target_compile_options(pixel_town_locations PRIVATE -Wall -Wextra -Wpedantic)
  target_compile_options(pixel_town_location_scene PRIVATE -Wall -Wextra -Wpedantic)
  target_compile_options(pixel_town_app_logic PRIVATE -Wall -Wextra -Wpedantic)
  target_compile_options(pixel_town_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

cmake_minimum_required(VERSION 3.14)
project(dungeon)

set(CMAKE_CXX_STANDARD 17)

add_executable(dungeon
    main.cpp
    src/core/level.cpp
    src/core/json_min.cpp
    src/core/renderer.cpp
    src/core/display.cpp
    src/core/console.cpp
    src/core/app.cpp
    src/core/tiles.cpp
    src/game/game_mode.cpp
    src/game/game_controls.cpp
    src/game/game_panel.cpp
    src/game/interactions.cpp
    src/editor/editor_mode.cpp
    src/editor/editor_controls.cpp
    src/editor/editor_panel.cpp
    src/generator/fragment.cpp
    src/generator/fragment_editor_mode.cpp
    src/generator/fragment_editor_controls.cpp
    src/generator/fragment_editor_panel.cpp
    src/generator/dungeon_generator.cpp
    src/generator/generator_mode.cpp
    src/settings/settings_mode.cpp
    src/settings/settings_panel.cpp
    src/help/help_mode.cpp
    src/help/help_panel.cpp
)

target_include_directories(dungeon PRIVATE src)

if(WIN32)

    set(SDL_ROOT ${CMAKE_SOURCE_DIR}/win_libs/x86_64)

    target_include_directories(dungeon PRIVATE
        ${SDL_ROOT}/include
    )

    target_link_directories(dungeon PRIVATE
        ${SDL_ROOT}/lib
    )

    target_compile_definitions(dungeon PRIVATE SDL_MAIN_HANDLED)

    target_link_libraries(dungeon
        SDL2
        SDL2_image
        SDL2_ttf
    )

else()

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2 REQUIRED sdl2 SDL2_image SDL2_ttf)

    target_include_directories(dungeon PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(dungeon ${SDL2_LIBRARIES})

endif()

# world/ and fragments/ (core/level.h's WORLD_DIR, generator/fragment.h's
# FRAGMENTS_DIR) now live under assets/ as real subdirectories, so linking
# assets/ into the build directory already exposes both — no separate
# symlink needed for either.
add_custom_command(TARGET dungeon POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm -f
        "${CMAKE_CURRENT_BINARY_DIR}/assets"
    COMMAND ${CMAKE_COMMAND} -E create_symlink
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "${CMAKE_CURRENT_BINARY_DIR}/assets"
    COMMENT "[100%] Linking assets/ into build directory")

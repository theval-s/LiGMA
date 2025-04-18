cmake_minimum_required(VERSION 3.24)

project(ligma-app)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

set(MAIN_SOURCES
        src/main.cpp
        src/native_instance_filesystem.cpp
        src/config.cpp
        src/plugin_handler.cpp
        src/overlayfs_mount.cpp
        src/game_launcher.cpp
        src/i_instance_filesystem.cpp
        src/proton_instance_filesystem.cpp
        src/steam_finder.cpp
)

set(MAIN_UI_SOURCES
        src/qt_ui/mainwindow.cpp
        src/qt_ui/new_instance_dialog.cpp
        src/qt_ui/game_instance.cpp
        src/qt_ui/instance_options.cpp
        src/qt_ui/add_mod_dialog.cpp
)

add_executable(main ${MAIN_SOURCES} ${MAIN_UI_SOURCES})
target_include_directories(main PRIVATE include)
target_link_libraries(main PUBLIC Qt6::Widgets Qt6::Core)
qt_finalize_executable(main)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)
add_subdirectory(plugins ${CMAKE_BINARY_DIR}/plugin_build)
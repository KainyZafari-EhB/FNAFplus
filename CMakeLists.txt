cmake_minimum_required(VERSION 3.16)
project(FNAF LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# Prefer the vendored SDL source if present, otherwise try to locate an installed SDL3 package.
if(EXISTS "${CMAKE_SOURCE_DIR}/vendored/SDL/CMakeLists.txt")
    add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)
else()
    find_package(SDL3 QUIET)
endif()

# Create the game executable target. (No WIN32 to ensure a console appears on Windows.)
add_executable(FNAF src/main.cpp
        src/Game.cpp
        include/Game.h
        src/Animatronic.cpp
        include/Animatronic.h
        src/Office.cpp
        include/Office.h
        src/Renderer.cpp
        include/Renderer.h)

# Link to the SDL3 imported target if it exists. If not found, warn the user.
if(TARGET SDL3::SDL3)
    target_link_libraries(FNAF PRIVATE SDL3::SDL3)
elseif(TARGET SDL3::SDL3-shared)
    target_link_libraries(FNAF PRIVATE SDL3::SDL3-shared)
elseif(TARGET SDL3::SDL3-static)
    target_link_libraries(FNAF PRIVATE SDL3::SDL3-static)
else()
    message(WARNING "SDL3 not found as an imported target. If you expect to use a system-installed SDL3, set SDL3_DIR or install the vendored SDL in vendored/SDL.")
endif()

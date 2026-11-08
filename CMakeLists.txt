# SPDX-License-Identifier: MIT
#
# CreateLeviLamina - CMakeLists.txt (noyau portable)
#
# Construit la bibliotheque du noyau + la couche monde, et les tests. Ne requiert
# ni Minecraft, ni LeviLamina : convient pour le developpement et la CI, y compris
# pour la future cible Android (via le NDK, en definissant ANDROID).
cmake_minimum_required(VERSION 3.20)
project(CreateLeviLamina LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(createleelamina_core STATIC
    Core/Rotation/Types.cpp
    Core/KineticSystem/KineticTypes.cpp
    Core/KineticSystem/KineticGraph.cpp
    Core/World/KineticWorld.cpp
    Platform/Platform.cpp
)
target_include_directories(createleelamina_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(createleelamina_core PRIVATE /W4)
else()
    target_compile_options(createleelamina_core PRIVATE -Wall -Wextra)
endif()

enable_testing()

foreach(t test_kinetic test_world test_bridge)
    add_executable(${t} Tests/${t}.cpp)
    target_link_libraries(${t} PRIVATE createleelamina_core)
    add_test(NAME ${t} COMMAND ${t})
endforeach()

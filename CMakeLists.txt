cmake_minimum_required(VERSION 3.22)
project(arsonkupik-obs-audio-enhancer VERSION 0.4.12 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Linux release builds link the static DSP library into the OBS module (.so).
# The DSP target must therefore be position-independent, otherwise ld fails with
# "recompile with -fPIC" when creating arsonkupik-obs-audio-enhancer.so.
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(BUILD_OBS_PLUGIN "Build native OBS plugin target" ON)
option(BUILD_STANDALONE_TESTS "Build standalone DSP smoke tests" ON)

add_library(arsonkupik_dsp STATIC
    src/arsonkupik_dsp.cpp
    src/arsonkupik_presets.cpp
)
target_include_directories(arsonkupik_dsp PUBLIC include)
set_target_properties(arsonkupik_dsp PROPERTIES POSITION_INDEPENDENT_CODE ON)

if (BUILD_OBS_PLUGIN)
    set(ARSONKUPIK_OBS_SOURCE_DIR "" CACHE PATH "OBS Studio source directory; fallback when libobsConfig.cmake is not available")
    set(ARSONKUPIK_OBS_IMPLIB "" CACHE FILEPATH "Path to obs.lib import library; fallback when libobsConfig.cmake is not available")

    if (ARSONKUPIK_OBS_SOURCE_DIR AND ARSONKUPIK_OBS_IMPLIB)
        if (NOT EXISTS "${ARSONKUPIK_OBS_SOURCE_DIR}/libobs/obs-module.h")
            message(FATAL_ERROR "ARSONKUPIK_OBS_SOURCE_DIR must point to the OBS Studio source root containing libobs/obs-module.h")
        endif()
        if (NOT EXISTS "${ARSONKUPIK_OBS_IMPLIB}")
            message(FATAL_ERROR "ARSONKUPIK_OBS_IMPLIB does not exist: ${ARSONKUPIK_OBS_IMPLIB}")
        endif()
        add_library(arsonkupik_obs_headers INTERFACE)
        target_include_directories(arsonkupik_obs_headers INTERFACE
            "${ARSONKUPIK_OBS_SOURCE_DIR}/libobs"
            "${ARSONKUPIK_OBS_SOURCE_DIR}/deps"
            "${ARSONKUPIK_OBS_SOURCE_DIR}/deps/obs-scripting")
        target_compile_definitions(arsonkupik_obs_headers INTERFACE
            OBS_INSTALL_PREFIX="."
            OBS_PLUGIN_PATH="obs-plugins"
            OBS_PLUGIN_DESTINATION="obs-plugins/64bit")
        set(ARSONKUPIK_OBS_LINK_TARGET "${ARSONKUPIK_OBS_IMPLIB}")
        message(STATUS "Using fallback OBS headers/import-lib mode")
    else()
        find_package(libobs CONFIG REQUIRED)
        set(ARSONKUPIK_OBS_LINK_TARGET OBS::libobs)
        message(STATUS "Using libobs CMake package")
    endif()

    add_library(arsonkupik-obs-audio-enhancer MODULE
        src/arsonkupik_filter.cpp
    )
    target_include_directories(arsonkupik-obs-audio-enhancer PRIVATE include)
    if (TARGET arsonkupik_obs_headers)
        target_link_libraries(arsonkupik-obs-audio-enhancer PRIVATE arsonkupik_obs_headers)
    endif()
    target_link_libraries(arsonkupik-obs-audio-enhancer PRIVATE arsonkupik_dsp ${ARSONKUPIK_OBS_LINK_TARGET})
    set_target_properties(arsonkupik-obs-audio-enhancer PROPERTIES PREFIX "")

    if (WIN32)
        set_target_properties(arsonkupik-obs-audio-enhancer PROPERTIES SUFFIX ".dll")
    endif()

    install(TARGETS arsonkupik-obs-audio-enhancer
        RUNTIME DESTINATION "obs-plugins/64bit"
        LIBRARY DESTINATION "obs-plugins/64bit")
    install(DIRECTORY data/ DESTINATION "data/obs-plugins/arsonkupik-obs-audio-enhancer")
endif()

if (BUILD_STANDALONE_TESTS)
    add_executable(arsonkupik_dsp_smoke tests/dsp_smoke_test.cpp)
    target_link_libraries(arsonkupik_dsp_smoke PRIVATE arsonkupik_dsp)
endif()

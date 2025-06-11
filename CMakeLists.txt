cmake_minimum_required(VERSION 3.16)
project(descriptor_compare)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build options
option(USE_CONAN "Use Conan for dependency management" OFF)
option(BUILD_PYTHON_BRIDGE "Build Python bridge for CNN descriptors" OFF)
option(USE_SYSTEM_PACKAGES "Prefer system packages over Conan" ON)

# Compiler flags
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -Wall")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall")
endif()

# Architecture-specific optimizations (if supported)
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-march=native" COMPILER_SUPPORTS_MARCH_NATIVE)
if(COMPILER_SUPPORTS_MARCH_NATIVE AND NOT CMAKE_CROSSCOMPILING)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
endif()

# Handle Conan vs System packages
if(USE_CONAN AND EXISTS ${CMAKE_BINARY_DIR}/conan_toolchain.cmake)
    include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
    list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})
    message(STATUS "Using Conan for dependency management")
else()
    message(STATUS "Using system packages for dependency management")
endif()

# Find packages with fallback support
find_package(PkgConfig QUIET)

# OpenCV - multiple detection methods
find_package(OpenCV QUIET)
if(NOT OpenCV_FOUND AND PKG_CONFIG_FOUND)
    pkg_check_modules(OpenCV REQUIRED opencv4)
    if(NOT OpenCV_FOUND)
        pkg_check_modules(OpenCV REQUIRED opencv)
    endif()
endif()

if(NOT OpenCV_FOUND)
    message(FATAL_ERROR "OpenCV not found. Please install OpenCV or use Conan.")
endif()

message(STATUS "OpenCV found: ${OpenCV_VERSION}")

# Boost
find_package(Boost REQUIRED COMPONENTS system filesystem)
message(STATUS "Boost found: ${Boost_VERSION}")

# TBB
find_package(TBB QUIET)
if(NOT TBB_FOUND AND PKG_CONFIG_FOUND)
    pkg_check_modules(TBB tbb)
endif()

if(TBB_FOUND)
    message(STATUS "TBB found")
else()
    message(WARNING "TBB not found - multithreading may be limited")
endif()

# Optional: PyTorch (for CNN descriptors)
if(BUILD_PYTHON_BRIDGE)
    find_package(Python3 COMPONENTS Interpreter Development QUIET)
    if(Python3_FOUND)
        find_package(pybind11 QUIET)
        message(STATUS "Python bridge support enabled")
    else()
        message(WARNING "Python not found - CNN bridge disabled")
    endif()

    # Try to find PyTorch
    find_package(Torch QUIET)
    if(Torch_FOUND)
        message(STATUS "PyTorch found: ${Torch_VERSION}")
    else()
        message(WARNING "PyTorch not found - some CNN features may be disabled")
    endif()
endif()

# Include directories
if(OpenCV_INCLUDE_DIRS)
    include_directories(${OpenCV_INCLUDE_DIRS})
endif()

# Source files
set(SOURCES
    descriptor_compare/main.cpp
    keypoints/VanillaSIFT.cpp
    keypoints/DSPSIFT.cpp
    keypoints/HoWH.cpp
    keypoints/HoNC.cpp
    keypoints/RGBSIFT.cpp
    descriptor_compare/image_processor.cpp
    descriptor_compare/processor_utils.cpp
    descriptor_compare/experiment_config.cpp
    descriptor_compare/locked_in_keypoints.cpp
)

set(HEADERS
    keypoints/VanillaSIFT.h
    keypoints/DSPSIFT.h
    keypoints/HoWH.h
    keypoints/HoNC.h
    keypoints/RGBSIFT.h
    descriptor_compare/image_processor.hpp
    descriptor_compare/processor_utils.hpp
    descriptor_compare/experiment_config.hpp
    descriptor_compare/locked_in_keypoints.hpp
)

# Main executable
add_executable(descriptor_compare ${SOURCES} ${HEADERS})

# Link libraries - handle different package discovery methods
target_link_libraries(descriptor_compare)

# OpenCV linking
if(TARGET opencv_core)
    # Conan-style targets
    target_link_libraries(descriptor_compare ${OpenCV_LIBS})
elseif(OpenCV_LIBRARIES)
    # CMake find_package style
    target_link_libraries(descriptor_compare ${OpenCV_LIBRARIES})
elseif(OpenCV_LDFLAGS)
    # pkg-config style
    target_link_libraries(descriptor_compare ${OpenCV_LDFLAGS})
endif()

# Boost linking
if(TARGET Boost::filesystem)
    target_link_libraries(descriptor_compare Boost::filesystem Boost::system)
else()
    target_link_libraries(descriptor_compare ${Boost_LIBRARIES})
    if(Boost_INCLUDE_DIRS)
        target_include_directories(descriptor_compare PRIVATE ${Boost_INCLUDE_DIRS})
    endif()
endif()

# TBB linking (optional)
if(TARGET TBB::tbb)
    target_link_libraries(descriptor_compare TBB::tbb)
elseif(TBB_LIBRARIES)
    target_link_libraries(descriptor_compare ${TBB_LIBRARIES})
elseif(TBB_LDFLAGS)
    target_link_libraries(descriptor_compare ${TBB_LDFLAGS})
else()
    # Fallback: try to link tbb directly
    find_library(TBB_LIB tbb)
    if(TBB_LIB)
        target_link_libraries(descriptor_compare ${TBB_LIB})
    endif()
endif()

# PyTorch linking (if enabled)
if(BUILD_PYTHON_BRIDGE AND Torch_FOUND)
    target_link_libraries(descriptor_compare ${TORCH_LIBRARIES})
    target_compile_definitions(descriptor_compare PRIVATE USE_TORCH)
endif()

# Python bridge (optional)
if(BUILD_PYTHON_BRIDGE AND Python3_FOUND AND pybind11_FOUND)
    pybind11_add_module(python_descriptor_bridge
        python_bridge/descriptor_bridge.cpp
    )

    target_link_libraries(python_descriptor_bridge PRIVATE pybind11::module)

    # Link OpenCV to Python bridge
    if(TARGET opencv_core)
        target_link_libraries(python_descriptor_bridge PRIVATE ${OpenCV_LIBS})
    elseif(OpenCV_LIBRARIES)
        target_link_libraries(python_descriptor_bridge PRIVATE ${OpenCV_LIBRARIES})
    endif()

    message(STATUS "Python bridge will be built")
endif()

# Define paths - these should work across environments
set(DATA_PATH "${CMAKE_CURRENT_SOURCE_DIR}/data/")
set(RESULTS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/results/")
set(KEYPOINTS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/reference_keypoints/")

target_compile_definitions(descriptor_compare PRIVATE
    DATA_PATH="${DATA_PATH}"
    RESULTS_PATH="${RESULTS_PATH}"
    KEYPOINTS_PATH="${KEYPOINTS_PATH}"
)

# Create necessary directories
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/results)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/reference_keypoints)

# Installation
install(TARGETS descriptor_compare
    RUNTIME DESTINATION bin
)

# Print configuration summary
message(STATUS "=== Configuration Summary ===")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "OpenCV: ${OpenCV_VERSION}")
message(STATUS "Boost: ${Boost_VERSION}")
if(TBB_FOUND)
    message(STATUS "TBB: Found")
else()
    message(STATUS "TBB: Not found")
endif()
if(BUILD_PYTHON_BRIDGE)
    message(STATUS "Python bridge: Enabled")
    if(Torch_FOUND)
        message(STATUS "PyTorch: ${Torch_VERSION}")
    endif()
else()
    message(STATUS "Python bridge: Disabled")
endif()
message(STATUS "Use Conan: ${USE_CONAN}")
message(STATUS "==============================")

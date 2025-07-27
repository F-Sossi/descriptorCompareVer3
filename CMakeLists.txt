cmake_minimum_required(VERSION 3.16)
project(descriptor_compare)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build options
option(USE_CONAN "Use Conan for dependency management" OFF)
option(BUILD_PYTHON_BRIDGE "Build Python bridge for CNN descriptors" OFF)
option(USE_SYSTEM_PACKAGES "Prefer system packages over Conan" ON)
option(BUILD_NEW_INTERFACE "Build new interface system (Stage 3+)" OFF)

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
find_package(Boost CONFIG REQUIRED COMPONENTS system filesystem)
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

# SQLite3 - for database storage
find_package(SQLite3 QUIET)
if(NOT SQLite3_FOUND AND PKG_CONFIG_FOUND)
    pkg_check_modules(SQLite3 sqlite3)
endif()

if(NOT SQLite3_FOUND)
    # Try to find SQLite3 manually
    find_path(SQLite3_INCLUDE_DIR sqlite3.h)
    find_library(SQLite3_LIBRARY sqlite3)

    if(SQLite3_INCLUDE_DIR AND SQLite3_LIBRARY)
        set(SQLite3_FOUND TRUE)
        set(SQLite3_INCLUDE_DIRS ${SQLite3_INCLUDE_DIR})
        set(SQLite3_LIBRARIES ${SQLite3_LIBRARY})
        message(STATUS "SQLite3 found manually")
    else()
        message(FATAL_ERROR "SQLite3 not found. Please install SQLite3 development package.")
    endif()
else()
    message(STATUS "SQLite3 found: ${SQLite3_VERSION}")
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/include)  # NEW: For thesis_project headers
include_directories(${CMAKE_SOURCE_DIR}/src)      # NEW: For interfaces

if(USE_CONAN)
    include_directories(${OpenCV_INCLUDE_DIRS})
    include_directories(${Boost_INCLUDE_DIRS})
    if(TBB_FOUND)
        include_directories(${TBB_INCLUDE_DIRS})
    endif()
    include_directories(${SQLite3_INCLUDE_DIRS})
endif()

# Link directories
if(USE_CONAN)
    link_directories(${OpenCV_LIB_DIRS})
    link_directories(${Boost_LIB_DIRS})
    if(TBB_FOUND)
        link_directories(${TBB_LIB_DIRS})
    endif()
    link_directories(${SQLite3_LIB_DIRS})
endif()

# ================================
# EXISTING KEYPOINTS LIBRARY
# ================================
set(KEYPOINTS_SOURCES
        keypoints/VanillaSIFT.cpp
        keypoints/DSPSIFT.cpp
        keypoints/RGBSIFT.cpp
        keypoints/HoNC.cpp
        keypoints/HoWH.cpp
)

add_library(keypoints ${KEYPOINTS_SOURCES})
target_include_directories(keypoints PUBLIC keypoints)

if(USE_CONAN)
    target_link_libraries(keypoints ${OpenCV_LIBS})
else()
    target_link_libraries(keypoints ${OpenCV_LIBRARIES})
endif()

# ================================
# EXISTING DATABASE LIBRARY
# ================================
set(DATABASE_SOURCES
        database/database_manager.cpp
)

add_library(database ${DATABASE_SOURCES})
target_include_directories(database PUBLIC database)

if(USE_CONAN)
    target_link_libraries(database ${SQLite3_LIBS})
else()
    target_link_libraries(database ${SQLite3_LIBRARIES})
endif()

# ================================
# EXISTING MAIN APPLICATION
# ================================
set(DESCRIPTOR_COMPARE_SOURCES
        descriptor_compare/main.cpp
        descriptor_compare/experiment_config.cpp
        descriptor_compare/image_processor.cpp
        descriptor_compare/processor_utils.cpp
        descriptor_compare/locked_in_keypoints.cpp
)

add_executable(descriptor_compare ${DESCRIPTOR_COMPARE_SOURCES})
target_include_directories(descriptor_compare PRIVATE descriptor_compare)

target_link_libraries(descriptor_compare
        keypoints
        database
        Boost::system
        Boost::filesystem
)

if(USE_CONAN)
    target_link_libraries(descriptor_compare ${OpenCV_LIBS})
    if(TBB_FOUND)
        target_link_libraries(descriptor_compare ${TBB_LIBS})
    endif()
    target_link_libraries(descriptor_compare ${SQLite3_LIBS})
else()
    target_link_libraries(descriptor_compare ${OpenCV_LIBRARIES})
    if(TBB_FOUND)
        target_link_libraries(descriptor_compare ${TBB_LIBRARIES})
    endif()
    target_link_libraries(descriptor_compare ${SQLite3_LIBRARIES})
endif()

# ================================
# PATH DEFINITIONS (from original)
# ================================

# Define paths - these should work across environments
set(DATA_PATH "${CMAKE_CURRENT_SOURCE_DIR}/data/")
set(RESULTS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/results/")
set(KEYPOINTS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/reference_keypoints/")
set(DATABASE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/experiments.db")

target_compile_definitions(descriptor_compare PRIVATE
        DATA_PATH="${DATA_PATH}"
        RESULTS_PATH="${RESULTS_PATH}"
        KEYPOINTS_PATH="${KEYPOINTS_PATH}"
        DATABASE_PATH="${DATABASE_PATH}"
)

# Create necessary directories
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/results)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/reference_keypoints)

# ================================
# NEW INTERFACE SYSTEM (Stage 3+)
# ================================
if(BUILD_NEW_INTERFACE)
    message(STATUS "Building new interface system")

    # NEW: Core library with interfaces
    set(THESIS_CORE_SOURCES
            # Interfaces (header-only for now)
            # Core implementations will be added in later stages
            # src/core/config/ConfigLoader.cpp
            # src/core/runner/ExperimentRunner.cpp
    )

    # Uncomment when we have actual source files
    # add_library(thesis_core ${THESIS_CORE_SOURCES})
    # target_include_directories(thesis_core PUBLIC include src)
    # target_link_libraries(thesis_core ${OpenCV_LIBRARIES})
endif()

# ================================
# TESTING (Stage 6+)
# ================================
option(BUILD_TESTS "Build unit tests" OFF)
if(BUILD_TESTS)
    # Tests will be added in later stages
    message(STATUS "Tests will be implemented in later stages")
endif()

# ================================
# INSTALL RULES
# ================================
install(TARGETS descriptor_compare DESTINATION bin)
install(DIRECTORY config/ DESTINATION share/descriptor_compare/config
        FILES_MATCHING PATTERN "*.yaml" PATTERN "*.yml")

# Print configuration summary
message(STATUS "")
message(STATUS "Configuration Summary:")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Use Conan: ${USE_CONAN}")
message(STATUS "  Use system packages: ${USE_SYSTEM_PACKAGES}")
message(STATUS "  Build new interface: ${BUILD_NEW_INTERFACE}")
message(STATUS "  Build tests: ${BUILD_TESTS}")
message(STATUS "")
message(STATUS "Dependencies:")
message(STATUS "  OpenCV: ${OpenCV_VERSION}")
message(STATUS "  Boost: ${Boost_VERSION}")
message(STATUS "  TBB: ${TBB_FOUND}")
message(STATUS "  SQLite3: ${SQLite3_FOUND}")
message(STATUS "")
cmake_minimum_required(VERSION 3.16)
project(descriptor_compare)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build options
option(USE_CONAN "Use Conan for dependency management" OFF)
option(BUILD_PYTHON_BRIDGE "Build Python bridge for CNN descriptors" OFF)
option(USE_SYSTEM_PACKAGES "Prefer system packages over Conan" ON)
option(BUILD_DATABASE "Build database integration" ON)

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

# Google Test - for modern unit testing
if(PKG_CONFIG_FOUND)
    pkg_check_modules(GTEST gtest)
    pkg_check_modules(GTEST_MAIN gtest_main)
endif()

if(GTEST_FOUND AND GTEST_MAIN_FOUND)
    message(STATUS "Google Test found: using system packages")
    set(GTEST_LIBRARIES ${GTEST_LIBRARIES})
    set(GTEST_MAIN_LIBRARIES ${GTEST_MAIN_LIBRARIES})
    set(GTEST_INCLUDE_DIRS ${GTEST_INCLUDE_DIRS})
else()
    message(STATUS "Google Test not found - legacy tests will continue to use manual main() functions")
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src)

if(USE_CONAN)
    include_directories(${OpenCV_INCLUDE_DIRS})
    include_directories(${Boost_INCLUDE_DIRS})
    if(TBB_FOUND)
        include_directories(${TBB_INCLUDE_DIRS})
    endif()
    include_directories(${SQLite3_INCLUDE_DIRS})
endif()

# Add Google Test include directories if found
if(GTEST_FOUND)
    include_directories(${GTEST_INCLUDE_DIRS})
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
# EXISTING LIBRARIES
# ================================

# KEYPOINTS LIBRARY
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

# DATABASE LIBRARY (LEGACY - REMOVED)
# Legacy database implementation removed in favor of src/core/database/DatabaseManager.cpp
# The modern implementation is in thesis_database library

# STAGE 5: NEW DATABASE LIBRARY (THESIS PROJECT)
if(BUILD_DATABASE AND EXISTS "${CMAKE_SOURCE_DIR}/src/core/database/DatabaseManager.cpp")
    add_library(thesis_database
        src/core/database/DatabaseManager.cpp
    )

    target_include_directories(thesis_database PUBLIC
        include
        ${SQLite3_INCLUDE_DIRS}
    )

    target_link_libraries(thesis_database
        ${SQLite3_LIBRARIES}
        ${OpenCV_LIBS}
    )

    target_compile_features(thesis_database PRIVATE cxx_std_17)
    target_compile_definitions(thesis_database PUBLIC BUILD_DATABASE=1)
    message(STATUS "Stage 5: Thesis database library configured")
else()
    message(STATUS "Stage 5: Database integration disabled or files not found")
endif()

# ================================
# MAIN APPLICATION
# ================================

# Legacy descriptor_compare executable removed - use experiment_runner CLI instead

# Database integration moved to CLI tools (experiment_runner, keypoint_manager)

# PATH DEFINITIONS (kept for compatibility with other targets)
set(DATA_PATH "${CMAKE_CURRENT_SOURCE_DIR}/data/")
set(RESULTS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/results/")
set(KEYPOINTS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/reference_keypoints/")
set(DATABASE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/experiments.db")

# Create necessary directories
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/results)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/reference_keypoints)

# ================================
# COMPREHENSIVE TESTING SYSTEM (CLEAN)
# ================================

# Enable testing
enable_testing()

# Helper function to safely create test executables
function(create_test_if_exists test_source test_name)
    if(EXISTS "${CMAKE_SOURCE_DIR}/${test_source}")
        add_executable(${test_name} ${test_source})
        target_compile_features(${test_name} PRIVATE cxx_std_17)

        # Link libraries based on test type
        if(${test_name} MATCHES "database")
            if(TARGET thesis_database)
                target_link_libraries(${test_name} thesis_database)
            endif()
            if(${test_name} MATCHES "integration" AND NOT ${test_name} MATCHES "simple")
                target_link_libraries(${test_name} keypoints)
                target_include_directories(${test_name} PRIVATE descriptor_compare keypoints)
            endif()
        elseif(${test_name} MATCHES "interface|headers")
            target_link_libraries(${test_name} keypoints)
            target_include_directories(${test_name} PRIVATE keypoints descriptor_compare)
        endif()

        # Add to CTest
        add_test(NAME ${test_name} COMMAND ${test_name})

        # Set test labels
        if(${test_name} MATCHES "database")
            set_tests_properties(${test_name} PROPERTIES LABELS "database;stage5" TIMEOUT 60)
        elseif(${test_name} MATCHES "interface|headers")
            set_tests_properties(${test_name} PROPERTIES LABELS "interface;stage3" TIMEOUT 60)
        elseif(${test_name} MATCHES "types")
            set_tests_properties(${test_name} PROPERTIES LABELS "types;stage2" TIMEOUT 60)
        elseif(${test_name} MATCHES "yaml|config")
            set_tests_properties(${test_name} PROPERTIES LABELS "config;stage4" TIMEOUT 60)
        else()
            set_tests_properties(${test_name} PROPERTIES TIMEOUT 60)
        endif()

        message(STATUS "Test configured: ${test_name}")
    else()
        message(STATUS "Test source not found (skipping): ${test_source}")
    endif()
endfunction()

# Helper function to create Google Test executables
function(create_gtest_if_exists test_source test_name)
    if(NOT GTEST_FOUND OR NOT GTEST_MAIN_FOUND)
        message(STATUS "Google Test not available - skipping: ${test_name}")
        return()
    endif()
    
    if(EXISTS "${CMAKE_SOURCE_DIR}/${test_source}")
        add_executable(${test_name} ${test_source})
        target_compile_features(${test_name} PRIVATE cxx_std_17)
        
        # Link Google Test libraries
        target_link_libraries(${test_name} ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES})
        
        # Link libraries based on test type (same logic as legacy tests)
        if(${test_name} MATCHES "database")
            if(TARGET thesis_database)
                target_link_libraries(${test_name} thesis_database)
            endif()
            if(${test_name} MATCHES "integration" AND NOT ${test_name} MATCHES "simple")
                target_link_libraries(${test_name} keypoints)
                target_include_directories(${test_name} PRIVATE descriptor_compare keypoints)
            endif()
        elseif(${test_name} MATCHES "interface|headers")
            target_link_libraries(${test_name} keypoints)
            target_include_directories(${test_name} PRIVATE keypoints descriptor_compare)
        endif()
        
        # Add to CTest
        add_test(NAME ${test_name} COMMAND ${test_name})
        
        # Set test labels with gtest suffix and proper working directory
        if(${test_name} MATCHES "database")
            set_tests_properties(${test_name} PROPERTIES LABELS "database;stage5;gtest" TIMEOUT 60)
        elseif(${test_name} MATCHES "interface|headers")
            set_tests_properties(${test_name} PROPERTIES LABELS "interface;stage3;gtest" TIMEOUT 60)
        elseif(${test_name} MATCHES "types")
            set_tests_properties(${test_name} PROPERTIES LABELS "types;stage2;gtest" TIMEOUT 60)
        elseif(${test_name} MATCHES "yaml|config")
            set_tests_properties(${test_name} PROPERTIES 
                LABELS "config;stage4;gtest" 
                TIMEOUT 60 
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
        else()
            set_tests_properties(${test_name} PROPERTIES LABELS "gtest" TIMEOUT 60)
        endif()
        
        message(STATUS "Google Test configured: ${test_name}")
    else()
        message(STATUS "Google Test source not found (skipping): ${test_source}")
    endif()
endfunction()

# Create all test executables (only if files exist)
create_test_if_exists("tests/unit/simple_interface_test.cpp" "simple_interface_test")
create_test_if_exists("tests/unit/test_yaml_config.cpp" "test_yaml_config")

# Google Test configuration tests
create_gtest_if_exists("tests/unit/test_yaml_config_gtest.cpp" "test_yaml_config_gtest")

# Database tests (in database subdirectory)
create_test_if_exists("tests/unit/database/test_database.cpp" "test_database")
create_test_if_exists("tests/unit/database/test_database_integration.cpp" "test_database_integration")
create_test_if_exists("tests/unit/database/test_database_integration_simple.cpp" "test_database_integration_simple")
create_test_if_exists("tests/unit/database/test_database_standalone.cpp" "test_database_standalone")

# Google Test database tests
create_gtest_if_exists("tests/unit/database/test_database_gtest.cpp" "test_database_gtest")

# Custom targets for running test groups
add_custom_target(run_all_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --verbose
    COMMENT "Running all tests"
)

add_custom_target(run_database_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -L database --output-on-failure --verbose
    COMMENT "Running database tests only"
)

add_custom_target(run_interface_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -L interface --output-on-failure --verbose
    COMMENT "Running interface tests only"
)

add_custom_target(run_config_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -L config --output-on-failure --verbose
    COMMENT "Running configuration tests only"
)

add_custom_target(run_stage_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -L "stage2|stage3|stage4|stage5" --output-on-failure --verbose
    COMMENT "Running all stage tests"
)

add_custom_target(run_gtest_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -L gtest --output-on-failure --verbose
    COMMENT "Running Google Test framework tests only"
)

# Install rules - descriptor_compare removed, use experiment_runner instead
install(DIRECTORY config/ DESTINATION share/descriptor_compare/config
        FILES_MATCHING PATTERN "*.yaml" PATTERN "*.yml")

# Print configuration summary
message(STATUS "")
message(STATUS "Configuration Summary:")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Use Conan: ${USE_CONAN}")
message(STATUS "  Use system packages: ${USE_SYSTEM_PACKAGES}")
message(STATUS "  Build database integration: ${BUILD_DATABASE}")
message(STATUS "")
message(STATUS "Dependencies:")
message(STATUS "  OpenCV: ${OpenCV_VERSION}")
message(STATUS "  Boost: ${Boost_VERSION}")
message(STATUS "  TBB: ${TBB_FOUND}")
message(STATUS "  SQLite3: ${SQLite3_FOUND}")
message(STATUS "")
message(STATUS "Testing:")
message(STATUS "  Available test targets: run_all_tests, run_database_tests, run_interface_tests, run_config_tests, run_gtest_tests")
if(GTEST_FOUND AND GTEST_MAIN_FOUND)
    message(STATUS "  Google Test: ENABLED - Modern test framework available")
else()
    message(STATUS "  Google Test: DISABLED - Using legacy manual tests")
endif()
message(STATUS "")

# ================================
# STAGE 6: ANALYSIS INTEGRATION
# ================================

# Analysis CLI runner (optional component)
option(BUILD_ANALYSIS "Build analysis integration" ON)

if(BUILD_ANALYSIS)
    message(STATUS "Building Stage 6 analysis integration")

    # Check if analysis runner exists
    if(EXISTS "${CMAKE_SOURCE_DIR}/cli/analysis_runner.cpp")
        add_executable(analysis_runner cli/analysis_runner.cpp)
        target_compile_features(analysis_runner PRIVATE cxx_std_17)

        # No special libraries needed for basic analysis runner
        # It calls Python scripts via system()

        message(STATUS "Analysis runner configured")

        # Create custom target for running analysis
        add_custom_target(run_analysis
            COMMAND ./analysis_runner "${CMAKE_SOURCE_DIR}/results" --full
            DEPENDS analysis_runner
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Running complete analysis pipeline on results folder"
        )

        add_custom_target(quick_analysis
            COMMAND ./analysis_runner "${CMAKE_SOURCE_DIR}/results" --plots-only
            DEPENDS analysis_runner
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Running quick analysis (plots only)"
        )

        message(STATUS "Analysis targets configured: run_analysis, quick_analysis")

    else()
        message(WARNING "analysis_runner.cpp not found - skipping analysis integration")
    endif()
else()
    message(STATUS "Analysis integration disabled (use -DBUILD_ANALYSIS=ON to enable)")
endif()

# ================================
# CLI TOOLS
# ================================

# CLI experiment runner using YAML configuration
option(BUILD_EXPERIMENT_CLI "Build CLI experiment runner with YAML support" ON)

# Keypoint manager CLI tool
option(BUILD_KEYPOINT_MANAGER "Build keypoint manager CLI tool" ON)

if(BUILD_EXPERIMENT_CLI)
    message(STATUS "Building CLI experiment runner")

    # Check if experiment runner exists
    if(EXISTS "${CMAKE_SOURCE_DIR}/cli/experiment_runner.cpp")
        add_executable(experiment_runner cli/experiment_runner.cpp
                       src/core/config/YAMLConfigLoader.cpp
                       src/core/config/ConfigurationBridge.cpp
                       src/core/pooling/NoPooling.cpp
                       src/core/pooling/DomainSizePooling.cpp
                       src/core/pooling/StackingPooling.cpp
                       src/core/pooling/PoolingFactory.cpp
                       src/core/matching/BruteForceMatching.cpp
                       src/core/matching/MatchingFactory.cpp
                       src/core/visualization/VisualVerification.cpp
                       src/core/metrics/TrueAveragePrecision.cpp
                       descriptor_compare/experiment_config.cpp
                       descriptor_compare/image_processor.cpp
                       descriptor_compare/processor_utils.cpp
                       descriptor_compare/locked_in_keypoints.cpp)
        
        target_compile_features(experiment_runner PRIVATE cxx_std_17)
        
        # Include directories
        target_include_directories(experiment_runner PRIVATE
            ${CMAKE_SOURCE_DIR}
            ${CMAKE_SOURCE_DIR}/include
            ${CMAKE_SOURCE_DIR}/src
            ${CMAKE_SOURCE_DIR}/descriptor_compare
        )

        # Path definitions (same as main descriptor_compare)
        target_compile_definitions(experiment_runner PRIVATE
            DATA_PATH="${DATA_PATH}"
            RESULTS_PATH="${RESULTS_PATH}"
            KEYPOINTS_PATH="${KEYPOINTS_PATH}"
            DATABASE_PATH="${DATABASE_PATH}"
        )

        # Link libraries
        target_link_libraries(experiment_runner
            keypoints
            Boost::system
            Boost::filesystem
        )

        # Add database integration if enabled
        if(BUILD_DATABASE)
            target_link_libraries(experiment_runner thesis_database)
            target_compile_definitions(experiment_runner PRIVATE BUILD_DATABASE=1)
        endif()

        # Add OpenCV and other dependencies
        if(USE_CONAN)
            target_link_libraries(experiment_runner ${OpenCV_LIBS})
            target_link_libraries(experiment_runner ${SQLite3_LIBS})
        else()
            target_link_libraries(experiment_runner ${OpenCV_LIBRARIES})
            target_link_libraries(experiment_runner ${SQLite3_LIBRARIES})
        endif()

        # YAML-cpp dependency
        find_package(PkgConfig QUIET)
        if(PkgConfig_FOUND)
            pkg_check_modules(YAMLCPP yaml-cpp)
        endif()
        
        if(YAMLCPP_FOUND)
            target_link_libraries(experiment_runner ${YAMLCPP_LIBRARIES})
            target_include_directories(experiment_runner PRIVATE ${YAMLCPP_INCLUDE_DIRS})
            message(STATUS "yaml-cpp found via pkg-config for experiment runner")
        else()
            # Try system yaml-cpp
            target_link_libraries(experiment_runner yaml-cpp)
            message(STATUS "Using system yaml-cpp library for experiment runner")
        endif()

        message(STATUS "CLI experiment runner configured")
        
    else()
        message(WARNING "experiment_runner.cpp not found - skipping CLI experiment runner")
    endif()
else()
    message(STATUS "CLI experiment runner disabled (use -DBUILD_EXPERIMENT_CLI=ON to enable)")
endif()

# Keypoint manager CLI tool
if(BUILD_KEYPOINT_MANAGER AND BUILD_DATABASE)
    message(STATUS "Building keypoint manager CLI tool")
    
    # Check if keypoint manager exists
    if(EXISTS "${CMAKE_SOURCE_DIR}/cli/keypoint_manager.cpp")
        add_executable(keypoint_manager cli/keypoint_manager.cpp
                       descriptor_compare/locked_in_keypoints.cpp
                       descriptor_compare/processor_utils.cpp
                       src/core/pooling/NoPooling.cpp
                       src/core/pooling/DomainSizePooling.cpp
                       src/core/pooling/StackingPooling.cpp
                       src/core/pooling/PoolingFactory.cpp
                       src/core/matching/BruteForceMatching.cpp
                       src/core/matching/MatchingFactory.cpp)
        
        target_compile_features(keypoint_manager PRIVATE cxx_std_17)
        
        # Include directories
        target_include_directories(keypoint_manager PRIVATE
            ${CMAKE_SOURCE_DIR}
            ${CMAKE_SOURCE_DIR}/include
            ${CMAKE_SOURCE_DIR}/src
            ${CMAKE_SOURCE_DIR}/descriptor_compare
        )

        # Path definitions
        target_compile_definitions(keypoint_manager PRIVATE
            DATA_PATH="${DATA_PATH}"
            RESULTS_PATH="${RESULTS_PATH}"
            KEYPOINTS_PATH="${KEYPOINTS_PATH}"
            DATABASE_PATH="${DATABASE_PATH}"
            BUILD_DATABASE=1
        )

        # Link libraries
        target_link_libraries(keypoint_manager
            keypoints
            thesis_database
            Boost::system
            Boost::filesystem
        )

        # Add OpenCV and other dependencies
        if(USE_CONAN)
            target_link_libraries(keypoint_manager ${OpenCV_LIBS})
            target_link_libraries(keypoint_manager ${SQLite3_LIBS})
        else()
            target_link_libraries(keypoint_manager ${OpenCV_LIBRARIES})
            target_link_libraries(keypoint_manager ${SQLite3_LIBRARIES})
        endif()

        message(STATUS "Keypoint manager CLI tool configured")
        
    else()
        message(WARNING "keypoint_manager.cpp not found - skipping keypoint manager CLI")
    endif()
else()
    if(NOT BUILD_DATABASE)
        message(STATUS "Keypoint manager disabled - requires database integration (use -DBUILD_DATABASE=ON)")
    else()
        message(STATUS "Keypoint manager disabled (use -DBUILD_KEYPOINT_MANAGER=ON to enable)")
    endif()
endif()

# ================================
# STAGE 7: INTERFACE MIGRATION
# ================================

# Interface migration system (optional component)
option(BUILD_INTERFACE_MIGRATION "Build interface migration system" ON)

if(BUILD_INTERFACE_MIGRATION)
    message(STATUS "Building Stage 7 interface migration")

    # Check if migration files exist
    if(EXISTS "${CMAKE_SOURCE_DIR}/src/core/descriptor/factories/DescriptorFactory.cpp")

        # Create the interface migration library
        add_library(thesis_migration
            src/core/descriptor/factories/DescriptorFactory.cpp
            src/core/descriptor/extractors/wrappers/SIFTWrapper.cpp
            src/core/descriptor/extractors/wrappers/RGBSIFTWrapper.cpp
            src/core/integration/ProcessorBridge.cpp
        )

        target_include_directories(thesis_migration PUBLIC
            src/interfaces
            include
            src
            keypoints
            descriptor_compare
         
            ${CMAKE_SOURCE_DIR}/src
        )

        target_link_libraries(thesis_migration
            keypoints
            ${OpenCV_LIBRARIES}
        )

        target_compile_features(thesis_migration PRIVATE cxx_std_17)

        # Stage 7 migration test
        if(EXISTS "${CMAKE_SOURCE_DIR}/tests/unit/integration/test_stage7_migration.cpp")
            add_executable(test_stage7_migration 
                tests/unit/integration/test_stage7_migration.cpp
                descriptor_compare/experiment_config.cpp
            )
            target_link_libraries(test_stage7_migration thesis_migration keypoints ${OpenCV_LIBRARIES})
            target_include_directories(test_stage7_migration PRIVATE
                src
                keypoints
                descriptor_compare
            )
            target_compile_features(test_stage7_migration PRIVATE cxx_std_17)
            message(STATUS "Stage 7 migration test configured")
        endif()

        message(STATUS "Interface migration system configured")

    else()
        message(WARNING "DescriptorFactory.cpp not found - skipping interface migration")
    endif()
else()
    message(STATUS "Interface migration disabled (use -DBUILD_INTERFACE_MIGRATION=ON to enable)")
endif()


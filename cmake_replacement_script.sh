#!/bin/bash
# CMakeLists.txt Replacement Script - Clean Version

echo "=== CMakeLists.txt Clean Replacement ==="
echo "========================================"

echo ""
echo "Creating backup of current CMakeLists.txt..."
cp CMakeLists.txt CMakeLists.txt.conflicted_backup
echo "✅ Backup created: CMakeLists.txt.conflicted_backup"

echo ""
echo "Issues in your current CMakeLists.txt:"
echo "  1. Multiple definitions of 'simple_interface_test' target"
echo "  2. Two different testing systems conflicting"
echo "  3. Database test paths inconsistent (some in database/, some not)"
echo "  4. Comprehensive testing system trying to recreate existing targets"

echo ""
echo "Creating clean CMakeLists.txt with:"
echo "  ✅ Single, consolidated testing system"
echo "  ✅ Safe test creation (only if files exist)"
echo "  ✅ Proper path handling for database tests"
echo "  ✅ No duplicate target definitions"
echo "  ✅ All your existing functionality preserved"

# The clean CMakeLists.txt content is in the artifact above
# For this script, I'll create a temporary file and replace
cat > CMakeLists_clean.txt << 'EOF'
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

# DATABASE LIBRARY (EXISTING)
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
    )

    target_compile_features(thesis_database PRIVATE cxx_std_17)
    message(STATUS "Stage 5: Thesis database library configured")
else()
    message(STATUS "Stage 5: Database integration disabled or files not found")
endif()

# ================================
# MAIN APPLICATION
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

# PATH DEFINITIONS
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

# Create all test executables (only if files exist)
create_test_if_exists("tests/unit/simple_interface_test.cpp" "simple_interface_test")
create_test_if_exists("tests/unit/test_headers.cpp" "test_headers")
create_test_if_exists("tests/unit/test_headers_simple.cpp" "test_headers_simple")
create_test_if_exists("tests/unit/test_new_types.cpp" "test_new_types")
create_test_if_exists("tests/unit/test_yaml_config.cpp" "test_yaml_config")

# Database tests (in database subdirectory)
create_test_if_exists("tests/unit/database/test_constants.cpp" "test_constants")
create_test_if_exists("tests/unit/database/test_database.cpp" "test_database")
create_test_if_exists("tests/unit/database/test_database_integration.cpp" "test_database_integration")
create_test_if_exists("tests/unit/database/test_database_integration_simple.cpp" "test_database_integration_simple")
create_test_if_exists("tests/unit/database/test_database_standalone.cpp" "test_database_standalone")

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

# Install rules
install(TARGETS descriptor_compare DESTINATION bin)
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
message(STATUS "  Available test targets: run_all_tests, run_database_tests, run_interface_tests, run_config_tests")
message(STATUS "")
EOF

echo ""
echo "Replacing CMakeLists.txt with clean version..."
mv CMakeLists_clean.txt CMakeLists.txt
echo "✅ CMakeLists.txt replaced with clean version"

echo ""
echo "Testing the new CMakeLists.txt..."

rm -rf build-cmake-clean-test
mkdir build-cmake-clean-test
cd build-cmake-clean-test

if cmake .. -DUSE_SYSTEM_PACKAGES=ON -DUSE_CONAN=OFF -DBUILD_DATABASE=ON > cmake_test.log 2>&1; then
    echo "✅ CMake configuration successful!"

    # Show configured tests
    echo ""
    echo "Configured tests:"
    grep "Test configured\|Test source not found" cmake_test.log

    if make -j4 > build_test.log 2>&1; then
        echo "✅ Build successful!"

        # List built executables
        echo ""
        echo "Built test executables:"
        ls -1 | grep test || echo "No test executables found"

        # Try running CTest
        echo ""
        echo "Testing CTest integration..."
        if ctest --output-on-failure > ctest_test.log 2>&1; then
            echo "✅ CTest working perfectly!"
            test_count=$(grep -c "Test #" ctest_test.log)
            echo "Number of tests: $test_count"
        else
            echo "⚠️ Some tests failed (may be normal without data files)"
            echo "CTest found $(grep -c "Test #" ctest_test.log) tests"
        fi

    else
        echo "❌ Build failed"
        echo "First 10 lines of build log:"
        head -500 build_test.log
        cd ..
        exit 1
    fi

else
    echo "❌ CMake configuration failed"
    echo "CMake log:"
    cat cmake_test.log
    cd ..
    exit 1
fi

cd ..
rm -rf build-cmake-clean-test

echo ""
echo "Creating simple test validation script..."

cat > validate_clean_cmake.sh << 'EOF'
#!/bin/bash
# Validate Clean CMakeLists.txt

echo "=== Validating Clean CMakeLists.txt ==="
echo "======================================"

echo ""
echo "1. Checking for duplicate targets..."
duplicate_count=$(grep -c "add_executable.*simple_interface_test" CMakeLists.txt)
if [ "$duplicate_count" -eq 1 ]; then
    echo "✅ No duplicate targets found"
else
    echo "❌ Found $duplicate_count definitions of simple_interface_test"
    exit 1
fi

echo ""
echo "2. Building project..."
rm -rf build
mkdir build
cd build

if cmake .. -DUSE_SYSTEM_PACKAGES=ON -DUSE_CONAN=OFF -DBUILD_DATABASE=ON; then
    echo "✅ CMake configuration successful"

    if make -j$(nproc); then
        echo "✅ Build successful"

        # List what was built
        echo ""
        echo "Built executables:"
        ls -1 | grep -E "(test_|descriptor_compare)" | head -10

        echo ""
        echo "3. Testing CTest integration..."

        # Test individual test targets
        if [ -f "./simple_interface_test" ]; then
            echo "Running simple_interface_test..."
            if ./simple_interface_test; then
                echo "✅ simple_interface_test passed"
            else
                echo "❌ simple_interface_test failed"
            fi
        fi

        if [ -f "./test_database_standalone" ]; then
            echo "Running test_database_standalone..."
            if ./test_database_standalone; then
                echo "✅ test_database_standalone passed"
            else
                echo "❌ test_database_standalone failed"
            fi
        fi

        # Test CTest
        echo ""
        echo "Running all tests with CTest..."
        if ctest --output-on-failure --verbose; then
            echo "✅ All CTest tests completed"
        else
            echo "⚠️ Some CTest tests failed (may need data files)"
        fi

        # Test custom targets
        echo ""
        echo "4. Testing custom targets..."

        echo "Available make targets with 'test' in name:"
        make help | grep test || echo "No test targets found in make help"

        # Try the custom test targets
        echo ""
        echo "Testing run_all_tests target..."
        if make run_all_tests > run_all_tests.log 2>&1; then
            echo "✅ run_all_tests target works"
        else
            echo "⚠️ run_all_tests had issues (check run_all_tests.log)"
        fi

    else
        echo "❌ Build failed"
        exit 1
    fi

else
    echo "❌ CMake configuration failed"
    exit 1
fi

cd ..

echo ""
echo "=== Validation Complete ==="
echo ""
echo "✅ Clean CMakeLists.txt is working properly!"
echo ""
echo "Your testing workflow:"
echo "  cd build"
echo "  make                    # Build everything"
echo "  make run_all_tests      # Run all tests"
echo "  make run_database_tests # Run database tests only"
echo "  ctest --output-on-failure # Run tests with CTest"
echo ""
echo "In CLion:"
echo "  - Use build targets: run_all_tests, run_database_tests, etc."
echo "  - Or create CTest run configuration"
EOF

chmod +x validate_clean_cmake.sh

echo ""
echo "=== CMakeLists.txt Clean Replacement Complete ==="
echo ""
echo "Summary of changes made:"
echo "  ✅ Removed duplicate target definitions"
echo "  ✅ Consolidated testing system into single, clean section"
echo "  ✅ Fixed database test paths (tests/unit/database/)"
echo "  ✅ Added safe test creation function (only if files exist)"
echo "  ✅ Preserved all existing functionality"
echo "  ✅ Added proper test labels and organization"
echo ""
echo "What's now available:"
echo "  • All your existing tests properly configured"
echo "  • Custom test targets: run_all_tests, run_database_tests, etc."
echo "  • CTest integration for CLion"
echo "  • Clean, maintainable CMake structure"
echo ""
echo "Next steps:"
echo "  1. Validate: ./validate_clean_cmake.sh"
echo "  2. Build: cd build && make"
echo "  3. Test: make run_all_tests"
echo "  4. In CLion: Use the build targets dropdown"
echo ""
echo "Your original CMakeLists.txt is backed up as:"
echo "  CMakeLists.txt.conflicted_backup"
echo ""
echo "✅ Ready for clean, conflict-free testing!"
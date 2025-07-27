#!/bin/bash
# Stage 1 Validation Script - Fixed Version
# Ensures existing functionality works after foundation setup

echo "=== Stage 1 Validation: Foundation Setup ==="
echo ""

# Check directory structure
echo "1. Checking new directory structure..."
if [ -d "src/interfaces" ] && [ -d "include/thesis_project" ] && [ -d "config/experiments" ]; then
    echo "✅ New directories created successfully"
else
    echo "❌ New directories missing"
    exit 1
fi

# Check existing structure preserved
echo ""
echo "2. Checking existing structure preserved..."
if [ -d "descriptor_compare" ] && [ -d "keypoints" ] && [ -d "database" ]; then
    echo "✅ Existing directories preserved"
else
    echo "❌ Existing directories missing"
    exit 1
fi

# Check if CMakeLists.txt can configure
echo ""
echo "3. Testing CMake configuration..."
rm -rf build-stage1-test
mkdir build-stage1-test
cd build-stage1-test

if cmake .. -DUSE_SYSTEM_PACKAGES=ON -DUSE_CONAN=OFF -DBUILD_NEW_INTERFACE=OFF; then
    echo "✅ CMake configuration successful"
else
    echo "❌ CMake configuration failed"
    cd ..
    exit 1
fi

# Check if project builds
echo ""
echo "4. Testing build..."
if make -j$(nproc) > build.log 2>&1; then
    echo "✅ Build successful"
else
    echo "❌ Build failed"
    echo "Build log:"
    cat build.log
    cd ..
    exit 1
fi

# Check if executable exists
echo ""
echo "5. Testing executable..."
if [ -f "./descriptor_compare" ]; then
    echo "✅ Executable created"
    echo "ℹ️  Executable ready for testing (may need HPatches dataset)"
else
    echo "❌ Executable not created"
    cd ..
    exit 1
fi

cd ..

# Check header files
echo ""
echo "6. Testing new header files..."
if [ -f "include/thesis_project/types.hpp" ] &&
   [ -f "include/thesis_project/constants.hpp" ] &&
   [ -f "include/thesis_project/logging.hpp" ]; then
    echo "✅ Core header files created"
else
    echo "❌ Core header files missing"
    exit 1
fi

# Test header compilation with proper OpenCV paths
echo ""
echo "7. Testing header compilation..."

# Get OpenCV include paths from pkg-config
OPENCV_CFLAGS=""
if command -v pkg-config >/dev/null 2>&1; then
    if pkg-config --exists opencv4; then
        OPENCV_CFLAGS=$(pkg-config --cflags opencv4)
    elif pkg-config --exists opencv; then
        OPENCV_CFLAGS=$(pkg-config --cflags opencv)
    fi
fi

# If pkg-config didn't work, try common system paths
if [ -z "$OPENCV_CFLAGS" ]; then
    if [ -d "/usr/include/opencv4" ]; then
        OPENCV_CFLAGS="-I/usr/include/opencv4"
    elif [ -d "/usr/local/include/opencv4" ]; then
        OPENCV_CFLAGS="-I/usr/local/include/opencv4"
    elif [ -d "/usr/include/opencv2" ]; then
        OPENCV_CFLAGS="-I/usr/include"
    fi
fi

cat > test_headers.cpp << 'EOF'
#include "include/thesis_project/types.hpp"
#include "include/thesis_project/constants.hpp"
#include "include/thesis_project/logging.hpp"

int main() {
    using namespace thesis_project;

    // Test enum access
    DescriptorType type = DescriptorType::SIFT;
    PoolingStrategy pooling = PoolingStrategy::NONE;

    // Test constants
    int desc_size = constants::SIFT_DESCRIPTOR_SIZE;

    // Test logging
    logging::Logger::info("Test message from Stage 1");

    return 0;
}

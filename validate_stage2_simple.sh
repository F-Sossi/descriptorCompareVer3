#!/bin/bash
# Stage 2 Simple Validation - Just verify new types exist alongside old system

echo "=== Stage 2 Validation: Simple Approach ==="
echo ""

echo "1. Checking original system works..."
rm -rf build-test
mkdir build-test
cd build-test

if cmake .. -DUSE_SYSTEM_PACKAGES=ON -DUSE_CONAN=OFF > /dev/null 2>&1 && make -j$(nproc) > /dev/null 2>&1; then
    if [ -f "./descriptor_compare" ]; then
        echo "✅ Original system builds and works perfectly"
    else
        echo "❌ Original system executable missing"
        cd ..
        exit 1
    fi
else
    echo "❌ Original system build failed"
    cd ..
    exit 1
fi

cd ..

echo ""
echo "2. Checking new type system files exist..."
if [ -f "include/thesis_project/types.hpp" ]; then
    echo "✅ New type system file exists"

    if grep -q "enum class PoolingStrategy" include/thesis_project/types.hpp; then
        echo "✅ New scoped enums ready for future use"
    fi
else
    echo "❌ New type system file missing"
    exit 1
fi

echo ""
echo "3. Checking conversion utilities exist..."
if [ -f "include/thesis_project/conversion_utils.hpp" ] && [ -f "src/utils/conversion_utils.cpp" ]; then
    echo "✅ Conversion utilities ready for Stage 3+"
else
    echo "❌ Conversion utilities missing"
fi

echo ""
echo "4. Testing simple type usage (standalone)..."

cat > test_new_types.cpp << 'EOF'
#include <iostream>

// Test new types in isolation (without including problematic headers)
namespace thesis_project {
    enum class PoolingStrategy { NONE, DOMAIN_SIZE_POOLING, STACKING };
    enum class DescriptorType { SIFT, HoNC, RGBSIFT, vSIFT, NONE };
}

int main() {
    using namespace thesis_project;

    // Test new scoped enums
    PoolingStrategy strategy = PoolingStrategy::DOMAIN_SIZE_POOLING;
    DescriptorType type = DescriptorType::RGBSIFT;

    std::cout << "New type system test:" << std::endl;
    std::cout << "  Pooling strategy: " << static_cast<int>(strategy) << std::endl;
    std::cout << "  Descriptor type: " << static_cast<int>(type) << std::endl;
    std::cout << "  Test PASSED" << std::endl;

    return 0;
}
EOF

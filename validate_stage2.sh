#!/bin/bash
# Stage 2 Validation Script

echo "=== Stage 2 Validation: Extract Common Types ==="
echo ""

echo "1. Checking if backup exists..."
if [ -f "descriptor_compare/experiment_config.hpp.stage1_backup" ]; then
    echo "✅ Backup file exists"
else
    echo "❌ Backup file missing"
    exit 1
fi

echo ""
echo "2. Checking if new types are included..."
if grep -q "thesis_project/types.hpp" descriptor_compare/experiment_config.hpp; then
    echo "✅ New types included in experiment_config.hpp"
else
    echo "❌ New types not included"
    exit 1
fi

echo ""
echo "3. Testing compilation..."
rm -rf build-stage2-test
mkdir build-stage2-test
cd build-stage2-test

if cmake .. -DUSE_SYSTEM_PACKAGES=ON -DUSE_CONAN=OFF -DBUILD_NEW_INTERFACE=OFF > cmake.log 2>&1; then
    echo "✅ CMake configuration successful"

    if make -j$(nproc) > build.log 2>&1; then
        echo "✅ Build successful with new type system"

        if [ -f "./descriptor_compare" ]; then
            echo "✅ Executable created successfully"
        else
            echo "❌ Executable not created"
            cd ..
            exit 1
        fi
    else
        echo "❌ Build failed"
        echo "Build log:"
        cat build.log
        cd ..
        exit 1
    fi
else
    echo "❌ CMake configuration failed"
    cat cmake.log
    cd ..
    exit 1
fi

cd ..

echo ""
echo "4. Testing type conversion functions..."

cat > test_type_conversion.cpp << 'EOF'
#include "descriptor_compare/experiment_config.hpp"
#include <iostream>

int main() {
    // Test old enum still works
    DescriptorType oldType = DESCRIPTOR_RGBSIFT;
    PoolingStrategy oldStrategy = DOMAIN_SIZE_POOLING;

    // Test conversion to new types
    auto newType = toNewDescriptorType(oldType);
    auto newStrategy = toNewPoolingStrategy(oldStrategy);

    // Test conversion back
    auto backToOld = toOldDescriptorType(newType);

    std::cout << "Old descriptor type: " << oldType << std::endl;
    std::cout << "Converted back: " << backToOld << std::endl;
    std::cout << "Conversion test: " << (oldType == backToOld ? "PASS" : "FAIL") << std::endl;

    return 0;
}

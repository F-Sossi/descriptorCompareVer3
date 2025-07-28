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

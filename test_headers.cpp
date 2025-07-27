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

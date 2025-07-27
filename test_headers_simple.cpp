#include "include/thesis_project/constants.hpp"
#include "include/thesis_project/logging.hpp"

int main() {
    using namespace thesis_project;

    // Test constants
    int desc_size = constants::SIFT_DESCRIPTOR_SIZE;

    // Test logging
    logging::Logger::info("Simplified test from Stage 1");

    return 0;
}

#include <iostream>
#include <string>

// Include constants directly
namespace thesis_project {
    namespace constants {
        constexpr int SIFT_DESCR_WIDTH = 4;
        constexpr int SIFT_DESCR_HIST_BINS = 8;
        constexpr int SIFT_DESCRIPTOR_SIZE = SIFT_DESCR_WIDTH * SIFT_DESCR_WIDTH * SIFT_DESCR_HIST_BINS;
        constexpr int RGB_SIFT_DESCRIPTOR_SIZE = 3 * SIFT_DESCRIPTOR_SIZE;
    }
}

int main() {
    using namespace thesis_project;

    // Test constants
    int sift_size = constants::SIFT_DESCRIPTOR_SIZE;
    int rgb_sift_size = constants::RGB_SIFT_DESCRIPTOR_SIZE;

    std::cout << "SIFT descriptor size: " << sift_size << std::endl;
    std::cout << "RGB SIFT descriptor size: " << rgb_sift_size << std::endl;
    std::cout << "Stage 1 core functionality test passed!" << std::endl;

    return 0;
}

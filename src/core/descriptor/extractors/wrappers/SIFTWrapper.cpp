#include "SIFTWrapper.hpp"
#include <sstream>

namespace thesis_project {
namespace wrappers {

SIFTWrapper::SIFTWrapper(const experiment_config& config)
    : config_(config) {
    // Initialize OpenCV SIFT with default parameters
    sift_ = cv::SIFT::create();
}

cv::Mat SIFTWrapper::extract(const cv::Mat& image,
                            const std::vector<cv::KeyPoint>& keypoints,
                            const DescriptorParams& params) {
    cv::Mat descriptors;
    std::vector<cv::KeyPoint> mutable_keypoints = keypoints;
    sift_->compute(image, mutable_keypoints, descriptors);
    return descriptors;
}

std::string SIFTWrapper::getConfiguration() const {
    std::stringstream ss;
    ss << "SIFT Wrapper Configuration:\n";
    ss << "  OpenCV SIFT with default parameters\n";
    ss << "  Descriptor size: " << descriptorSize() << "\n";
    ss << "  Pooling Strategy: " << static_cast<int>(config_.descriptorOptions.poolingStrategy) << "\n";
    return ss.str();
}

} // namespace wrappers
} // namespace thesis_project

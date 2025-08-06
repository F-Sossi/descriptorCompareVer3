#include "ProcessorBridge.hpp"
#include "../descriptor/factories/DescriptorFactory.hpp"
#include <iostream>

namespace thesis_project {
namespace integration {

std::pair<std::vector<cv::KeyPoint>, cv::Mat>
ProcessorBridge::detectAndComputeWithConfig(const cv::Mat& image, const experiment_config& config) {
    // Try to use new interface first
    auto extractor = factories::DescriptorFactory::tryCreate(config);

    if (extractor) {
        // New path: Use modern interface
        std::cout << "[ProcessorBridge] Using new interface for: " << extractor->name() << std::endl;

        // TODO: Implement keypoint detection based on config
        std::vector<cv::KeyPoint> keypoints;
        cv::Ptr<cv::SIFT> detector = cv::SIFT::create();
        detector->detect(image, keypoints);

        // Extract descriptors using new interface
        cv::Mat descriptors = extractor->extract(image, keypoints);

        return {keypoints, descriptors};
    } else {
        // Fallback: Use legacy implementation
        std::cout << "[ProcessorBridge] Falling back to legacy implementation" << std::endl;
        return detectAndComputeLegacy(image, config);
    }
}

std::string ProcessorBridge::getImplementationInfo(const experiment_config& config) {
    if (factories::DescriptorFactory::isSupported(config)) {
        return "Using new interface implementation";
    } else {
        return "Using legacy implementation";
    }
}

bool ProcessorBridge::isUsingNewInterface(const experiment_config& config) {
    return factories::DescriptorFactory::isSupported(config);
}

std::pair<std::vector<cv::KeyPoint>, cv::Mat>
ProcessorBridge::detectAndComputeLegacy(const cv::Mat& image, const experiment_config& config) {
    // TODO: Call existing processor_utils functions
    // For now, just throw to indicate not implemented
    throw std::runtime_error("Legacy fallback not yet implemented - needs processor_utils integration");
}

std::pair<std::vector<cv::KeyPoint>, cv::Mat>
ProcessorBridge::detectAndComputeNew(const cv::Mat& image, const experiment_config& config) {
    auto extractor = factories::DescriptorFactory::create(config); // Will throw if not supported

    std::cout << "[ProcessorBridge] Forced new interface for: " << extractor->name() << std::endl;

    // TODO: Implement keypoint detection based on config
    std::vector<cv::KeyPoint> keypoints;
    cv::Ptr<cv::SIFT> detector = cv::SIFT::create();
    detector->detect(image, keypoints);

    // Extract descriptors using new interface
    cv::Mat descriptors = extractor->extract(image, keypoints);

    return {keypoints, descriptors};
}

} // namespace integration
} // namespace thesis_project

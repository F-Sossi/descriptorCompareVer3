#include "StackingPooling.hpp"
#include "descriptor_compare/experiment_config.hpp"
#include "keypoints/VanillaSIFT.h"
#include <iostream>

namespace thesis_project::pooling {

cv::Mat StackingPooling::computeDescriptors(
    const cv::Mat& image,
    const std::vector<cv::KeyPoint>& keypoints,
    const cv::Ptr<cv::Feature2D>& detector,
    const experiment_config& config
) {
    // Validate that we have a secondary detector for stacking
    if (!config.detector2) {
        std::cerr << "[ERROR] No secondary detector configured for stacking" << std::endl;
        return cv::Mat();
    }

    // Prepare images for each descriptor's color space requirements
    cv::Mat image1 = prepareImageForColorSpace(image, config.descriptorOptions.descriptorColorSpace);
    cv::Mat image2 = prepareImageForColorSpace(image, config.descriptorOptions.descriptorColorSpace2);

    // Compute first descriptor
    std::vector<cv::KeyPoint> keypoints1 = keypoints; // Copy to avoid modification
    cv::Mat descriptor1 = computeDescriptorsWithDetector(image1, keypoints1, detector);

    // Compute second descriptor  
    std::vector<cv::KeyPoint> keypoints2 = keypoints; // Copy to avoid modification
    cv::Mat descriptor2 = computeDescriptorsWithDetector(image2, keypoints2, config.detector2);

    // Validate descriptors before concatenation
    if (descriptor1.empty()) {
        std::cerr << "[ERROR] First descriptor is empty in stacking" << std::endl;
        return cv::Mat();
    }
    if (descriptor2.empty()) {
        std::cerr << "[ERROR] Second descriptor is empty in stacking" << std::endl;
        return cv::Mat();
    }
    
    // Check dimensional compatibility
    if (descriptor1.rows != descriptor2.rows) {
        std::cerr << "[ERROR] Descriptor row mismatch: " << descriptor1.rows 
                  << " vs " << descriptor2.rows << std::endl;
        return cv::Mat();
    }

    // Horizontally concatenate descriptors
    cv::Mat stackedDescriptor;
    cv::hconcat(descriptor1, descriptor2, stackedDescriptor);

    return stackedDescriptor;
}

cv::Mat StackingPooling::prepareImageForColorSpace(const cv::Mat& sourceImage, int targetColorSpace) const {
    cv::Mat targetImage = sourceImage.clone();
    
    if (targetColorSpace == D_BW && sourceImage.channels() > 1) {
        // Convert to grayscale for BW descriptors
        cv::cvtColor(sourceImage, targetImage, cv::COLOR_BGR2GRAY);
    } else if (targetColorSpace == D_COLOR && sourceImage.channels() == 1) {
        // Convert grayscale to color for color descriptors  
        cv::cvtColor(sourceImage, targetImage, cv::COLOR_GRAY2BGR);
    }
    
    return targetImage;
}

cv::Mat StackingPooling::computeDescriptorsWithDetector(
    const cv::Mat& image,
    const std::vector<cv::KeyPoint>& keypoints,
    const cv::Ptr<cv::Feature2D>& detector
) const {
    cv::Mat descriptors;
    std::vector<cv::KeyPoint> keypointsCopy = keypoints;
    
    // Handle VanillaSIFT vs standard Feature2D interface
    if (auto vanillaSift = std::dynamic_pointer_cast<VanillaSIFT>(detector)) {
        vanillaSift->compute(image, keypointsCopy, descriptors);
    } else {
        detector->compute(image, keypointsCopy, descriptors);
    }
    
    return descriptors;
}

} // namespace thesis_project::pooling
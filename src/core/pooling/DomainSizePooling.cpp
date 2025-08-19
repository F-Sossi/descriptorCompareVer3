#include "DomainSizePooling.hpp"
#include "descriptor_compare/experiment_config.hpp"
#include "keypoints/VanillaSIFT.h"

namespace thesis_project::pooling {

cv::Mat DomainSizePooling::computeDescriptors(
    const cv::Mat& image,
    const std::vector<cv::KeyPoint>& keypoints,
    const cv::Ptr<cv::Feature2D>& detector,
    const experiment_config& config
) {
    cv::Mat sumOfDescriptors;
    cv::Mat processedImage = image.clone();

    // Convert image to grayscale if needed for this descriptor
    if (config.descriptorOptions.descriptorColorSpace == D_BW && image.channels() > 1) {
        cv::cvtColor(image, processedImage, cv::COLOR_BGR2GRAY);
    }

    // Compute descriptors at multiple scales and sum them
    for (auto scale : config.descriptorOptions.scales) {
        cv::Mat imageScaled;
        cv::resize(processedImage, imageScaled, cv::Size(), scale, scale);

        cv::Mat descriptorsScaled;
        std::vector<cv::KeyPoint> keypointsScaled;
        
        // Scale keypoints to match the resized image
        for (const auto& kp : keypoints) {
            keypointsScaled.emplace_back(cv::KeyPoint(kp.pt * scale, kp.size * scale));
        }

        // Compute descriptors using appropriate interface
        if (auto vanillaSift = std::dynamic_pointer_cast<VanillaSIFT>(detector)) {
            vanillaSift->compute(imageScaled, keypointsScaled, descriptorsScaled);
        } else {
            detector->compute(imageScaled, keypointsScaled, descriptorsScaled);
        }

        // Apply normalization before pooling if configured
        if (config.descriptorOptions.normalizationStage == BEFORE_POOLING) {
            cv::normalize(descriptorsScaled, descriptorsScaled, 1, 0, config.descriptorOptions.normType);
        }

        // Apply rooting before pooling if configured
        if (config.descriptorOptions.rootingStage == R_BEFORE_POOLING) {
            applyRooting(descriptorsScaled);
        }

        // Initialize sum matrix on first iteration
        if (sumOfDescriptors.empty()) {
            sumOfDescriptors = cv::Mat::zeros(descriptorsScaled.rows, descriptorsScaled.cols, descriptorsScaled.type());
        }

        // Accumulate descriptors across scales
        sumOfDescriptors += descriptorsScaled;
    }

    // Apply normalization after pooling if configured
    if (config.descriptorOptions.normalizationStage == AFTER_POOLING) {
        cv::normalize(sumOfDescriptors, sumOfDescriptors, 1, 0, config.descriptorOptions.normType);
    }

    // Apply rooting after pooling if configured
    if (config.descriptorOptions.rootingStage == R_AFTER_POOLING) {
        applyRooting(sumOfDescriptors);
    }

    return sumOfDescriptors;
}

void DomainSizePooling::applyRooting(cv::Mat& descriptors) const {
    // Apply square root to each descriptor element
    // This is a common technique in computer vision to reduce the influence of large values
    for (int i = 0; i < descriptors.rows; ++i) {
        for (int j = 0; j < descriptors.cols; ++j) {
            float& val = descriptors.at<float>(i, j);
            if (val >= 0) {
                val = std::sqrt(val);
            } else {
                val = -std::sqrt(-val); // Handle negative values
            }
        }
    }
}

} // namespace thesis_project::pooling
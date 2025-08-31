#pragma once

#include "PoolingStrategy.hpp"

namespace thesis_project::pooling {

/**
 * @brief No pooling strategy - standard descriptor computation
 * 
 * This is the default strategy that simply computes descriptors normally
 * without any pooling modifications. Equivalent to the original SIFT, RGBSIFT, etc.
 */
class NoPooling : public PoolingStrategy {
public:
    cv::Mat computeDescriptors(
        const cv::Mat& image,
        const std::vector<cv::KeyPoint>& keypoints,
        const cv::Ptr<cv::Feature2D>& detector,
        const experiment_config& config
    ) override;

    // New interface overload
    cv::Mat computeDescriptors(
        const cv::Mat& image,
        const std::vector<cv::KeyPoint>& keypoints,
        thesis_project::IDescriptorExtractor& extractor,
        const experiment_config& config
    ) override;

    std::string getName() const override {
        return "None";
    }

    float getDimensionalityMultiplier() const override {
        return 1.0f;
    }

    bool requiresColorInput() const override {
        return false; // Works with any image type
    }
};

} // namespace thesis_project::pooling

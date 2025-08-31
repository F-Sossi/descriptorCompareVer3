#pragma once

#include <opencv2/opencv.hpp>
#include <utility>
#include <vector>

// Forward declare legacy config
class experiment_config;

namespace thesis_project {
namespace integration {

// Returns true if DescriptorFactory supports the descriptor in config
bool isNewInterfaceSupported(const experiment_config& config);

// Runs a minimal detect+compute using the new interface; may throw
std::pair<std::vector<cv::KeyPoint>, cv::Mat>
smokeDetectAndCompute(const cv::Mat& image, const experiment_config& config);

} // namespace integration
} // namespace thesis_project


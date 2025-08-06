#pragma once

#include "descriptor_compare/experiment_config.hpp"
#include "../descriptor/factories/DescriptorFactory.hpp"
#include <opencv2/opencv.hpp>
#include <vector>
#include <utility>
#include <string>

namespace thesis_project {
namespace integration {

/**
 * @brief Bridge between legacy experiment_config system and new interface system
 *
 * Provides gradual migration path from old to new architecture
 */
class ProcessorBridge {
public:
    /**
     * @brief Process image using config, automatically choosing implementation
     * @param image Input image
     * @param config Experiment configuration
     * @return Pair of (keypoints, descriptors)
     */
    static std::pair<std::vector<cv::KeyPoint>, cv::Mat>
    detectAndComputeWithConfig(const cv::Mat& image, const experiment_config& config);

    /**
     * @brief Get information about which implementation will be used
     * @param config Experiment configuration
     * @return String describing the implementation path
     */
    static std::string getImplementationInfo(const experiment_config& config);

    /**
     * @brief Check if the new interface will be used for this config
     * @param config Experiment configuration
     * @return true if new interface will be used, false for legacy
     */
    static bool isUsingNewInterface(const experiment_config& config);

    /**
     * @brief Force use of legacy implementation (for testing)
     * @param config Experiment configuration
     * @return Processing results using legacy path
     */
    static std::pair<std::vector<cv::KeyPoint>, cv::Mat>
    detectAndComputeLegacy(const cv::Mat& image, const experiment_config& config);

    /**
     * @brief Force use of new interface (will throw if not supported)
     * @param config Experiment configuration
     * @return Processing results using new interface
     */
    static std::pair<std::vector<cv::KeyPoint>, cv::Mat>
    detectAndComputeNew(const cv::Mat& image, const experiment_config& config);
};

} // namespace integration
} // namespace thesis_project

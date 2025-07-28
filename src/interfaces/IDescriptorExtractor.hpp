#pragma once

#include "include/thesis_project/types.hpp"
#include <opencv4/opencv2/opencv.hpp>
#include <vector>
#include <memory>
#include <string>

namespace thesis_project {

    /**
     * @brief Interface for descriptor extraction
     *
     * This interface provides a unified way to work with all descriptor types
     * in the system, including existing ones (RGBSIFT, HoNC, VanillaSIFT) and
     * future additions.
     */
    class IDescriptorExtractor {
    public:
        virtual ~IDescriptorExtractor() = default;

        /**
         * @brief Extract descriptors from image at given keypoints
         * @param image Input image (color or grayscale)
         * @param keypoints Keypoints where to extract descriptors
         * @param params Extraction parameters (pooling, normalization, etc.)
         * @return Matrix of descriptors (one row per keypoint)
         */
        virtual cv::Mat extract(const cv::Mat& image,
                               const std::vector<cv::KeyPoint>& keypoints,
                               const DescriptorParams& params = {}) = 0;

        /**
         * @brief Get human-readable name of the descriptor
         * @return Name string (e.g., "RGBSIFT", "HoNC", "SIFT")
         */
        virtual std::string name() const = 0;

        /**
         * @brief Get the size of a single descriptor in floats
         * @return Descriptor size (e.g., 128 for SIFT, 384 for RGBSIFT)
         */
        virtual int descriptorSize() const = 0;

        /**
         * @brief Check if this descriptor supports color images
         * @return true if color is supported/required, false for grayscale only
         */
        virtual bool supportsColor() const = 0;

        /**
         * @brief Check if this descriptor supports pooling strategies
         * @return true if pooling can be applied, false otherwise
         */
        virtual bool supportsPooling() const = 0;

        /**
         * @brief Get the descriptor type enum
         * @return Type identifier
         */
        virtual DescriptorType type() const = 0;
    };

    /**
     * @brief Interface for keypoint detection
     *
     * Separate from descriptor extraction to allow different combinations
     */
    class IKeypointDetector {
    public:
        virtual ~IKeypointDetector() = default;

        /**
         * @brief Detect keypoints in image
         * @param image Input image
         * @param params Detection parameters
         * @return Vector of detected keypoints
         */
        virtual std::vector<cv::KeyPoint> detect(const cv::Mat& image,
                                                const KeypointParams& params = {}) = 0;

        /**
         * @brief Get detector name
         * @return Name string
         */
        virtual std::string name() const = 0;

        /**
         * @brief Get the detector type enum
         * @return Type identifier
         */
        virtual KeypointGenerator type() const = 0;
    };

    /**
     * @brief Combined interface for detect + compute operations
     *
     * For descriptors that do both keypoint detection and descriptor extraction
     */
    class IFeatureDetector {
    public:
        virtual ~IFeatureDetector() = default;

        /**
         * @brief Detect keypoints and compute descriptors in one step
         * @param image Input image
         * @param keypoint_params Keypoint detection parameters
         * @param descriptor_params Descriptor extraction parameters
         * @return Pair of (keypoints, descriptors)
         */
        virtual std::pair<std::vector<cv::KeyPoint>, cv::Mat>
        detectAndCompute(const cv::Mat& image,
                        const KeypointParams& keypoint_params = {},
                        const DescriptorParams& descriptor_params = {}) = 0;

        virtual std::string name() const = 0;
        virtual DescriptorType descriptorType() const = 0;
        virtual KeypointGenerator keypointType() const = 0;
    };

} // namespace thesis_project
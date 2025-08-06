#ifndef THESIS_PROJECT_IDESCRIPTOR_EXTRACTOR_HPP
#define THESIS_PROJECT_IDESCRIPTOR_EXTRACTOR_HPP

#include <opencv2/opencv.hpp>
#include <vector>
#include <memory>
#include <string>

namespace thesis_project {

    // Forward declarations
    struct DescriptorParams {
        // Add descriptor-specific parameters here
        int param1 = 0;
        double param2 = 1.0;
    };

    /**
     * @brief Interface for descriptor extraction algorithms
     */
    class IDescriptorExtractor {
    public:
        virtual ~IDescriptorExtractor() = default;

        /**
         * @brief Extract descriptors from keypoints
         */
        virtual cv::Mat extract(const cv::Mat& image,
                               const std::vector<cv::KeyPoint>& keypoints,
                               const DescriptorParams& params = {}) = 0;

        /**
         * @brief Get the descriptor name
         */
        virtual std::string name() const = 0;

        /**
         * @brief Get the descriptor size (dimensions)
         */
        virtual int descriptorSize() const = 0;

        /**
         * @brief Get the descriptor type
         */
        virtual int descriptorType() const = 0;
    };

} // namespace thesis_project

#endif // THESIS_PROJECT_IDESCRIPTOR_EXTRACTOR_HPP

#pragma once

#include "thesis_project/types.hpp"
#include <opencv2/opencv.hpp>
#include <vector>
#include <memory>

namespace thesis_project {

    // Interface for descriptor extraction
    // This will be implemented in Stage 3
    class IDescriptorExtractor {
    public:
        virtual ~IDescriptorExtractor() = default;
        
        // TODO: Add interface methods in Stage 3
        // virtual cv::Mat extract(const cv::Mat& image, 
        //                        const std::vector<cv::KeyPoint>& keypoints,
        //                        const DescriptorParams& params = {}) = 0;
        // virtual std::string name() const = 0;
        // virtual int descriptor_size() const = 0;
    };

} // namespace thesis_project
#pragma once

#include "../../../../interfaces/IDescriptorExtractor.hpp"
#include <opencv4/opencv2/opencv.hpp>
#include <opencv4/opencv2/features2d.hpp>

namespace thesis_project {
namespace descriptors {

    /**
     * @brief Wrapper for OpenCV SIFT descriptor
     * 
     * Adapts OpenCV's built-in SIFT to work with the new interface system.
     */
    class OpenCVSIFTWrapper : public IDescriptorExtractor {
    private:
        cv::Ptr<cv::SIFT> sift_detector_;
        
    public:
        OpenCVSIFTWrapper() {
            sift_detector_ = cv::SIFT::create();
        }
        
        cv::Mat extract(const cv::Mat& image, 
                       const std::vector<cv::KeyPoint>& keypoints,
                       const DescriptorParams& params = {}) override {
            
            cv::Mat descriptors;
            
            // Handle color space - OpenCV SIFT works with grayscale
            cv::Mat processed_image = image;
            if (image.channels() == 3) {
                cv::cvtColor(image, processed_image, cv::COLOR_BGR2GRAY);
            }
            
            // Convert keypoints to mutable vector for OpenCV interface
            std::vector<cv::KeyPoint> mutable_keypoints = keypoints;
            
            try {
                // Use OpenCV SIFT compute method
                sift_detector_->compute(processed_image, mutable_keypoints, descriptors);
            } catch (const cv::Exception& e) {
                throw std::runtime_error("OpenCV SIFT extraction failed: " + std::string(e.what()));
            }
            
            // Apply pooling if requested
            if (params.pooling != PoolingStrategy::NONE) {
                descriptors = applyPooling(descriptors, params);
            }
            
            // Apply normalization if requested
            if (params.normalize_after_pooling) {
                cv::normalize(descriptors, descriptors, 1.0, 0.0, params.norm_type);
            }
            
            return descriptors;
        }
        
        std::string name() const override {
            return "OpenCV_SIFT";
        }
        
        int descriptorSize() const override {
            return 128;  // Standard SIFT descriptor size
        }
        
        bool supportsColor() const override {
            return false;  // OpenCV SIFT works with grayscale
        }
        
        bool supportsPooling() const override {
            return true;  // Can apply pooling strategies
        }
        
        DescriptorType type() const override {
            return DescriptorType::SIFT;
        }
        
    private:
        cv::Mat applyPooling(const cv::Mat& descriptors, const DescriptorParams& params) {
            switch (params.pooling) {
                case PoolingStrategy::DOMAIN_SIZE_POOLING:
                    return applyDomainSizePooling(descriptors, params.scales);
                case PoolingStrategy::STACKING:
                    return descriptors;  // Handled at higher level
                case PoolingStrategy::NONE:
                default:
                    return descriptors;
            }
        }
        
        cv::Mat applyDomainSizePooling(const cv::Mat& descriptors, 
                                      const std::vector<float>& scales) {
            // TODO: Implement domain size pooling for OpenCV SIFT
            return descriptors;
        }
    };

} // namespace descriptors
} // namespace thesis_project
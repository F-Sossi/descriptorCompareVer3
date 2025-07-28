#pragma once

#include "../../../../interfaces/IDescriptorExtractor.hpp"
#include "../../../../../keypoints/RGBSIFT.h"
#include <opencv4/opencv2/opencv.hpp>

namespace thesis_project {
namespace descriptors {

    /**
     * @brief Wrapper for existing RGBSIFT class
     * 
     * Adapts the existing RGBSIFT implementation to work with the new interface system
     * while preserving all original functionality.
     */
    class RGBSIFTWrapper : public IDescriptorExtractor {
    private:
        cv::Ptr<cv::RGBSIFT> rgbsift_detector_;
        
    public:
        RGBSIFTWrapper() {
            rgbsift_detector_ = cv::RGBSIFT::create();
        }
        
        cv::Mat extract(const cv::Mat& image, 
                       const std::vector<cv::KeyPoint>& keypoints,
                       const DescriptorParams& params = {}) override {
            
            cv::Mat descriptors;
            
            // Handle color space conversion
            cv::Mat processed_image = image;
            if (!params.use_color && image.channels() == 3) {
                cv::cvtColor(image, processed_image, cv::COLOR_BGR2GRAY);
                // Note: RGBSIFT actually needs color, but we respect the parameter
                // This might need to be handled differently based on requirements
            }
            
            // Convert keypoints to mutable vector for RGBSIFT interface
            std::vector<cv::KeyPoint> mutable_keypoints = keypoints;
            
            // Use existing RGBSIFT operator() method
            try {
                rgbsift_detector_->operator()(processed_image, cv::noArray(), 
                                            mutable_keypoints, descriptors, 
                                            true);  // useProvidedKeypoints = true
            } catch (const cv::Exception& e) {
                throw std::runtime_error("RGBSIFT extraction failed: " + std::string(e.what()));
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
            return "RGBSIFT";
        }
        
        int descriptorSize() const override {
            return rgbsift_detector_->descriptorSize();  // Should return 384 (3 * 128)
        }
        
        bool supportsColor() const override {
            return true;  // RGBSIFT requires color images
        }
        
        bool supportsPooling() const override {
            return true;  // RGBSIFT supports domain size pooling
        }
        
        DescriptorType type() const override {
            return DescriptorType::RGBSIFT;
        }
        
    private:
        /**
         * @brief Apply pooling strategy to descriptors
         * @param descriptors Input descriptors
         * @param params Pooling parameters
         * @return Pooled descriptors
         */
        cv::Mat applyPooling(const cv::Mat& descriptors, const DescriptorParams& params) {
            switch (params.pooling) {
                case PoolingStrategy::DOMAIN_SIZE_POOLING:
                    return applyDomainSizePooling(descriptors, params.scales);
                case PoolingStrategy::STACKING:
                    // Stacking would need another descriptor - handle in higher level
                    return descriptors;
                case PoolingStrategy::NONE:
                default:
                    return descriptors;
            }
        }
        
        /**
         * @brief Apply domain size pooling
         * @param descriptors Input descriptors
         * @param scales Scales for pooling
         * @return Pooled descriptors
         */
        cv::Mat applyDomainSizePooling(const cv::Mat& descriptors, 
                                      const std::vector<float>& scales) {
            // TODO: Implement domain size pooling logic
            // For now, return original descriptors
            // This would involve re-extracting descriptors at different scales
            // and combining them according to the DSP strategy
            return descriptors;
        }
    };

} // namespace descriptors
} // namespace thesis_project
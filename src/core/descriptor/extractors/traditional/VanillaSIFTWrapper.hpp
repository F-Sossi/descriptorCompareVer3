#pragma once

#include "../../../../interfaces/IDescriptorExtractor.hpp"
#include "../../../../../keypoints/VanillaSIFT.h"
#include <opencv4/opencv2/opencv.hpp>

namespace thesis_project {
namespace descriptors {

    /**
     * @brief Wrapper for existing VanillaSIFT class
     * 
     * Adapts the existing VanillaSIFT implementation to work with the new interface system.
     */
    class VanillaSIFTWrapper : public IDescriptorExtractor {
    private:
        cv::Ptr<VanillaSIFT> vanilla_sift_detector_;
        
    public:
        VanillaSIFTWrapper() {
            vanilla_sift_detector_ = VanillaSIFT::create();
        }
        
        cv::Mat extract(const cv::Mat& image, 
                       const std::vector<cv::KeyPoint>& keypoints,
                       const DescriptorParams& params = {}) override {
            
            cv::Mat descriptors;
            
            // Handle color space - VanillaSIFT typically works with grayscale
            cv::Mat processed_image = image;
            if (image.channels() == 3) {
                cv::cvtColor(image, processed_image, cv::COLOR_BGR2GRAY);
            }
            
            // Convert keypoints to mutable vector for VanillaSIFT interface
            std::vector<cv::KeyPoint> mutable_keypoints = keypoints;
            
            try {
                // Use VanillaSIFT's compute method (it has both operator() and compute())
                vanilla_sift_detector_->compute(processed_image, mutable_keypoints, descriptors);
            } catch (const cv::Exception& e) {
                throw std::runtime_error("VanillaSIFT extraction failed: " + std::string(e.what()));
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
            return "VanillaSIFT";
        }
        
        int descriptorSize() const override {
            // VanillaSIFT should return 128 (standard SIFT descriptor size)
            return 128;  // SIFT_DESCR_WIDTH * SIFT_DESCR_WIDTH * SIFT_DESCR_HIST_BINS
        }
        
        bool supportsColor() const override {
            return false;  // VanillaSIFT works with grayscale
        }
        
        bool supportsPooling() const override {
            return true;  // Can apply pooling strategies
        }
        
        DescriptorType type() const override {
            return DescriptorType::vSIFT;
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
            // TODO: Implement domain size pooling for VanillaSIFT
            return descriptors;
        }
    };

} // namespace descriptors
} // namespace thesis_project
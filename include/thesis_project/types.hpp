#pragma once

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <memory>

namespace thesis_project {

    // Forward declarations
    class IKeypointGenerator;
    class IDescriptorExtractor;
    class IEvaluator;

    // Core enums - extracted from experiment_config.hpp
    enum class PoolingStrategy {
        NONE,
        DOMAIN_SIZE_POOLING,
        STACKING
    };

    enum class DescriptorType {
        SIFT,
        RGBSIFT,
        HoNC,
        HoWH,
        VANILLA_SIFT,
        DSP_SIFT
    };

    enum class KeypointGenerator {
        SIFT,
        HARRIS,
        ORB,
        LOCKED_IN  // For using pre-computed keypoints
    };

    enum class MatchingMethod {
        BRUTE_FORCE,
        FLANN
    };

    enum class ValidationMethod {
        HOMOGRAPHY,
        CROSS_IMAGE,
        NONE
    };

    // Experiment results structure
    struct ExperimentMetrics {
        float precision = 0.0f;
        float recall = 0.0f;
        float f1_score = 0.0f;
        
        // Timing metrics
        double keypoint_extraction_time_ms = 0.0;
        double descriptor_extraction_time_ms = 0.0;
        double matching_time_ms = 0.0;
        
        // Count metrics  
        int keypoints_detected = 0;
        int descriptors_computed = 0;
        int matches_found = 0;
        int correct_matches = 0;
        
        // Resource metrics
        double memory_peak_mb = 0.0;
    };

    struct ExperimentResults {
        std::string experiment_name;
        std::string scene_name;
        std::string descriptor_name;
        std::string keypoint_generator_name;
        
        ExperimentMetrics metrics;
        
        // Optional data
        std::vector<cv::KeyPoint> keypoints_image1;
        std::vector<cv::KeyPoint> keypoints_image2;
        cv::Mat descriptors_image1;
        cv::Mat descriptors_image2;
        std::vector<cv::DMatch> matches;
        
        // Paths to saved outputs
        std::string output_directory;
        std::string visualization_path;
        
        // Metadata
        std::string timestamp;
        std::string config_hash;
    };

    // Parameter structures for configuration
    struct KeypointParams {
        int max_features = 2000;
        float contrast_threshold = 0.04f;
        float edge_threshold = 10.0f;
        float sigma = 1.6f;
        int num_octaves = 4;
        bool use_locked_keypoints = false;
        std::string locked_keypoints_path;
    };

    struct DescriptorParams {
        PoolingStrategy pooling = PoolingStrategy::NONE;
        std::vector<float> scales = {1.0f, 1.5f, 2.0f};
        bool normalize_before_pooling = false;
        bool normalize_after_pooling = true;
        int norm_type = cv::NORM_L2;
        bool use_color = false;
        
        // For stacking
        DescriptorType secondary_descriptor = DescriptorType::SIFT;
        float stacking_weight = 0.5f;
    };

    struct EvaluationParams {
        MatchingMethod matching_method = MatchingMethod::BRUTE_FORCE;
        int norm_type = cv::NORM_L2;
        bool cross_check = true;
        float match_threshold = 0.8f;
        
        ValidationMethod validation_method = ValidationMethod::HOMOGRAPHY;
        float validation_threshold = 0.05f; // pixels
        int min_matches_for_homography = 10;
    };

    struct DatabaseParams {
        bool enabled = false;
        std::string connection_string = "sqlite:///experiments.db";
        bool save_keypoints = true;
        bool save_descriptors = false;
        bool save_matches = false;
        bool save_visualizations = true;
    };

} // namespace thesis_project
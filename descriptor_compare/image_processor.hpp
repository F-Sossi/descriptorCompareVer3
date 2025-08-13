#ifndef DESCRIPTOR_COMPARE_IMAGE_PROCESSOR_HPP
#define DESCRIPTOR_COMPARE_IMAGE_PROCESSOR_HPP

#include <string>
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <iostream>
#include <vector>
#include <fstream>
#include <random>
#include <map>


#include "experiment_config.hpp"
#include "processor_utils.hpp"

/**
 * @brief Structure to hold comprehensive experiment results metrics
 */
struct ExperimentMetrics {
    // Per-image precision values
    std::vector<double> precisions_per_image;
    
    // Aggregate metrics
    double mean_precision = 0.0;
    double mean_average_precision = 0.0;
    double precision_at_1 = 0.0;
    double precision_at_5 = 0.0;
    double recall_at_1 = 0.0;
    double recall_at_5 = 0.0;
    
    // Count metrics
    int total_matches = 0;
    int total_keypoints = 0;
    int total_images_processed = 0;
    
    // Per-scene breakdown (e.g., "i_dome", "v_wall")
    std::map<std::string, double> per_scene_precision;
    std::map<std::string, int> per_scene_matches;
    std::map<std::string, int> per_scene_keypoints;
    
    // Processing metadata
    double processing_time_ms = 0.0;
    bool success = false;
    std::string error_message;
    
    /**
     * @brief Calculate mean precision from per-image values
     */
    void calculateMeanPrecision() {
        if (!precisions_per_image.empty()) {
            double sum = 0.0;
            for (double p : precisions_per_image) {
                sum += p;
            }
            mean_precision = sum / precisions_per_image.size();
            mean_average_precision = mean_precision; // For now, same as mean precision
        }
    }
    
    /**
     * @brief Add precision result for a specific image and scene
     */
    void addImageResult(const std::string& scene_name, double precision, int matches, int keypoints) {
        precisions_per_image.push_back(precision);
        total_matches += matches;
        total_keypoints += keypoints;
        total_images_processed++;
        
        // Update per-scene statistics
        if (per_scene_precision.find(scene_name) == per_scene_precision.end()) {
            per_scene_precision[scene_name] = 0.0;
            per_scene_matches[scene_name] = 0;
            per_scene_keypoints[scene_name] = 0;
        }
        
        // Running average for scene precision
        int scene_count = per_scene_matches[scene_name] > 0 ? 
            per_scene_matches[scene_name] / per_scene_keypoints[scene_name] : 0;
        scene_count++;
        per_scene_precision[scene_name] = 
            (per_scene_precision[scene_name] * (scene_count - 1) + precision) / scene_count;
        
        per_scene_matches[scene_name] += matches;
        per_scene_keypoints[scene_name] += keypoints;
    }
};


/**
 * @brief The image_processor class provides functionality for processing image directories
 *        and performing various image processing tasks.
 */
class image_processor {
public:
    /**
     * @brief Processes a directory of image folders and performs image processing based on the provided configuration.
     * @param data_folder The path to the directory containing image folders.
     * @param results_folder The path to the directory where the results will be stored.
     * @param config The experiment configuration specifying the processing options.
     * @return ExperimentMetrics containing all computed metrics and success status.
     */
    static ExperimentMetrics process_directory(const std::string& data_folder, const std::string& results_folder, const experiment_config& config);


private:
    /**
     * @brief Processes a single image folder and performs image processing based on the provided configuration.
     * @param folder The path to the image folder.
     * @param results_folder The path to the directory where the results will be stored.
     * @param config The experiment configuration specifying the processing options.
     * @return ExperimentMetrics for this specific folder/scene.
     */
    static ExperimentMetrics process_image_folder_keypoints_unlocked(const std::string& folder, const std::string& results_folder, const experiment_config& config);
    static ExperimentMetrics process_image_folder_keypoints_locked(const std::string& folder, const std::string& results_folder, const experiment_config& config);
    /**
     * @brief Performs visual verification of descriptor matches between image pairs in a folder.
     * @param folder The path to the image folder.
     * @param results_folder The path to the directory where the results will be stored.
     * @param config The experiment configuration specifying the processing options.
     */
    static void visual_verification_matches(const std::string &folder, const std::string &results_folder, const experiment_config &config);
    /**
     * @brief Performs visual verification of keypoint projection using homography matrices between image pairs in a folder.
     * @param folder The path to the image folder.
     * @param results_folder The path to the directory where the results will be stored.
     * @param config The experiment configuration specifying the processing options.
     */
    static void visual_verification_homography(const std::string& folder, const std::string& results_folder, const experiment_config& config);
};


#endif //DESCRIPTOR_COMPARE_IMAGE_PROCESSOR_HPP

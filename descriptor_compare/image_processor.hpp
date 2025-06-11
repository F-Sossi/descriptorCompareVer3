#ifndef DESCRIPTOR_COMPARE_IMAGE_PROCESSOR_HPP
#define DESCRIPTOR_COMPARE_IMAGE_PROCESSOR_HPP

#include <string>
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <iostream>
#include <vector>
#include <fstream>
#include <random>


#include "experiment_config.hpp"
#include "processor_utils.hpp"


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
     * @return True if the processing is successful, false otherwise.
     */
    static bool process_directory(const std::string& data_folder, const std::string& results_folder, const experiment_config& config);


private:
    /**
     * @brief Processes a single image folder and performs image processing based on the provided configuration.
     * @param folder The path to the image folder.
     * @param results_folder The path to the directory where the results will be stored.
     * @param config The experiment configuration specifying the processing options.
     */
    static void process_image_folder_keypoints_unlocked(const std::string& folder, const std::string& results_folder, const experiment_config& config);
    static void process_image_folder_keypoints_locked(const std::string& folder, const std::string& results_folder, const experiment_config& config);
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

#include <string>
#include "image_processor.hpp"
#include "experiment_config.hpp"
#include "locked_in_keypoints.hpp"
#include <boost/filesystem.hpp>  // Add this include
#ifdef BUILD_DATABASE
#include "thesis_project/database/DatabaseManager.hpp"
#endif
#include <chrono>

namespace fs = boost::filesystem;  // Add this namespace alias

//#include "tqdm.hpp"

// Helper function to convert enum and settings to string for naming
std::string poolingStrategyToString(PoolingStrategy strategy) {
    switch (strategy) {
        case NONE: return "None";
        case DOMAIN_SIZE_POOLING: return "Dom";
        case STACKING: return "Stack";
        default: return "UnkPool";
    }
}

std::string normalizationStageToString(NormalizationStage stage) {
    switch (stage) {
        case BEFORE_POOLING: return "Bef";
        case AFTER_POOLING: return "Aft";
        case NO_NORMALIZATION: return "NoNorm";
        default: return "UnkNorm";
    }
}

std::string rootingStageToString(RootingStage stage) {
    switch (stage) {
        case R_BEFORE_POOLING: return "RBef";
        case R_AFTER_POOLING: return "RAft";
        case R_NONE: return "NoRoot";
        default: return "UnkRoot";
    }
}

std::string normTypeToString(int normType) {
    switch (normType) {
        case cv::NORM_L1: return "L1";
        case cv::NORM_L2: return "L2";
        default: return "UnkNormType";
    }
}

std::string descriptorTypeToString(DescriptorType type) {
    switch (type) {
        case DESCRIPTOR_SIFT: return "SIFT";
        case DESCRIPTOR_HoNC: return "HoNC";
        case DESCRIPTOR_RGBSIFT: return "RGBSIFT";
        case DESCRIPTOR_vSIFT: return "vSIFT";
            // Add more cases as needed
        default: return "Unknown";
    }
}

std::string imageTypeToString(ImageType imageType) {
    switch (imageType) {
        case COLOR: return "CLR";
        case BW: return "BW";
        default: return "Unknown";
    }
}

/*#include <torch/torch.h>
#include <iostream>

int main() {
    torch::Tensor tensor = torch::rand({2, 3});
    std::cout << tensor << std::endl;
}*/

int main() {

    // Data path defined in the CMakeLists.txt for portable paths
    std::string dataPath = DATA_PATH;
    std::string resultsPath = RESULTS_PATH;

#ifdef BUILD_DATABASE
    // Initialize database for experiment tracking
    thesis_project::database::DatabaseManager db("experiments.db", true);
    if (db.isEnabled()) {
        std::cout << "Database tracking enabled\n";
    } else {
        std::cout << "Database tracking disabled\n";
    }
#endif

    bool success = false;

    // Simple SIFT baseline experiment
    std::vector<PoolingStrategy> poolingStrategies = {NONE};
    std::vector<NormalizationStage> normalizationStages = {NO_NORMALIZATION};
    std::vector<RootingStage> rootingStages = {R_NONE};
    std::vector<int> normTypes = {cv::NORM_L2}; // Standard L2 norm for SIFT
    std::vector<DescriptorType> descriptorTypes = {DESCRIPTOR_SIFT}; // Just OpenCV SIFT

    // Create a progress bar
//    tqdm bar;
//    bar.set_theme_braille();
//    bar.set_total(totalIterations);

    // Iterate over all combinations of options, including descriptor types
    for (auto &descriptorType: descriptorTypes) {
        for (auto &pooling: poolingStrategies) {
            for (auto &normalization: normalizationStages) {
                for (auto &rooting: rootingStages) {
                    for (auto &normType: normTypes) {

                        std::string experimentPath = resultsPath;
                        // Configure the descriptor options
                        DescriptorOptions options;
                        options.poolingStrategy = pooling;
                        options.scales = {1.0f,1.5f,2.0f}; // Example scales, modify as needed
                        options.normType = normType;
                        options.normalizationStage = normalization;
                        options.rootingStage = rooting;
                        options.descriptorType2 = DESCRIPTOR_SIFT;
                        // Set to grayscale for standard SIFT
                        options.imageType = BW;
                        // Standard SIFT uses grayscale
                        options.descriptorColorSpace = D_BW;

                        // Create experiment configuration with descriptor options and type
                        experiment_config config(options);

                        // Create a descriptive experiment name
                        std::string descriptorName = descriptorTypeToString(descriptorType) + "-" +
                                                     imageTypeToString(options.imageType) + "-" +
                                                     poolingStrategyToString(pooling) + "-" +
                                                     normalizationStageToString(normalization) + "-" +
                                                     rootingStageToString(rooting) + "-" +
                                                     normTypeToString(normType); // + "-Baseline";

                        // append the descriptor name to the results path
                        experimentPath += descriptorName;
                        
#ifdef BUILD_DATABASE
                        // Record experiment configuration
                        thesis_project::database::ExperimentConfig dbConfig;
                        dbConfig.descriptor_type = descriptorName;
                        dbConfig.dataset_path = dataPath;
                        dbConfig.pooling_strategy = poolingStrategyToString(pooling);
                        dbConfig.similarity_threshold = 0.7; // Default threshold
                        dbConfig.max_features = 1000; // Default max features
                        dbConfig.parameters["normalization"] = normalizationStageToString(normalization);
                        dbConfig.parameters["rooting"] = rootingStageToString(rooting);
                        dbConfig.parameters["norm_type"] = normTypeToString(normType);
                        dbConfig.parameters["image_type"] = imageTypeToString(options.imageType);
                        
                        int experiment_id = db.recordConfiguration(dbConfig);
                        
                        // Set experiment ID in config for descriptor storage
                        config.experiment_id = experiment_id;
#endif
                        
                        // Run the descriptor extraction process and get real metrics
                        ExperimentMetrics experiment_metrics = image_processor::process_directory(dataPath, experimentPath, config);
                        success = experiment_metrics.success;
                        
#ifdef BUILD_DATABASE
                        if (experiment_id != -1) {
                            // Record experiment results with real computed metrics
                            thesis_project::database::ExperimentResults results;
                            results.experiment_id = experiment_id;
                            results.descriptor_type = descriptorName;
                            results.dataset_name = dataPath;
                            results.processing_time_ms = experiment_metrics.processing_time_ms;
                            results.mean_average_precision = experiment_metrics.mean_average_precision;
                            results.precision_at_1 = experiment_metrics.precision_at_1;
                            results.precision_at_5 = experiment_metrics.precision_at_5;
                            results.recall_at_1 = experiment_metrics.recall_at_1;
                            results.recall_at_5 = experiment_metrics.recall_at_5;
                            results.total_matches = experiment_metrics.total_matches;
                            results.total_keypoints = experiment_metrics.total_keypoints;
                            results.metadata["success"] = experiment_metrics.success ? "true" : "false";
                            results.metadata["total_images"] = std::to_string(experiment_metrics.total_images_processed);
                            
                            // Add per-scene precision to metadata
                            for (const auto& [scene, precision] : experiment_metrics.per_scene_precision) {
                                results.metadata[scene + "_precision"] = std::to_string(precision);
                                results.metadata[scene + "_matches"] = std::to_string(experiment_metrics.per_scene_matches.at(scene));
                                results.metadata[scene + "_keypoints"] = std::to_string(experiment_metrics.per_scene_keypoints.at(scene));
                            }
                            
                            if (!experiment_metrics.error_message.empty()) {
                                results.metadata["error"] = experiment_metrics.error_message;
                            }
                            
                            return db.recordExperiment(results);
                        }
#endif

//                        // Update the progress bar
//                        bar.update();
                    }
                }
            }
        }
    }
    std::cout << "Hostname: " << getenv("HOSTNAME") << std::endl;
    std::cout << "PWD: " << getenv("PWD") << std::endl;
    return success ? 0 : 1;
}

// int main() {
//     std::string dataFolderPath = DATA_PATH;
//     std::string keypointFolderPath = KEYPOINTS_PATH;
//
//     std::cout << "=== Locked-in Keypoints Visualization ===" << std::endl;
//     std::cout << "Data path: " << dataFolderPath << std::endl;
//     std::cout << "Keypoints path: " << keypointFolderPath << std::endl;
//
//     try {
//         // Check if keypoints already exist
//         if (!fs::exists(keypointFolderPath) || fs::is_empty(fs::path(keypointFolderPath))) {
//             std::cout << "\nGenerating locked-in keypoints..." << std::endl;
//             LockedInKeypoints::generateLockedInKeypoints(dataFolderPath, keypointFolderPath);
//         } else {
//             std::cout << "\nUsing existing keypoints from: " << keypointFolderPath << std::endl;
//         }
//
//         // Save visualizations instead of displaying
//         std::cout << "\nCreating keypoint visualizations..." << std::endl;
//         LockedInKeypoints::saveLockedInKeypointsBorder(dataFolderPath);  // Changed from displayLockedInKeypointsBorder
//
//         std::cout << "\n=== Visualization Complete ===" << std::endl;
//
//     } catch (const std::exception& e) {
//         std::cerr << "Error: " << e.what() << std::endl;
//         return 1;
//     }
//
//     return 0;
// }


// Old Main for running simple runs

/*int main() {

    // Data path defined in the CMakeLists.txt for portable paths
    std::string dataPath = DATA_PATH;
    std::string resultsPath = RESULTS_PATH;

    experiment_config config;
    config.setDescriptorType(DESCRIPTOR_RGBSIFT);
    config.setDescriptorColorSpace(D_COLOR);


    bool success = image_processor::process_directory(dataPath, resultsPath, config);

    return success ? 0 : 1;
}*/

#pragma once

#include "thesis_project/types.hpp"
#include <string>
#include <vector>
#include <memory>

namespace thesis_project {
namespace config {

    /**
     * @brief Modern experiment configuration using Stage 2 types
     *
     * This replaces the old experiment_config class with a modern,
     * YAML-configurable system using our type-safe enums.
     */
    struct ExperimentConfig {
        // Experiment metadata
        struct Experiment {
            std::string name;
            std::string description;
            std::string version = "1.0";
            std::string author;
        } experiment;

        // Dataset configuration
        struct Dataset {
            std::string type = "hpatches";
            std::string path = "data/hpatches/";
            std::vector<std::string> scenes;  // Empty = all scenes
        } dataset;

        // Keypoint detection configuration
        struct Keypoints {
            KeypointGenerator generator = KeypointGenerator::SIFT;
            KeypointParams params;
        } keypoints;

        // Descriptor configurations (can have multiple for comparison)
        struct DescriptorConfig {
            std::string name;
            DescriptorType type;
            DescriptorParams params;
        };
        std::vector<DescriptorConfig> descriptors;

        // Evaluation configuration
        struct Evaluation {
            EvaluationParams params;
        } evaluation;

        // Output configuration
        struct Output {
            std::string results_path = "results/";
            bool save_keypoints = false;
            bool save_descriptors = false;
            bool save_matches = false;
            bool save_visualizations = true;
        } output;

        // Database configuration
        DatabaseParams database;
    };

} // namespace config
} // namespace thesis_project

#include "../src/core/config/YAMLConfigLoader.hpp"
#include "../src/core/config/ConfigurationBridge.hpp"
#include "../descriptor_compare/image_processor.hpp"
#include "thesis_project/logging.hpp"
#include "thesis_project/types.hpp"
#ifdef BUILD_DATABASE
#include "thesis_project/database/DatabaseManager.hpp"
#endif
#include <iostream>
#include <filesystem>
#include <chrono>

using namespace thesis_project;

/**
 * @brief New experiment runner using YAML configuration
 *
 * This CLI tool demonstrates the new configuration system
 * while using existing image processing code.
 */
int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <config.yaml>" << std::endl;
        std::cout << "Example: " << argv[0] << " config/experiments/sift_baseline.yaml" << std::endl;
        return 1;
    }

    std::string config_path = argv[1];

    try {
        // Load YAML configuration
        LOG_INFO("Loading experiment configuration from: " + config_path);
        auto yaml_config = config::YAMLConfigLoader::loadFromFile(config_path);

#ifdef BUILD_DATABASE
        // Initialize database for experiment tracking
        thesis_project::database::DatabaseManager db("experiments.db", true);
        if (db.isEnabled()) {
            LOG_INFO("Database tracking enabled");
        } else {
            LOG_INFO("Database tracking disabled");
        }
#endif

        LOG_INFO("Experiment: " + yaml_config.experiment.name);
        LOG_INFO("Description: " + yaml_config.experiment.description);
        LOG_INFO("Dataset: " + yaml_config.dataset.path);
        LOG_INFO("Descriptors: " + std::to_string(yaml_config.descriptors.size()));

        // Create results directory
        std::string results_base = yaml_config.output.results_path + yaml_config.experiment.name;
        std::filesystem::create_directories(results_base);

        // Run experiment for each descriptor configuration
        for (size_t i = 0; i < yaml_config.descriptors.size(); ++i) {
            const auto& desc_config = yaml_config.descriptors[i];

            LOG_INFO("Running experiment with descriptor: " + desc_config.name);

            // Convert to old configuration format for existing image processor
            auto old_config = config::ConfigurationBridge::createOldConfigForDescriptor(yaml_config, i);

            // Create descriptor-specific results directory
            std::string results_path = results_base + "/" + desc_config.name;
            std::filesystem::create_directories(results_path);

#ifdef BUILD_DATABASE
            // Record experiment configuration
            thesis_project::database::ExperimentConfig dbConfig;
            dbConfig.descriptor_type = desc_config.name;
            dbConfig.dataset_path = yaml_config.dataset.path;
            dbConfig.pooling_strategy = toString(desc_config.params.pooling);
            dbConfig.similarity_threshold = yaml_config.evaluation.params.match_threshold;
            dbConfig.max_features = yaml_config.keypoints.params.max_features;
            dbConfig.parameters["experiment_name"] = yaml_config.experiment.name;
            dbConfig.parameters["descriptor_type"] = toString(desc_config.type);
            dbConfig.parameters["pooling_strategy"] = toString(desc_config.params.pooling);
            dbConfig.parameters["norm_type"] = std::to_string(desc_config.params.norm_type);
            
            auto start_time = std::chrono::high_resolution_clock::now();
            int experiment_id = db.recordConfiguration(dbConfig);
#endif

            // Run existing image processing pipeline
            auto experiment_metrics = image_processor::process_directory(
                yaml_config.dataset.path,
                results_path,
                old_config
            );
            
#ifdef BUILD_DATABASE
            if (experiment_id != -1) {
                auto end_time = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
                
                // Record experiment results
                thesis_project::database::ExperimentResults results;
                results.experiment_id = experiment_id;
                results.descriptor_type = desc_config.name;
                results.dataset_name = yaml_config.dataset.path;
                results.processing_time_ms = duration.count();
                results.mean_average_precision = experiment_metrics.mean_average_precision;
                results.precision_at_1 = experiment_metrics.mean_precision;
                results.precision_at_5 = experiment_metrics.mean_precision;
                results.recall_at_1 = 0.0;
                results.recall_at_5 = 0.0;
                results.total_matches = experiment_metrics.total_matches;
                results.total_keypoints = experiment_metrics.total_keypoints;
                results.metadata["success"] = experiment_metrics.success ? "true" : "false";
                results.metadata["experiment_name"] = yaml_config.experiment.name;
                
                return db.recordExperiment(results);
            }
#endif

            if (experiment_metrics.success) {
                LOG_INFO("✅ Completed descriptor: " + desc_config.name);
            } else {
                LOG_ERROR("❌ Failed descriptor: " + desc_config.name);
            }
        }

        LOG_INFO("🎉 Experiment completed: " + yaml_config.experiment.name);
        LOG_INFO("Results saved to: " + results_base);

        return 0;

    } catch (const std::exception& e) {
        LOG_ERROR("Experiment failed: " + std::string(e.what()));
        return 1;
    }
}

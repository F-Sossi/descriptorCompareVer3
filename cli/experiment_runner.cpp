#include "../src/core/config/YAMLConfigLoader.hpp"
#include "../src/core/config/ConfigurationBridge.hpp"
#include "../descriptor_compare/image_processor.hpp"
#include "thesis_project/logging.hpp"
#include <iostream>
#include <filesystem>

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

            // Run existing image processing pipeline
            bool success = image_processor::process_directory(
                yaml_config.dataset.path,
                results_path,
                old_config
            );

            if (success) {
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

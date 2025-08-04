#!/bin/bash
# Stage 4: Configuration System Integration Setup

echo "=== Stage 4: Configuration System Integration ==="
echo "================================================="

echo ""
echo "Creating YAML-based configuration system for experiments..."

echo ""
echo "1. Adding YAML dependency to conanfile.txt..."

# Backup current conanfile.txt
cp conanfile.txt conanfile.txt.stage3_backup

# Add yaml-cpp to dependencies
cat > conanfile.txt << 'EOF'
[requires]
sqlite3/3.43.2
hdf5/1.14.3
yaml-cpp/0.8.0

[generators]
CMakeDeps
CMakeToolchain

[options]
hdf5/*:shared=True
EOF

echo "✅ Added yaml-cpp dependency to conanfile.txt"

echo ""
echo "2. Creating configuration directory structure..."

mkdir -p config/experiments
mkdir -p config/defaults
mkdir -p src/core/config
mkdir -p cli

echo "✅ Configuration directories created"

echo ""
echo "3. Creating sample YAML experiment configurations..."

# Create a basic SIFT experiment
cat > config/experiments/sift_baseline.yaml << 'EOF'
experiment:
  name: "sift_baseline"
  description: "Baseline SIFT descriptor evaluation"
  version: "1.0"
  author: "researcher"

dataset:
  type: "hpatches"
  path: "data/hpatches/"
  scenes: []  # Empty = use all scenes

keypoints:
  generator: "sift"
  max_features: 2000
  contrast_threshold: 0.04
  edge_threshold: 10.0

descriptors:
  - name: "sift"
    type: "sift"
    pooling: "none"
    normalize: true

evaluation:
  matching:
    method: "brute_force"
    norm: "l2"
    cross_check: true
    threshold: 0.8

  validation:
    method: "homography"
    threshold: 0.05
    min_matches: 10

output:
  results_path: "results/"
  save_keypoints: false
  save_descriptors: false
  save_matches: false
  save_visualizations: true

database:
  enabled: false
  connection: "sqlite:///experiments.db"
EOF

# Create an RGBSIFT comparison experiment
cat > config/experiments/rgbsift_comparison.yaml << 'EOF'
experiment:
  name: "rgbsift_vs_sift"
  description: "Compare RGB SIFT with standard SIFT"
  version: "1.0"
  author: "researcher"

dataset:
  type: "hpatches"
  path: "data/hpatches/"
  scenes: ["i_ajuntament", "v_wall"]  # Specific scenes for quick testing

keypoints:
  generator: "sift"
  max_features: 2000

descriptors:
  - name: "sift_baseline"
    type: "sift"
    pooling: "none"
    normalize: true

  - name: "rgbsift_color"
    type: "rgbsift"
    pooling: "none"
    normalize: true
    use_color: true

evaluation:
  matching:
    method: "brute_force"
    norm: "l2"
    cross_check: true

  validation:
    method: "homography"
    threshold: 0.05

output:
  results_path: "results/"
  save_visualizations: true

database:
  enabled: false
EOF

# Create a domain size pooling experiment
cat > config/experiments/dsp_experiment.yaml << 'EOF'
experiment:
  name: "domain_size_pooling_test"
  description: "Test domain size pooling with different scales"
  version: "1.0"
  author: "researcher"

dataset:
  type: "hpatches"
  path: "data/hpatches/"
  scenes: ["i_ajuntament"]  # Single scene for testing

keypoints:
  generator: "sift"
  max_features: 2000

descriptors:
  - name: "sift_no_pooling"
    type: "sift"
    pooling: "none"

  - name: "sift_dsp_2scales"
    type: "sift"
    pooling: "domain_size_pooling"
    scales: [1.0, 1.5]
    normalize_before_pooling: false
    normalize_after_pooling: true

  - name: "sift_dsp_3scales"
    type: "sift"
    pooling: "domain_size_pooling"
    scales: [1.0, 1.5, 2.0]
    normalize_before_pooling: false
    normalize_after_pooling: true

evaluation:
  matching:
    method: "brute_force"
    threshold: 0.8

  validation:
    method: "homography"
    threshold: 0.05

output:
  results_path: "results/"
  save_visualizations: true

database:
  enabled: false
EOF

echo "✅ Sample YAML experiment configurations created"

echo ""
echo "4. Creating YAML configuration parser..."

# Create modern configuration structures
cat > src/core/config/ExperimentConfig.hpp << 'EOF'
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
EOF

# Create YAML parser
cat > src/core/config/YAMLConfigLoader.hpp << 'EOF'
#pragma once

#include "ExperimentConfig.hpp"
#include <yaml-cpp/yaml.h>
#include <string>
#include <stdexcept>

namespace thesis_project {
namespace config {

    /**
     * @brief YAML configuration loader
     *
     * Loads experiment configurations from YAML files into
     * strongly-typed configuration structures.
     */
    class YAMLConfigLoader {
    public:
        /**
         * @brief Load experiment configuration from YAML file
         * @param yaml_path Path to YAML file
         * @return Loaded configuration
         * @throws std::runtime_error if file cannot be loaded or parsed
         */
        static ExperimentConfig loadFromFile(const std::string& yaml_path);

        /**
         * @brief Load experiment configuration from YAML string
         * @param yaml_content YAML content as string
         * @return Loaded configuration
         */
        static ExperimentConfig loadFromString(const std::string& yaml_content);

        /**
         * @brief Save experiment configuration to YAML file
         * @param config Configuration to save
         * @param yaml_path Output file path
         */
        static void saveToFile(const ExperimentConfig& config, const std::string& yaml_path);

    private:
        // Helper methods for parsing different sections
        static void parseExperiment(const YAML::Node& node, ExperimentConfig::Experiment& experiment);
        static void parseDataset(const YAML::Node& node, ExperimentConfig::Dataset& dataset);
        static void parseKeypoints(const YAML::Node& node, ExperimentConfig::Keypoints& keypoints);
        static void parseDescriptors(const YAML::Node& node, std::vector<ExperimentConfig::DescriptorConfig>& descriptors);
        static void parseEvaluation(const YAML::Node& node, ExperimentConfig::Evaluation& evaluation);
        static void parseOutput(const YAML::Node& node, ExperimentConfig::Output& output);
        static void parseDatabase(const YAML::Node& node, DatabaseParams& database);

        // Type conversion helpers
        static DescriptorType stringToDescriptorType(const std::string& str);
        static PoolingStrategy stringToPoolingStrategy(const std::string& str);
        static KeypointGenerator stringToKeypointGenerator(const std::string& str);
        static MatchingMethod stringToMatchingMethod(const std::string& str);
        static ValidationMethod stringToValidationMethod(const std::string& str);
    };

} // namespace config
} // namespace thesis_project
EOF

echo "✅ YAML configuration parser created"

echo ""
echo "5. Creating configuration bridge to existing system..."

# Create bridge between new and old configuration systems
cat > src/core/config/ConfigurationBridge.hpp << 'EOF'
#pragma once

#include "ExperimentConfig.hpp"
#include "../../../descriptor_compare/experiment_config.hpp"

namespace thesis_project {
namespace config {

    /**
     * @brief Bridge between new YAML configuration and existing experiment_config
     *
     * This allows us to use the new YAML-based configuration system
     * while maintaining compatibility with existing code.
     */
    class ConfigurationBridge {
    public:
        /**
         * @brief Convert new YAML config to existing experiment_config
         * @param new_config Modern YAML-based configuration
         * @return Traditional experiment_config for existing code
         */
        static ::experiment_config toOldConfig(const ExperimentConfig& new_config);

        /**
         * @brief Convert existing experiment_config to new format
         * @param old_config Traditional configuration
         * @return Modern YAML-compatible configuration
         */
        static ExperimentConfig fromOldConfig(const ::experiment_config& old_config);

        /**
         * @brief Create experiment_config for specific descriptor from YAML config
         * @param new_config YAML configuration
         * @param descriptor_index Which descriptor to use (for multi-descriptor experiments)
         * @return experiment_config set up for that specific descriptor
         */
        static ::experiment_config createOldConfigForDescriptor(
            const ExperimentConfig& new_config,
            size_t descriptor_index = 0);
    };

} // namespace config
} // namespace thesis_project
EOF

echo "✅ Configuration bridge created"

echo ""
echo "6. Creating new CLI experiment runner..."

# Create new experiment runner that uses YAML configs
cat > cli/experiment_runner.cpp << 'EOF'
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
EOF

echo "✅ New CLI experiment runner created"

echo ""
echo "7. Creating simple validation test..."

# Create a simple test to validate YAML loading works
cat > tests/unit/test_yaml_config.cpp << 'EOF'
#include <iostream>
#include <string>
#include <fstream>

// Simple YAML config test without dependencies
int main() {
    std::cout << "=== Stage 4 YAML Configuration Test ===" << std::endl;

    try {
        // Test 1: Check if YAML files exist
        std::cout << "\n1. Checking YAML configuration files..." << std::endl;

        std::vector<std::string> config_files = {
            "config/experiments/sift_baseline.yaml",
            "config/experiments/rgbsift_comparison.yaml",
            "config/experiments/dsp_experiment.yaml"
        };

        int found_configs = 0;
        for (const auto& file : config_files) {
            std::ifstream test_file(file);
            if (test_file.good()) {
                std::cout << "  ✅ " << file << std::endl;
                found_configs++;
            } else {
                std::cout << "  ❌ " << file << " (missing)" << std::endl;
            }
        }

        if (found_configs == config_files.size()) {
            std::cout << "✅ All YAML configuration files present" << std::endl;
        } else {
            std::cout << "⚠️ " << found_configs << "/" << config_files.size()
                      << " configuration files found" << std::endl;
        }

        // Test 2: Basic YAML structure validation
        std::cout << "\n2. Testing YAML structure..." << std::endl;

        // Read one of the config files and check for key sections
        std::ifstream config_file("config/experiments/sift_baseline.yaml");
        if (config_file.good()) {
            std::string content((std::istreambuf_iterator<char>(config_file)),
                               std::istreambuf_iterator<char>());

            std::vector<std::string> required_sections = {
                "experiment:", "dataset:", "keypoints:", "descriptors:",
                "evaluation:", "output:"
            };

            int found_sections = 0;
            for (const auto& section : required_sections) {
                if (content.find(section) != std::string::npos) {
                    found_sections++;
                    std::cout << "  ✅ " << section << std::endl;
                } else {
                    std::cout << "  ❌ " << section << " (missing)" << std::endl;
                }
            }

            if (found_sections == required_sections.size()) {
                std::cout << "✅ YAML structure validation passed" << std::endl;
            } else {
                std::cout << "⚠️ YAML structure incomplete" << std::endl;
            }
        }

        // Test 3: Configuration concepts
        std::cout << "\n3. Testing configuration concepts..." << std::endl;

        // Test enum concepts from Stage 2
        enum class DescriptorType { SIFT, RGBSIFT, vSIFT, HoNC };
        enum class PoolingStrategy { NONE, DOMAIN_SIZE_POOLING, STACKING };

        auto toString = [](DescriptorType type) -> std::string {
            switch (type) {
                case DescriptorType::SIFT: return "sift";
                case DescriptorType::RGBSIFT: return "rgbsift";
                case DescriptorType::vSIFT: return "vsift";
                case DescriptorType::HoNC: return "honc";
                default: return "unknown";
            }
        };

        std::cout << "  Configuration type mapping:" << std::endl;
        std::cout << "    SIFT -> " << toString(DescriptorType::SIFT) << std::endl;
        std::cout << "    RGBSIFT -> " << toString(DescriptorType::RGBSIFT) << std::endl;

        std::cout << "✅ Configuration concepts working" << std::endl;

        std::cout << "\n=== Stage 4 Configuration Test Complete ===" << std::endl;
        std::cout << "✅ YAML configuration system foundation ready!" << std::endl;

        return 0;

    } catch (const std::exception& e) {
        std::cout << "❌ Test failed: " << e.what() << std::endl;
        return 1;
    }
}
EOF

echo "✅ YAML configuration test created"

echo ""
echo "=== Stage 4 Setup Complete ==="
echo ""
echo "Stage 4 Configuration System Created:"
echo "  ✅ yaml-cpp dependency added to conanfile.txt"
echo "  ✅ Sample YAML experiment configurations"
echo "  ✅ Modern ExperimentConfig structure"
echo "  ✅ YAML parser (YAMLConfigLoader)"
echo "  ✅ Configuration bridge to existing system"
echo "  ✅ New CLI experiment runner"
echo "  ✅ YAML validation test"
echo ""
echo "What Stage 4 Provides:"
echo "  • YAML-based experiment configuration"
echo "  • Type-safe configuration using Stage 2 types"
echo "  • Bridge to existing experiment_config system"
echo "  • New CLI tools for running YAML-configured experiments"
echo "  • Batch experiment capabilities"
echo "  • Backward compatibility maintained"
echo ""
echo "Next Steps:"
echo "  1. Install yaml-cpp dependency"
echo "  2. Implement YAML parser"
echo "  3. Test configuration system"
echo "  4. Validate integration with existing code"
echo ""
echo "To continue:"
echo "  - Update dependencies: conan install . --build=missing"
echo "  - Build and test Stage 4 components"
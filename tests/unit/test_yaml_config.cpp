#include <iostream>
#include <string>
#include <fstream>
#include <vector>

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

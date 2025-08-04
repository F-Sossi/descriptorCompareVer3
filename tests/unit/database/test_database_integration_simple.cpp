#include "thesis_project/database/DatabaseManager.hpp"
#include <string>
#include <iostream>
#include <filesystem>

// Simple mock of experiment_config for testing (no linking needed)
enum DescriptorType { DESCRIPTOR_SIFT, DESCRIPTOR_RGBSIFT, DESCRIPTOR_HoNC, DESCRIPTOR_vSIFT };
enum PoolingStrategy { NONE, STACKING, DOMAIN_SIZE_POOLING };
enum ImageType { COLOR, BW };
enum DescriptorColorSpace { D_COLOR, D_BW };

struct DescriptorOptions {
    DescriptorType descriptorType = DESCRIPTOR_SIFT;
    PoolingStrategy poolingStrategy = NONE;
    int normType = 2;
    ImageType imageType = COLOR;
    DescriptorColorSpace descriptorColorSpace = D_COLOR;
};

struct MockExperimentConfig {
    DescriptorOptions descriptorOptions;
    bool useMultiThreading = true;
    double matchThreshold = 0.05;
};

// Simple integration functions (no external dependencies)
namespace simple_integration {

    inline thesis_project::database::ExperimentConfig toDbConfig(const MockExperimentConfig& config) {
        thesis_project::database::ExperimentConfig db_config;

        // Convert descriptor type
        if (config.descriptorOptions.descriptorType == DESCRIPTOR_SIFT) {
            db_config.descriptor_type = "SIFT";
        } else if (config.descriptorOptions.descriptorType == DESCRIPTOR_RGBSIFT) {
            db_config.descriptor_type = "RGBSIFT";
        } else if (config.descriptorOptions.descriptorType == DESCRIPTOR_HoNC) {
            db_config.descriptor_type = "HoNC";
        } else if (config.descriptorOptions.descriptorType == DESCRIPTOR_vSIFT) {
            db_config.descriptor_type = "vSIFT";
        } else {
            db_config.descriptor_type = "UNKNOWN";
        }

        // Convert pooling strategy
        if (config.descriptorOptions.poolingStrategy == STACKING) {
            db_config.pooling_strategy = "STACKING";
        } else if (config.descriptorOptions.poolingStrategy == DOMAIN_SIZE_POOLING) {
            db_config.pooling_strategy = "DOMAIN_SIZE_POOLING";
        } else {
            db_config.pooling_strategy = "NONE";
        }

        db_config.max_features = 1000;
        db_config.similarity_threshold = config.matchThreshold;

        // Add parameters
        db_config.parameters["normType"] = std::to_string(config.descriptorOptions.normType);
        db_config.parameters["useMultiThreading"] = config.useMultiThreading ? "true" : "false";
        db_config.parameters["imageType"] = (config.descriptorOptions.imageType == COLOR) ? "COLOR" : "BW";
        db_config.parameters["descriptorColorSpace"] = (config.descriptorOptions.descriptorColorSpace == D_COLOR) ? "D_COLOR" : "D_BW";

        return db_config;
    }
}

int main() {
    std::cout << "=== Stage 5 Database Integration Test (Simple) ===" << std::endl;

    try {
        // Test database integration with mock config
        std::cout << "\n1. Testing mock config integration..." << std::endl;

        MockExperimentConfig config;
        config.descriptorOptions.descriptorType = DESCRIPTOR_RGBSIFT;
        config.descriptorOptions.poolingStrategy = STACKING;
        config.descriptorOptions.normType = 2;
        config.descriptorOptions.imageType = COLOR;
        config.descriptorOptions.descriptorColorSpace = D_COLOR;
        config.useMultiThreading = true;
        config.matchThreshold = 0.05;

        // Convert to database format
        auto db_config = simple_integration::toDbConfig(config);

        std::cout << "✅ Configuration conversion successful:" << std::endl;
        std::cout << "   Descriptor type: " << db_config.descriptor_type << std::endl;
        std::cout << "   Pooling strategy: " << db_config.pooling_strategy << std::endl;
        std::cout << "   Max features: " << db_config.max_features << std::endl;
        std::cout << "   Similarity threshold: " << db_config.similarity_threshold << std::endl;

        // Test database with converted config
        std::string test_db = "test_integration_simple.db";
        if (std::filesystem::exists(test_db)) {
            std::filesystem::remove(test_db);
        }

        thesis_project::database::DatabaseManager db(test_db, true);

        if (!db.isEnabled()) {
            std::cout << "❌ Database not enabled" << std::endl;
            return 1;
        }

        // Record the converted configuration
        int exp_id = db.recordConfiguration(db_config);
        if (exp_id <= 0) {
            std::cout << "❌ Failed to record configuration" << std::endl;
            return 1;
        }

        std::cout << "✅ Configuration recorded with ID: " << exp_id << std::endl;

        // Test results creation
        thesis_project::database::ExperimentResults results;
        results.experiment_id = exp_id;
        results.descriptor_type = "RGBSIFT";
        results.dataset_name = "i_ajuntament";
        results.mean_average_precision = 0.87;
        results.processing_time_ms = 245.3;

        if (db.recordExperiment(results)) {
            std::cout << "✅ Results recorded successfully" << std::endl;
        } else {
            std::cout << "❌ Failed to record results" << std::endl;
            return 1;
        }

        // Test detailed results
        thesis_project::database::ExperimentResults detailed_results;
        detailed_results.experiment_id = exp_id;
        detailed_results.descriptor_type = "RGBSIFT";
        detailed_results.dataset_name = "i_ajuntament";
        detailed_results.mean_average_precision = 0.87;
        detailed_results.precision_at_1 = 0.92;
        detailed_results.precision_at_5 = 0.88;
        detailed_results.recall_at_1 = 0.85;
        detailed_results.recall_at_5 = 0.83;
        detailed_results.total_matches = 150;
        detailed_results.total_keypoints = 1000;
        detailed_results.processing_time_ms = 245.3;

        if (db.recordExperiment(detailed_results)) {
            std::cout << "✅ Detailed results recorded successfully" << std::endl;
        } else {
            std::cout << "❌ Failed to record detailed results" << std::endl;
            return 1;
        }

        // Test retrieval
        std::cout << "\n2. Testing data retrieval..." << std::endl;
        auto recent_results = db.getRecentResults(5);

        if (!recent_results.empty()) {
            std::cout << "✅ Retrieved " << recent_results.size() << " results" << std::endl;
            for (const auto& result : recent_results) {
                std::cout << "   MAP: " << result.mean_average_precision
                          << ", Descriptor: " << result.descriptor_type << std::endl;
            }
        } else {
            std::cout << "❌ No results retrieved" << std::endl;
            return 1;
        }

        // Test statistics
        auto stats = db.getStatistics();
        if (!stats.empty()) {
            std::cout << "✅ Statistics retrieved:" << std::endl;
            for (const auto& [key, value] : stats) {
                std::cout << "   " << key << ": " << value << std::endl;
            }
        }

        // Cleanup
        if (std::filesystem::exists(test_db)) {
            std::filesystem::remove(test_db);
        }

        std::cout << "\n✅ Simple integration test completed successfully!" << std::endl;
        return 0;

    } catch (const std::exception& e) {
        std::cout << "❌ Simple integration test failed: " << e.what() << std::endl;
        return 1;
    }
}

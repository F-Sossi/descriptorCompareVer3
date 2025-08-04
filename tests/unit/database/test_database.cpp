#include "thesis_project/database/DatabaseManager.hpp"
#include <string>
#include <iostream>
#include <filesystem>

int main() {
    std::cout << "=== Stage 5 Database Test ===" << std::endl;

    try {
        // Test 1: Disabled database
        std::cout << "\n1. Testing disabled database..." << std::endl;
        thesis_project::database::DatabaseManager db_disabled("", false);

        if (!db_disabled.isEnabled()) {
            std::cout << "✅ Disabled database works correctly" << std::endl;
        } else {
            std::cout << "❌ Disabled database should not be enabled" << std::endl;
            return 1;
        }

        // Test 2: Enabled database
        std::cout << "\n2. Testing enabled database..." << std::endl;
        std::string test_db = "test_stage5.db";

        // Remove existing test database
        if (std::filesystem::exists(test_db)) {
            std::filesystem::remove(test_db);
        }

        thesis_project::database::DatabaseManager db_enabled(test_db, true);

        if (db_enabled.isEnabled()) {
            std::cout << "✅ Enabled database initialized successfully" << std::endl;
        } else {
            std::cout << "❌ Failed to initialize database" << std::endl;
            return 1;
        }

        // Test 3: Record configuration
        std::cout << "\n3. Testing configuration recording..." << std::endl;
        thesis_project::database::ExperimentConfig config;
        config.descriptor_type = "SIFT";
        config.dataset_path = "/test/data";
        config.pooling_strategy = "NONE";
        config.max_features = 1000;
        config.similarity_threshold = 0.7;

        int exp_id = db_enabled.recordConfiguration(config);
        if (exp_id > 0) {
            std::cout << "✅ Configuration recorded with ID: " << exp_id << std::endl;
        } else {
            std::cout << "❌ Failed to record configuration" << std::endl;
            return 1;
        }

        // Test 4: Record results
        std::cout << "\n4. Testing results recording..." << std::endl;
        thesis_project::database::ExperimentResults results;
        results.experiment_id = exp_id;
        results.descriptor_type = "SIFT";
        results.dataset_name = "test_dataset";
        results.mean_average_precision = 0.85;
        results.precision_at_1 = 0.9;
        results.total_matches = 150;
        results.total_keypoints = 1000;
        results.processing_time_ms = 250.5;

        if (db_enabled.recordExperiment(results)) {
            std::cout << "✅ Results recorded successfully" << std::endl;
        } else {
            std::cout << "❌ Failed to record results" << std::endl;
            return 1;
        }

        // Test 5: Retrieve results
        std::cout << "\n5. Testing results retrieval..." << std::endl;
        auto recent_results = db_enabled.getRecentResults(5);

        if (!recent_results.empty()) {
            std::cout << "✅ Retrieved " << recent_results.size() << " results" << std::endl;
            std::cout << "   Latest MAP: " << recent_results[0].mean_average_precision << std::endl;
        } else {
            std::cout << "❌ No results retrieved" << std::endl;
            return 1;
        }

        // Test 6: Statistics
        std::cout << "\n6. Testing statistics..." << std::endl;
        auto stats = db_enabled.getStatistics();

        if (!stats.empty()) {
            std::cout << "✅ Statistics retrieved:" << std::endl;
            for (const auto& [key, value] : stats) {
                std::cout << "   " << key << ": " << value << std::endl;
            }
        } else {
            std::cout << "❌ No statistics retrieved" << std::endl;
            return 1;
        }

        // Cleanup
        if (std::filesystem::exists(test_db)) {
            std::filesystem::remove(test_db);
            std::cout << "\n✅ Test database cleaned up" << std::endl;
        }

        std::cout << "\n=== Stage 5 Database Test Complete ===" << std::endl;
        std::cout << "✅ All database functionality working!" << std::endl;

        return 0;

    } catch (const std::exception& e) {
        std::cout << "❌ Test failed: " << e.what() << std::endl;
        return 1;
    }
}

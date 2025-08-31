#include "src/core/config/YAMLConfigLoader.hpp"
#include "src/core/config/ConfigurationBridge.hpp"
#include "descriptor_compare/image_processor.hpp"
#include "src/core/integration/ProcessorBridgeFacade.hpp"
#include "src/core/integration/MigrationToggle.hpp"
#include "thesis_project/logging.hpp"
#include "thesis_project/types.hpp"
#ifdef BUILD_DATABASE
#include "thesis_project/database/DatabaseManager.hpp"
#endif
#include <iostream>
#include <filesystem>
#include <numeric>
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

        // Results directory creation removed - using database storage only
        std::string results_base = yaml_config.output.results_path + yaml_config.experiment.name;

        // Set migration toggle globally for optional Stage 7 routing
        thesis_project::integration::MigrationToggle::setEnabled(yaml_config.migration.use_new_interface);

        // Run experiment for each descriptor configuration
        for (size_t i = 0; i < yaml_config.descriptors.size(); ++i) {
            const auto& desc_config = yaml_config.descriptors[i];

            LOG_INFO("Running experiment with descriptor: " + desc_config.name);

            // Convert to old configuration format for existing image processor
            auto old_config = config::ConfigurationBridge::createOldConfigForDescriptor(yaml_config, i);
            
            // Refresh detectors after configuration bridge updates
            old_config.refreshDetectors();

            // Descriptor-specific directory creation removed - using database storage only
            std::string results_path = results_base + "/" + desc_config.name;

            // Optional Stage 7 migration smoke test (does not alter main pipeline)
            if (yaml_config.migration.use_new_interface) {
                try {
                    bool supported = thesis_project::integration::isNewInterfaceSupported(old_config);
                    if (supported) {
                        LOG_INFO("[Migration] New interface supported for descriptor: " + desc_config.name);
                        cv::Mat test_image = cv::Mat::zeros(100, 100, CV_8UC3);
                        auto result = thesis_project::integration::smokeDetectAndCompute(test_image, old_config);
                        LOG_INFO("[Migration] Smoke test: " + std::to_string(result.first.size()) + " keypoints, " + std::to_string(result.second.rows) + " descriptors");
                    } else {
                        LOG_INFO("[Migration] New interface not supported for descriptor: " + desc_config.name + ", using legacy path.");
                    }
                } catch (const std::exception& e) {
                    LOG_INFO(std::string("[Migration] New interface smoke test failed: ") + e.what());
                }
            }

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
                results.mean_average_precision = experiment_metrics.legacy_macro_precision_by_scene;
                results.precision_at_1 = experiment_metrics.mean_precision;
                results.precision_at_5 = experiment_metrics.mean_precision;
                results.recall_at_1 = 0.0;
                results.recall_at_5 = 0.0;
                results.total_matches = experiment_metrics.total_matches;
                results.total_keypoints = experiment_metrics.total_keypoints;
                results.metadata["success"] = experiment_metrics.success ? "true" : "false";
                results.metadata["experiment_name"] = yaml_config.experiment.name;
                // True IR-style mAP metrics (conditional - excluding R=0)
                results.metadata["true_map_micro"] = std::to_string(experiment_metrics.true_map_micro);
                results.metadata["true_map_macro_by_scene"] = std::to_string(experiment_metrics.true_map_macro_by_scene);
                // True IR-style mAP metrics (punitive - including R=0 as AP=0)
                results.metadata["true_map_micro_with_zeros"] = std::to_string(experiment_metrics.true_map_micro_including_zeros);
                results.metadata["true_map_macro_with_zeros"] = std::to_string(experiment_metrics.true_map_macro_by_scene_including_zeros);
                // Query statistics
                results.metadata["total_queries_processed"] = std::to_string(experiment_metrics.total_queries_processed);
                results.metadata["total_queries_excluded"] = std::to_string(experiment_metrics.total_queries_excluded);
                // Precision@K and Recall@K metrics
                results.metadata["precision_at_1"] = std::to_string(experiment_metrics.precision_at_1);
                results.metadata["precision_at_5"] = std::to_string(experiment_metrics.precision_at_5);
                results.metadata["precision_at_10"] = std::to_string(experiment_metrics.precision_at_10);
                results.metadata["recall_at_1"] = std::to_string(experiment_metrics.recall_at_1);
                results.metadata["recall_at_5"] = std::to_string(experiment_metrics.recall_at_5);
                results.metadata["recall_at_10"] = std::to_string(experiment_metrics.recall_at_10);
                // R=0 rate for transparency
                int total_all = experiment_metrics.total_queries_processed + experiment_metrics.total_queries_excluded;
                double r0_rate = total_all > 0 ? (double)experiment_metrics.total_queries_excluded / total_all : 0.0;
                results.metadata["r0_rate"] = std::to_string(r0_rate);
                
                // Per-scene True mAP breakdown
                for (const auto& [scene_name, scene_aps] : experiment_metrics.per_scene_ap) {
                    if (scene_aps.empty()) continue;
                    
                    double scene_ap_sum = std::accumulate(scene_aps.begin(), scene_aps.end(), 0.0);
                    double scene_true_map = scene_ap_sum / static_cast<double>(scene_aps.size());
                    results.metadata[scene_name + "_true_map"] = std::to_string(scene_true_map);
                    results.metadata[scene_name + "_query_count"] = std::to_string(scene_aps.size());
                    
                    // Per-scene with zeros (punitive)
                    int excluded_count = experiment_metrics.per_scene_excluded.count(scene_name) ? 
                                       experiment_metrics.per_scene_excluded.at(scene_name) : 0;
                    int total_scene_queries = static_cast<int>(scene_aps.size()) + excluded_count;
                    if (total_scene_queries > 0) {
                        double scene_true_map_with_zeros = scene_ap_sum / static_cast<double>(total_scene_queries);
                        results.metadata[scene_name + "_true_map_with_zeros"] = std::to_string(scene_true_map_with_zeros);
                        results.metadata[scene_name + "_excluded_count"] = std::to_string(excluded_count);
                    }
                }
                
                db.recordExperiment(results);
            }
#endif

            if (experiment_metrics.success) {
                LOG_INFO("✅ Completed descriptor: " + desc_config.name);
            } else {
                LOG_ERROR("❌ Failed descriptor: " + desc_config.name);
            }
        }

        LOG_INFO("🎉 Experiment completed: " + yaml_config.experiment.name);
        LOG_INFO("📊 Experiment results saved to database");

        // Reset migration toggle to avoid leaking state
        thesis_project::integration::MigrationToggle::setEnabled(false);
        return 0;

    } catch (const std::exception& e) {
        LOG_ERROR("Experiment failed: " + std::string(e.what()));
        return 1;
    }
}

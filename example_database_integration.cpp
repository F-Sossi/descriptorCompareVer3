#include "thesis_project/database/DatabaseManager.hpp"
#include "descriptor_compare/experiment_config.hpp"

// Example of how to add database tracking to existing image_processor workflow
void demonstrate_database_integration() {
    // Your existing experiment_config setup (unchanged)
    experiment_config config;
    config.descriptorOptions.descriptorType = DESCRIPTOR_RGBSIFT;
    config.descriptorOptions.poolingStrategy = POOLING_NONE;
    config.descriptorOptions.maxFeatures = 1000;

    // NEW: Optional database tracking (completely optional)
    thesis_project::database::DatabaseManager database("experiments.db", true);

    // Convert your existing config to database format
    auto db_config = database_integration::toDbConfig(config);
    db_config.dataset_path = "/data/i_ajuntament";

    // Record the experiment configuration
    int experiment_id = database.recordConfiguration(db_config);

    // Your existing processing code (unchanged)
    // ... run descriptor comparison experiments ...

    // NEW: Record results after processing (optional)
    if (experiment_id > 0) {
        auto results = database_integration::createDbResults(
            experiment_id,
            "RGBSIFT",
            "i_ajuntament",
            0.85,  // MAP score from your results
            250.0  // processing time
        );

        database.recordExperiment(results);
    }

    // Your existing workflow continues unchanged
}

// The key insight: database is completely optional and doesn't change existing code

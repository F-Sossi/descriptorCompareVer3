#include "thesis_project/database/DatabaseIntegration.hpp"

// Example of how to add database tracking to existing workflow
void demonstrate_corrected_database_integration() {
    // Your existing experiment_config setup (unchanged)
    experiment_config config;
    config.descriptorOptions.descriptorType = DESCRIPTOR_RGBSIFT;
    config.descriptorOptions.poolingStrategy = STACKING;
    config.descriptorOptions.normType = 2;
    config.useMultiThreading = true;
    config.matchThreshold = 0.05;

    // NEW: Optional database tracking (completely optional)
    thesis_project::database::DatabaseManager database("experiments.db", false); // disabled by default

    // Only proceed if database is enabled
    if (database.isEnabled()) {
        // Convert your existing config to database format using corrected integration
        auto db_config = thesis_project::database::integration::toDbConfig(config);
        db_config.dataset_path = "/data/i_ajuntament";

        // Record the experiment configuration
        int experiment_id = database.recordConfiguration(db_config);

        // Your existing processing code (unchanged)
        // ... run descriptor comparison experiments ...
        // double map_score = ... // your results
        // double processing_time = ... // your timing

        // NEW: Record results after processing (optional)
        if (experiment_id > 0) {
            auto results = thesis_project::database::integration::createDbResults(
                experiment_id,
                "RGBSIFT",
                "i_ajuntament",
                0.85,  // MAP score from your results
                250.0  // processing time
            );

            database.recordExperiment(results);
        }
    }

    // Your existing workflow continues unchanged
}

// Key benefits of the corrected approach:
// 1. Database integration is in separate header (no pollution of experiment_config.hpp)
// 2. Uses correct enum names from your actual system
// 3. Completely optional - zero impact when disabled
// 4. Clean separation of concerns

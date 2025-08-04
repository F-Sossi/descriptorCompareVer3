# Stage 5: Database Integration

## Overview
Stage 5 adds optional experiment tracking capabilities without disrupting the existing workflow. The database integration is completely optional and disabled by default.

## Key Features

### 1. Optional Integration
- Database tracking can be enabled/disabled at build time
- When disabled, zero overhead and no dependencies
- When enabled, automatic experiment tracking

### 2. Backward Compatibility
- Existing `descriptor_compare` workflow unchanged
- All existing functionality preserved
- No changes required to existing code

### 3. SQLite-Based Storage
- Lightweight, embedded database
- No external database server required
- Human-readable data storage

## Usage

### Basic Usage (Database Disabled)
```bash
# Build without database (default)
cmake .. -DBUILD_DATABASE=OFF
make
./descriptor_compare  # Works exactly as before
```

### Advanced Usage (Database Enabled)
```bash
# Build with database support
cmake .. -DBUILD_DATABASE=ON
make

# Run experiments (automatically tracked if database enabled in code)
./descriptor_compare

# Test database functionality
./test_database
```

### Code Integration Example
```cpp
#include "thesis_project/database/DatabaseManager.hpp"

// In your existing image_processor workflow:
void processExperiments() {
    // Optional database (disabled by default)
    thesis_project::database::DatabaseManager db("experiments.db", false);

    // Your existing experiment code unchanged
    experiment_config config;
    // ... existing setup ...

    // Optional: Record experiment if database enabled
    if (db.isEnabled()) {
        auto db_config = database_integration::toDbConfig(config);
        int exp_id = db.recordConfiguration(db_config);

        // After processing...
        auto results = database_integration::createDbResults(
            exp_id, "RGBSIFT", "i_ajuntament", map_score, time_ms);
        db.recordExperiment(results);
    }
}
```

## Database Schema

### Experiments Table
- `id`: Primary key
- `descriptor_type`: SIFT, RGBSIFT, etc.
- `dataset_name`: Dataset identifier
- `pooling_strategy`: NONE, STACKING, etc.
- `similarity_threshold`: Matching threshold
- `max_features`: Feature limit
- `timestamp`: When experiment was run
- `parameters`: Additional parameters

### Results Table
- `id`: Primary key
- `experiment_id`: Foreign key to experiments
- `mean_average_precision`: MAP score
- `precision_at_1`, `precision_at_5`: Precision metrics
- `recall_at_1`, `recall_at_5`: Recall metrics
- `total_matches`: Number of matches found
- `total_keypoints`: Total keypoints detected
- `processing_time_ms`: Processing duration
- `timestamp`: When results were recorded
- `metadata`: Additional result metadata

## API Reference

### DatabaseManager Class
```cpp
class DatabaseManager {
public:
    DatabaseManager(const std::string& db_path, bool enabled = false);

    bool isEnabled() const;
    int recordConfiguration(const ExperimentConfig& config);
    bool recordExperiment(const ExperimentResults& results);
    std::vector<ExperimentResults> getRecentResults(int limit = 10);
    std::map<std::string, double> getStatistics();
};
```

### Helper Functions
```cpp
// Convert existing config to database format
auto db_config = database_integration::toDbConfig(experiment_config);

// Create results structure
auto results = database_integration::createDbResults(
    exp_id, descriptor_name, dataset_name, map_score, time_ms);
```

## Benefits

1. **Non-Disruptive**: Existing workflow unchanged
2. **Optional**: Can be completely disabled
3. **Lightweight**: SQLite embedded database
4. **Automatic**: Tracks experiments transparently
5. **Analyzable**: Provides statistics and history
6. **Flexible**: Easy to extend with new metrics

## Migration Path

Stage 5 is designed for zero-impact migration:

1. **Immediate**: All existing code works unchanged
2. **Optional**: Enable database when ready for tracking
3. **Gradual**: Add tracking to specific experiments as needed
4. **Future**: Foundation for Stage 6 analysis tools

## Testing

Run the validation script to test all functionality:
```bash
./validate_stage5.sh
```

This tests:
- Database creation and initialization
- Experiment recording and retrieval
- Statistics generation
- Build system integration
- Backward compatibility preservation

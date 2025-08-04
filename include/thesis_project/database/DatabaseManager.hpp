#ifndef THESIS_PROJECT_DATABASE_MANAGER_HPP
#define THESIS_PROJECT_DATABASE_MANAGER_HPP

#include <string>
#include <memory>
#include <map>
#include <vector>

namespace thesis_project {
namespace database {

// Forward declarations
struct ExperimentResults;
struct ExperimentConfig;
struct DatabaseConfig;

/**
 * @brief Optional database manager for experiment tracking
 *
 * This class provides experiment tracking capabilities without disrupting
 * the existing workflow. All methods are safe to call - if database is
 * disabled, they silently do nothing.
 */
class DatabaseManager {
public:
    /**
     * @brief Construct database manager
     * @param config Database configuration (connection string, enabled flag, etc.)
     */
    explicit DatabaseManager(const DatabaseConfig& config);

    /**
     * @brief Construct with simple parameters
     * @param db_path Path to SQLite database file
     * @param enabled Whether database tracking is enabled
     */
    DatabaseManager(const std::string& db_path, bool enabled = false);

    /**
     * @brief Destructor - safely closes database connection
     */
    ~DatabaseManager();

    // Copy/move operations
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;
    DatabaseManager(DatabaseManager&&) = default;
    DatabaseManager& operator=(DatabaseManager&&) = default;

    /**
     * @brief Check if database is enabled and working
     * @return true if database operations will work
     */
    bool isEnabled() const;

    /**
     * @brief Record experiment results
     * @param results Results from descriptor comparison experiment
     * @return true if successfully recorded (or disabled), false on error
     */
    bool recordExperiment(const ExperimentResults& results);

    /**
     * @brief Record experiment configuration
     * @param config Configuration used for experiment
     * @return experiment_id for linking results, or -1 if disabled/error
     */
    int recordConfiguration(const ExperimentConfig& config);

    /**
     * @brief Get recent experiment results
     * @param limit Maximum number of results to return
     * @return Vector of recent experiment results
     */
    std::vector<ExperimentResults> getRecentResults(int limit = 10) const;

    /**
     * @brief Get experiment statistics
     * @return Map of statistic name -> value
     */
    std::map<std::string, double> getStatistics() const;

    /**
     * @brief Initialize database tables (safe to call multiple times)
     * @return true if successful or disabled
     */
    bool initializeTables();

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

/**
 * @brief Configuration for database connection
 */
struct DatabaseConfig {
    std::string connection_string;
    bool enabled = false;
    int connection_timeout = 30;
    bool create_if_missing = true;

    // Factory methods
    static DatabaseConfig disabled() {
        return DatabaseConfig{};
    }

    static DatabaseConfig sqlite(const std::string& path) {
        DatabaseConfig config;
        config.connection_string = path;
        config.enabled = true;
        return config;
    }
};

/**
 * @brief Experiment results structure for database storage
 */
struct ExperimentResults {
    int experiment_id = -1;
    std::string descriptor_type;
    std::string dataset_name;
    double mean_average_precision = 0.0;
    double precision_at_1 = 0.0;
    double precision_at_5 = 0.0;
    double recall_at_1 = 0.0;
    double recall_at_5 = 0.0;
    int total_matches = 0;
    int total_keypoints = 0;
    double processing_time_ms = 0.0;
    std::string timestamp;
    std::map<std::string, std::string> metadata;
};

/**
 * @brief Experiment configuration for database storage
 */
struct ExperimentConfig {
    std::string descriptor_type;
    std::string dataset_path;
    std::string pooling_strategy;
    double similarity_threshold = 0.7;
    int max_features = 1000;
    std::map<std::string, std::string> parameters;
    std::string timestamp;
};

} // namespace database
} // namespace thesis_project

#endif // THESIS_PROJECT_DATABASE_MANAGER_HPP

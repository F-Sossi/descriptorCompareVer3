#include "thesis_project/database/DatabaseManager.hpp"
#include <sqlite3.h>
#include <iostream>
#include <sstream>
#include <chrono>
#include <iomanip>

namespace thesis_project {
namespace database {

// PIMPL implementation to hide SQLite details
class DatabaseManager::Impl {
public:
    sqlite3* db = nullptr;
    bool enabled = false;
    DatabaseConfig config;

    explicit Impl(const DatabaseConfig& cfg) : config(cfg), enabled(cfg.enabled) {
        if (!enabled) {
            std::cout << "DatabaseManager: Disabled - no experiment tracking" << std::endl;
            return;
        }

        // Try to open database
        int rc = sqlite3_open(config.connection_string.c_str(), &db);
        if (rc != SQLITE_OK) {
            std::cerr << "DatabaseManager: Failed to open database: "
                      << sqlite3_errmsg(db) << std::endl;
            enabled = false;
            if (db) {
                sqlite3_close(db);
                db = nullptr;
            }
        } else {
            std::cout << "DatabaseManager: Connected to " << config.connection_string << std::endl;
        }
    }

    ~Impl() {
        if (db) {
            sqlite3_close(db);
        }
    }

    bool initializeTables() {
        if (!enabled || !db) return !enabled; // Success if disabled

        const char* create_experiments_table = R"(
            CREATE TABLE IF NOT EXISTS experiments (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                descriptor_type TEXT NOT NULL,
                dataset_name TEXT NOT NULL,
                pooling_strategy TEXT,
                similarity_threshold REAL,
                max_features INTEGER,
                timestamp TEXT NOT NULL,
                parameters TEXT
            );
        )";

        const char* create_results_table = R"(
            CREATE TABLE IF NOT EXISTS results (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                experiment_id INTEGER,
                mean_average_precision REAL,
                precision_at_1 REAL,
                precision_at_5 REAL,
                recall_at_1 REAL,
                recall_at_5 REAL,
                total_matches INTEGER,
                total_keypoints INTEGER,
                processing_time_ms REAL,
                timestamp TEXT NOT NULL,
                metadata TEXT,
                FOREIGN KEY(experiment_id) REFERENCES experiments(id)
            );
        )";

        char* error_msg = nullptr;

        int rc1 = sqlite3_exec(db, create_experiments_table, nullptr, nullptr, &error_msg);
        if (rc1 != SQLITE_OK) {
            std::cerr << "Failed to create experiments table: " << error_msg << std::endl;
            sqlite3_free(error_msg);
            return false;
        }

        int rc2 = sqlite3_exec(db, create_results_table, nullptr, nullptr, &error_msg);
        if (rc2 != SQLITE_OK) {
            std::cerr << "Failed to create results table: " << error_msg << std::endl;
            sqlite3_free(error_msg);
            return false;
        }

        return true;
    }

    std::string getCurrentTimestamp() const {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }
};

// DatabaseManager implementation
DatabaseManager::DatabaseManager(const DatabaseConfig& config)
    : impl_(std::make_unique<Impl>(config)) {
    if (impl_->enabled) {
        initializeTables();
    }
}

DatabaseManager::DatabaseManager(const std::string& db_path, bool enabled)
    : DatabaseManager(enabled ? DatabaseConfig::sqlite(db_path) : DatabaseConfig::disabled()) {
}

DatabaseManager::~DatabaseManager() = default;

bool DatabaseManager::isEnabled() const {
    return impl_->enabled && impl_->db != nullptr;
}

bool DatabaseManager::initializeTables() {
    return impl_->initializeTables();
}

int DatabaseManager::recordConfiguration(const ExperimentConfig& config) {
    if (!isEnabled()) return -1;

    const char* sql = R"(
        INSERT INTO experiments (descriptor_type, dataset_name, pooling_strategy,
                               similarity_threshold, max_features, timestamp, parameters)
        VALUES (?, ?, ?, ?, ?, ?, ?);
    )";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(impl_->db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(impl_->db) << std::endl;
        return -1;
    }

    // Build parameters string
    std::stringstream params_ss;
    for (const auto& [key, value] : config.parameters) {
        params_ss << key << "=" << value << ";";
    }
    std::string params_str = params_ss.str();

    // Bind parameters
    sqlite3_bind_text(stmt, 1, config.descriptor_type.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, config.dataset_path.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, config.pooling_strategy.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 4, config.similarity_threshold);
    sqlite3_bind_int(stmt, 5, config.max_features);
    sqlite3_bind_text(stmt, 6, impl_->getCurrentTimestamp().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 7, params_str.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    int experiment_id = -1;

    if (rc == SQLITE_DONE) {
        experiment_id = static_cast<int>(sqlite3_last_insert_rowid(impl_->db));
        std::cout << "Recorded experiment config with ID: " << experiment_id << std::endl;
    } else {
        std::cerr << "Failed to insert experiment: " << sqlite3_errmsg(impl_->db) << std::endl;
    }

    sqlite3_finalize(stmt);
    return experiment_id;
}

bool DatabaseManager::recordExperiment(const ExperimentResults& results) {
    if (!isEnabled()) return true; // Success if disabled

    const char* sql = R"(
        INSERT INTO results (experiment_id, mean_average_precision, precision_at_1,
                           precision_at_5, recall_at_1, recall_at_5, total_matches,
                           total_keypoints, processing_time_ms, timestamp, metadata)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);
    )";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(impl_->db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(impl_->db) << std::endl;
        return false;
    }

    // Build metadata string
    std::stringstream metadata_ss;
    for (const auto& [key, value] : results.metadata) {
        metadata_ss << key << "=" << value << ";";
    }
    std::string metadata_str = metadata_ss.str();

    // Bind parameters
    sqlite3_bind_int(stmt, 1, results.experiment_id);
    sqlite3_bind_double(stmt, 2, results.mean_average_precision);
    sqlite3_bind_double(stmt, 3, results.precision_at_1);
    sqlite3_bind_double(stmt, 4, results.precision_at_5);
    sqlite3_bind_double(stmt, 5, results.recall_at_1);
    sqlite3_bind_double(stmt, 6, results.recall_at_5);
    sqlite3_bind_int(stmt, 7, results.total_matches);
    sqlite3_bind_int(stmt, 8, results.total_keypoints);
    sqlite3_bind_double(stmt, 9, results.processing_time_ms);
    sqlite3_bind_text(stmt, 10, impl_->getCurrentTimestamp().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 11, metadata_str.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    bool success = (rc == SQLITE_DONE);

    if (success) {
        std::cout << "Recorded experiment results (MAP: "
                  << results.mean_average_precision << ")" << std::endl;
    } else {
        std::cerr << "Failed to insert results: " << sqlite3_errmsg(impl_->db) << std::endl;
    }

    sqlite3_finalize(stmt);
    return success;
}

std::vector<ExperimentResults> DatabaseManager::getRecentResults(int limit) const {
    std::vector<ExperimentResults> results;
    if (!isEnabled()) return results;

    const char* sql = R"(
        SELECT r.experiment_id, e.descriptor_type, e.dataset_name,
               r.mean_average_precision, r.precision_at_1, r.precision_at_5,
               r.recall_at_1, r.recall_at_5, r.total_matches,
               r.total_keypoints, r.processing_time_ms, r.timestamp
        FROM results r
        JOIN experiments e ON r.experiment_id = e.id
        ORDER BY r.timestamp DESC
        LIMIT ?;
    )";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(impl_->db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return results;
    }

    sqlite3_bind_int(stmt, 1, limit);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        ExperimentResults result;
        result.experiment_id = sqlite3_column_int(stmt, 0);
        result.descriptor_type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        result.dataset_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        result.mean_average_precision = sqlite3_column_double(stmt, 3);
        result.precision_at_1 = sqlite3_column_double(stmt, 4);
        result.precision_at_5 = sqlite3_column_double(stmt, 5);
        result.recall_at_1 = sqlite3_column_double(stmt, 6);
        result.recall_at_5 = sqlite3_column_double(stmt, 7);
        result.total_matches = sqlite3_column_int(stmt, 8);
        result.total_keypoints = sqlite3_column_int(stmt, 9);
        result.processing_time_ms = sqlite3_column_double(stmt, 10);
        result.timestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));

        results.push_back(result);
    }

    sqlite3_finalize(stmt);
    return results;
}

std::map<std::string, double> DatabaseManager::getStatistics() const {
    std::map<std::string, double> stats;
    if (!isEnabled()) return stats;

    const char* sql = R"(
        SELECT
            COUNT(*) as total_experiments,
            AVG(mean_average_precision) as avg_map,
            MAX(mean_average_precision) as best_map,
            AVG(processing_time_ms) as avg_time
        FROM results;
    )";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(impl_->db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return stats;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        stats["total_experiments"] = sqlite3_column_double(stmt, 0);
        stats["average_map"] = sqlite3_column_double(stmt, 1);
        stats["best_map"] = sqlite3_column_double(stmt, 2);
        stats["average_time_ms"] = sqlite3_column_double(stmt, 3);
    }

    sqlite3_finalize(stmt);
    return stats;
}

} // namespace database
} // namespace thesis_project

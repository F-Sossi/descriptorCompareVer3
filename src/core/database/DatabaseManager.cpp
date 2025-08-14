#include "thesis_project/database/DatabaseManager.hpp"
#include <sqlite3.h>
#include <iostream>
#include <sstream>
#include <chrono>
#include <iomanip>

namespace thesis_project::database {

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

    bool initializeTables() const {
        if (!enabled || !db) return !enabled; // Success if disabled

        const auto create_experiments_table = R"(
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

        const auto create_results_table = R"(
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

        const auto create_keypoints_table = R"(
            CREATE TABLE IF NOT EXISTS locked_keypoints (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                scene_name TEXT NOT NULL,
                image_name TEXT NOT NULL,
                x REAL NOT NULL,
                y REAL NOT NULL,
                size REAL NOT NULL,
                angle REAL NOT NULL,
                response REAL NOT NULL,
                octave INTEGER NOT NULL,
                class_id INTEGER NOT NULL,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                UNIQUE(scene_name, image_name, x, y)
            );
        )";

        const auto create_descriptors_table = R"(
            CREATE TABLE IF NOT EXISTS descriptors (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                experiment_id INTEGER NOT NULL,
                scene_name TEXT NOT NULL,
                image_name TEXT NOT NULL,
                keypoint_x REAL NOT NULL,
                keypoint_y REAL NOT NULL,
                descriptor_vector BLOB NOT NULL,
                descriptor_dimension INTEGER NOT NULL,
                processing_method TEXT,
                normalization_applied TEXT,
                rooting_applied TEXT,
                pooling_applied TEXT,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY(experiment_id) REFERENCES experiments(id),
                UNIQUE(experiment_id, scene_name, image_name, keypoint_x, keypoint_y)
            );
        )";

        const auto create_descriptor_indexes = R"(
            CREATE INDEX IF NOT EXISTS idx_descriptors_experiment ON descriptors(experiment_id, processing_method);
            CREATE INDEX IF NOT EXISTS idx_descriptors_keypoint ON descriptors(scene_name, image_name, keypoint_x, keypoint_y);
            CREATE INDEX IF NOT EXISTS idx_descriptors_method ON descriptors(processing_method, normalization_applied, rooting_applied);
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

        int rc3 = sqlite3_exec(db, create_keypoints_table, nullptr, nullptr, &error_msg);
        if (rc3 != SQLITE_OK) {
            std::cerr << "Failed to create locked_keypoints table: " << error_msg << std::endl;
            sqlite3_free(error_msg);
            return false;
        }

        int rc4 = sqlite3_exec(db, create_descriptors_table, nullptr, nullptr, &error_msg);
        if (rc4 != SQLITE_OK) {
            std::cerr << "Failed to create descriptors table: " << error_msg << std::endl;
            sqlite3_free(error_msg);
            return false;
        }

        int rc5 = sqlite3_exec(db, create_descriptor_indexes, nullptr, nullptr, &error_msg);
        if (rc5 != SQLITE_OK) {
            std::cerr << "Failed to create descriptor indexes: " << error_msg << std::endl;
            sqlite3_free(error_msg);
            return false;
        }

        return true;
    }

    static std::string getCurrentTimestamp() {
        const auto now = std::chrono::system_clock::now();
        const auto time_t = std::chrono::system_clock::to_time_t(now);
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

bool DatabaseManager::initializeTables() const {
    return impl_->initializeTables();
}

int DatabaseManager::recordConfiguration(const ExperimentConfig& config) const {
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

bool DatabaseManager::recordExperiment(const ExperimentResults& results) const {
    if (!isEnabled()) return true; // Success if disabled

    const auto sql = R"(
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
    const bool success = (rc == SQLITE_DONE);

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

    const auto sql = R"(
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

bool DatabaseManager::storeLockedKeypoints(const std::string& scene_name, const std::string& image_name, const std::vector<cv::KeyPoint>& keypoints) const {
    if (!isEnabled()) return true; // Success if disabled

    const auto sql = R"(
        INSERT OR REPLACE INTO locked_keypoints (scene_name, image_name, x, y, size, angle, response, octave, class_id)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);
    )";

    // First, clear existing keypoints for this scene/image
    const auto clear_sql = "DELETE FROM locked_keypoints WHERE scene_name = ? AND image_name = ?";
    sqlite3_stmt* clear_stmt;
    int rc = sqlite3_prepare_v2(impl_->db, clear_sql, -1, &clear_stmt, nullptr);
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(clear_stmt, 1, scene_name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(clear_stmt, 2, image_name.c_str(), -1, SQLITE_STATIC);
        sqlite3_step(clear_stmt);
    }
    sqlite3_finalize(clear_stmt);

    sqlite3_stmt* stmt;
    rc = sqlite3_prepare_v2(impl_->db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare keypoint insert statement: " << sqlite3_errmsg(impl_->db) << std::endl;
        return false;
    }

    int stored_count = 0;
    for (const auto& kp : keypoints) {
        sqlite3_bind_text(stmt, 1, scene_name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, image_name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, 3, kp.pt.x);
        sqlite3_bind_double(stmt, 4, kp.pt.y);
        sqlite3_bind_double(stmt, 5, kp.size);
        sqlite3_bind_double(stmt, 6, kp.angle);
        sqlite3_bind_double(stmt, 7, kp.response);
        sqlite3_bind_int(stmt, 8, kp.octave);
        sqlite3_bind_int(stmt, 9, kp.class_id);

        rc = sqlite3_step(stmt);
        if (rc == SQLITE_DONE) {
            stored_count++;
        }
        sqlite3_reset(stmt);
    }

    sqlite3_finalize(stmt);
    
    std::cout << "Stored " << stored_count << " keypoints for " << scene_name << "/" << image_name << std::endl;
    return stored_count == keypoints.size();
}

std::vector<cv::KeyPoint> DatabaseManager::getLockedKeypoints(const std::string& scene_name, const std::string& image_name) const {
    std::vector<cv::KeyPoint> keypoints;
    if (!isEnabled()) return keypoints;

    const char* sql = R"(
        SELECT x, y, size, angle, response, octave, class_id
        FROM locked_keypoints
        WHERE scene_name = ? AND image_name = ?
        ORDER BY id;
    )";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(impl_->db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return keypoints;
    }

    sqlite3_bind_text(stmt, 1, scene_name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, image_name.c_str(), -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        cv::KeyPoint kp;
        kp.pt.x = sqlite3_column_double(stmt, 0);
        kp.pt.y = sqlite3_column_double(stmt, 1);
        kp.size = sqlite3_column_double(stmt, 2);
        kp.angle = sqlite3_column_double(stmt, 3);
        kp.response = sqlite3_column_double(stmt, 4);
        kp.octave = sqlite3_column_int(stmt, 5);
        kp.class_id = sqlite3_column_int(stmt, 6);
        keypoints.push_back(kp);
    }

    sqlite3_finalize(stmt);
    return keypoints;
}

std::vector<std::string> DatabaseManager::getAvailableScenes() const {
    std::vector<std::string> scenes;
    if (!isEnabled()) return scenes;

    const char* sql = "SELECT DISTINCT scene_name FROM locked_keypoints ORDER BY scene_name;";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(impl_->db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return scenes;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        scenes.push_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
    }

    sqlite3_finalize(stmt);
    return scenes;
}

std::vector<std::string> DatabaseManager::getAvailableImages(const std::string& scene_name) const {
    std::vector<std::string> images;
    if (!isEnabled()) return images;

    const char* sql = "SELECT DISTINCT image_name FROM locked_keypoints WHERE scene_name = ? ORDER BY image_name;";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(impl_->db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return images;
    }

    sqlite3_bind_text(stmt, 1, scene_name.c_str(), -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        images.emplace_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
    }

    sqlite3_finalize(stmt);
    return images;
}

bool DatabaseManager::clearSceneKeypoints(const std::string& scene_name) const {
    if (!isEnabled()) return true; // Success if disabled

    const auto sql = "DELETE FROM locked_keypoints WHERE scene_name = ?;";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(impl_->db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare clear statement: " << sqlite3_errmsg(impl_->db) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, scene_name.c_str(), -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    
    bool success = (rc == SQLITE_DONE);
    if (success) {
        int deleted_count = sqlite3_changes(impl_->db);
        std::cout << "Cleared " << deleted_count << " keypoints for scene: " << scene_name << std::endl;
    }

    sqlite3_finalize(stmt);
    return success;
}

bool DatabaseManager::storeDescriptors(int experiment_id,
                                      const std::string& scene_name,
                                      const std::string& image_name,
                                      const std::vector<cv::KeyPoint>& keypoints,
                                      const cv::Mat& descriptors,
                                      const std::string& processing_method,
                                      const std::string& normalization_applied,
                                      const std::string& rooting_applied,
                                      const std::string& pooling_applied) const {
    if (!impl_->enabled || !impl_->db) return !impl_->enabled;

    if (keypoints.size() != descriptors.rows) {
        std::cerr << "Error: Keypoints count (" << keypoints.size() 
                  << ") does not match descriptor rows (" << descriptors.rows << ")" << std::endl;
        return false;
    }

    const auto sql = R"(
        INSERT OR REPLACE INTO descriptors 
        (experiment_id, scene_name, image_name, keypoint_x, keypoint_y, 
         descriptor_vector, descriptor_dimension, processing_method, 
         normalization_applied, rooting_applied, pooling_applied) 
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(impl_->db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare descriptor insert statement: " << sqlite3_errmsg(impl_->db) << std::endl;
        return false;
    }

    // Begin transaction for efficiency
    sqlite3_exec(impl_->db, "BEGIN TRANSACTION", nullptr, nullptr, nullptr);

    bool success = true;
    for (size_t i = 0; i < keypoints.size(); ++i) {
        const cv::KeyPoint& kp = keypoints[i];
        cv::Mat descriptor_row = descriptors.row(i);

        // Bind parameters
        sqlite3_bind_int(stmt, 1, experiment_id);
        sqlite3_bind_text(stmt, 2, scene_name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, image_name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, 4, kp.pt.x);
        sqlite3_bind_double(stmt, 5, kp.pt.y);

        // Store descriptor as binary blob
        sqlite3_bind_blob(stmt, 6, descriptor_row.data, 
                         descriptor_row.total() * descriptor_row.elemSize(), SQLITE_STATIC);
        sqlite3_bind_int(stmt, 7, descriptor_row.cols);
        sqlite3_bind_text(stmt, 8, processing_method.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 9, normalization_applied.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 10, rooting_applied.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 11, pooling_applied.c_str(), -1, SQLITE_STATIC);

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            std::cerr << "Failed to insert descriptor " << i << ": " << sqlite3_errmsg(impl_->db) << std::endl;
            success = false;
            break;
        }

        sqlite3_reset(stmt);
    }

    sqlite3_finalize(stmt);

    if (success) {
        sqlite3_exec(impl_->db, "COMMIT", nullptr, nullptr, nullptr);
        std::cout << "Stored " << keypoints.size() << " descriptors for " 
                  << scene_name << "/" << image_name << " (experiment " << experiment_id << ")" << std::endl;
    } else {
        sqlite3_exec(impl_->db, "ROLLBACK", nullptr, nullptr, nullptr);
    }

    return success;
}

cv::Mat DatabaseManager::getDescriptors(int experiment_id,
                                       const std::string& scene_name,
                                       const std::string& image_name) const {
    if (!impl_->enabled || !impl_->db) return cv::Mat();

    const char* sql = R"(
        SELECT descriptor_vector, descriptor_dimension 
        FROM descriptors 
        WHERE experiment_id = ? AND scene_name = ? AND image_name = ?
        ORDER BY keypoint_x, keypoint_y
    )";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(impl_->db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare descriptor select statement: " << sqlite3_errmsg(impl_->db) << std::endl;
        return {};
    }

    sqlite3_bind_int(stmt, 1, experiment_id);
    sqlite3_bind_text(stmt, 2, scene_name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, image_name.c_str(), -1, SQLITE_STATIC);

    std::vector<cv::Mat> descriptor_rows;
    int descriptor_dim = 0;

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        const void* blob_data = sqlite3_column_blob(stmt, 0);
        int blob_size = sqlite3_column_bytes(stmt, 0);
        descriptor_dim = sqlite3_column_int(stmt, 1);

        // Create cv::Mat from blob data
        cv::Mat row(1, descriptor_dim, CV_32F);
        memcpy(row.data, blob_data, blob_size);
        descriptor_rows.push_back(row);
    }

    sqlite3_finalize(stmt);

    if (descriptor_rows.empty()) {
        return {};
    }

    // Combine all rows into single Mat
    cv::Mat result;
    cv::vconcat(descriptor_rows, result);
    return result;
}

std::vector<std::tuple<std::string, std::string, cv::Mat>> DatabaseManager::getDescriptorsByMethod(
    const std::string& processing_method,
    const std::string& normalization_applied,
    const std::string& rooting_applied) const {
    
    std::vector<std::tuple<std::string, std::string, cv::Mat>> results;
    if (!impl_->enabled || !impl_->db) return results;

    std::string sql = "SELECT DISTINCT scene_name, image_name FROM descriptors WHERE processing_method = ?";
    std::vector<std::string> params = {processing_method};

    if (!normalization_applied.empty()) {
        sql += " AND normalization_applied = ?";
        params.push_back(normalization_applied);
    }
    if (!rooting_applied.empty()) {
        sql += " AND rooting_applied = ?";
        params.push_back(rooting_applied);
    }

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(impl_->db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare descriptor method query: " << sqlite3_errmsg(impl_->db) << std::endl;
        return results;
    }

    for (size_t i = 0; i < params.size(); ++i) {
        sqlite3_bind_text(stmt, i + 1, params[i].c_str(), -1, SQLITE_STATIC);
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        std::string scene = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string image = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        
        // Get descriptors for this scene/image combination
        cv::Mat descriptors = getDescriptors(-1, scene, image); // Use -1 to get latest
        results.emplace_back(scene, image, descriptors);
    }

    sqlite3_finalize(stmt);
    return results;
}

std::vector<std::string> DatabaseManager::getAvailableProcessingMethods() const {
    std::vector<std::string> methods;
    if (!impl_->enabled || !impl_->db) return methods;

    const char* sql = "SELECT DISTINCT processing_method FROM descriptors ORDER BY processing_method";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(impl_->db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare processing methods query: " << sqlite3_errmsg(impl_->db) << std::endl;
        return methods;
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        const auto method = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        if (method) {
            methods.emplace_back(method);
        }
    }

    sqlite3_finalize(stmt);
    return methods;
}

} // namespace thesis_project::database


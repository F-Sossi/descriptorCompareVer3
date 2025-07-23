#include "database_manager.hpp"
#include <iostream>

DatabaseManager::DatabaseManager(const std::string& database_path)
    : db(nullptr), db_path(database_path) {  // Fixed order: db first, then db_path
    int rc = sqlite3_open(db_path.c_str(), &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
    }
}

DatabaseManager::~DatabaseManager() {
    if (db) {
        sqlite3_close(db);
    }
}

bool DatabaseManager::initialize() {
    // TODO: Implement schema initialization
    return true;
}
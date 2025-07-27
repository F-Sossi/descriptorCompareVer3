#ifndef DATABASE_MANAGER_HPP
#define DATABASE_MANAGER_HPP

#include <sqlite3.h>
#include <string>
#include <vector>
#include <memory>
//#include <opencv2/core.hpp>

class DatabaseManager {
private:
    sqlite3* db;
    std::string db_path;

public:
    DatabaseManager(const std::string& database_path);
    ~DatabaseManager();

    bool initialize();

    // Add other methods as needed
};

#endif

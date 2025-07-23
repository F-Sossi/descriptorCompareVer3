#include "database_manager.hpp"
#include <iostream>

int main() {
    std::cout << "Testing database connection..." << std::endl;

    DatabaseManager db("test.db");
    if (db.initialize()) {
        std::cout << "Database initialized successfully!" << std::endl;
    } else {
        std::cerr << "Failed to initialize database" << std::endl;
        return 1;
    }

    return 0;
}

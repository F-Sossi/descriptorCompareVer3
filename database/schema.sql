-- Placeholder schema for database
CREATE TABLE IF NOT EXISTS experiments (
                                           id INTEGER PRIMARY KEY AUTOINCREMENT,
                                           name TEXT NOT NULL,
                                           created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
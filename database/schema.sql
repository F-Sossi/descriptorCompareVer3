-- Database schema for descriptor research experiments
-- This matches the schema defined in DatabaseManager.cpp

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

-- Locked-in keypoints storage
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
#include "thesis_project/database/DatabaseManager.hpp"
#include "../descriptor_compare/locked_in_keypoints.hpp"
#include "thesis_project/logging.hpp"
#include <iostream>
#include <filesystem>
#include <boost/filesystem.hpp>
#include <fstream>

using namespace thesis_project;

/**
 * @brief CLI tool for managing locked-in keypoints in the database
 */
int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <command> [options]" << std::endl;
        std::cout << "Commands:" << std::endl;
        std::cout << "  1. import-csv <csv_folder>     - Import keypoints from CSV files for reproducibility" << std::endl;
        std::cout << "  2. generate <data_folder>      - Clear DB and generate fresh locked keypoints from images" << std::endl;
        std::cout << "  3. export-csv <output_folder>  - Export keypoints from DB to CSV for long-term storage" << std::endl;
        std::cout << "  4. list-scenes                 - List all available scenes in database" << std::endl;
        std::cout << "  5. count <scene> <image>       - Count keypoints for specific scene/image" << std::endl;
        return 1;
    }

    std::string command = argv[1];

    // Initialize database
    database::DatabaseManager db("experiments.db", true);
    if (!db.isEnabled()) {
        std::cerr << "❌ Failed to connect to database" << std::endl;
        return 1;
    }

    if (command == "import-csv") {
        if (argc != 3) {
            std::cerr << "Usage: " << argv[0] << " import-csv <csv_folder>" << std::endl;
            std::cerr << "  Example: " << argv[0] << " import-csv ../reference_keypoints" << std::endl;
            return 1;
        }

        std::string csv_folder = argv[2];
        LOG_INFO("🔄 Importing keypoints from CSV folder: " + csv_folder);

        namespace fs = boost::filesystem;
        
        if (!fs::exists(csv_folder) || !fs::is_directory(csv_folder)) {
            std::cerr << "❌ CSV folder does not exist: " << csv_folder << std::endl;
            return 1;
        }

        int total_imported = 0;
        
        // Iterate through each scene folder
        for (const auto& scene_entry : fs::directory_iterator(csv_folder)) {
            if (!fs::is_directory(scene_entry)) continue;
            
            std::string scene_name = scene_entry.path().filename().string();
            LOG_INFO("📁 Processing scene: " + scene_name);
            
            // Iterate through each CSV file in the scene folder
            for (const auto& csv_entry : fs::directory_iterator(scene_entry)) {
                if (csv_entry.path().extension() != ".csv") continue;
                
                std::string csv_file = csv_entry.path().string();
                std::string image_name = csv_entry.path().stem().string() + ".ppm";
                
                try {
                    std::vector<cv::KeyPoint> keypoints = LockedInKeypoints::readKeypointsFromCSV(csv_file);
                    
                    if (db.storeLockedKeypoints(scene_name, image_name, keypoints)) {
                        total_imported += keypoints.size();
                        LOG_INFO("  ✅ " + scene_name + "/" + image_name + ": " + std::to_string(keypoints.size()) + " keypoints");
                    } else {
                        LOG_ERROR("  ❌ Failed to store: " + scene_name + "/" + image_name);
                    }
                } catch (const std::exception& e) {
                    LOG_ERROR("  ❌ Error processing " + csv_file + ": " + std::string(e.what()));
                }
            }
        }
        
        LOG_INFO("🎉 Import complete! Total keypoints imported: " + std::to_string(total_imported));

    } else if (command == "generate") {
        if (argc != 3) {
            std::cerr << "Usage: " << argv[0] << " generate <data_folder>" << std::endl;
            std::cerr << "  Example: " << argv[0] << " generate ../data" << std::endl;
            return 1;
        }
        
        std::string data_folder = argv[2];
        LOG_INFO("🔄 Generating fresh locked keypoints from: " + data_folder);
        
        namespace fs = boost::filesystem;
        if (!fs::exists(data_folder) || !fs::is_directory(data_folder)) {
            std::cerr << "❌ Data folder does not exist: " << data_folder << std::endl;
            return 1;
        }

        // Clear existing keypoints for all scenes in the data folder
        LOG_INFO("🗑️  Clearing existing keypoints from database...");
        for (const auto& scene_entry : fs::directory_iterator(data_folder)) {
            if (!fs::is_directory(scene_entry)) continue;
            std::string scene_name = scene_entry.path().filename().string();
            db.clearSceneKeypoints(scene_name);
        }
        
        // Generate fresh keypoints directly to database
        LOG_INFO("🔍 Generating new locked keypoints...");
        int total_generated = 0;
        
        try {
            for (const auto& scene_entry : fs::directory_iterator(data_folder)) {
                if (!fs::is_directory(scene_entry)) continue;
                
                std::string scene_path = scene_entry.path().string();
                std::string scene_name = scene_entry.path().filename().string();
                
                LOG_INFO("📁 Processing scene: " + scene_name);
                
                // Read the first image to get reference keypoints
                cv::Mat image1 = cv::imread(scene_path + "/1.ppm", cv::IMREAD_COLOR);
                if (image1.empty()) {
                    LOG_ERROR("  ❌ Failed to read reference image: " + scene_path + "/1.ppm");
                    continue;
                }
                
                // Convert to grayscale for SIFT
                cv::Mat gray;
                cv::cvtColor(image1, gray, cv::COLOR_BGR2GRAY);
                
                // Generate keypoints using SIFT
                cv::Ptr<cv::SIFT> detector = cv::SIFT::create();
                std::vector<cv::KeyPoint> keypoints;
                cv::Mat descriptors;
                detector->detectAndCompute(gray, cv::noArray(), keypoints, descriptors);
                
                // Store keypoints for image 1.ppm
                if (db.storeLockedKeypoints(scene_name, "1.ppm", keypoints)) {
                    total_generated += keypoints.size();
                    LOG_INFO("  ✅ Generated " + std::to_string(keypoints.size()) + " keypoints for " + scene_name + "/1.ppm");
                } else {
                    LOG_ERROR("  ❌ Failed to store keypoints for " + scene_name + "/1.ppm");
                }
                
                // TODO: For other images, we could generate locked keypoints using homography
                // but for now we'll just use the same keypoints (this can be enhanced later)
                for (int i = 2; i <= 6; i++) {
                    std::string image_name = std::to_string(i) + ".ppm";
                    std::string image_path = scene_path + "/" + image_name;
                    
                    if (fs::exists(image_path)) {
                        if (db.storeLockedKeypoints(scene_name, image_name, keypoints)) {
                            total_generated += keypoints.size();
                            LOG_INFO("  ✅ Stored " + std::to_string(keypoints.size()) + " keypoints for " + scene_name + "/" + image_name);
                        }
                    }
                }
            }
            
            LOG_INFO("🎉 Generation complete! Total keypoints generated: " + std::to_string(total_generated));
            
        } catch (const std::exception& e) {
            LOG_ERROR("❌ Error generating keypoints: " + std::string(e.what()));
            return 1;
        }

    } else if (command == "export-csv") {
        if (argc != 3) {
            std::cerr << "Usage: " << argv[0] << " export-csv <output_folder>" << std::endl;
            std::cerr << "  Example: " << argv[0] << " export-csv ./exported_keypoints" << std::endl;
            return 1;
        }
        
        std::string output_folder = argv[2];
        LOG_INFO("💾 Exporting keypoints to CSV folder: " + output_folder);
        
        // Create output directory
        std::filesystem::create_directories(output_folder);
        
        auto scenes = db.getAvailableScenes();
        if (scenes.empty()) {
            LOG_INFO("ℹ️  No keypoints found in database to export");
            return 0;
        }
        
        int total_exported = 0;
        
        for (const auto& scene : scenes) {
            std::string scene_folder = output_folder + "/" + scene;
            std::filesystem::create_directories(scene_folder);
            
            LOG_INFO("📁 Exporting scene: " + scene);
            
            auto images = db.getAvailableImages(scene);
            for (const auto& image : images) {
                auto keypoints = db.getLockedKeypoints(scene, image);
                
                if (!keypoints.empty()) {
                    // Convert .ppm to .csv filename
                    std::string csv_filename = image;
                    if (csv_filename.length() >= 4 && csv_filename.substr(csv_filename.length() - 4) == ".ppm") {
                        csv_filename = csv_filename.substr(0, csv_filename.length() - 4) + "ppm.csv";
                    } else {
                        csv_filename += ".csv";
                    }
                    
                    std::string csv_path = scene_folder + "/" + csv_filename;
                    
                    // Write CSV file
                    std::ofstream file(csv_path);
                    if (!file.is_open()) {
                        LOG_ERROR("  ❌ Failed to create file: " + csv_path);
                        continue;
                    }
                    
                    // Write header
                    file << "x,y,size,angle,response,octave,class_id" << std::endl;
                    
                    // Write keypoints
                    for (const auto& kp : keypoints) {
                        file << kp.pt.x << "," << kp.pt.y << "," << kp.size << "," 
                             << kp.angle << "," << kp.response << "," << kp.octave << "," 
                             << kp.class_id << std::endl;
                    }
                    
                    file.close();
                    total_exported += keypoints.size();
                    LOG_INFO("  ✅ " + scene + "/" + csv_filename + ": " + std::to_string(keypoints.size()) + " keypoints");
                }
            }
        }
        
        LOG_INFO("🎉 Export complete! Total keypoints exported: " + std::to_string(total_exported));

    } else if (command == "list-scenes") {
        auto scenes = db.getAvailableScenes();
        std::cout << "📋 Available scenes (" << scenes.size() << "):" << std::endl;
        for (const auto& scene : scenes) {
            auto images = db.getAvailableImages(scene);
            int total_keypoints = 0;
            for (const auto& image : images) {
                auto keypoints = db.getLockedKeypoints(scene, image);
                total_keypoints += keypoints.size();
            }
            std::cout << "  📁 " << scene << " (" << images.size() << " images, " << total_keypoints << " total keypoints)" << std::endl;
        }

    } else if (command == "count") {
        if (argc != 4) {
            std::cerr << "Usage: " << argv[0] << " count <scene> <image>" << std::endl;
            std::cerr << "  Example: " << argv[0] << " count i_dome 1.ppm" << std::endl;
            return 1;
        }
        
        std::string scene = argv[2];
        std::string image = argv[3];
        auto keypoints = db.getLockedKeypoints(scene, image);
        std::cout << "🔢 Keypoints for " << scene << "/" << image << ": " << keypoints.size() << std::endl;

    } else {
        std::cerr << "❌ Unknown command: " << command << std::endl;
        std::cerr << "Run without arguments to see available commands." << std::endl;
        return 1;
    }

    return 0;
}
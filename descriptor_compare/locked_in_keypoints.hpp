#ifndef LOCKED_IN_KEYPOINTS_HPP
#define LOCKED_IN_KEYPOINTS_HPP

#include <string>
#include <opencv2/core.hpp>
#include <opencv2/features2d.hpp>
#include <boost/filesystem.hpp>  // Add this include
#include <vector>  // Add this for std::vector

#ifdef BUILD_DATABASE
#include "thesis_project/database/DatabaseManager.hpp"
#endif

namespace fs = boost::filesystem;  // Add this namespace alias

class LockedInKeypoints {
public:
    static void generateLockedInKeypoints(const std::string &dataFolderPath, const std::string &referenceKeypointsBaseFolder);
    static void displayLockedInKeypoints(const std::string& dataFolderPath);
    static std::vector<cv::KeyPoint> readKeypointsFromCSV(const std::string& filePath);
    static void displayLockedInKeypointsBorder(const std::string &dataFolderPath);
    static void saveLockedInKeypointsBorder(const std::string& dataFolderPath);

#ifdef BUILD_DATABASE
    /**
     * @brief Generate locked-in keypoints with proper boundary filtering and store in database
     * @param dataFolderPath Path to HPatches data folder containing scene subdirectories
     * @param db Database manager for storing keypoints
     */
    static void generateLockedInKeypointsToDatabase(const std::string& dataFolderPath, 
                                                   const thesis_project::database::DatabaseManager& db);
#endif

private:
    static void saveKeypointsToCSV(const std::string& filePath, const std::vector<cv::KeyPoint>& keypoints);
    static cv::Mat readHomography(const std::string& filePath);
    static void createSummaryImage(const std::vector<std::string>& imageFilenames,
                                   const fs::path& referenceKeypointsFolder,
                                   const std::string& outputPath,
                                   const std::string& sceneName);
};

#endif // LOCKED_IN_KEYPOINTS_HPP
#include "locked_in_keypoints.hpp"
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>
#include <opencv2/features2d.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
#include <sstream>
#include <opencv2/highgui.hpp>
#include <iostream>

namespace fs = boost::filesystem;

constexpr int BORDER = 40;

void LockedInKeypoints::generateLockedInKeypoints(const std::string& dataFolderPath, const std::string& referenceKeypointsBaseFolder) {
    if (!fs::exists(dataFolderPath) || !fs::is_directory(dataFolderPath)) {
        throw std::runtime_error("Invalid data folder path: " + dataFolderPath);
    }

    for (const auto& entry : fs::directory_iterator(dataFolderPath)) {
        const fs::path subfolderPath = entry.path();
        if (fs::is_directory(subfolderPath)) {
            const std::string subfolderName = subfolderPath.filename().generic_string();
            const fs::path referenceKeypointsFolder = fs::path(referenceKeypointsBaseFolder) / subfolderName;
            fs::create_directories(referenceKeypointsFolder);

            const fs::path image1Path = subfolderPath / "1.ppm";
            cv::Mat image1 = cv::imread(image1Path.generic_string(), cv::IMREAD_COLOR);
            if (image1.empty()) {
                throw std::runtime_error("Failed to read image: " + image1Path.generic_string());
            }

            cv::Ptr<cv::Feature2D> sift = cv::SIFT::create();
            std::vector<cv::KeyPoint> keypoints;
            sift->detect(image1, keypoints);

            // Filter keypoints that are too close to the image border
            keypoints.erase(std::remove_if(keypoints.begin(), keypoints.end(), [image1](const cv::KeyPoint& keypoint) {
                return keypoint.pt.x < static_cast<float>(BORDER) || keypoint.pt.y < static_cast<float>(BORDER) ||
                       keypoint.pt.x > static_cast<float>(image1.cols - BORDER) || keypoint.pt.y > static_cast<float>(image1.rows - BORDER);
            }), keypoints.end());

            // If no keypoints are left after filtering, skip this series
            if (keypoints.empty()) {
                std::cerr << "All keypoints are too close to the border for folder: " << subfolderName << std::endl;
                continue;
            }

            std::sort(keypoints.begin(), keypoints.end(), [](const cv::KeyPoint& a, const cv::KeyPoint& b) {
                return a.response > b.response;
            });

            const int maxKeypoints = 2000;
            if (keypoints.size() > maxKeypoints) {
                keypoints.resize(maxKeypoints);
            }

            // Print the number of keypoints and the folder name
            std::cout << "Number of keypoints: " << keypoints.size() << " Folder name: " << subfolderName << std::endl;

            const fs::path keypointsFile1 = referenceKeypointsFolder / "1ppm.csv";
            saveKeypointsToCSV(keypointsFile1.generic_string(), keypoints);

            for (int i = 2; i <= 6; ++i) {
                const fs::path homographyFile = subfolderPath / ("H_1_" + std::to_string(i));
                cv::Mat homography = readHomography(homographyFile.generic_string());

                std::vector<cv::Point2f> points;
                for (const auto& keypoint : keypoints) {
                    points.push_back(keypoint.pt);
                }

                std::vector<cv::Point2f> transformedPoints;
                cv::perspectiveTransform(points, transformedPoints, homography);

                std::vector<cv::KeyPoint> transformedKeypoints;
                for (size_t j = 0; j < transformedPoints.size(); ++j) {
                    const cv::KeyPoint& originalKeypoint = keypoints[j];
                    const cv::Point2f& transformedPoint = transformedPoints[j];
                    transformedKeypoints.emplace_back(transformedPoint, originalKeypoint.size, originalKeypoint.angle,
                                                      originalKeypoint.response, originalKeypoint.octave, originalKeypoint.class_id);
                }

                // Filter transformed keypoints that are too close to the image border
                transformedKeypoints.erase(std::remove_if(transformedKeypoints.begin(), transformedKeypoints.end(), [image1](const cv::KeyPoint& keypoint) {
                    return keypoint.pt.x < static_cast<float>(BORDER) || keypoint.pt.y < static_cast<float>(BORDER) ||
                           keypoint.pt.x > static_cast<float>(image1.cols - BORDER) || keypoint.pt.y > static_cast<float>(image1.rows - BORDER);
                }), transformedKeypoints.end());

                // If no transformed keypoints are left after filtering, skip this file
                if (transformedKeypoints.empty()) {
                    std::cerr << "All transformed keypoints are too close to the border for file: " << i << " in folder: " << subfolderName << std::endl;
                    continue;
                }

                const fs::path keypointsFile = referenceKeypointsFolder / (std::to_string(i) + "ppm.csv");
                saveKeypointsToCSV(keypointsFile.generic_string(), transformedKeypoints);
            }
        }
    }
}


/*void LockedInKeypoints::generateLockedInKeypoints(const std::string& dataFolderPath, const std::string& referenceKeypointsBaseFolder) {
    if (!fs::exists(dataFolderPath) || !fs::is_directory(dataFolderPath)) {
        throw std::runtime_error("Invalid data folder path: " + dataFolderPath);
    }

    for (const auto& entry : fs::directory_iterator(dataFolderPath)) {
        const fs::path subfolderPath = entry.path();
        if (fs::is_directory(subfolderPath)) {
            const std::string subfolderName = subfolderPath.filename().generic_string();
            const fs::path referenceKeypointsFolder = fs::path(referenceKeypointsBaseFolder) / subfolderName;
            fs::create_directories(referenceKeypointsFolder);

            const fs::path image1Path = subfolderPath / "1.ppm";
            cv::Mat image1 = cv::imread(image1Path.generic_string(), cv::IMREAD_COLOR);
            if (image1.empty()) {
                throw std::runtime_error("Failed to read image: " + image1Path.generic_string());
            }

            cv::Ptr<cv::Feature2D> sift = cv::SIFT::create();
            std::vector<cv::KeyPoint> keypoints;
            sift->detect(image1, keypoints);

            // Filter keypoints that are too close to the image border
            keypoints.erase(std::remove_if(keypoints.begin(), keypoints.end(), [image1](const cv::KeyPoint& keypoint) {
                return keypoint.pt.x < static_cast<float>(BORDER) || keypoint.pt.y < static_cast<float>(BORDER) ||
                       keypoint.pt.x > static_cast<float>(image1.cols - BORDER) || keypoint.pt.y > static_cast<float>(image1.rows - BORDER);
            }), keypoints.end());

            std::sort(keypoints.begin(), keypoints.end(), [](const cv::KeyPoint& a, const cv::KeyPoint& b) {
                return a.response > b.response;
            });

            const int maxKeypoints = 2000;
            if (keypoints.size() > maxKeypoints) {
                keypoints.resize(maxKeypoints);
            }

            // Print the number of keypoints and the folder name
            std::cout << "Number of keypoints: " << keypoints.size() << " Folder name: " << subfolderName << std::endl;

            const fs::path keypointsFile1 = referenceKeypointsFolder / "1ppm.csv";
            saveKeypointsToCSV(keypointsFile1.generic_string(), keypoints);

            for (int i = 2; i <= 6; ++i) {
                const fs::path homographyFile = subfolderPath / ("H_1_" + std::to_string(i));
                cv::Mat homography = readHomography(homographyFile.generic_string());

                std::vector<cv::Point2f> points;
                for (const auto& keypoint : keypoints) {
                    points.push_back(keypoint.pt);
                }

                std::vector<cv::Point2f> transformedPoints;
                cv::perspectiveTransform(points, transformedPoints, homography);

                std::vector<cv::KeyPoint> transformedKeypoints;
                for (size_t j = 0; j < transformedPoints.size(); ++j) {
                    const cv::KeyPoint& originalKeypoint = keypoints[j];
                    const cv::Point2f& transformedPoint = transformedPoints[j];
                    transformedKeypoints.emplace_back(transformedPoint, originalKeypoint.size, originalKeypoint.angle,
                                                      originalKeypoint.response, originalKeypoint.octave, originalKeypoint.class_id);
                }

                const fs::path keypointsFile = referenceKeypointsFolder / (std::to_string(i) + "ppm.csv");
                saveKeypointsToCSV(keypointsFile.generic_string(), transformedKeypoints);
            }
        }
    }
}*/

void LockedInKeypoints::saveKeypointsToCSV(const std::string& filePath, const std::vector<cv::KeyPoint>& keypoints) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + filePath);
    }

    file << "x,y,size,angle,response,octave,class_id\n";
    for (const auto& keypoint : keypoints) {
        file << keypoint.pt.x << "," << keypoint.pt.y << "," << keypoint.size << "," << keypoint.angle << ","
             << keypoint.response << "," << keypoint.octave << "," << keypoint.class_id << "\n";
    }

    file.close();
}

cv::Mat LockedInKeypoints::readHomography(const std::string& filePath) {
    cv::Mat homography(3, 3, CV_64F);
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for reading: " + filePath);
    }

    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            file >> homography.at<double>(i, j);
        }
    }

    file.close();
    return homography;
}

void LockedInKeypoints::displayLockedInKeypoints(const std::string& dataFolderPath) {
    if (!fs::exists(dataFolderPath) || !fs::is_directory(dataFolderPath)) {
        throw std::runtime_error("Invalid data folder path: " + dataFolderPath);
    }

    for (const auto& entry : fs::directory_iterator(dataFolderPath)) {
        const fs::path subfolderPath = entry.path();
        if (fs::is_directory(subfolderPath)) {
            const std::string subfolderName = subfolderPath.filename().generic_string();
            const fs::path referenceKeypointsFolder = fs::path(KEYPOINTS_PATH) / subfolderName;

            // Create a vector to store the image filenames
            std::vector<std::string> imageFilenames;
            for (int i = 1; i <= 6; i++) {
                fs::path imageFilename = subfolderPath / (std::to_string(i) + ".ppm");
                imageFilenames.push_back(imageFilename.generic_string());
            }

            // Create a vector to store the image matrices and keypoints
            std::vector<std::pair<cv::Mat, std::vector<cv::KeyPoint>>> imagesWithKeypoints;

            // Process each image and store the image matrix and keypoints
            for (size_t i = 0; i < imageFilenames.size(); i++) {
                const std::string& imageFilename = imageFilenames[i];
                cv::Mat image = cv::imread(imageFilename, cv::IMREAD_COLOR);

                if (image.empty()) {
                    std::cerr << "Failed to read image: " << imageFilename << std::endl;
                    continue;
                }

                fs::path keypointsFilename = referenceKeypointsFolder / (std::to_string(i + 1) + "ppm.csv");
                std::vector<cv::KeyPoint> keypoints = readKeypointsFromCSV(keypointsFilename.generic_string());

                imagesWithKeypoints.emplace_back(image, keypoints);
            }

            // Display each image with keypoints in a separate window
            for (size_t i = 0; i < imagesWithKeypoints.size(); i++) {
                const auto& imageWithKeypoints = imagesWithKeypoints[i];
                const cv::Mat& image = imageWithKeypoints.first;
                const std::vector<cv::KeyPoint>& keypoints = imageWithKeypoints.second;

                cv::Mat imageWithKeypointsDisplay = image.clone();
                cv::drawKeypoints(image, keypoints, imageWithKeypointsDisplay, cv::Scalar::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

                std::string windowName = subfolderName + " - Image " + std::to_string(i + 1) + " - Locked-In Keypoints";
                cv::imshow(windowName, imageWithKeypointsDisplay);
            }

            cv::waitKey(0); // Wait for a key press to continue

            // Close all the displayed image windows
            for (size_t i = 0; i < imagesWithKeypoints.size(); i++) {
                std::string windowName = subfolderName + " - Image " + std::to_string(i + 1) + " - Locked-In Keypoints";
                cv::destroyWindow(windowName);
            }
        }
    }
}

void LockedInKeypoints::displayLockedInKeypointsBorder(const std::string& dataFolderPath) {
    if (!fs::exists(dataFolderPath) || !fs::is_directory(dataFolderPath)) {
        throw std::runtime_error("Invalid data folder path: " + dataFolderPath);
    }

    for (const auto& entry : fs::directory_iterator(dataFolderPath)) {
        const fs::path subfolderPath = entry.path();
        if (fs::is_directory(subfolderPath)) {
            const std::string subfolderName = subfolderPath.filename().generic_string();
            const fs::path referenceKeypointsFolder = fs::path(KEYPOINTS_PATH) / subfolderName;

            // Create a vector to store the image filenames
            std::vector<std::string> imageFilenames;
            for (int i = 1; i <= 6; i++) {
                fs::path imageFilename = subfolderPath / (std::to_string(i) + ".ppm");
                imageFilenames.push_back(imageFilename.generic_string());
            }

            // Create a vector to store the image matrices and keypoints
            std::vector<std::pair<cv::Mat, std::vector<cv::KeyPoint>>> imagesWithKeypoints;

            // Process each image and store the image matrix and keypoints
            for (size_t i = 0; i < imageFilenames.size(); i++) {
                const std::string& imageFilename = imageFilenames[i];
                cv::Mat image = cv::imread(imageFilename, cv::IMREAD_COLOR);

                if (image.empty()) {
                    std::cerr << "Failed to read image: " << imageFilename << std::endl;
                    continue;
                }

                fs::path keypointsFilename = referenceKeypointsFolder / (std::to_string(i + 1) + "ppm.csv");
                std::vector<cv::KeyPoint> keypoints = readKeypointsFromCSV(keypointsFilename.generic_string());

                imagesWithKeypoints.emplace_back(image, keypoints);
            }

            // Display each image with keypoints in a separate window
            for (size_t i = 0; i < imagesWithKeypoints.size(); i++) {
                const auto& imageWithKeypoints = imagesWithKeypoints[i];
                const cv::Mat& image = imageWithKeypoints.first;
                const std::vector<cv::KeyPoint>& keypoints = imageWithKeypoints.second;

                cv::Mat imageWithKeypointsDisplay = image.clone();
                cv::drawKeypoints(image, keypoints, imageWithKeypointsDisplay, cv::Scalar::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

                // Draw a 65x65 pixel square centered on each keypoint
                for (const auto& keypoint : keypoints) {
                    cv::Point2f pt = keypoint.pt;
                    cv::rectangle(imageWithKeypointsDisplay,
                                  cv::Point(pt.x - 32.5, pt.y - 32.5),
                                  cv::Point(pt.x + 32.5, pt.y + 32.5),
                                  cv::Scalar(0, 255, 0), 2);
                }

                std::string windowName = subfolderName + " - Image " + std::to_string(i + 1) + " - Locked-In Keypoints";
                cv::imshow(windowName, imageWithKeypointsDisplay);
            }

            cv::waitKey(0); // Wait for a key press to continue

            // Close all the displayed image windows
            for (size_t i = 0; i < imagesWithKeypoints.size(); i++) {
                std::string windowName = subfolderName + " - Image " + std::to_string(i + 1) + " - Locked-In Keypoints";
                cv::destroyWindow(windowName);
            }
        }
    }
}


std::vector<cv::KeyPoint> LockedInKeypoints::readKeypointsFromCSV(const std::string& filePath) {
    std::vector<cv::KeyPoint> keypoints;
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for reading: " + filePath);
    }

    std::string line;
    std::getline(file, line); // Skip the header line

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string value;

        cv::KeyPoint keypoint;
        std::getline(ss, value, ',');
        keypoint.pt.x = std::stof(value);
        std::getline(ss, value, ',');
        keypoint.pt.y = std::stof(value);
        std::getline(ss, value, ',');
        keypoint.size = std::stof(value);
        std::getline(ss, value, ',');
        keypoint.angle = std::stof(value);
        std::getline(ss, value, ',');
        keypoint.response = std::stof(value);
        std::getline(ss, value, ',');
        keypoint.octave = std::stoi(value);
        std::getline(ss, value);
        keypoint.class_id = std::stoi(value);

        keypoints.push_back(keypoint);
    }

    file.close();
    return keypoints;
}
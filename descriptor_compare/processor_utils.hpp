#ifndef DESCRIPTOR_COMPARE_PROCESSOR_UTILS_HPP
#define DESCRIPTOR_COMPARE_PROCESSOR_UTILS_HPP

#include <opencv2/features2d.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <random>
#include "experiment_config.hpp"

class processor_utils {

public:
    static cv::Mat applyGaussianNoise(const cv::Mat& image, double mean, double stddev);

    static std::pair<std::vector<cv::KeyPoint>, cv::Mat> detectAndCompute(cv::Ptr<cv::Feature2D> detector, const cv::Mat& image);

    static void sumPoolingDetectAndCompute(const cv::Ptr<cv::Feature2D>& detector, const cv::Mat& image,
                                           std::vector<cv::KeyPoint>& keypoints, cv::Mat& descriptors);

    static void rootDescriptors(cv::Mat& descriptors);

    static std::vector<cv::DMatch> matchDescriptors(const cv::Mat& descriptors1, const cv::Mat& descriptors2) ;

    static void saveResults(const std::string& filePath, const std::vector<std::string>& headers,
                            const std::vector<std::vector<std::string>>& dataRows);

    static void saveKeypointsToCSV(const std::string& filePath, const std::vector<cv::KeyPoint>& keypoints);

    static void saveDescriptorsToCSV(const std::string& filePath, const cv::Mat& descriptors);

    static cv::Mat readHomography(const std::string& filePath);


    static cv::Mat computeDSPDescriptor(const cv::Mat &image, const std::vector<cv::KeyPoint> &keypoints,
                                 const cv::Ptr<cv::Feature2D> &featureExtractor, const experiment_config &config);

    static std::pair<std::vector<cv::KeyPoint>, cv::Mat>
    detectAndComputeWithConfig(const cv::Mat &image, const experiment_config &config);

    static double calculateRelativeScalingFactor(const cv::Mat &image);

    static double adjustMatchThresholdForImageSet(double baseThreshold, double scaleFactor);

    static double calculatePrecision(const std::vector<cv::DMatch> &matches, const std::vector<cv::KeyPoint> &keypoints2,
                              const std::vector<cv::Point2f> &projectedPoints, double matchThreshold);

    static cv::Mat computeStackedDescriptor(const cv::Mat &image, std::vector<cv::KeyPoint> &keypoints,
                                     const experiment_config &config);

    static std::pair<std::vector<cv::KeyPoint>, cv::Mat>
    detectAndComputeWithConfigLocked(const cv::Mat &image, const std::vector<cv::KeyPoint> &lockedKeypoints,
                               const experiment_config &config);
};


#endif //DESCRIPTOR_COMPARE_PROCESSOR_UTILS_HPP

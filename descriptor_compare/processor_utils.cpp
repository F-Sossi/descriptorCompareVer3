#include "processor_utils.hpp"
#include "experiment_config.hpp"
#include "../keypoints/VanillaSIFT.h"
#include <opencv2/features2d.hpp>
#include <opencv2/core.hpp> // For cv::Ptr and core functionalities

// Assume baseWidth and baseHeight represent the dimensions of your base resolution
double baseWidth = 800.0; // Example base width
double baseHeight = 600.0; // Example base height

double processor_utils::calculateRelativeScalingFactor(const cv::Mat& image) {
    double scaleFactorWidth = image.cols / baseWidth;
    double scaleFactorHeight = image.rows / baseHeight;
    // You can use either width or height or a combination based on your application's needs
    // Here we take the average to accommodate both dimensions
    double averageScaleFactor = (scaleFactorWidth + scaleFactorHeight) / 2.0;
    return averageScaleFactor;
}

double processor_utils::adjustMatchThresholdForImageSet(double baseThreshold, double scaleFactor) {
    // Adjust the threshold based on the scale factor
    return baseThreshold * scaleFactor;
}

cv::Mat processor_utils::applyGaussianNoise(const cv::Mat& image, double mean, double stddev) {
    cv::Mat noisyImage = image.clone();
    cv::Mat noise(image.size(), image.type());

    cv::randn(noise, mean, stddev); // Generate Gaussian noise
    noisyImage += noise; // Add the noise to the original image

    return noisyImage;
}

std::pair<std::vector<cv::KeyPoint>, cv::Mat> processor_utils::detectAndCompute(const cv::Ptr<cv::Feature2D>& detector, const cv::Mat& image) {
    std::vector<cv::KeyPoint> keypoints;
    cv::Mat descriptors;
    detector->detectAndCompute(image, cv::noArray(), keypoints, descriptors);
    return std::make_pair(keypoints, descriptors);
}

void processor_utils::sumPoolingDetectAndCompute(const cv::Ptr<cv::Feature2D>& detector,
                                       const cv::Mat& image,
                                       std::vector<cv::KeyPoint>& keypoints,
                                       cv::Mat& descriptors) {
}

void processor_utils::rootDescriptors(cv::Mat& descriptors) {
    for (int i = 0; i < descriptors.rows; ++i) {
        for (int j = 0; j < descriptors.cols; ++j) {
            descriptors.at<float>(i, j) = std::sqrt(descriptors.at<float>(i, j));
        }
    }
}

std::vector<cv::DMatch> processor_utils::matchDescriptors(const cv::Mat& descriptors1, const cv::Mat& descriptors2) {
    cv::BFMatcher matcher(cv::NORM_L2, true);
    std::vector<cv::DMatch> matches;
    matcher.match(descriptors1, descriptors2, matches);
    return matches;
}

double processor_utils::calculatePrecision(const std::vector<cv::DMatch>& matches,
                                           const std::vector<cv::KeyPoint>& keypoints2,
                                           const std::vector<cv::Point2f>& projectedPoints,
                                           double matchThreshold) {
    int truePositives = 0;
    for (const auto& match : matches) {
        // Calculate the distance between the projected point and the corresponding match point in the second image
        if (cv::norm(projectedPoints[match.queryIdx] - keypoints2[match.trainIdx].pt) <= matchThreshold) {
            truePositives++;
        }
    }
    // Calculate precision
    return matches.empty() ? 0 : static_cast<double>(truePositives) / matches.size();
}

void processor_utils::saveResults(const std::string& filePath, const std::vector<std::string>& headers, const std::vector<std::vector<std::string>>& dataRows) {
    std::ofstream file;

    // Check if the file already exists
    const bool fileExists = std::ifstream(filePath).good();

    // Open the file in append mode
    file.open(filePath, std::ios::out | std::ios::app);

    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file: " + filePath);
    }

    // Write headers only if the file doesn't exist
    if (!fileExists) {
        for (std::size_t i = 0; i < headers.size(); ++i) {
            file << headers[i];
            if (i < headers.size() - 1) file << ",";
        }
        file << "\n";
    }

    // Write data rows
    for (const auto& row : dataRows) {
        for (std::size_t i = 0; i < row.size(); ++i) {
            file << row[i];
            if (i < row.size() - 1) file << ",";
        }
        file << "\n";
    }

    file.close();
}

void processor_utils::saveKeypointsToCSV(const std::string& filePath, const std::vector<cv::KeyPoint>& keypoints) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return;
    }

    // Write header
    file << "x,y,size,angle,response,octave,class_id" << std::endl;

    // Write keypoints
    for (const auto& kp : keypoints) {
        file << kp.pt.x << "," << kp.pt.y << "," << kp.size << "," << kp.angle << "," << kp.response << "," << kp.octave << "," << kp.class_id << std::endl;
    }

    file.close();
}

void processor_utils::saveDescriptorsToCSV(const std::string& filePath, const cv::Mat& descriptors) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return;
    }

    // Write descriptors
    for (int row = 0; row < descriptors.rows; ++row) {
        std::stringstream ss;
        for (int col = 0; col < descriptors.cols; ++col) {
            ss << descriptors.at<float>(row, col);
            if (col < descriptors.cols - 1) {
                ss << ",";
            }
        }
        file << ss.str() << std::endl;
    }

    file.close();
}

cv::Mat processor_utils::readHomography(const std::string& filePath) {
    cv::Mat H = cv::Mat::zeros(3, 3, CV_64F);
    std::ifstream file(filePath);
    if (!file) {
        std::cerr << "Unable to open the homography file: " << filePath << "\n";
        exit(-1);
    }

    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            file >> H.at<double>(row, col);
        }
    }

    file.close();
    return H;
}

cv::Mat processor_utils::computeDSPDescriptor(const cv::Mat& image, const std::vector<cv::KeyPoint>& keypoints, const cv::Ptr<cv::Feature2D>& featureExtractor, const experiment_config& config) {
    cv::Mat sumOfDescriptors;

    cv::Mat im = image.clone();

    // Convert image to grayscale if needed
    if (config.descriptorOptions.descriptorColorSpace == D_BW) {
        cv::cvtColor(image, im, cv::COLOR_BGR2GRAY);
    }

    for (auto scale : config.descriptorOptions.scales) {
        cv::Mat im_scaled;
        cv::resize(im, im_scaled, cv::Size(), scale, scale);

        cv::Mat descriptors_scaled;
        std::vector<cv::KeyPoint> keypoints_scaled;
        for (const auto& kp : keypoints) {
            keypoints_scaled.emplace_back(cv::KeyPoint(kp.pt * scale, kp.size * scale));
        }

        // TODO: Need to modify VanillaSift implementation to work with updated Feature2D interface
        // Attempt to dynamically cast featureExtractor to a VanillaSIFT pointer this ia a bit of a kludge
        // however it is due to the differences in interfaces between vanilla SIFT based descriptors and the
        // Features2D interface.
        if (const cv::Ptr<VanillaSIFT> vanillaSiftExtractor = dynamic_pointer_cast<VanillaSIFT>(featureExtractor)) {
            // featureExtractor is pointing to a VanillaSIFT object
            vanillaSiftExtractor->compute(im_scaled, keypoints_scaled, descriptors_scaled);
        } else {
            // featureExtractor is pointing to a different class
            featureExtractor->compute(im_scaled, keypoints_scaled, descriptors_scaled);
        }

        if(config.descriptorOptions.normalizationStage == BEFORE_POOLING) {
            cv::normalize(descriptors_scaled, descriptors_scaled, 1, 0, config.descriptorOptions.normType);
        }

        if(config.descriptorOptions.rootingStage == R_BEFORE_POOLING) {
            rootDescriptors(descriptors_scaled);
        }

        if (sumOfDescriptors.empty()) {
            sumOfDescriptors = cv::Mat::zeros(descriptors_scaled.rows, descriptors_scaled.cols, descriptors_scaled.type());
        }

        sumOfDescriptors += descriptors_scaled;
    }

    if(config.descriptorOptions.normalizationStage == AFTER_POOLING) {
        cv::normalize(sumOfDescriptors, sumOfDescriptors, 1, 0, config.descriptorOptions.normType);
    }
    return sumOfDescriptors;
}

cv::Mat processor_utils::computeStackedDescriptor(const cv::Mat& image, std::vector<cv::KeyPoint>& keypoints, const experiment_config& config) {
    cv::Mat descriptor1;
    cv::Mat descriptor2;
    cv::Mat stackedDescriptor;
    cv::Ptr<cv::Feature2D> detector1 = config.detector;
    cv::Ptr<cv::Feature2D> detector2 = config.detector2;

    // Set temp image to the correct colorspace
    cv::Mat image1 = image.clone();

    // Convert image to grayscale if needed
    if (config.descriptorOptions.descriptorColorSpace == D_BW) {
        cv::cvtColor(image, image1, cv::COLOR_BGR2GRAY);
    }
    // Compute first descriptor
    // TODO: Need to modify VanillaSift implementation to work with updated Feature2D interface
    // Attempt to dynamically cast featureExtractor to a VanillaSIFT pointer this ia a bit of a kludge
    // however it is due to the differences in interfaces between vanilla SIFT based descriptors and the
    // Features2D interface.
    if (cv::Ptr<VanillaSIFT> vanillaSiftExtractor = dynamic_pointer_cast<VanillaSIFT>(detector1)) {
        // featureExtractor is pointing to a VanillaSIFT object
        vanillaSiftExtractor->compute(image1, keypoints, descriptor1);
    } else {
        // featureExtractor is pointing to a different class
        detector1->compute(image1, keypoints, descriptor1);
    }

    cv::Mat image2 = image.clone();

    // Convert image to grayscale if needed
    if (config.descriptorOptions.descriptorColorSpace2 == D_BW) {
        cv::cvtColor(image, image2, cv::COLOR_BGR2GRAY);
    }

    // Compute second descriptor with detector 2
    if (cv::Ptr<VanillaSIFT> vanillaSiftExtractor2 = dynamic_pointer_cast<VanillaSIFT>(detector2)) {
        // featureExtractor is pointing to a VanillaSIFT object
        vanillaSiftExtractor2->compute(image2, keypoints, descriptor2);
    } else {
        // featureExtractor is pointing to a different class
        detector2->compute(image2, keypoints, descriptor2);
    }

    // combine the two descriptors descriptor1 + descriptor2
    cv::hconcat(descriptor1, descriptor2, stackedDescriptor);

    return stackedDescriptor;
}

std::pair<std::vector<cv::KeyPoint>, cv::Mat> processor_utils::detectAndComputeWithConfigLocked(const cv::Mat& image, const std::vector<cv::KeyPoint>& lockedKeypoints, const experiment_config &config) {
    std::pair<std::vector<cv::KeyPoint>, cv::Mat> result;

    if (config.descriptorOptions.UseLockedInKeypoints) {
        // Use the provided locked-in keypoints
        result.first = lockedKeypoints;
    } else {
        // Detect keypoints using the specified detector
        result = processor_utils::detectAndCompute(config.detector, image);
    }

    switch (config.descriptorOptions.poolingStrategy) {
        case NONE: {

            cv::Mat im = image.clone();

            // Convert image to grayscale if needed
            if (config.descriptorOptions.descriptorColorSpace == D_BW) {
                cv::cvtColor(image, im, cv::COLOR_BGR2GRAY);
            }
            // Compute first descriptor
            // TODO: Need to modify VanillaSift implementation to work with updated Feature2D interface
            // Attempt to dynamically cast featureExtractor to a VanillaSIFT pointer this ia a bit of a kludge
            // however it is due to the differences in interfaces between vanilla SIFT based descriptors and the
            // Features2D interface.
            if (const cv::Ptr<VanillaSIFT> vanillaSiftExtractor = dynamic_pointer_cast<VanillaSIFT>(config.detector)) {
                // featureExtractor is pointing to a VanillaSIFT object
                vanillaSiftExtractor->compute(im, result.first, result.second);
            } else {
                // featureExtractor is pointing to a different class
                config.detector->compute(im, result.first, result.second);
            }
            break;
        }
        case DOMAIN_SIZE_POOLING: {
            // Compute descriptors, then apply domain size pooling
            result.second = processor_utils::computeDSPDescriptor(image, result.first, config.detector, config);
            break;
        }
        case STACKING: {
            // Compute descriptors, then apply stacking
            result.second = processor_utils::computeStackedDescriptor(image, result.first, config);
            break;
        }
        default:
            std::cerr << "Unsupported or invalid pooling strategy: " << config.descriptorOptions.poolingStrategy << std::endl;
            break;
    }

    return result;
}

std::pair<std::vector<cv::KeyPoint>, cv::Mat> processor_utils::detectAndComputeWithConfig(const cv::Mat& image, const experiment_config &config) {

    std::pair<std::vector<cv::KeyPoint>, cv::Mat> result;
    switch (config.descriptorOptions.poolingStrategy) {
        case NONE: {
            // Directly detect and compute without pooling
            result = processor_utils::detectAndCompute(config.detector, image);
            break;
        }
        case DOMAIN_SIZE_POOLING: {
            // Detect and compute, then apply domain size pooling
            result = processor_utils::detectAndCompute(config.detector, image);
            result.second = processor_utils::computeDSPDescriptor(image, result.first, config.detector, config);
            break;
        }
        case STACKING: {
            // Detect and compute, then apply stacking
            result = processor_utils::detectAndCompute(config.detector, image);
            result.second = processor_utils::computeStackedDescriptor(image, result.first, config);
            break;
        }
        default:
            std::cerr << "Unsupported or invalid pooling strategy: " << config.descriptorOptions.poolingStrategy << std::endl;
            break;
    }

    return result;
}

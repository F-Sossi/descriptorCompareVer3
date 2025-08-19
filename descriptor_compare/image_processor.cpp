#include <boost/filesystem.hpp>
#include <iostream>
#include <vector>
#include <future>
#include <string>
#include <chrono>

#include "experiment_config.hpp"
#include "image_processor.hpp"
#include "locked_in_keypoints.hpp"
#ifdef BUILD_DATABASE
#include "thesis_project/database/DatabaseManager.hpp"
#endif
#include "thesis_project/types.hpp"
#include "src/core/pooling/PoolingFactory.hpp"

namespace fs = boost::filesystem;
// TODO: Add another method to visual check the locked in keypoints

// Helper functions for descriptive database strings (research-friendly)
std::string descriptorTypeToDescriptiveString(DescriptorType type) {
    switch (type) {
        case DESCRIPTOR_SIFT: return "sift";
        case DESCRIPTOR_vSIFT: return "vanilla_sift";
        case DESCRIPTOR_RGBSIFT: return "rgb_sift";
        case DESCRIPTOR_HoNC: return "histogram_of_normalized_colors";
        case NO_DESCRIPTOR: return "no_descriptor";
        default: return "unknown_descriptor";
    }
}

std::string poolingStrategyToDescriptiveString(PoolingStrategy strategy) {
    switch (strategy) {
        case NONE: return "no_pooling";
        case DOMAIN_SIZE_POOLING: return "domain_size_pooling";
        case STACKING: return "stacking";
        default: return "unknown_pooling";
    }
}

std::string normalizationStageToDescriptiveString(NormalizationStage stage) {
    switch (stage) {
        case BEFORE_POOLING: return "before_pooling";
        case AFTER_POOLING: return "after_pooling";
        case NO_NORMALIZATION: return "no_normalization";
        default: return "unknown_normalization";
    }
}

std::string rootingStageToDescriptiveString(RootingStage stage) {
    switch (stage) {
        case R_BEFORE_POOLING: return "rooting_before_pooling";
        case R_AFTER_POOLING: return "rooting_after_pooling";
        case R_NONE: return "no_rooting";
        default: return "unknown_rooting";
    }
}

std::string descriptorColorSpaceToDescriptiveString(DescriptorColorSpace colorSpace) {
    switch (colorSpace) {
        case D_COLOR: return "color";
        case D_BW: return "grayscale";
        default: return "unknown_color_space";
    }
}

std::string normTypeToDescriptiveString(int normType) {
    switch (normType) {
        case cv::NORM_L1: return "l1_norm";
        case cv::NORM_L2: return "l2_norm";
        default: return "unknown_norm_type";
    }
}

cv::Scalar getKeypointColor(size_t index) {
    static const std::vector<cv::Scalar> colors = {
            cv::Scalar(255, 0, 0),     // Blue
            cv::Scalar(0, 255, 0),     // Green
            cv::Scalar(0, 0, 255),     // Red
            cv::Scalar(255, 255, 0),   // Cyan
            cv::Scalar(255, 0, 255),   // Magenta
            cv::Scalar(0, 255, 255),   // Yellow
            // Add more colors if needed
    };
    return colors[index % colors.size()];
}

ExperimentMetrics image_processor::process_directory(const std::string &data_folder, const std::string &results_folder, const experiment_config &config) {
    ExperimentMetrics overall_metrics;
    auto start_time = std::chrono::high_resolution_clock::now();
    try {
        if (!fs::exists(data_folder) || !fs::is_directory(data_folder)) {
            std::cerr << "Invalid data folder: " << data_folder << std::endl;
            overall_metrics.success = false;
            overall_metrics.error_message = "Invalid data folder: " + data_folder;
            return overall_metrics;
        }

        if (!fs::exists(results_folder)) {
            fs::create_directories(results_folder);
        }

        std::vector<std::future<ExperimentMetrics>> futures;
        std::vector<ExperimentMetrics> folder_metrics;

        for (const auto &entry : fs::directory_iterator(data_folder)) {
            const auto &path = entry.path();
            if (fs::is_directory(path)) {
                std::string subfolder = path.filename().string();
                std::string results_subfolder = results_folder;
                results_subfolder.append("/").append(subfolder);

                // Asynchronous execution path using std::async
                if (config.useMultiThreading) {
                    auto fut = std::async(std::launch::async, [&, path, results_subfolder, subfolder]() -> ExperimentMetrics {
                        try {
                            if(config.descriptorOptions.UseLockedInKeypoints) {
                                return process_image_folder_keypoints_locked(path.string(), results_subfolder, config);
                            } else {
                                return process_image_folder_keypoints_unlocked(path.string(), results_subfolder, config);
                            }
                        } catch (const std::exception &e) {
                            std::cerr << "Exception in async task: " << e.what() << std::endl;
                            ExperimentMetrics error_metrics;
                            error_metrics.success = false;
                            error_metrics.error_message = "Exception in async task: " + std::string(e.what());
                            return error_metrics;
                        } catch (...) {
                            std::cerr << "Unknown exception in async task" << std::endl;
                            ExperimentMetrics error_metrics;
                            error_metrics.success = false;
                            error_metrics.error_message = "Unknown exception in async task";
                            return error_metrics;
                        }
                    });
                    futures.push_back(std::move(fut));
                }
                    // Synchronous (sequential) execution path
                else {
                    try {
                        ExperimentMetrics folder_result;
                        if (config.verificationType == MATCHES) {
                            visual_verification_matches(path.string(), results_subfolder, config);
                            // Visual verification doesn't return metrics, mark as successful but no data
                            folder_result.success = true;
                        } else if (config.verificationType == HOMOGRAPHY) {
                            visual_verification_homography(path.string(), results_subfolder, config);
                            // Visual verification doesn't return metrics, mark as successful but no data
                            folder_result.success = true;
                        } else {
                            if(config.descriptorOptions.UseLockedInKeypoints) {
                                folder_result = process_image_folder_keypoints_locked(path.string(), results_subfolder, config);
                            } else {
                                folder_result = process_image_folder_keypoints_unlocked(path.string(), results_subfolder, config);
                            }
                        }
                        folder_metrics.push_back(folder_result);
                    } catch (const std::exception &e) {
                        std::cerr << "Exception in synchronous task: " << e.what() << std::endl;
                        ExperimentMetrics error_metrics;
                        error_metrics.success = false;
                        error_metrics.error_message = "Exception in synchronous task: " + std::string(e.what());
                        folder_metrics.push_back(error_metrics);
                    } catch (...) {
                        std::cerr << "Unknown exception in synchronous task" << std::endl;
                        ExperimentMetrics error_metrics;
                        error_metrics.success = false;
                        error_metrics.error_message = "Unknown exception in synchronous task";
                        folder_metrics.push_back(error_metrics);
                    }
                }
            }
        }

        // Wait for all asynchronous tasks to complete and collect results
        if (config.useMultiThreading) {
            for (auto &fut : futures) {
                try {
                    ExperimentMetrics result = fut.get(); // This blocks until the future's task is complete
                    folder_metrics.push_back(result);
                } catch (const std::exception &e) {
                    std::cerr << "Exception caught during future.get(): " << e.what() << std::endl;
                    ExperimentMetrics error_metrics;
                    error_metrics.success = false;
                    error_metrics.error_message = "Exception during future.get(): " + std::string(e.what());
                    folder_metrics.push_back(error_metrics);
                } catch (...) {
                    std::cerr << "Unknown exception caught during future.get()" << std::endl;
                    ExperimentMetrics error_metrics;
                    error_metrics.success = false;
                    error_metrics.error_message = "Unknown exception during future.get()";
                    folder_metrics.push_back(error_metrics);
                }
            }
        }

        // Aggregate all folder metrics into overall metrics
        overall_metrics.success = true;
        for (const auto& folder_metric : folder_metrics) {
            if (!folder_metric.success) {
                overall_metrics.success = false;
                if (!overall_metrics.error_message.empty()) {
                    overall_metrics.error_message += "; ";
                }
                overall_metrics.error_message += folder_metric.error_message;
            }
            
            // Aggregate metrics
            for (double precision : folder_metric.precisions_per_image) {
                overall_metrics.precisions_per_image.push_back(precision);
            }
            overall_metrics.total_matches += folder_metric.total_matches;
            overall_metrics.total_keypoints += folder_metric.total_keypoints;
            overall_metrics.total_images_processed += folder_metric.total_images_processed;
            
            // Merge per-scene statistics
            for (const auto& [scene, precision] : folder_metric.per_scene_precision) {
                overall_metrics.per_scene_precision[scene] = precision;
                overall_metrics.per_scene_matches[scene] = folder_metric.per_scene_matches.at(scene);
                overall_metrics.per_scene_keypoints[scene] = folder_metric.per_scene_keypoints.at(scene);
            }
        }
        
        // Calculate final metrics
        overall_metrics.calculateMeanPrecision();
        
        // Calculate processing time
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        overall_metrics.processing_time_ms = duration.count();

        return overall_metrics;
    } catch (const std::exception &e) {
        std::cerr << "Error processing directory: " << e.what() << std::endl;
        overall_metrics.success = false;
        overall_metrics.error_message = "Error processing directory: " + std::string(e.what());
        return overall_metrics;
    }
}

ExperimentMetrics image_processor::process_image_folder_keypoints_locked(const std::string &folder, const std::string &results_folder,
                                                            const experiment_config &config) {
    ExperimentMetrics metrics;
    std::string scene_name = boost::filesystem::path(folder).filename().string();
    std::cout << "Processing locked folder: " << folder << "\nResults folder: " << results_folder << std::endl;

    boost::filesystem::create_directories(results_folder);

    // Read in 1.ppm as the base image
    cv::Mat image1 = cv::imread(folder + "/1.ppm", cv::IMREAD_COLOR);

    if (image1.empty()) {
        std::cerr << "Failed to read image: " << folder + "/1.ppm" << std::endl;
        metrics.success = false;
        metrics.error_message = "Failed to read image: " + folder + "/1.ppm";
        return metrics;
    }

    // Load locked-in keypoints for image 1
    std::vector<cv::KeyPoint> keypoints1;
    
#ifdef BUILD_DATABASE
    // Load keypoints from database only (no CSV fallback)
    thesis_project::database::DatabaseManager db("experiments.db", true);
    keypoints1 = db.getLockedKeypoints(scene_name, "1.ppm");
    
    if (keypoints1.empty()) {
        std::cerr << "[ERROR] No keypoints found in database for " << scene_name << "/1.ppm. Use keypoint_manager to generate keypoints first." << std::endl;
        return ExperimentMetrics(); // Return empty metrics
    }
    std::cout << "[INFO] Loaded " << keypoints1.size() << " keypoints from database for " << scene_name << "/1.ppm" << std::endl;
#else
    std::cerr << "[ERROR] Database support not enabled. Build with -DBUILD_DATABASE=ON" << std::endl;
    return ExperimentMetrics(); // Return empty metrics
#endif

    // Compute descriptors for image 1 using the locked-in keypoints
    std::pair<std::vector<cv::KeyPoint>, cv::Mat> result1 = processor_utils::detectAndComputeWithConfigLocked(image1, keypoints1, config);
    cv::Mat descriptors1 = result1.second;

#ifdef BUILD_DATABASE
    // Store descriptors in database for research analysis
    if (db.isEnabled() && !descriptors1.empty() && config.experiment_id != -1) {
        // Build descriptive processing method string for database queries
        std::string processing_method = descriptorTypeToDescriptiveString(config.descriptorOptions.descriptorType) + "_" +
                                       descriptorColorSpaceToDescriptiveString(config.descriptorOptions.descriptorColorSpace) + "_" +
                                       poolingStrategyToDescriptiveString(config.descriptorOptions.poolingStrategy) + "_" +
                                       normalizationStageToDescriptiveString(config.descriptorOptions.normalizationStage) + "_" +
                                       rootingStageToDescriptiveString(config.descriptorOptions.rootingStage);
        
        // Store descriptors with experiment linking and descriptive metadata
        db.storeDescriptors(config.experiment_id, scene_name, "1.ppm", keypoints1, descriptors1, processing_method,
                           normalizationStageToDescriptiveString(config.descriptorOptions.normalizationStage),
                           rootingStageToDescriptiveString(config.descriptorOptions.rootingStage),
                           poolingStrategyToDescriptiveString(config.descriptorOptions.poolingStrategy));
    }
#endif

    // Save keypoints and descriptors of the first image
    if(config.descriptorOptions.recordKeypoints) {
        processor_utils::saveKeypointsToCSV(results_folder + "/keypoints_1.csv", keypoints1);
    }
    if(config.descriptorOptions.recordDescriptors){
        processor_utils::saveDescriptorsToCSV(results_folder + "/descriptors_1.csv", descriptors1);
    }

    for (int i = 2; i <= 6; i++) {
        std::string imagePath = folder + "/" + std::to_string(i) + ".ppm";
        cv::Mat image2 = cv::imread(imagePath, cv::IMREAD_COLOR);

        if (image2.empty()) continue;

        // Load locked-in keypoints for image i
        std::vector<cv::KeyPoint> keypoints2;
        std::string image_name = std::to_string(i) + ".ppm";
        
#ifdef BUILD_DATABASE
        // Load keypoints from database only (no CSV fallback)
        keypoints2 = db.getLockedKeypoints(scene_name, image_name);
        
        if (keypoints2.empty()) {
            std::cerr << "[ERROR] No keypoints found in database for " << scene_name << "/" << image_name << ". Use keypoint_manager to generate keypoints first." << std::endl;
            continue; // Skip this image
        }
        std::cout << "[INFO] Loaded " << keypoints2.size() << " keypoints from database for " << scene_name << "/" << image_name << std::endl;
#else
        std::cerr << "[ERROR] Database support not enabled. Build with -DBUILD_DATABASE=ON" << std::endl;
        return ExperimentMetrics(); // Return empty metrics
#endif

        // Compute descriptors for image i using the locked-in keypoints
        std::pair<std::vector<cv::KeyPoint>, cv::Mat> result2 = processor_utils::detectAndComputeWithConfigLocked(image2, keypoints2, config);
        cv::Mat descriptors2 = result2.second;

#ifdef BUILD_DATABASE
        // Store descriptors in database for research analysis
        if (db.isEnabled() && !descriptors2.empty() && config.experiment_id != -1) {
            // Build descriptive processing method string (same as image 1)
            std::string processing_method = descriptorTypeToDescriptiveString(config.descriptorOptions.descriptorType) + "_" +
                                           descriptorColorSpaceToDescriptiveString(config.descriptorOptions.descriptorColorSpace) + "_" +
                                           poolingStrategyToDescriptiveString(config.descriptorOptions.poolingStrategy) + "_" +
                                           normalizationStageToDescriptiveString(config.descriptorOptions.normalizationStage) + "_" +
                                           rootingStageToDescriptiveString(config.descriptorOptions.rootingStage);
            
            // Store descriptors with experiment linking and descriptive metadata
            db.storeDescriptors(config.experiment_id, scene_name, image_name, keypoints2, descriptors2, processing_method,
                               normalizationStageToDescriptiveString(config.descriptorOptions.normalizationStage),
                               rootingStageToDescriptiveString(config.descriptorOptions.rootingStage),
                               poolingStrategyToDescriptiveString(config.descriptorOptions.poolingStrategy));
        }
#endif

        // Save keypoints and descriptors for each image processed in the loop
        if(config.descriptorOptions.recordKeypoints) {
            processor_utils::saveKeypointsToCSV(results_folder + "/keypoints_" + std::to_string(i) + ".csv", keypoints2);
        }
        if(config.descriptorOptions.recordDescriptors){
            processor_utils::saveDescriptorsToCSV(results_folder + "/descriptors_" + std::to_string(i) + ".csv", descriptors2);
        }

        // Match descriptors
        std::vector<cv::DMatch> matches = processor_utils::matchDescriptors(descriptors1, descriptors2, config.matchingStrategy);

        /*
         * Geometric evaluation for homography-transformed keypoints:
         * 
         * Since keypoints are now properly transformed using homography matrices, we need to evaluate
         * matches based on spatial accuracy rather than index correspondence. The keypoints1 and keypoints2
         * represent the same physical features, but keypoints2 are transformed to the coordinate system
         * of image i.
         * 
         * For each descriptor match, we check if the matched keypoints are spatially close to their
         * expected transformed positions. This accounts for:
         * - Descriptor matching accuracy (how well descriptors represent the same feature)
         * - Geometric consistency (whether matched features are in the right spatial relationship)
         * - Transformation accuracy (how well the homography represents the actual viewpoint change)
         */

        // Evaluate precision using known keypoint correspondences
        // Since keypoints2 are transformed versions of keypoints1, we know the ground truth correspondences
        int correctMatches = 0;
        
        for (const auto& match : matches) {
            // Check if the descriptor matcher correctly identified the corresponding keypoints
            // match.queryIdx should correspond to match.trainIdx since they represent the same physical feature
            if (match.queryIdx == match.trainIdx) {
                correctMatches++;
            }
        }
        
        double precision = matches.empty() ? 0.0 : static_cast<double>(correctMatches) / matches.size();

        // Capture metrics (in addition to CSV for now)
        metrics.addImageResult(scene_name, precision, matches.size(), keypoints2.size());

        std::vector<std::string> headers = {"Image_Reference", "Precision"};
        processor_utils::saveResults(results_folder + "/results.csv", headers, {{std::to_string(i), std::to_string(precision)}});
    }

    // Finalize metrics calculation
    metrics.calculateMeanPrecision();
    metrics.success = true;
    return metrics;
}

ExperimentMetrics image_processor::process_image_folder_keypoints_unlocked(const std::string &folder, const std::string &results_folder,
                                                              const experiment_config &config) {
    ExperimentMetrics metrics;
    std::string scene_name = boost::filesystem::path(folder).filename().string();
    std::cout << "Processing folder: " << folder << "\nResults folder: " << results_folder << std::endl;

    boost::filesystem::create_directories(results_folder);

    // Read in 1.ppm as the base image
    cv::Mat image1 = cv::imread(folder + "/1.ppm", cv::IMREAD_COLOR);

    if (config.descriptorOptions.descriptorColorSpace == D_BW) {
        // convert image1 to greyscale
        cv::cvtColor(image1, image1, cv::COLOR_BGR2GRAY);
    }

    std::pair<std::vector<cv::KeyPoint>, cv::Mat> result1 = processor_utils::detectAndComputeWithConfig(image1, config);
    std::vector<cv::KeyPoint> keypoints1 = result1.first;
    cv::Mat descriptors1 = result1.second;

    // Save keypoints and descriptors of the first image
    if(config.descriptorOptions.recordKeypoints) {
        processor_utils::saveKeypointsToCSV(results_folder + "/keypoints_1.csv", keypoints1);
    }

    if(config.descriptorOptions.recordDescriptors){
        processor_utils::saveDescriptorsToCSV(results_folder + "/descriptors_1.csv", descriptors1);
    }


    for (int i = 2; i <= 6; i++) {
        std::string imagePath = folder + "/" + std::to_string(i) + ".ppm";
        cv::Mat image2 = cv::imread(imagePath, cv::IMREAD_COLOR);

        if (image2.empty()) continue;

        std::pair<std::vector<cv::KeyPoint>, cv::Mat> result2 = processor_utils::detectAndComputeWithConfig(image2, config);
        std::vector<cv::KeyPoint> keypoints2 = result2.first;
        cv::Mat descriptors2 = result2.second;

        // Save keypoints and descriptors for each image processed in the loop
        if(config.descriptorOptions.recordKeypoints) {
            processor_utils::saveKeypointsToCSV(results_folder + "/keypoints_" + std::to_string(i) + ".csv", keypoints2);
        }
        if(config.descriptorOptions.recordDescriptors){
            processor_utils::saveDescriptorsToCSV(results_folder + "/descriptors_" + std::to_string(i) + ".csv", descriptors2);
        }

        // Match normal descriptors
        std::vector<cv::DMatch> matches = processor_utils::matchDescriptors(descriptors1, descriptors2, config.matchingStrategy);
        std::sort(matches.begin(), matches.end(), [](const cv::DMatch& a, const cv::DMatch& b) {
            return a.distance < b.distance;
        });
        std::vector<cv::DMatch> topMatches = std::vector<cv::DMatch>(matches.begin(), matches.begin() + std::min(2000, (int)matches.size()));

        // Read the corresponding homography matrix
        std::string homographyFilename = folder + "/H_1_" + std::to_string(i);
        cv::Mat homography = processor_utils::readHomography(homographyFilename);

        // Project keypoints from the first image to the second using the homography matrix
        std::vector<cv::Point2f> points1;
        for (const auto& kp : keypoints1) {
            points1.push_back(kp.pt);
        }

        std::vector<cv::Point2f> projectedPoints;
        cv::perspectiveTransform(points1, projectedPoints, homography);

        double scaleFactor = processor_utils::calculateRelativeScalingFactor(image1);
        double adjustedThreshold = processor_utils::adjustMatchThresholdForImageSet(config.matchThreshold, scaleFactor);

        // Calculate precision
        double precision = processor_utils::calculatePrecision(topMatches, keypoints2, projectedPoints, adjustedThreshold);

        // Capture metrics (in addition to CSV for now)
        metrics.addImageResult(scene_name, precision, topMatches.size(), keypoints2.size());

        std::vector<std::string> headers = {"Image_Reference", "Precision"};
        processor_utils::saveResults(results_folder + "/results.csv", headers, {{std::to_string(i), std::to_string(precision)}});

    }

    // Finalize metrics calculation
    metrics.calculateMeanPrecision();
    metrics.success = true;
    return metrics;
}

// TODO: Reach goal fix this so you can compare different modifications (not needed but nice to have)
void image_processor::visual_verification_matches(const std::string &folder, const std::string &results_folder, const experiment_config &config) {
    std::cout << "Processing folder: " << folder << "\nResults folder: " << results_folder << std::endl;

    // Read in 1.ppm as the base image
    cv::Mat image1 = cv::imread(folder + "/1.ppm", cv::IMREAD_COLOR);
    if (config.descriptorOptions.descriptorColorSpace == D_BW) {
        cv::cvtColor(image1, image1, cv::COLOR_BGR2RGB);
    }

    // Use detector from config to detect keypoints in image 1 using the processor utils detect and compute function
    std::pair<std::vector<cv::KeyPoint>, cv::Mat> result1 = processor_utils::detectAndCompute(config.detector, image1);
    std::vector<cv::KeyPoint> keypoints1 = result1.first;
    cv::Mat descriptors1 = result1.second;

    // Compute DSP descriptors for image 1 using factory system
    auto dspPoolingStrategy = thesis_project::pooling::PoolingFactory::createStrategy(DOMAIN_SIZE_POOLING);
    cv::Mat dspDescriptors1 = dspPoolingStrategy->computeDescriptors(image1, keypoints1, config.detector, config);

    for (int i = 2; i <= 6; i++) {
        std::string imagePath = folder + "/" + std::to_string(i) + ".ppm";
        cv::Mat image2 = cv::imread(imagePath, cv::IMREAD_COLOR);

        if (image2.empty()) continue;
        if (config.descriptorOptions.descriptorColorSpace == D_BW) {
            cv::cvtColor(image2, image2, cv::COLOR_BGR2RGB);
        }

        // Detect and compute for image 2
        std::pair<std::vector<cv::KeyPoint>, cv::Mat> result2 = processor_utils::detectAndCompute(config.detector, image2);
        std::vector<cv::KeyPoint>keypoints2 = result2.first;
        cv::Mat descriptors2 = result2.second;

        // Compute DSP descriptors for image 2 using factory system
        cv::Mat dspDescriptors2 = dspPoolingStrategy->computeDescriptors(image2, keypoints2, config.detector, config);

        // Match normal descriptors
        std::vector<cv::DMatch> matches = processor_utils::matchDescriptors(descriptors1, descriptors2, config.matchingStrategy);
        std::sort(matches.begin(), matches.end(), [](const cv::DMatch& a, const cv::DMatch& b) {
            return a.distance < b.distance;
        });
        std::vector<cv::DMatch> topMatches = std::vector<cv::DMatch>(matches.begin(), matches.begin() + std::min(100, (int)matches.size()));

        // Match DSP descriptors
        std::vector<cv::DMatch> dspMatches = processor_utils::matchDescriptors(dspDescriptors1, dspDescriptors2, config.matchingStrategy);
        std::sort(dspMatches.begin(), dspMatches.end(), [](const cv::DMatch& a, const cv::DMatch& b) {
            return a.distance < b.distance;
        });
        std::vector<cv::DMatch> topDspMatches = std::vector<cv::DMatch>(dspMatches.begin(), dspMatches.begin() + std::min(50, (int)dspMatches.size()));

        // Visual comparison
        cv::Mat img_matches, dsp_img_matches;
        cv::drawMatches(image1, keypoints1, image2, keypoints2, topMatches, img_matches, cv::Scalar::all(-1),
                        cv::Scalar::all(-1), std::vector<char>(), cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
        cv::drawMatches(image1, keypoints1, image2, keypoints2, topDspMatches, dsp_img_matches, cv::Scalar::all(-1),
                        cv::Scalar::all(-1), std::vector<char>(), cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

        // Show both matches side by side
        cv::imshow("Normal Descriptors Matches", img_matches);
        cv::imshow("DSP Descriptors Matches", dsp_img_matches);
        cv::waitKey(0); // Wait for a key press

        // Destroy the windows to clean up and move to the next iteration
        cv::destroyWindow("Normal Descriptors Matches");
        cv::destroyWindow("DSP Descriptors Matches");
    }
}

void image_processor::visual_verification_homography(const std::string& folder, const std::string& results_folder, const experiment_config& config) {
    // Read in 1.ppm as the reference image
    cv::Mat referenceImage = cv::imread(folder + "/1.ppm", cv::IMREAD_COLOR);

    // Detect keypoints and compute descriptors for the reference image
    std::vector<cv::KeyPoint> referenceKeypoints;
    cv::Mat referenceDescriptors;
    config.detector->detectAndCompute(referenceImage, cv::noArray(), referenceKeypoints, referenceDescriptors);

    // Sort the keypoints based on their response (strength)
    std::sort(referenceKeypoints.begin(), referenceKeypoints.end(), [](const cv::KeyPoint& a, const cv::KeyPoint& b) {
        return a.response > b.response;
    });

    // Select the top 100 keypoints
    std::vector<cv::KeyPoint> top100Keypoints(referenceKeypoints.begin(), referenceKeypoints.begin() + std::min(1000, static_cast<int>(referenceKeypoints.size())));

    // Extract the point coordinates from the top 100 keypoints
    std::vector<cv::Point2f> top100Points;
    for (const auto& keypoint : top100Keypoints) {
        top100Points.push_back(keypoint.pt);
    }

    // Draw the keypoints on the reference image
    cv::Mat referenceImageWithKeypoints = referenceImage.clone();
    for (size_t i = 0; i < top100Keypoints.size(); i++) {
        const cv::KeyPoint& keypoint = top100Keypoints[i];
        cv::Scalar color = getKeypointColor(i);
        cv::drawKeypoints(referenceImageWithKeypoints, std::vector<cv::KeyPoint>{keypoint}, referenceImageWithKeypoints, color, cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
    }

    // Iterate over the other images (2.ppm - 6.ppm)
    for (int i = 2; i <= 6; i++) {
        std::string imageFilename = folder + "/" + std::to_string(i) + ".ppm";
        cv::Mat image = cv::imread(imageFilename, cv::IMREAD_COLOR);

        if (image.empty()) {
            std::cerr << "Failed to read image: " << imageFilename << std::endl;
            continue;
        }

        // Read the corresponding homography matrix
        std::string homographyFilename = folder + "/H_1_" + std::to_string(i);
        cv::Mat homography = processor_utils::readHomography(homographyFilename);

        // Project the top 100 points from the reference image onto the current image
        std::vector<cv::Point2f> projectedPoints;
        cv::perspectiveTransform(top100Points, projectedPoints, homography);

        // Convert the projected points to cv::KeyPoint objects
        std::vector<cv::KeyPoint> projectedKeypoints;
        for (size_t j = 0; j < projectedPoints.size(); j++) {
            const cv::Point2f& point = projectedPoints[j];
            const cv::KeyPoint& referenceKeypoint = top100Keypoints[j];
            float size = referenceKeypoint.size;
            float angle = referenceKeypoint.angle;
            projectedKeypoints.emplace_back(cv::KeyPoint(point, size, angle));
        }

        // Draw the projected keypoints on the current image
        cv::Mat imageWithProjectedKeypoints = image.clone();
        for (size_t y = 0; y < projectedKeypoints.size(); y++) {
            const cv::KeyPoint& keypoint = projectedKeypoints[y];
            cv::Scalar color = getKeypointColor(y);
            cv::drawKeypoints(imageWithProjectedKeypoints, std::vector<cv::KeyPoint>{keypoint}, imageWithProjectedKeypoints, color, cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
        }

        // Display the reference image with keypoints
        std::string referenceWindowName = "Reference Image - 1.ppm";
        cv::imshow(referenceWindowName, referenceImageWithKeypoints);

        // Display the current image with projected keypoints in a separate window
        std::string currentImageWindowName = "Projected Keypoints - " + std::to_string(i) + ".ppm";
        cv::imshow(currentImageWindowName, imageWithProjectedKeypoints);

        cv::waitKey(0); // Wait for a key press to continue

        // Close the displayed images windows
        cv::destroyWindow(referenceWindowName);
        cv::destroyWindow(currentImageWindowName);

    }
}

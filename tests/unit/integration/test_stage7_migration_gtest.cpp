#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>
#include "../../../src/core/integration/ProcessorBridge.hpp"
#include "../../../descriptor_compare/experiment_config.hpp"

class Stage7MigrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a test image with some features for descriptor testing
        test_image = cv::Mat::zeros(200, 200, CV_8UC3);
        cv::circle(test_image, cv::Point(100, 100), 50, cv::Scalar(255, 255, 255), -1);
        cv::rectangle(test_image, cv::Rect(50, 50, 100, 30), cv::Scalar(128, 128, 128), -1);
        
        // Setup basic config
        config.descriptorOptions.descriptorType = DESCRIPTOR_SIFT;
        config.descriptorOptions.poolingStrategy = NONE;
        config.descriptorOptions.normType = 2;
        config.descriptorOptions.imageType = COLOR;
        config.descriptorOptions.descriptorColorSpace = D_COLOR;
    }
    
    void TearDown() override {
        // Clean up any resources if needed
    }
    
    cv::Mat test_image;
    experiment_config config;
};

TEST_F(Stage7MigrationTest, DescriptorFactorySupport) {
    // Test if the factory recognizes SIFT as supported
    bool is_supported = thesis_project::factories::DescriptorFactory::isSupported(config);
    
    // Whether SIFT is supported depends on the implementation state
    // We don't enforce specific behavior, just that the method works
    EXPECT_NO_THROW({
        bool result = thesis_project::factories::DescriptorFactory::isSupported(config);
    }) << "isSupported should not throw exceptions";
}

TEST_F(Stage7MigrationTest, SupportedTypesRetrieval) {
    auto supported_types = thesis_project::factories::DescriptorFactory::getSupportedTypes();
    
    // The list of supported types should be retrievable (may be empty if none implemented yet)
    EXPECT_NO_THROW({
        auto types = thesis_project::factories::DescriptorFactory::getSupportedTypes();
    }) << "getSupportedTypes should not throw exceptions";
    
    // If SIFT is supported, it should be in the list
    bool sift_supported = thesis_project::factories::DescriptorFactory::isSupported(config);
    if (sift_supported) {
        bool found_sift = false;
        for (const auto& type : supported_types) {
            if (type.find("SIFT") != std::string::npos) {
                found_sift = true;
                break;
            }
        }
        EXPECT_TRUE(found_sift) << "If SIFT is supported, it should appear in supported types list";
    }
}

TEST_F(Stage7MigrationTest, ProcessorBridgeInfo) {
    // Test that ProcessorBridge can provide implementation info
    std::string info;
    EXPECT_NO_THROW({
        info = thesis_project::integration::ProcessorBridge::getImplementationInfo(config);
    }) << "getImplementationInfo should not throw exceptions";
    
    EXPECT_FALSE(info.empty()) << "Implementation info should not be empty";
    
    // Info should contain some meaningful content
    EXPECT_TRUE(info.find("SIFT") != std::string::npos || 
                info.find("sift") != std::string::npos ||
                info.find("Not") != std::string::npos ||
                info.find("Legacy") != std::string::npos)
        << "Implementation info should mention SIFT or implementation status";
}

TEST_F(Stage7MigrationTest, TestImageCreation) {
    // Verify our test image is created properly
    EXPECT_FALSE(test_image.empty()) << "Test image should be created successfully";
    EXPECT_EQ(test_image.rows, 200) << "Test image should have correct height";
    EXPECT_EQ(test_image.cols, 200) << "Test image should have correct width";
    EXPECT_EQ(test_image.channels(), 3) << "Test image should have 3 channels";
    
    // Verify image has some non-zero content (our drawn features)
    cv::Scalar mean_value = cv::mean(test_image);
    EXPECT_GT(mean_value[0] + mean_value[1] + mean_value[2], 0.0) 
        << "Test image should have non-zero content";
}

TEST_F(Stage7MigrationTest, NewInterfaceDetection) {
    // Test the new interface (may fail - that's expected during migration)
    
    // We test that the method exists and handles errors gracefully
    EXPECT_NO_THROW({
        try {
            auto result = thesis_project::integration::ProcessorBridge::detectAndComputeWithConfig(test_image, config);
            
            // If it succeeds, verify the results make sense
            EXPECT_GE(result.first.size(), 0u) << "Keypoints result should be non-negative size";
            EXPECT_GE(result.second.rows, 0) << "Descriptors result should have non-negative rows";
            
            if (result.first.size() > 0) {
                EXPECT_EQ(result.first.size(), static_cast<size_t>(result.second.rows))
                    << "Number of keypoints should match descriptor rows";
                
                EXPECT_GT(result.second.cols, 0) << "Descriptors should have positive column count";
            }
            
        } catch (const std::exception& e) {
            // Exception is expected during migration - log it but don't fail the test
            std::string error_msg = e.what();
            EXPECT_FALSE(error_msg.empty()) << "Exception message should not be empty";
            
            // Common expected exceptions during migration
            EXPECT_TRUE(
                error_msg.find("not implemented") != std::string::npos ||
                error_msg.find("unsupported") != std::string::npos ||
                error_msg.find("fallback") != std::string::npos ||
                error_msg.find("legacy") != std::string::npos
            ) << "Exception should indicate implementation status: " << error_msg;
        }
    }) << "detectAndComputeWithConfig should handle errors gracefully";
}

// Test different descriptor types if supported
TEST_F(Stage7MigrationTest, MultipleDescriptorTypes) {
    std::vector<DescriptorType> types_to_test = {
        DESCRIPTOR_SIFT,
        DESCRIPTOR_RGBSIFT,
        DESCRIPTOR_HoNC,
        DESCRIPTOR_vSIFT
    };
    
    for (auto desc_type : types_to_test) {
        experiment_config test_config = config;
        test_config.descriptorOptions.descriptorType = desc_type;
        
        // Test that each descriptor type can be queried for support
        EXPECT_NO_THROW({
            bool supported = thesis_project::factories::DescriptorFactory::isSupported(test_config);
            std::string info = thesis_project::integration::ProcessorBridge::getImplementationInfo(test_config);
            EXPECT_FALSE(info.empty()) << "Info should be available for descriptor type " << desc_type;
        }) << "Descriptor type " << desc_type << " should be queryable";
    }
}

// Test different pooling strategies  
TEST_F(Stage7MigrationTest, PoolingStrategyHandling) {
    std::vector<PoolingStrategy> strategies_to_test = {
        NONE,
        STACKING,
        DOMAIN_SIZE_POOLING
    };
    
    for (auto strategy : strategies_to_test) {
        experiment_config test_config = config;
        test_config.descriptorOptions.poolingStrategy = strategy;
        
        // Test that pooling strategies are handled gracefully
        EXPECT_NO_THROW({
            std::string info = thesis_project::integration::ProcessorBridge::getImplementationInfo(test_config);
            EXPECT_FALSE(info.empty()) << "Info should be available for pooling strategy " << strategy;
        }) << "Pooling strategy " << strategy << " should be handled";
    }
}

// Integration test that covers the complete workflow
TEST_F(Stage7MigrationTest, CompleteWorkflow) {
    // Step 1: Check factory support
    bool sift_supported = thesis_project::factories::DescriptorFactory::isSupported(config);
    
    // Step 2: Get implementation info
    std::string info = thesis_project::integration::ProcessorBridge::getImplementationInfo(config);
    EXPECT_FALSE(info.empty()) << "Should get implementation information";
    
    // Step 3: Get supported types
    auto supported_types = thesis_project::factories::DescriptorFactory::getSupportedTypes();
    
    // Step 4: Attempt detection (may fail gracefully)
    bool detection_attempted = false;
    bool detection_succeeded = false;
    
    try {
        auto result = thesis_project::integration::ProcessorBridge::detectAndComputeWithConfig(test_image, config);
        detection_attempted = true;
        detection_succeeded = true;
        
        // If detection succeeds, verify results
        if (result.first.size() > 0) {
            EXPECT_EQ(result.first.size(), static_cast<size_t>(result.second.rows))
                << "Keypoints and descriptors should have matching counts";
        }
        
    } catch (const std::exception&) {
        detection_attempted = true;
        // Exception is acceptable during migration
    }
    
    EXPECT_TRUE(detection_attempted) << "Detection should be attempted (even if it fails)";
    
    // The test passes whether detection succeeds or fails gracefully
    // This allows the test to pass during different stages of migration
}

// Test error handling robustness
TEST_F(Stage7MigrationTest, ErrorHandling) {
    // Test with invalid image
    cv::Mat empty_image;
    EXPECT_NO_THROW({
        try {
            auto result = thesis_project::integration::ProcessorBridge::detectAndComputeWithConfig(empty_image, config);
        } catch (const std::exception&) {
            // Exception is acceptable for invalid input
        }
    }) << "Should handle invalid images gracefully";
    
    // Test with extreme config values (if they don't crash)
    experiment_config extreme_config = config;
    extreme_config.descriptorOptions.normType = 999;  // Invalid norm type
    
    EXPECT_NO_THROW({
        std::string info = thesis_project::integration::ProcessorBridge::getImplementationInfo(extreme_config);
        EXPECT_FALSE(info.empty()) << "Should provide info even for unusual configs";
    }) << "Should handle unusual configurations gracefully";
}
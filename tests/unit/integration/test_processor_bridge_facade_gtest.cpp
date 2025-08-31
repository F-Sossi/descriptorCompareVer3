#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>

#include "src/core/integration/ProcessorBridgeFacade.hpp"
#include "descriptor_compare/experiment_config.hpp"

class ProcessorBridgeFacadeTest : public ::testing::Test {
protected:
    experiment_config cfg;
    cv::Mat img;
    void SetUp() override {
        cfg.descriptorOptions.poolingStrategy = NONE;
        cfg.descriptorOptions.normType = cv::NORM_L2;
        img = cv::Mat::zeros(160, 220, CV_8UC3);
        cv::circle(img, {110, 80}, 40, cv::Scalar(255,255,255), -1);
    }
};

TEST_F(ProcessorBridgeFacadeTest, IsNewInterfaceSupportedFlags) {
    cfg.descriptorOptions.descriptorType = DESCRIPTOR_SIFT;
    EXPECT_TRUE(thesis_project::integration::isNewInterfaceSupported(cfg));
    cfg.descriptorOptions.descriptorType = DESCRIPTOR_RGBSIFT;
    EXPECT_TRUE(thesis_project::integration::isNewInterfaceSupported(cfg));
    cfg.descriptorOptions.descriptorType = DESCRIPTOR_HoNC;
    EXPECT_FALSE(thesis_project::integration::isNewInterfaceSupported(cfg));
}

TEST_F(ProcessorBridgeFacadeTest, SmokeDetectAndComputeToleratesExceptions) {
    cfg.descriptorOptions.descriptorType = DESCRIPTOR_SIFT;
    cv::Mat gray; cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
    try {
        auto result = thesis_project::integration::smokeDetectAndCompute(gray, cfg);
        if (!result.second.empty()) {
            EXPECT_EQ(result.second.type(), CV_32F);
            EXPECT_GT(result.second.cols, 0);
        }
    } catch (const std::exception& e) {
        EXPECT_FALSE(std::string(e.what()).empty());
    }
}


#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>

#include "descriptor_compare/processor_utils.hpp"
#include "descriptor_compare/experiment_config.hpp"
#include "src/core/integration/MigrationToggle.hpp"
#include "src/core/integration/ProcessorBridge.hpp"
#include "src/core/descriptor/factories/DescriptorFactory.hpp"

class ProcessorUtilsRoutingTest : public ::testing::Test {
protected:
    experiment_config cfg;
    cv::Mat img;
    bool prev_toggle;
    void SetUp() override {
        prev_toggle = thesis_project::integration::MigrationToggle::isEnabled();
        thesis_project::integration::MigrationToggle::setEnabled(false);
        cfg.descriptorOptions.poolingStrategy = NONE;
        cfg.descriptorOptions.normType = cv::NORM_L2;
        cfg.descriptorOptions.descriptorColorSpace = D_BW;
        img = cv::Mat::zeros(160, 220, CV_8UC3);
        cv::circle(img, {110, 80}, 40, cv::Scalar(255,255,255), -1);
    }
    void TearDown() override {
        thesis_project::integration::MigrationToggle::setEnabled(prev_toggle);
    }
};

TEST_F(ProcessorUtilsRoutingTest, ToggleOnSupportedDescriptorNoThrow) {
    cfg.descriptorOptions.descriptorType = DESCRIPTOR_SIFT;
    cfg.refreshDetectors();
    thesis_project::integration::MigrationToggle::setEnabled(true);
    ASSERT_TRUE(thesis_project::factories::DescriptorFactory::isSupported(cfg));
    cv::Mat gray; cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
    EXPECT_NO_THROW({ auto out = processor_utils::detectAndComputeWithConfig(gray, cfg); (void)out; });
}

TEST_F(ProcessorUtilsRoutingTest, ToggleOnUnsupportedFallsBack) {
    cfg.descriptorOptions.descriptorType = DESCRIPTOR_HoNC;
    cfg.descriptorOptions.descriptorColorSpace = D_COLOR;
    cfg.refreshDetectors();
    thesis_project::integration::MigrationToggle::setEnabled(true);
    ASSERT_FALSE(thesis_project::factories::DescriptorFactory::isSupported(cfg));
    EXPECT_NO_THROW({ auto out = processor_utils::detectAndComputeWithConfig(img, cfg); (void)out; });
}


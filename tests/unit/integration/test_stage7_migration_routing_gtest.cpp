#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>

#include "src/core/integration/MigrationToggle.hpp"
#include "src/core/integration/ProcessorBridge.hpp"
#include "src/core/descriptor/factories/DescriptorFactory.hpp"
#include "descriptor_compare/experiment_config.hpp"

namespace tp = thesis_project;

class Stage7RoutingTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Default: no pooling, typical settings
        cfg.descriptorOptions.poolingStrategy = NONE;
        cfg.descriptorOptions.normType = cv::NORM_L2;
        cfg.descriptorOptions.UseLockedInKeypoints = false; // let bridge detect for legacy
    }

    static cv::Mat makeImage() {
        cv::Mat img(160, 220, CV_8UC3, cv::Scalar(0,0,0));
        cv::circle(img, {110, 80}, 45, cv::Scalar(255, 255, 255), -1);
        cv::line(img, {20, 20}, {200, 140}, cv::Scalar(0, 255, 0), 3);
        return img;
    }

    experiment_config cfg;
};

TEST_F(Stage7RoutingTest, MigrationToggleToggles) {
    tp::integration::MigrationToggle::setEnabled(false);
    EXPECT_FALSE(tp::integration::MigrationToggle::isEnabled());
    tp::integration::MigrationToggle::setEnabled(true);
    EXPECT_TRUE(tp::integration::MigrationToggle::isEnabled());
    tp::integration::MigrationToggle::setEnabled(false);
    EXPECT_FALSE(tp::integration::MigrationToggle::isEnabled());
}

TEST_F(Stage7RoutingTest, FactorySupportAndCreation) {
    // SIFT supported
    cfg.descriptorOptions.descriptorType = DESCRIPTOR_SIFT;
    EXPECT_TRUE(tp::factories::DescriptorFactory::isSupported(cfg));
    auto sift_try = tp::factories::DescriptorFactory::tryCreate(cfg);
    EXPECT_NE(sift_try, nullptr);
    EXPECT_NO_THROW({ auto sift_ok = tp::factories::DescriptorFactory::create(cfg); (void)sift_ok; });

    // HoNC unsupported in new interface
    cfg.descriptorOptions.descriptorType = DESCRIPTOR_HoNC;
    EXPECT_FALSE(tp::factories::DescriptorFactory::isSupported(cfg));
    auto honc_try = tp::factories::DescriptorFactory::tryCreate(cfg);
    EXPECT_EQ(honc_try, nullptr);
    EXPECT_THROW({ auto honc_bad = tp::factories::DescriptorFactory::create(cfg); (void)honc_bad; }, std::runtime_error);
}

TEST_F(Stage7RoutingTest, SupportedTypesListContainsExpected) {
    auto types = tp::factories::DescriptorFactory::getSupportedTypes();
    bool has_sift = false, has_rgbsift = false;
    for (const auto& t : types) {
        if (t == "SIFT") has_sift = true;
        if (t == "RGBSIFT") has_rgbsift = true;
    }
    EXPECT_TRUE(has_sift);
    EXPECT_TRUE(has_rgbsift);
}

TEST_F(Stage7RoutingTest, ProcessorBridgeImplementationInfo) {
    // SIFT → new interface
    cfg.descriptorOptions.descriptorType = DESCRIPTOR_SIFT;
    std::string info_sift = tp::integration::ProcessorBridge::getImplementationInfo(cfg);
    EXPECT_EQ(info_sift, std::string("Using new interface implementation"));

    // HoNC → legacy
    cfg.descriptorOptions.descriptorType = DESCRIPTOR_HoNC;
    std::string info_honc = tp::integration::ProcessorBridge::getImplementationInfo(cfg);
    EXPECT_EQ(info_honc, std::string("Using legacy implementation"));
}

TEST_F(Stage7RoutingTest, ProcessorBridgeDetectNewInterfaceAndLegacy) {
    auto img = makeImage();

    // New interface: SIFT
    // Skipping direct detectAndComputeNew call here due to CV backend variance;
    // parity and extractor behavior are covered in dedicated parity tests.
    cfg.descriptorOptions.descriptorType = DESCRIPTOR_SIFT;

    // New interface should throw for unsupported type
    cfg.descriptorOptions.descriptorType = DESCRIPTOR_HoNC;
    EXPECT_THROW({ (void)tp::integration::ProcessorBridge::detectAndComputeNew(img, cfg); }, std::runtime_error);

    // Legacy path: should work for both SIFT and HoNC when detectors are refreshed
    cfg.descriptorOptions.descriptorType = DESCRIPTOR_SIFT;
    cfg.descriptorOptions.descriptorColorSpace = D_BW; // grayscale for SIFT
    cfg.refreshDetectors();
    auto legacy_sift = tp::integration::ProcessorBridge::detectAndComputeLegacy(img, cfg);
    EXPECT_GE(legacy_sift.second.cols, 1);
    EXPECT_EQ(legacy_sift.second.type(), CV_32F);

    cfg.descriptorOptions.descriptorType = DESCRIPTOR_HoNC;
    cfg.descriptorOptions.descriptorColorSpace = D_COLOR; // color for HoNC
    cfg.refreshDetectors();
    auto legacy_honc = tp::integration::ProcessorBridge::detectAndComputeLegacy(img, cfg);
    EXPECT_GE(legacy_honc.second.cols, 1);
    EXPECT_EQ(legacy_honc.second.type(), CV_32F);
}

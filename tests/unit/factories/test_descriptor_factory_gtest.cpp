#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>

#include "src/core/descriptor/factories/DescriptorFactory.hpp"
#include "descriptor_compare/experiment_config.hpp"

namespace tf = thesis_project::factories;

class DescriptorFactoryTest : public ::testing::Test {
protected:
    experiment_config cfg;
};

TEST_F(DescriptorFactoryTest, CreateSIFT) {
    cfg.descriptorOptions.descriptorType = DESCRIPTOR_SIFT;
    auto ext = tf::DescriptorFactory::create(cfg);
    ASSERT_NE(ext, nullptr);
    EXPECT_FALSE(ext->name().empty());
    EXPECT_EQ(ext->descriptorSize(), 128);
    EXPECT_EQ(ext->descriptorType(), DESCRIPTOR_SIFT);
}

TEST_F(DescriptorFactoryTest, TryCreateRGBSIFT) {
    cfg.descriptorOptions.descriptorType = DESCRIPTOR_RGBSIFT;
    auto ext = tf::DescriptorFactory::tryCreate(cfg);
    ASSERT_NE(ext, nullptr);
    EXPECT_EQ(ext->descriptorSize(), 384);
    EXPECT_EQ(ext->descriptorType(), DESCRIPTOR_RGBSIFT);
}

TEST_F(DescriptorFactoryTest, UnsupportedTypes) {
    cfg.descriptorOptions.descriptorType = DESCRIPTOR_HoNC;
    EXPECT_FALSE(tf::DescriptorFactory::isSupported(cfg));
    auto none = tf::DescriptorFactory::tryCreate(cfg);
    EXPECT_EQ(none, nullptr);
    EXPECT_THROW({ auto x = tf::DescriptorFactory::create(cfg); (void)x; }, std::runtime_error);
}

TEST_F(DescriptorFactoryTest, SupportedTypesList) {
    auto types = tf::DescriptorFactory::getSupportedTypes();
    bool has_sift = false, has_rgbsift = false;
    for (const auto& t : types) { if (t == "SIFT") has_sift = true; if (t == "RGBSIFT") has_rgbsift = true; }
    EXPECT_TRUE(has_sift);
    EXPECT_TRUE(has_rgbsift);
}

TEST_F(DescriptorFactoryTest, IsSupportedFlags) {
    cfg.descriptorOptions.descriptorType = DESCRIPTOR_SIFT;
    EXPECT_TRUE(tf::DescriptorFactory::isSupported(cfg));
    cfg.descriptorOptions.descriptorType = DESCRIPTOR_RGBSIFT;
    EXPECT_TRUE(tf::DescriptorFactory::isSupported(cfg));
}


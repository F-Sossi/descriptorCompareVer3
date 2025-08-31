#include <gtest/gtest.h>
#include "src/core/config/ConfigurationBridge.hpp"
#include "src/core/config/ExperimentConfig.hpp"
#include "descriptor_compare/experiment_config.hpp"

using thesis_project::config::ConfigurationBridge;
using thesis_project::config::ExperimentConfig;

TEST(ConfigurationBridgeRoundTrip, OldToNewAndBackBasic) {
    experiment_config old;
    old.descriptorOptions.descriptorType = DESCRIPTOR_RGBSIFT;
    old.descriptorOptions.poolingStrategy = DOMAIN_SIZE_POOLING;
    old.descriptorOptions.normType = cv::NORM_L2;
    old.descriptorOptions.scales = {1.0f, 1.5f};
    old.descriptorOptions.UseLockedInKeypoints = true;
    old.matchThreshold = 0.07;
    old.verificationType = HOMOGRAPHY;

    ExperimentConfig mid = ConfigurationBridge::fromOldConfig(old);
    ASSERT_FALSE(mid.descriptors.empty());
    EXPECT_EQ(mid.descriptors[0].type, thesis_project::DescriptorType::RGBSIFT);
    EXPECT_EQ(mid.descriptors[0].params.pooling, thesis_project::PoolingStrategy::DOMAIN_SIZE_POOLING);
    EXPECT_EQ(mid.keypoints.params.source, thesis_project::KeypointSource::HOMOGRAPHY_PROJECTION);
    EXPECT_NEAR(mid.evaluation.params.match_threshold, 0.07f, 1e-6);

    experiment_config back = ConfigurationBridge::toOldConfig(mid);
    EXPECT_EQ(back.descriptorOptions.descriptorType, DESCRIPTOR_RGBSIFT);
    EXPECT_EQ(back.descriptorOptions.poolingStrategy, DOMAIN_SIZE_POOLING);
    EXPECT_TRUE(back.descriptorOptions.UseLockedInKeypoints);
}


#include <gtest/gtest.h>
#include "src/core/config/ConfigurationBridge.hpp"
#include "src/core/config/ExperimentConfig.hpp"
#include "descriptor_compare/experiment_config.hpp"

using thesis_project::config::ConfigurationBridge;
using thesis_project::config::ExperimentConfig;

TEST(ConfigurationBridgeWeighting, ProceduralWeightingRoundTrip) {
    experiment_config old;
    old.descriptorOptions.scale_weighting_mode = 2; // gaussian
    old.descriptorOptions.scale_weight_sigma = 0.2f;
    old.descriptorOptions.scales = {0.75f, 1.0f, 1.25f};

    ExperimentConfig mid = ConfigurationBridge::fromOldConfig(old);
    ASSERT_FALSE(mid.descriptors.empty());
    EXPECT_NEAR(mid.descriptors[0].params.scale_weight_sigma, 0.2f, 1e-6);
    // Ensure we didn't accidentally set explicit weights
    EXPECT_TRUE(mid.descriptors[0].params.scale_weights.empty());

    experiment_config back = ConfigurationBridge::toOldConfig(mid);
    EXPECT_EQ(back.descriptorOptions.scale_weighting_mode, 2);
    EXPECT_NEAR(back.descriptorOptions.scale_weight_sigma, 0.2f, 1e-6);
}


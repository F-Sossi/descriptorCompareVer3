#include <gtest/gtest.h>
#include <string>

#include "src/core/config/YAMLConfigLoader.hpp"
#include "src/core/config/ConfigurationBridge.hpp"

using thesis_project::config::YAMLConfigLoader;
using thesis_project::config::ConfigurationBridge;

static const char* kYamlMinimal = R"YAML(
experiment:
  name: "sift_baseline"
  version: "1.0"
dataset:
  type: "hpatches"
  path: "data/hpatches"
keypoints:
  generator: "sift"
  max_features: 500
  source: "locked_in"
descriptors:
  - name: "sift"
    type: "sift"
    pooling: "none"
    scales: [1.0, 1.5]
    norm_type: "l2"
    use_color: false
evaluation:
  matching:
    method: "brute_force"
    threshold: 0.05
migration:
  use_new_interface: true
)YAML";

TEST(YAMLLoaderBridgeTest, LoadAndBridgeSIFTBaseline) {
    // Load new ExperimentConfig from YAML string
    auto new_cfg = YAMLConfigLoader::loadFromString(kYamlMinimal);

    // Sanity checks on loaded new-style config
    ASSERT_FALSE(new_cfg.descriptors.empty());
    EXPECT_EQ(new_cfg.descriptors[0].name, std::string("sift"));
    EXPECT_EQ(static_cast<int>(new_cfg.descriptors[0].type), static_cast<int>(thesis_project::DescriptorType::SIFT));
    EXPECT_EQ(static_cast<int>(new_cfg.descriptors[0].params.pooling), static_cast<int>(thesis_project::PoolingStrategy::NONE));
    EXPECT_FALSE(new_cfg.descriptors[0].params.use_color);
    EXPECT_EQ(new_cfg.keypoints.params.max_features, 500);
    EXPECT_EQ(new_cfg.evaluation.params.match_threshold, 0.05f);
    EXPECT_TRUE(new_cfg.migration.use_new_interface);

    // Bridge to legacy experiment_config
    ::experiment_config old_cfg = ConfigurationBridge::toOldConfig(new_cfg);

    // Validate essential mappings
    EXPECT_EQ(old_cfg.descriptorOptions.descriptorType, DESCRIPTOR_SIFT);
    EXPECT_EQ(old_cfg.descriptorOptions.poolingStrategy, NONE);
    EXPECT_EQ(old_cfg.descriptorOptions.normType, cv::NORM_L2);
    EXPECT_EQ(old_cfg.descriptorOptions.descriptorColorSpace, D_BW);
    EXPECT_TRUE(old_cfg.descriptorOptions.UseLockedInKeypoints); // from source: locked_in
    EXPECT_EQ(old_cfg.descriptorOptions.max_features, 500);
    EXPECT_FLOAT_EQ(static_cast<float>(old_cfg.matchThreshold), 0.05f);
}


#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>

#include "src/core/matching/MatchingFactory.hpp"
#include "src/core/matching/MatchingStrategy.hpp"
#include "descriptor_compare/experiment_config.hpp"

using thesis_project::matching::MatchingFactory;

class MatchingFactoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Minimal descriptor mats for a smoke test
        desc1 = cv::Mat::zeros(5, 8, CV_32F);
        desc2 = cv::Mat::zeros(5, 8, CV_32F);
        for (int r = 0; r < 5; ++r) {
            for (int c = 0; c < 8; ++c) {
                desc1.at<float>(r,c) = static_cast<float>(r + c);
                desc2.at<float>(r,c) = static_cast<float>(r + c);
            }
        }
    }

    cv::Mat desc1, desc2;
};

TEST_F(MatchingFactoryTest, CreateBruteForceStrategy) {
    auto strategy = MatchingFactory::createStrategy(BRUTE_FORCE);
    ASSERT_NE(strategy, nullptr);
    EXPECT_EQ(strategy->getName(), "BruteForce");
}

TEST_F(MatchingFactoryTest, CreateFromConfigDefaultBruteForce) {
    experiment_config cfg; // default matchingStrategy = BRUTE_FORCE
    auto strategy = MatchingFactory::createFromConfig(cfg);
    ASSERT_NE(strategy, nullptr);
    EXPECT_EQ(strategy->getName(), "BruteForce");

    // Smoke match
    auto matches = strategy->matchDescriptors(desc1, desc2);
    EXPECT_EQ(matches.size(), static_cast<size_t>(desc1.rows));
}

TEST_F(MatchingFactoryTest, CreateStrategyUnknownsThrow) {
    EXPECT_THROW(MatchingFactory::createStrategy(FLANN), std::runtime_error);
    EXPECT_THROW(MatchingFactory::createStrategy(RATIO_TEST), std::runtime_error);
    EXPECT_THROW(MatchingFactory::createStrategy(static_cast<MatchingStrategy>(99)), std::runtime_error);
}

TEST_F(MatchingFactoryTest, GetAvailableStrategiesContainsBruteForce) {
    auto list = MatchingFactory::getAvailableStrategies();
    bool hasBF = false;
    for (const auto& name : list) if (name == "BruteForce") hasBF = true;
    EXPECT_TRUE(hasBF);
}


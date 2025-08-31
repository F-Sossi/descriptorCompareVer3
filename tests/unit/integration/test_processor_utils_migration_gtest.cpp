#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>

#include "descriptor_compare/processor_utils.hpp"
#include "descriptor_compare/experiment_config.hpp"
#include "src/core/pooling/PoolingFactory.hpp"
#include "src/core/integration/MigrationToggle.hpp"
#include "src/core/descriptor/factories/DescriptorFactory.hpp"

using thesis_project::pooling::PoolingFactory;

namespace {

cv::Mat makeImage(int w = 200, int h = 160, bool color = false) {
    if (color) {
        cv::Mat img(h, w, CV_8UC3, cv::Scalar(0,0,0));
        cv::rectangle(img, {w/4, h/4, w/2, h/2}, cv::Scalar(120,20,200), -1);
        return img;
    } else {
        cv::Mat img(h, w, CV_8UC1, cv::Scalar(0));
        cv::circle(img, {w/2, h/2}, std::min(w,h)/4, cv::Scalar(200), -1);
        return img;
    }
}

std::vector<cv::KeyPoint> gridKeypoints(int w, int h, int step=24) {
    std::vector<cv::KeyPoint> kps;
    for (int y = step; y < h - step; y += step) {
        for (int x = step; x < w - step; x += step) {
            kps.emplace_back(static_cast<float>(x), static_cast<float>(y), 12.0f);
        }
    }
    return kps;
}

void expectNear(const cv::Mat& a, const cv::Mat& b, double atol=1e-4, double rtol=1e-4) {
    ASSERT_EQ(a.type(), b.type());
    ASSERT_EQ(a.rows, b.rows);
    ASSERT_EQ(a.cols, b.cols);
    for (int r=0; r<a.rows; ++r) {
        const float* pa = a.ptr<float>(r);
        const float* pb = b.ptr<float>(r);
        for (int c=0; c<a.cols; ++c) {
            double va = pa[c], vb = pb[c];
            double diff = std::abs(va - vb);
            double tol = atol + rtol * std::max(std::abs(va), std::abs(vb));
            ASSERT_LE(diff, tol) << "Mismatch at ("<<r<<","<<c<<")";
        }
    }
}

} // namespace

class ProcessorUtilsMigrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        tp_on = thesis_project::integration::MigrationToggle::isEnabled();
        thesis_project::integration::MigrationToggle::setEnabled(false);
        cfg.descriptorOptions.poolingStrategy = NONE; // Only routing for NONE is enabled
        cfg.descriptorOptions.normType = cv::NORM_L2;
        cfg.descriptorOptions.UseLockedInKeypoints = true; // We'll pass locked keypoints
    }
    void TearDown() override {
        thesis_project::integration::MigrationToggle::setEnabled(tp_on);
    }
    bool tp_on{};
    experiment_config cfg;
};

TEST_F(ProcessorUtilsMigrationTest, LockedKeypoints_LegacyPath_SIFT) {
    cfg.descriptorOptions.descriptorType = DESCRIPTOR_SIFT;
    cfg.descriptorOptions.descriptorColorSpace = D_BW;
    cfg.refreshDetectors();

    cv::Mat image = makeImage(200,160,false);
    auto locked = gridKeypoints(image.cols, image.rows, 26);

    // Legacy path: toggle off
    thesis_project::integration::MigrationToggle::setEnabled(false);
    auto result = processor_utils::detectAndComputeWithConfigLocked(image, locked, cfg);

    // Expected legacy computation using detector interface via NoPooling
    auto pooling = PoolingFactory::createFromConfig(cfg);
    cv::Mat expected = pooling->computeDescriptors(image, locked, cfg.detector, cfg);

    ASSERT_EQ(result.first.size(), locked.size());
    expectNear(result.second, expected);
}

TEST_F(ProcessorUtilsMigrationTest, LockedKeypoints_NewInterface_SIFT) {
    cfg.descriptorOptions.descriptorType = DESCRIPTOR_SIFT;
    cfg.descriptorOptions.descriptorColorSpace = D_BW;
    cfg.refreshDetectors();

    cv::Mat image = makeImage(200,160,false);
    auto locked = gridKeypoints(image.cols, image.rows, 26);

    // New path: toggle on and supported descriptor
    thesis_project::integration::MigrationToggle::setEnabled(true);
    ASSERT_TRUE(thesis_project::factories::DescriptorFactory::isSupported(cfg));

    auto result = processor_utils::detectAndComputeWithConfigLocked(image, locked, cfg);

    // Expected new-interface computation via NoPooling overload
    auto extractor = thesis_project::factories::DescriptorFactory::create(cfg);
    auto pooling = PoolingFactory::createFromConfig(cfg);
    cv::Mat expected = pooling->computeDescriptors(image, locked, *extractor, cfg);

    ASSERT_EQ(result.first.size(), locked.size());
    expectNear(result.second, expected);
}


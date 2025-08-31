#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>

#include "descriptor_compare/experiment_config.hpp"
#include "src/core/pooling/PoolingFactory.hpp"
#include "src/core/pooling/NoPooling.hpp"
#include "src/core/descriptor/factories/DescriptorFactory.hpp"

using thesis_project::pooling::PoolingFactory;

namespace {

cv::Mat makeTestImage(int w = 240, int h = 180, bool color = true) {
    if (color) {
        cv::Mat img(h, w, CV_8UC3);
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                img.at<cv::Vec3b>(y, x) = cv::Vec3b(
                    static_cast<uint8_t>((x * 255) / std::max(1, w - 1)),
                    static_cast<uint8_t>((y * 255) / std::max(1, h - 1)),
                    static_cast<uint8_t>(((x + y) * 255) / std::max(1, w + h - 2)));
            }
        }
        // Add a few shapes to ensure texture
        cv::circle(img, {w / 3, h / 2}, std::min(w, h) / 6, cv::Scalar(255, 255, 255), -1);
        cv::rectangle(img, {w / 2, h / 3, w / 4, h / 6}, cv::Scalar(32, 192, 64), -1);
        return img;
    } else {
        cv::Mat img(h, w, CV_8UC1);
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                img.at<uint8_t>(y, x) = static_cast<uint8_t>((x ^ y) & 0xFF);
            }
        }
        cv::circle(img, {w / 2, h / 2}, std::min(w, h) / 5, cv::Scalar(200), -1);
        return img;
    }
}

std::vector<cv::KeyPoint> makeLockedKeypoints(int w, int h, int step = 20) {
    std::vector<cv::KeyPoint> kps;
    // Avoid borders to reduce out-of-bounds rejection during compute
    for (int y = step; y < h - step; y += step) {
        for (int x = step; x < w - step; x += step) {
            kps.emplace_back(static_cast<float>(x), static_cast<float>(y), 12.0f);
        }
    }
    return kps;
}

// Compare two float descriptor matrices with tolerance
void expectDescriptorsNear(const cv::Mat& a, const cv::Mat& b, double atol = 1e-4, double rtol = 1e-4) {
    ASSERT_EQ(a.type(), b.type()) << "Descriptor dtype mismatch";
    ASSERT_EQ(a.rows, b.rows) << "Descriptor row count mismatch";
    ASSERT_EQ(a.cols, b.cols) << "Descriptor col count mismatch";
    if (a.empty()) return;

    // Element-wise tolerance check
    for (int r = 0; r < a.rows; ++r) {
        const float* pa = a.ptr<float>(r);
        const float* pb = b.ptr<float>(r);
        for (int c = 0; c < a.cols; ++c) {
            double va = static_cast<double>(pa[c]);
            double vb = static_cast<double>(pb[c]);
            double diff = std::abs(va - vb);
            double tol = atol + rtol * std::max(std::abs(va), std::abs(vb));
            ASSERT_LE(diff, tol) << "Mismatch at (" << r << "," << c << ")";
        }
    }
}

} // namespace

class Stage7ParityTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Default config
        config.descriptorOptions.poolingStrategy = NONE;
        config.descriptorOptions.normType = cv::NORM_L2;
        config.descriptorOptions.UseLockedInKeypoints = true;
    }

    // Compute legacy descriptors using Feature2D-based path
    cv::Mat computeLegacy(const cv::Mat& image, const std::vector<cv::KeyPoint>& keypoints) {
        auto pooling = PoolingFactory::createFromConfig(config);
        return pooling->computeDescriptors(image, keypoints, config.detector, config);
    }

    // Compute new descriptors using IDescriptorExtractor via NoPooling overload
    cv::Mat computeNewIface(const cv::Mat& image, const std::vector<cv::KeyPoint>& keypoints) {
        auto extractor = thesis_project::factories::DescriptorFactory::create(config);
        auto pooling = PoolingFactory::createFromConfig(config);
        return pooling->computeDescriptors(image, keypoints, *extractor, config);
    }

    experiment_config config;
};

TEST_F(Stage7ParityTest, SIFT_NoPooling_LockedKeypoints) {
    // Prepare grayscale-friendly config
    config.descriptorOptions.descriptorType = DESCRIPTOR_SIFT;
    config.descriptorOptions.descriptorColorSpace = D_BW;
    config.refreshDetectors();

    // Prepare data
    cv::Mat image = makeTestImage(240, 180, /*color=*/false);
    auto keypoints = makeLockedKeypoints(image.cols, image.rows, 22);

    // Compute both paths
    cv::Mat legacy = computeLegacy(image, keypoints);
    cv::Mat modern = computeNewIface(image, keypoints);

    // Basic shape/type assertions
    ASSERT_EQ(legacy.type(), CV_32F);
    ASSERT_EQ(modern.type(), CV_32F);
    ASSERT_EQ(legacy.cols, 128);
    ASSERT_EQ(modern.cols, 128);
    ASSERT_EQ(legacy.rows, modern.rows);

    // Value parity (allow small tolerance)
    expectDescriptorsNear(legacy, modern, 1e-4, 1e-4);
}

TEST_F(Stage7ParityTest, RGBSIFT_NoPooling_LockedKeypoints) {
    // Prepare color config
    config.descriptorOptions.descriptorType = DESCRIPTOR_RGBSIFT;
    config.descriptorOptions.descriptorColorSpace = D_COLOR;
    config.refreshDetectors();

    // Prepare data
    cv::Mat image = makeTestImage(240, 180, /*color=*/true);
    auto keypoints = makeLockedKeypoints(image.cols, image.rows, 22);

    // Compute both paths
    cv::Mat legacy = computeLegacy(image, keypoints);
    cv::Mat modern = computeNewIface(image, keypoints);

    // Basic shape/type assertions
    ASSERT_EQ(legacy.type(), CV_32F);
    ASSERT_EQ(modern.type(), CV_32F);
    ASSERT_EQ(legacy.cols, 384);  // 3x128
    ASSERT_EQ(modern.cols, 384);
    ASSERT_EQ(legacy.rows, modern.rows);

    // Value parity (allow small tolerance)
    expectDescriptorsNear(legacy, modern, 1e-4, 1e-4);
}


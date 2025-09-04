#include "DNNPatchWrapper.hpp"
#include <opencv2/imgproc.hpp>
#include <cmath>

namespace thesis_project {
namespace wrappers {

DNNPatchWrapper::DNNPatchWrapper(const std::string& onnx_model_path,
                                 int input_size,
                                 float support_multiplier,
                                 bool rotate_to_upright,
                                 float mean,
                                 float std,
                                 bool per_patch_standardize)
    : input_size_(input_size),
      support_mult_(support_multiplier),
      rotate_upright_(rotate_to_upright),
      mean_(mean),
      std_(std),
      per_patch_standardize_(per_patch_standardize)
{
    try {
        net_ = cv::dnn::readNetFromONNX(onnx_model_path);
        if (net_.empty()) {
            throw std::runtime_error("Failed to load ONNX model - network is empty");
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("ONNX model loading failed: " + std::string(e.what()));
    }
}

static cv::Mat extractPatch(const cv::Mat& image, const cv::KeyPoint& kp, int N, float support_mult, bool rotate_upright) {
    const float S = std::max(1.0f, support_mult * kp.size);
    // To rotate the patch to upright, we must undo the keypoint angle
    const float angle_deg = rotate_upright ? -kp.angle : 0.0f;
    const double scale = static_cast<double>(N) / static_cast<double>(S);
    // Build rotation (about kp.pt) and scale, then translate so kp maps to patch center
    cv::Mat M = cv::getRotationMatrix2D(kp.pt, angle_deg, scale); // 2x3
    // Shift so that kp.pt -> (N/2, N/2)
    M.at<double>(0, 2) += (static_cast<double>(N) * 0.5 - kp.pt.x);
    M.at<double>(1, 2) += (static_cast<double>(N) * 0.5 - kp.pt.y);
    cv::Mat patch;
    cv::warpAffine(image, patch, M, cv::Size(N, N), cv::INTER_LINEAR, cv::BORDER_REPLICATE);
    return patch;
}

cv::Mat DNNPatchWrapper::extract(const cv::Mat& image,
                                 const std::vector<cv::KeyPoint>& keypoints,
                                 const DescriptorParams& params) {
    const int N = input_size_;
    cv::Mat descriptors(static_cast<int>(keypoints.size()), descriptorSize(), CV_32F, cv::Scalar(0));
    int row = 0;
    for (const auto& kp : keypoints) {
        cv::Mat patch = extractPatch(image, kp, N, support_mult_, rotate_upright_);
        cv::Mat patchF;
        patch.convertTo(patchF, CV_32F, 1.0/255.0);
        // Optional per-patch standardization to zero mean / unit variance
        if (per_patch_standardize_) {
            cv::Scalar mu, sigma;
            cv::meanStdDev(patchF, mu, sigma);
            float eps = 1e-6f;
            float denom = static_cast<float>(sigma[0]) > eps ? static_cast<float>(sigma[0]) : eps;
            patchF = (patchF - static_cast<float>(mu[0])) / denom;
        } else {
            if (mean_ != 0.0f || std_ != 1.0f) patchF = (patchF - mean_) / std_;
        }
        cv::Mat blob = cv::dnn::blobFromImage(patchF); // CHW
        net_.setInput(blob);
        cv::Mat out;
        try {
            out = net_.forward();
            if (out.empty()) {
                throw std::runtime_error("DNN forward pass returned empty Mat");
            }
        } catch (const std::exception& e) {
            throw std::runtime_error("DNN forward pass failed: " + std::string(e.what()));
        }
        
        // Handle different output shapes (1xC or 1xCx1x1)
        if (out.dims == 4 && out.size[2] == 1 && out.size[3] == 1) {
            // Reshape from [1,C,1,1] to [1,C]
            out = out.reshape(1, out.size[1]);
        } else if (out.dims == 2) {
            // Already [1,C], no reshape needed
        } else {
            // Fallback: flatten to 1D and take as descriptor
            out = out.reshape(1, 1);
        }
        if (out.type() != CV_32F) {
            out.convertTo(out, CV_32F);
        }
        // L2 normalise
        cv::Mat dstRow = descriptors.row(row);
        if (out.cols >= dstRow.cols) {
            out.colRange(0, dstRow.cols).copyTo(dstRow);
        } else {
            dstRow.colRange(0, out.cols) = out;
        }
        cv::normalize(dstRow, dstRow, 1.0, 0.0, cv::NORM_L2);
        row++;
    }
    return descriptors;
}

} // namespace wrappers
} // namespace thesis_project

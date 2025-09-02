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
                                 float std)
    : input_size_(input_size), support_mult_(support_multiplier), rotate_upright_(rotate_to_upright), mean_(mean), std_(std)
{
    net_ = cv::dnn::readNetFromONNX(onnx_model_path);
}

static cv::Mat extractPatch(const cv::Mat& image, const cv::KeyPoint& kp, int N, float support_mult, bool rotate_upright) {
    float S = std::max(1.0f, support_mult * kp.size);
    float angle = rotate_upright ? kp.angle : 0.0f;
    // Build affine that maps target N×N square to source rotated square of side S centered at kp
    float c = std::cos(-angle * static_cast<float>(CV_PI) / 180.0f);
    float s = std::sin(-angle * static_cast<float>(CV_PI) / 180.0f);
    float half = S * 0.5f;
    // Source quad corners around kp (rotated): use three points to estimate affine
    cv::Point2f srcTri[3] = {
        cv::Point2f(kp.pt.x - half * c + 0 * s, kp.pt.y - half * s - 0 * c),
        cv::Point2f(kp.pt.x + half * c + 0 * s, kp.pt.y + half * s - 0 * c),
        cv::Point2f(kp.pt.x - half * c - half * s, kp.pt.y - half * s + half * c)
    };
    cv::Point2f dstTri[3] = {
        cv::Point2f(0, 0),
        cv::Point2f(static_cast<float>(N - 1), 0),
        cv::Point2f(0, static_cast<float>(N - 1))
    };
    cv::Mat M = cv::getAffineTransform(srcTri, dstTri);
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
        if (mean_ != 0.0f || std_ != 1.0f) patchF = (patchF - mean_) / std_;
        cv::Mat blob = cv::dnn::blobFromImage(patchF); // CHW
        net_.setInput(blob);
        cv::Mat out = net_.forward();
        // Assume out is 1xC or 1xCx1x1
        out = out.reshape(1, 1);
        out.convertTo(out, CV_32F);
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


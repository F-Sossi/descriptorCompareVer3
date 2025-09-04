#pragma once

#include "interfaces/IDescriptorExtractor.hpp"
#include "descriptor_compare/experiment_config.hpp"
#include <opencv2/dnn.hpp>

namespace thesis_project {
namespace wrappers {

class DNNPatchWrapper : public IDescriptorExtractor {
private:
    cv::dnn::Net net_;
    int input_size_ = 32;
    float support_mult_ = 1.0f;
    bool rotate_upright_ = true;
    float mean_ = 0.0f;
    float std_ = 1.0f;
    bool per_patch_standardize_ = false;

public:
    explicit DNNPatchWrapper(const std::string& onnx_model_path,
                             int input_size,
                             float support_multiplier,
                             bool rotate_to_upright,
                             float mean,
                             float std,
                             bool per_patch_standardize);

    cv::Mat extract(const cv::Mat& image,
                    const std::vector<cv::KeyPoint>& keypoints,
                    const DescriptorParams& params = {}) override;

    std::string name() const override { return "DNNPatch"; }
    int descriptorSize() const override { return 128; }
    int descriptorType() const override { return DESCRIPTOR_SIFT; }
};

} // namespace wrappers
} // namespace thesis_project

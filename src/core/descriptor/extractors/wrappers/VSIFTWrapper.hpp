#ifndef THESIS_PROJECT_VSIFT_WRAPPER_HPP
#define THESIS_PROJECT_VSIFT_WRAPPER_HPP

#include "../../../interfaces/IDescriptorExtractor.hpp"
#include "../../../../../descriptor_compare/experiment_config.hpp"

namespace thesis_project {
namespace extractors {

class VSIFTWrapper : public IDescriptorExtractor {
private:
    experiment_config config_;

public:
    explicit VSIFTWrapper(const experiment_config& config) : config_(config) {}

    cv::Mat extract(const cv::Mat& image,
                   const std::vector<cv::KeyPoint>& keypoints) override {
        // TODO: Implement vSIFT wrapper
        throw std::runtime_error("vSIFT wrapper not yet implemented");
    }

    std::string name() const override { return "vSIFT_Wrapper"; }
    int descriptorSize() const override { return 128; } // Placeholder
    std::string getConfiguration() const override { return "vSIFT Wrapper (TODO)"; }
};

} // namespace extractors
} // namespace thesis_project

#endif

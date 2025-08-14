#ifndef THESIS_PROJECT_HONC_WRAPPER_HPP
#define THESIS_PROJECT_HONC_WRAPPER_HPP

#include "src/interfaces/IDescriptorExtractor.hpp"
#include "descriptor_compare/experiment_config.cpp"

namespace thesis_project {
namespace extractors {

class HoNCWrapper : public IDescriptorExtractor {
private:
    experiment_config config_;

public:
    explicit HoNCWrapper(const experiment_config& config) : config_(config) {}

    static cv::Mat extract(const cv::Mat& image,
                           const std::vector<cv::KeyPoint>& keypoints) override {
        // TODO: Implement HoNC wrapper
        throw std::runtime_error("HoNC wrapper not yet implemented");
    }

    std::string name() const override { return "HoNC_Wrapper"; }
    int descriptorSize() const override { return 256; } // Placeholder
    static std::string getConfiguration() override { return "HoNC Wrapper (TODO)"; }
};

} // namespace extractors
} // namespace thesis_project

#endif

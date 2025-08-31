#include "ProcessorBridgeFacade.hpp"
#include "ProcessorBridge.hpp"
#include "../descriptor/factories/DescriptorFactory.hpp"
#include "descriptor_compare/experiment_config.hpp"

namespace thesis_project {
namespace integration {

bool isNewInterfaceSupported(const experiment_config& config) {
    return factories::DescriptorFactory::isSupported(config);
}

std::pair<std::vector<cv::KeyPoint>, cv::Mat>
smokeDetectAndCompute(const cv::Mat& image, const experiment_config& config) {
    return ProcessorBridge::detectAndComputeNew(image, config);
}

} // namespace integration
} // namespace thesis_project


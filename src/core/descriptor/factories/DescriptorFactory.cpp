#include "DescriptorFactory.hpp"
#include "../extractors/wrappers/SIFTWrapper.hpp"
#include "../extractors/wrappers/RGBSIFTWrapper.hpp"
// #include "../extractors/wrappers/HoNCWrapper.hpp"  // TODO: Implement later
// #include "../extractors/wrappers/VSIFTWrapper.hpp" // TODO: Implement later
#include <stdexcept>

namespace thesis_project {
namespace factories {

std::unique_ptr<IDescriptorExtractor> DescriptorFactory::create(const experiment_config& config) {
    switch (config.descriptorOptions.descriptorType) {
        case DESCRIPTOR_SIFT:
            return createSIFT(config);
        case DESCRIPTOR_RGBSIFT:
            return createRGBSIFT(config);
        // TODO: Add other descriptor types
        default:
            throw std::runtime_error("Unsupported descriptor type in factory");
    }
}

std::unique_ptr<IDescriptorExtractor> DescriptorFactory::tryCreate(const experiment_config& config) noexcept {
    try {
        return create(config);
    } catch (...) {
        return nullptr;
    }
}

bool DescriptorFactory::isSupported(const experiment_config& config) {
    switch (config.descriptorOptions.descriptorType) {
        case DESCRIPTOR_SIFT:
        case DESCRIPTOR_RGBSIFT:
            return true;
        default:
            return false;
    }
}

std::vector<std::string> DescriptorFactory::getSupportedTypes() {
    return {"SIFT", "RGBSIFT"};
}

std::unique_ptr<IDescriptorExtractor> DescriptorFactory::createSIFT(const experiment_config& config) {
    return std::make_unique<wrappers::SIFTWrapper>(config);
}

std::unique_ptr<IDescriptorExtractor> DescriptorFactory::createRGBSIFT(const experiment_config& config) {
    return std::make_unique<wrappers::RGBSIFTWrapper>(config);
}

// New-config overloads
std::unique_ptr<IDescriptorExtractor> DescriptorFactory::create(thesis_project::DescriptorType type) {
    switch (type) {
        case thesis_project::DescriptorType::SIFT:
            return createSIFT();
        case thesis_project::DescriptorType::RGBSIFT:
            return createRGBSIFT();
        default:
            throw std::runtime_error("Unsupported descriptor type in factory (new-config)");
    }
}

bool DescriptorFactory::isSupported(thesis_project::DescriptorType type) {
    switch (type) {
        case thesis_project::DescriptorType::SIFT:
        case thesis_project::DescriptorType::RGBSIFT:
            return true;
        default:
            return false;
    }
}

std::unique_ptr<IDescriptorExtractor> DescriptorFactory::createSIFT() {
    return std::make_unique<wrappers::SIFTWrapper>();
}

std::unique_ptr<IDescriptorExtractor> DescriptorFactory::createRGBSIFT() {
    return std::make_unique<wrappers::RGBSIFTWrapper>();
}

} // namespace factories
} // namespace thesis_project

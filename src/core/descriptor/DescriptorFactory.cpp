#include "DescriptorFactory.hpp"
#include "extractors/traditional/OpenCVSIFTWrapper.hpp"
#include "extractors/traditional/RGBSIFTWrapper.hpp"
#include "extractors/traditional/VanillaSIFTWrapper.hpp"
// #include "extractors/traditional/HoNCWrapper.hpp"  // TODO: Create HoNC wrapper

namespace thesis_project {

    std::unique_ptr<IDescriptorExtractor> DescriptorFactory::createOpenCVSIFT() {
        return std::make_unique<descriptors::OpenCVSIFTWrapper>();
    }
    
    std::unique_ptr<IDescriptorExtractor> DescriptorFactory::createRGBSIFT() {
        return std::make_unique<descriptors::RGBSIFTWrapper>();
    }
    
    std::unique_ptr<IDescriptorExtractor> DescriptorFactory::createVanillaSIFT() {
        return std::make_unique<descriptors::VanillaSIFTWrapper>();
    }
    
    std::unique_ptr<IDescriptorExtractor> DescriptorFactory::createHoNC() {
        // TODO: Implement HoNC wrapper
        throw std::runtime_error("HoNC wrapper not yet implemented");
        // return std::make_unique<descriptors::HoNCWrapper>();
    }

} // namespace thesis_project
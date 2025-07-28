#pragma once

#include "../../interfaces/IDescriptorExtractor.hpp"
#include "thesis_project/types.hpp"
#include <memory>
#include <stdexcept>

namespace thesis_project {

    /**
     * @brief Factory for creating descriptor extractors
     * 
     * Provides a unified way to create any descriptor type through the interface system.
     * This allows easy switching between different descriptor implementations.
     */
    class DescriptorFactory {
    public:
        /**
         * @brief Create a descriptor extractor of specified type
         * @param type The type of descriptor to create
         * @return Unique pointer to descriptor extractor
         * @throws std::invalid_argument if type is not supported
         */
        static std::unique_ptr<IDescriptorExtractor> create(DescriptorType type) {
            switch (type) {
                case DescriptorType::SIFT:
                    return createOpenCVSIFT();
                    
                case DescriptorType::RGBSIFT:
                    return createRGBSIFT();
                    
                case DescriptorType::vSIFT:
                    return createVanillaSIFT();
                    
                case DescriptorType::HoNC:
                    return createHoNC();
                    
                case DescriptorType::NONE:
                default:
                    throw std::invalid_argument("Unsupported descriptor type: " + 
                                              toString(type));
            }
        }
        
        /**
         * @brief Create descriptor from string name (useful for configuration)
         * @param name String name of descriptor ("sift", "rgbsift", etc.)
         * @return Unique pointer to descriptor extractor
         */
        static std::unique_ptr<IDescriptorExtractor> create(const std::string& name) {
            if (name == "sift" || name == "SIFT") {
                return create(DescriptorType::SIFT);
            } else if (name == "rgbsift" || name == "RGBSIFT") {
                return create(DescriptorType::RGBSIFT);
            } else if (name == "vsift" || name == "vSIFT" || name == "vanilla_sift") {
                return create(DescriptorType::vSIFT);
            } else if (name == "honc" || name == "HoNC") {
                return create(DescriptorType::HoNC);
            } else {
                throw std::invalid_argument("Unknown descriptor name: " + name);
            }
        }
        
        /**
         * @brief Get list of all available descriptor types
         * @return Vector of supported descriptor types
         */
        static std::vector<DescriptorType> getAvailableTypes() {
            return {
                DescriptorType::SIFT,
                DescriptorType::RGBSIFT,
                DescriptorType::vSIFT,
                DescriptorType::HoNC
            };
        }
        
        /**
         * @brief Get list of all available descriptor names
         * @return Vector of supported descriptor names
         */
        static std::vector<std::string> getAvailableNames() {
            return {"sift", "rgbsift", "vsift", "honc"};
        }
        
        /**
         * @brief Check if a descriptor type is available
         * @param type Descriptor type to check
         * @return true if available, false otherwise
         */
        static bool isAvailable(DescriptorType type) {
            try {
                auto extractor = create(type);
                return true;
            } catch (const std::exception&) {
                return false;
            }
        }
        
    private:
        // Forward declarations - implementations will be in .cpp file
        static std::unique_ptr<IDescriptorExtractor> createOpenCVSIFT();
        static std::unique_ptr<IDescriptorExtractor> createRGBSIFT();
        static std::unique_ptr<IDescriptorExtractor> createVanillaSIFT();
        static std::unique_ptr<IDescriptorExtractor> createHoNC();
    };

} // namespace thesis_project
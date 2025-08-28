#include "ConfigurationBridge.hpp"
#include "thesis_project/types.hpp"

namespace thesis_project::config {

    ::experiment_config ConfigurationBridge::toOldConfig(const ExperimentConfig& new_config) {
        ::experiment_config old_config;
        
        // Use the first descriptor configuration if multiple exist
        if (!new_config.descriptors.empty()) {
            const auto& desc_config = new_config.descriptors[0];
            
            // Map descriptor type
            old_config.descriptorOptions.descriptorType = static_cast<::DescriptorType>(toOldDescriptorType(desc_config.type));
            
            // Map pooling strategy
            auto pooling_strategy = static_cast<::PoolingStrategy>(toOldPoolingStrategy(desc_config.params.pooling));
            old_config.descriptorOptions.poolingStrategy = pooling_strategy;
            
            // Map normalization settings
            if (desc_config.params.normalize_after_pooling) {
                old_config.descriptorOptions.normalizationStage = ::AFTER_POOLING;
            } else if (desc_config.params.normalize_before_pooling) {
                old_config.descriptorOptions.normalizationStage = ::BEFORE_POOLING;
            } else {
                old_config.descriptorOptions.normalizationStage = ::NO_NORMALIZATION;
            }
            
            // Map other descriptor parameters
            old_config.descriptorOptions.normType = desc_config.params.norm_type;
            old_config.descriptorOptions.scales = desc_config.params.scales;
            
            // Color/BW image type and descriptor color space
            if (desc_config.params.use_color) {
                old_config.descriptorOptions.imageType = ::COLOR;
                old_config.descriptorOptions.descriptorColorSpace = ::D_COLOR;
            } else {
                old_config.descriptorOptions.imageType = ::BW;
                old_config.descriptorOptions.descriptorColorSpace = ::D_BW;
            }
            
            // Stacking parameters
            if (desc_config.params.pooling == PoolingStrategy::STACKING) {
                // Set secondary descriptor type
                old_config.descriptorOptions.descriptorType2 = static_cast<::DescriptorType>(toOldDescriptorType(desc_config.params.secondary_descriptor));
                
                // Set appropriate color space for secondary descriptor based on its type
                switch (desc_config.params.secondary_descriptor) {
                    case DescriptorType::SIFT:
                    case DescriptorType::vSIFT:
                        old_config.descriptorOptions.descriptorColorSpace2 = ::D_BW;
                        break;
                    case DescriptorType::RGBSIFT:
                    case DescriptorType::HoNC:
                        old_config.descriptorOptions.descriptorColorSpace2 = ::D_COLOR;
                        break;
                    default:
                        old_config.descriptorOptions.descriptorColorSpace2 = ::D_BW;
                        break;
                }
            } else {
            }
            
            // Keypoint source configuration (NEW)
            // Map keypoint source to legacy locked keypoints flag
            bool use_locked = (new_config.keypoints.params.source == KeypointSource::HOMOGRAPHY_PROJECTION);
            
            // Handle backward compatibility: if use_locked_keypoints is explicitly set, respect it
            if (new_config.keypoints.params.use_locked_keypoints) {
                use_locked = true;
            }
            
            old_config.descriptorOptions.UseLockedInKeypoints = use_locked;
            
            // Max features for keypoint detection
            old_config.descriptorOptions.max_features = new_config.keypoints.params.max_features;
        }
        
        // Map evaluation parameters
        old_config.matchThreshold = new_config.evaluation.params.match_threshold;
        
        // Map verification type
        switch (new_config.evaluation.params.validation_method) {
            case ValidationMethod::HOMOGRAPHY:
                old_config.verificationType = ::HOMOGRAPHY;
                break;
            case ValidationMethod::CROSS_IMAGE:
                old_config.verificationType = ::MATCHES;
                break;
            case ValidationMethod::NONE:
                old_config.verificationType = ::NO_VISUAL_VERIFICATION;
                break;
        }
        
        return old_config;
    }
    
    ExperimentConfig ConfigurationBridge::fromOldConfig(const ::experiment_config& old_config) {
        ExperimentConfig new_config;
        
        // Convert basic experiment info
        new_config.experiment.name = "converted_experiment";
        new_config.experiment.description = "Converted from legacy configuration";
        
        // Convert dataset info (use default path if old one is not available)
        new_config.dataset.path = "data/";
        
        // Convert keypoint parameters (using defaults since old config doesn't store these directly)
        new_config.keypoints.params.use_locked_keypoints = old_config.descriptorOptions.UseLockedInKeypoints;
        
        // Map legacy locked keypoints flag to new keypoint source
        new_config.keypoints.params.source = old_config.descriptorOptions.UseLockedInKeypoints 
            ? KeypointSource::HOMOGRAPHY_PROJECTION 
            : KeypointSource::INDEPENDENT_DETECTION;
            
        new_config.keypoints.params.max_features = old_config.descriptorOptions.max_features;
        
        // Convert descriptor configuration
        ExperimentConfig::DescriptorConfig desc_config;
        desc_config.type = toNewDescriptorType(static_cast<int>(old_config.descriptorOptions.descriptorType));
        desc_config.name = toString(desc_config.type);
        desc_config.params.pooling = toNewPoolingStrategy(static_cast<int>(old_config.descriptorOptions.poolingStrategy));
        
        // Convert normalization settings
        switch (old_config.descriptorOptions.normalizationStage) {
            case ::BEFORE_POOLING:
                desc_config.params.normalize_before_pooling = true;
                desc_config.params.normalize_after_pooling = false;
                break;
            case ::AFTER_POOLING:
                desc_config.params.normalize_before_pooling = false;
                desc_config.params.normalize_after_pooling = true;
                break;
            case ::NO_NORMALIZATION:
                desc_config.params.normalize_before_pooling = false;
                desc_config.params.normalize_after_pooling = false;
                break;
        }
        
        desc_config.params.use_color = (old_config.descriptorOptions.imageType == ::COLOR);
        desc_config.params.norm_type = old_config.descriptorOptions.normType;
        desc_config.params.scales = old_config.descriptorOptions.scales;
        
        // Stacking parameters
        if (desc_config.params.pooling == PoolingStrategy::STACKING) {
            desc_config.params.secondary_descriptor = toNewDescriptorType(static_cast<int>(old_config.descriptorOptions.descriptorType2));
        }
        
        new_config.descriptors.push_back(desc_config);
        
        // Convert evaluation parameters
        new_config.evaluation.params.match_threshold = old_config.matchThreshold;
        
        // Convert validation method
        switch (old_config.verificationType) {
            case ::HOMOGRAPHY:
                new_config.evaluation.params.validation_method = ValidationMethod::HOMOGRAPHY;
                break;
            case ::MATCHES:
                new_config.evaluation.params.validation_method = ValidationMethod::CROSS_IMAGE;
                break;
            case ::NO_VISUAL_VERIFICATION:
                new_config.evaluation.params.validation_method = ValidationMethod::NONE;
                break;
            default:
                new_config.evaluation.params.validation_method = ValidationMethod::HOMOGRAPHY;
        }
        
        return new_config;
    }
    
    ::experiment_config ConfigurationBridge::createOldConfigForDescriptor(
        const ExperimentConfig& new_config,
        size_t descriptor_index) {
        
        if (descriptor_index >= new_config.descriptors.size()) {
            throw std::runtime_error("Descriptor index out of range");
        }
        
        // Create a temporary config with just the specified descriptor
        ExperimentConfig temp_config = new_config;
        
        // Keep only the specified descriptor
        auto selected_descriptor = temp_config.descriptors[descriptor_index];
        temp_config.descriptors.clear();
        temp_config.descriptors.push_back(selected_descriptor);
        
        return toOldConfig(temp_config);
    }

} // namespace thesis_project::config
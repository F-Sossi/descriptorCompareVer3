#pragma once

#include "ExperimentConfig.hpp"
#include "../../../descriptor_compare/experiment_config.hpp"

namespace thesis_project {
namespace config {

    /**
     * @brief Bridge between new YAML configuration and existing experiment_config
     *
     * This allows us to use the new YAML-based configuration system
     * while maintaining compatibility with existing code.
     */
    class ConfigurationBridge {
    public:
        /**
         * @brief Convert new YAML config to existing experiment_config
         * @param new_config Modern YAML-based configuration
         * @return Traditional experiment_config for existing code
         */
        static ::experiment_config toOldConfig(const ExperimentConfig& new_config);

        /**
         * @brief Convert existing experiment_config to new format
         * @param old_config Traditional configuration
         * @return Modern YAML-compatible configuration
         */
        static ExperimentConfig fromOldConfig(const ::experiment_config& old_config);

        /**
         * @brief Create experiment_config for specific descriptor from YAML config
         * @param new_config YAML configuration
         * @param descriptor_index Which descriptor to use (for multi-descriptor experiments)
         * @return experiment_config set up for that specific descriptor
         */
        static ::experiment_config createOldConfigForDescriptor(
            const ExperimentConfig& new_config,
            size_t descriptor_index = 0);
    };

} // namespace config
} // namespace thesis_project

#include "PoolingFactory.hpp"
#include "NoPooling.hpp"
#include "DomainSizePooling.hpp"
#include "StackingPooling.hpp"
#include <stdexcept>

namespace thesis_project::pooling {

PoolingStrategyPtr PoolingFactory::createStrategy(::PoolingStrategy strategy) {
    switch (strategy) {
        case NONE:
            return std::make_unique<NoPooling>();
            
        case DOMAIN_SIZE_POOLING:
            return std::make_unique<DomainSizePooling>();
            
        case STACKING:
            return std::make_unique<StackingPooling>();
            
        default:
            throw std::runtime_error("Unknown pooling strategy: " + std::to_string(static_cast<int>(strategy)));
    }
}

PoolingStrategyPtr PoolingFactory::createFromConfig(const experiment_config& config) {
    return createStrategy(config.descriptorOptions.poolingStrategy);
}

std::vector<std::string> PoolingFactory::getAvailableStrategies() {
    return {
        "None",
        "DomainSizePooling", 
        "Stacking"
    };
}

} // namespace thesis_project::pooling
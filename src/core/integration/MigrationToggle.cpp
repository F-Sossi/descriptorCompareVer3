#include "MigrationToggle.hpp"
#include <atomic>

namespace thesis_project {
namespace integration {

static std::atomic<bool> g_enabled{false};

void MigrationToggle::setEnabled(bool value) { g_enabled.store(value, std::memory_order_relaxed); }
bool MigrationToggle::isEnabled() { return g_enabled.load(std::memory_order_relaxed); }

} // namespace integration
} // namespace thesis_project


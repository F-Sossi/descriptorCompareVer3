#pragma once

namespace thesis_project {
namespace integration {

class MigrationToggle {
public:
    static void setEnabled(bool value);
    static bool isEnabled();
};

} // namespace integration
} // namespace thesis_project


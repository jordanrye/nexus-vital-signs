#ifndef MIGRATION_H
#define MIGRATION_H

#include <string>

#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace Migration
{
    constexpr int SCHEMA_VERSION = 3;

    bool MigrateSettingsConfig(json& config, const std::string& fileName);
    bool MigrateLayoutConfig(json& config, const std::string& fileName);
    bool MigratePresetConfig(json& config, const std::string& fileName);
}

#endif // MIGRATION_H

#ifndef MIGRATION_H
#define MIGRATION_H

#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace Migration
{
    constexpr int SCHEMA_VERSION = 2;

    void MigrateLayoutConfig(json& config);
    void MigratePresetConfig(json& config);
}

#endif // MIGRATION_H

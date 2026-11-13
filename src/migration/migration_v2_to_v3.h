#ifndef MIGRATION_V2_TO_V3_H
#define MIGRATION_V2_TO_V3_H

#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace Migration
{
    void MigrateSettingsV2ToV3(json& config);
    void MigrateLayoutV2ToV3(json& config);
}

#endif // MIGRATION_V2_TO_V3_H

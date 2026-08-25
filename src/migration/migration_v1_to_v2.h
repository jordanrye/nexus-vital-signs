#ifndef MIGRATION_V1_TO_V2_H
#define MIGRATION_V1_TO_V2_H

#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace Migration
{
    void MigrateLayoutV1ToV2(json& config);
    void MigratePresetV1ToV2(json& config);
}

#endif // MIGRATION_V1_TO_V2_H

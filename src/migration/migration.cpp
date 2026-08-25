#include "migration.h"
#include "migration_v1_to_v2.h"
#include "../shared.h"

namespace Migration
{
    bool MigrateSettingsConfig(json& config, const std::string& fileName)
    {
        if (config.is_null()) return true;

        int version = config.value("schemaVersion", 1);

        if (version > SCHEMA_VERSION)
        {
            std::string message = "Settings file '" + fileName + "' is using an unknown schema version. "
                "You may be using an older version of Vital Signs, or you've got a configuration file from the future. "
                "Attempting to parse file, but some data may be lost.";
            APIDefs->Log(ELogLevel_WARNING, "VitalSigns", message.c_str());
        }
        else if (version < SCHEMA_VERSION)
        {
            std::string message = "Settings file '" + fileName + "' is using an older schema version. "
                "Migrating to current schema version.";
            APIDefs->Log(ELogLevel_INFO, "VitalSigns", message.c_str());
        }

        return true;
    }

    bool MigrateLayoutConfig(json& config, const std::string& fileName)
    {
        if (config.is_null()) return true;

        bool isLoaded = false;
        int version = config.value("schemaVersion", 1);

        if (version > SCHEMA_VERSION)
        {
            std::string message = "Layout file '" + fileName + "' is using an unknown schema version. "
                "You may be using an older version of Vital Signs, or you've got a configuration file from the future. "
                "Layout will not be loaded to avoid corruption.";
            APIDefs->Log(ELogLevel_CRITICAL, "VitalSigns", message.c_str());
        }
        else if (version == SCHEMA_VERSION)
        {
            isLoaded = true;
        }
        else if (version < SCHEMA_VERSION)
        {
            std::string message = "Layout file '" + fileName + "' is using an older schema version. "
                "Migrating to current schema version.";
            APIDefs->Log(ELogLevel_INFO, "VitalSigns", message.c_str());
            
            if (version < 2) { MigrateLayoutV1ToV2(config); }
    
            config["schemaVersion"] = SCHEMA_VERSION;

            isLoaded = true;
        }

        return isLoaded;
    }

    bool MigratePresetConfig(json& config, const std::string& fileName)
    {
        if (config.is_null()) return true;

        int version = config.value("schemaVersion", 1);

        if (version > SCHEMA_VERSION)
        {
            std::string message = "Presets file '" + fileName + "' is using an unknown schema version. "
                "You may be using an older version of Vital Signs, or you've got a configuration file from the future. "
                "Attempting to parse file, but some data may be lost.";
            APIDefs->Log(ELogLevel_WARNING, "VitalSigns", message.c_str());
        }
        else if (version < SCHEMA_VERSION)
        {
            std::string message = "Presets file '" + fileName + "' is using an older schema version. "
                "Migrating to current schema version.";
            APIDefs->Log(ELogLevel_INFO, "VitalSigns", message.c_str());
            
            if (version < 2) { MigratePresetV1ToV2(config); }
        }

        return true;
    }
}

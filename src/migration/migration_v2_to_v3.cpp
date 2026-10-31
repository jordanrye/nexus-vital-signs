#include "migration_v2_to_v3.h"

namespace Migration
{
    static void MigrateTriggersV2ToV3(json& node)
    {
        // Triggers are nested under many indicator types (group, glow, text, bar, icon list, etc.),
        // so walk the whole tree rather than tracking each location individually.
        if (node.is_object())
        {
            if (node.contains("trigger") && node["trigger"].is_object())
            {
                auto& trigger = node["trigger"];

                if (trigger.contains("category") && trigger["category"].is_string())
                {
                    const std::string category = trigger["category"].get<std::string>();
                    const std::string effect = (trigger.contains("effect") && trigger["effect"].is_string()) ? trigger["effect"].get<std::string>() : "";

                    // "Roles" renamed to "Party/Squad Roles", except "Hovered" and "Selected" which moved to "Targeting"
                    if (category == "Roles")
                    {
                        trigger["category"] = (effect == "Hovered" || effect == "Selected") ? "Targeting" : "Party/Squad Roles";
                    }
                    // "Primary Roles" and "Secondary Roles" merged into "Assigned Roles"
                    else if (category == "Primary Roles" || category == "Secondary Roles")
                    {
                        trigger["category"] = "Assigned Roles";
                    }
                }
            }

            for (auto& [key, value] : node.items())
            {
                MigrateTriggersV2ToV3(value);
            }
        }
        else if (node.is_array())
        {
            for (auto& element : node)
            {
                MigrateTriggersV2ToV3(element);
            }
        }
    }

    void MigrateLayoutV2ToV3(json& config)
    {
        if (config.contains("indicators") && config["indicators"].is_array())
        {
            MigrateTriggersV2ToV3(config["indicators"]);
        }
    }
}

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

    void MigrateSettingsV2ToV3(json& config)
    {
        // "Hide native frames" per group type moved into the "Native Frames" section
        config["native-hide-party-frames"] = config["party-hide-native"];
        config["native-hide-raid-frames"] = config["raid-hide-native"];
        config["native-hide-squad-frames"] = config["squad-hide-native"];

        config.erase("party-hide-native");
        config.erase("raid-hide-native");
        config.erase("squad-hide-native");
    }

    void MigrateLayoutV2ToV3(json& config)
    {
        if (config.contains("indicators") && config["indicators"].is_array())
        {
            MigrateTriggersV2ToV3(config["indicators"]);
        }
    }
}

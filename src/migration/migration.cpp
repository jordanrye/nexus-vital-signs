#include "migration.h"

namespace Migration
{
    static void MigrateIndicatorsV1ToV2(json& indicators)
    {
        for (auto& indicator : indicators)
        {
            if (indicator.contains("type"))
            {
                if (indicator["type"].get<std::string>() == "Highlight")
                {
                    json glow = json::object();
                    
                    // Convert type
                    indicator["type"] = "Glow";

                    // Convert properties
                    if (indicator.contains("highlight"))
                    {
                        auto& highlight = indicator["highlight"];
                        
                        // New properties hardcoded to mimic old "Highlight" style
                        glow["position"] = "Inner";
                        glow["thicknessType"] = "Percentage";
                        glow["hardness"] = 0.0f;

                        // Old properties which can be copied across
                        glow["color"] = highlight.contains("color") ? highlight["color"] : json::array({1.0, 1.0, 1.0, 1.0});
                        glow["thickness"] = highlight.contains("size") ? highlight["size"].get<float>() : 20.0f;
                        std::string oldPosition = highlight.contains("position") ? highlight["position"].get<std::string>() : "All";
                        glow["directionTop"] = (oldPosition == "All" || oldPosition == "Top" || oldPosition == "Full");
                        glow["directionBottom"] = (oldPosition == "All" || oldPosition == "Bottom" || oldPosition == "Full");
                        glow["directionLeft"] = (oldPosition == "All" || oldPosition == "Left" || oldPosition == "Full");
                        glow["directionRight"] = (oldPosition == "All" || oldPosition == "Right" || oldPosition == "Full");
                        if (highlight.contains("trigger") && !highlight["trigger"].is_null() && !highlight["trigger"].empty())
                        {
                            glow["trigger"] = highlight["trigger"];
                        }
                        
                        // Erase old properties
                        indicator.erase("highlight");
                    }
                    // Insert new properties
                    indicator["glow"] = glow;
                }
                else if (indicator["type"].get<std::string>() == "Group")
                {
                    if (indicator.contains("group") && indicator["group"].contains("indicators") && indicator["group"]["indicators"].is_array())
                    {
                        MigrateIndicatorsV1ToV2(indicator["group"]["indicators"]);
                    }
                }
            }
        }
    }

    void MigrateLayoutConfig(json& config)
    {
        if (config.is_null()) return;

        int version = 1;

        if (config.contains("schemaVersion"))
        {
            version = config["schemaVersion"].get<int>();
        }

        if (version < 2)
        {
            // Upgrade v1 to v2
            if (config.contains("indicators") && config["indicators"].is_array())
            {
                MigrateIndicatorsV1ToV2(config["indicators"]);
            }

            if (config.contains("layout") && config["layout"].is_object())
            {
                auto& layout = config["layout"];
                if (layout.contains("spacing"))
                {
                    int legacySpacing = layout["spacing"].get<int>();
                    
                    if (layout.contains("grid") && layout["grid"].is_object())
                    {
                        layout["grid"]["spacing-horizontal"] = legacySpacing;
                        layout["grid"]["spacing-vertical"] = legacySpacing;
                    }
                    
                    if (layout.contains("radial") && layout["radial"].is_object())
                    {
                        layout["radial"]["spacing"] = legacySpacing;
                    }

                    layout.erase("spacing");
                }
            }

            config["schemaVersion"] = 2;
        }
    }

    void MigratePresetConfig(json& config)
    {
        if (config.is_null()) return;

        int version = 1;

        if (config.contains("schemaVersion"))
        {
            version = config["schemaVersion"].get<int>();
        }

        if (version < 2)
        {
            // Upgrade v1 to v2

            config["schemaVersion"] = 2;
        }
    }
}

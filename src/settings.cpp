#include "settings.h"

#include <filesystem>
#include <fstream>
#include <map>

#include "shared.h"
#include "json_addon_serialisers.h"
#include "json_imgui_serialisers.h"
#include "json_serialisers.h"

using json = nlohmann::json;

namespace Settings
{
    std::mutex Mutex;
    json s_LastSavedSettings = json::object();
    json s_LastSavedPresets = json::object();
    std::map<std::filesystem::path, json> s_LastSavedLayouts;

    static bool jsonParse(const std::filesystem::path& aFilePath, json& jObject);
    static json jsonSerialiseSettings();
    static json jsonSerialisePresets();
    static json jsonSerialiseLayouts(const LayoutConfig_t& layoutConfig);
    static void LoadLayout(const std::filesystem::path& aFilePath);
    static void SaveLayout(const std::filesystem::path& aFilePath);

    void LoadSettings()
    {
        json settings = json::object();

        if (jsonParse(APIDefs->Paths.GetAddonDirectory("VitalSigns/settings.json"), settings))
        {
            if (!settings.is_null())
            {
                dser_GeneralConfig_t(settings, ConfigGeneral);
            }
        }

        s_LastSavedSettings = jsonSerialiseSettings();
    }

    void SaveSettings()
    {
        json settings = jsonSerialiseSettings();
        s_LastSavedSettings = settings;

        /* write updated configuration to disk */
        Mutex.lock();
        {
            std::ofstream file(APIDefs->Paths.GetAddonDirectory("VitalSigns/settings.json"));
            file << settings.dump(1, '\t') << std::endl;
            file.close();
        }
        Mutex.unlock();
    }

    bool IsDirtySettings()
    {
        json current = jsonSerialiseSettings();
        return current != s_LastSavedSettings;
    }

    void LoadPresets()
    {
        json settings = json::object();

        /* load settings */
        if (jsonParse(APIDefs->Paths.GetAddonDirectory("VitalSigns/presets.json"), settings))
        {
            if (!settings.is_null())
            {
                /* colors settings */
                if (!settings["colors"].is_null())
                {
                    auto& colors = settings["colors"];

                    /* Default Colours */
                    dser_ImColor(colors["default-background"], ColourPresets.COLOUR_BACKGROUND);
                    dser_ImColor(colors["default-health"], ColourPresets.COLOUR_HEALTH);
                    dser_ImColor(colors["default-health-downed"], ColourPresets.COLOUR_HEALTH_DOWNED);
                    dser_ImColor(colors["default-health-defeated"], ColourPresets.COLOUR_HEALTH_DEFEATED);
                    dser_ImColor(colors["default-health-shroud-necromancer"], ColourPresets.COLOUR_SHROUD_NECROMANCER);
                    dser_ImColor(colors["default-health-shroud-specter"], ColourPresets.COLOUR_SHROUD_SPECTER);
                    dser_ImColor(colors["default-barrier"], ColourPresets.COLOUR_BARRIER);
                    dser_ImColor(colors["default-hovered"], ColourPresets.COLOUR_HOVERED);

                    /* Profession Colours */
                    dser_ImColor(colors["profession-background"], ColourPresets.COLOUR_PROF_BACKGROUND);
                    dser_ImColor(colors["profession-health-elementalist"], ColourPresets.COLOUR_PROF_HEALTH_ELEMENTALIST);
                    dser_ImColor(colors["profession-health-engineer"], ColourPresets.COLOUR_PROF_HEALTH_ENGINEER);
                    dser_ImColor(colors["profession-health-guardian"], ColourPresets.COLOUR_PROF_HEALTH_GUARDIAN);
                    dser_ImColor(colors["profession-health-mesmer"], ColourPresets.COLOUR_PROF_HEALTH_MESMER);
                    dser_ImColor(colors["profession-health-necromancer"], ColourPresets.COLOUR_PROF_HEALTH_NECROMANCER);
                    dser_ImColor(colors["profession-health-ranger"], ColourPresets.COLOUR_PROF_HEALTH_RANGER);
                    dser_ImColor(colors["profession-health-revenant"], ColourPresets.COLOUR_PROF_HEALTH_REVENANT);
                    dser_ImColor(colors["profession-health-thief"], ColourPresets.COLOUR_PROF_HEALTH_THIEF);
                    dser_ImColor(colors["profession-health-warrior"], ColourPresets.COLOUR_PROF_HEALTH_WARRIOR);
                    dser_ImColor(colors["profession-health-downed"], ColourPresets.COLOUR_PROF_HEALTH_DOWNED);
                    dser_ImColor(colors["profession-health-defeated"], ColourPresets.COLOUR_PROF_HEALTH_DEFEATED);
                    dser_ImColor(colors["profession-health-shroud-necromancer"], ColourPresets.COLOUR_PROF_SHROUD_NECROMANCER);
                    dser_ImColor(colors["profession-health-shroud-specter"], ColourPresets.COLOUR_PROF_SHROUD_SPECTER);
                    dser_ImColor(colors["profession-barrier"], ColourPresets.COLOUR_PROF_BARRIER);
                    dser_ImColor(colors["profession-hovered"], ColourPresets.COLOUR_PROF_HOVERED);
                    
                    /* Heat Map Colours */
                    dser_ImColor(colors["heat-map-background"], ColourPresets.COLOUR_HEATMAP_BACKGROUND);
                    dser_ImColor(colors["heat-map-health-100"], ColourPresets.COLOUR_HEATMAP_HEALTH_100);
                    dser_ImColor(colors["heat-map-health-75"], ColourPresets.COLOUR_HEATMAP_HEALTH_75);
                    dser_ImColor(colors["heat-map-health-50"], ColourPresets.COLOUR_HEATMAP_HEALTH_50);
                    dser_ImColor(colors["heat-map-health-25"], ColourPresets.COLOUR_HEATMAP_HEALTH_25);
                    dser_ImColor(colors["heat-map-health-0"], ColourPresets.COLOUR_HEATMAP_HEALTH_0);
                    dser_ImColor(colors["heat-map-health-downed"], ColourPresets.COLOUR_HEATMAP_HEALTH_DOWNED);
                    dser_ImColor(colors["heat-map-health-defeated"], ColourPresets.COLOUR_HEATMAP_HEALTH_DEFEATED);
                    dser_ImColor(colors["heat-map-health-shroud-necromancer"], ColourPresets.COLOUR_HEATMAP_SHROUD_NECROMANCER);
                    dser_ImColor(colors["heat-map-health-shroud-specter"], ColourPresets.COLOUR_HEATMAP_SHROUD_SPECTER);
                    dser_ImColor(colors["heat-map-barrier"], ColourPresets.COLOUR_HEATMAP_BARRIER);
                    dser_ImColor(colors["heat-map-hovered"], ColourPresets.COLOUR_HEATMAP_HOVERED);

                    /** TODO: Move into separate `Border` config. */
                    dser_ImColor(colors["border"], BorderPresets.COLOUR_BORDER);
                }

                if (!settings["text"].is_null())
                {
                    dser_TextStyle_t(settings["text"], ConfigText);
                }

                if (!settings["icon-duration"].is_null())
                {
                    dser_IconText_t(settings["icon-duration"], ConfigIconDuration);
                }

                if (!settings["icon-stacks"].is_null())
                {
                    dser_IconText_t(settings["icon-stacks"], ConfigIconStacks);
                }
            }
        }

        s_LastSavedPresets = jsonSerialisePresets();
    }

    void SavePresets()
    {
        json settings = jsonSerialisePresets();
        s_LastSavedPresets = settings;

        /* write updated configuration to disk */
        Mutex.lock();
        {
            std::ofstream file(APIDefs->Paths.GetAddonDirectory("VitalSigns/presets.json"));
            file << settings.dump(1, '\t') << std::endl;
            file.close();
        }
        Mutex.unlock();
    }

    bool IsDirtyPresets()
    {
        json current = jsonSerialisePresets();
        return current != s_LastSavedPresets;
    }

    void LoadAllLayouts()
    {
        for (const auto& file : std::filesystem::directory_iterator(PacksDir))
        {
            if (file.is_regular_file())
            {
                const std::filesystem::path filePath = file.path();
                
                if (filePath.extension() == ".json")
                {
                    LoadLayout(filePath);
                } 
                else 
                {
                    std::string warningMessage = filePath.filename().string() + " is not a valid layout file.";
                    APIDefs->Log(ELogLevel_WARNING, "VitalSigns", warningMessage.c_str());
                }
            }
        }
    }

    void SaveAllLayouts()
    {
        for (const auto& file : g_LayoutManager.GetAllLayouts())
        {
            SaveLayout(file.first);
        }
    }

    bool IsDirtyLayouts()
    {
        for (const auto& file : g_LayoutManager.GetAllLayouts())
        {
            auto it = s_LastSavedLayouts.find(file.first);
            if (it == s_LastSavedLayouts.end()) return true;

            json current = jsonSerialiseLayouts(file.second);
            if (current != it->second) return true;
        }
        return false;
    }

    bool jsonParse(const std::filesystem::path& aFilePath, json& jObject)
    {
        bool success = false;

        /* Attempt to parse JSON file. */
        Mutex.lock();
        {
            try
            {
                std::ifstream file(aFilePath);
                jObject = json::parse(file);
                file.close();
                success = true;
            }
            catch (json::parse_error& ex)
            {
                std::string warningMessage = aFilePath.filename().string() + " could not be parsed.";
                APIDefs->Log(ELogLevel_WARNING, "VitalSigns", warningMessage.c_str());
                APIDefs->Log(ELogLevel_WARNING, "VitalSigns", ex.what());
            }
        }
        Mutex.unlock();

        return success;
    }

    json jsonSerialiseSettings()
    {
        return ser_GeneralConfig_t(ConfigGeneral);
    }

    json jsonSerialisePresets()
    {
        json settings = json::object();

        /* colors settings */
        settings["colors"] = json::object();
        {
            auto& colors = settings["colors"];

            /* Default Colours  */
            colors["default-background"] = ser_ImColor(ColourPresets.COLOUR_BACKGROUND);
            colors["default-health"] = ser_ImColor(ColourPresets.COLOUR_HEALTH);
            colors["default-health-downed"] = ser_ImColor(ColourPresets.COLOUR_HEALTH_DOWNED);
            colors["default-health-defeated"] = ser_ImColor(ColourPresets.COLOUR_HEALTH_DEFEATED);
            colors["default-health-shroud-necromancer"] = ser_ImColor(ColourPresets.COLOUR_SHROUD_NECROMANCER);
            colors["default-health-shroud-specter"] = ser_ImColor(ColourPresets.COLOUR_SHROUD_SPECTER);
            colors["default-barrier"] = ser_ImColor(ColourPresets.COLOUR_BARRIER);
            colors["default-hovered"] = ser_ImColor(ColourPresets.COLOUR_HOVERED);
            
            /* Profession Colours */
            colors["profession-background"] = ser_ImColor(ColourPresets.COLOUR_PROF_BACKGROUND);
            colors["profession-health-elementalist"] = ser_ImColor(ColourPresets.COLOUR_PROF_HEALTH_ELEMENTALIST);
            colors["profession-health-engineer"] = ser_ImColor(ColourPresets.COLOUR_PROF_HEALTH_ENGINEER);
            colors["profession-health-guardian"] = ser_ImColor(ColourPresets.COLOUR_PROF_HEALTH_GUARDIAN);
            colors["profession-health-mesmer"] = ser_ImColor(ColourPresets.COLOUR_PROF_HEALTH_MESMER);
            colors["profession-health-necromancer"] = ser_ImColor(ColourPresets.COLOUR_PROF_HEALTH_NECROMANCER);
            colors["profession-health-ranger"] = ser_ImColor(ColourPresets.COLOUR_PROF_HEALTH_RANGER);
            colors["profession-health-revenant"] = ser_ImColor(ColourPresets.COLOUR_PROF_HEALTH_REVENANT);
            colors["profession-health-thief"] = ser_ImColor(ColourPresets.COLOUR_PROF_HEALTH_THIEF);
            colors["profession-health-warrior"] = ser_ImColor(ColourPresets.COLOUR_PROF_HEALTH_WARRIOR);
            colors["profession-health-downed"] = ser_ImColor(ColourPresets.COLOUR_PROF_HEALTH_DOWNED);
            colors["profession-health-defeated"] = ser_ImColor(ColourPresets.COLOUR_PROF_HEALTH_DEFEATED);
            colors["profession-health-shroud-necromancer"] = ser_ImColor(ColourPresets.COLOUR_PROF_SHROUD_NECROMANCER);
            colors["profession-health-shroud-specter"] = ser_ImColor(ColourPresets.COLOUR_PROF_SHROUD_SPECTER);
            colors["profession-barrier"] = ser_ImColor(ColourPresets.COLOUR_PROF_BARRIER);
            colors["profession-hovered"] = ser_ImColor(ColourPresets.COLOUR_PROF_HOVERED);
            
            /* Heat Map Colours */
            colors["heat-map-background"] = ser_ImColor(ColourPresets.COLOUR_HEATMAP_BACKGROUND);
            colors["heat-map-health-100"] = ser_ImColor(ColourPresets.COLOUR_HEATMAP_HEALTH_100);
            colors["heat-map-health-75"] = ser_ImColor(ColourPresets.COLOUR_HEATMAP_HEALTH_75);
            colors["heat-map-health-50"] = ser_ImColor(ColourPresets.COLOUR_HEATMAP_HEALTH_50);
            colors["heat-map-health-25"] = ser_ImColor(ColourPresets.COLOUR_HEATMAP_HEALTH_25);
            colors["heat-map-health-0"] = ser_ImColor(ColourPresets.COLOUR_HEATMAP_HEALTH_0);
            colors["heat-map-health-downed"] = ser_ImColor(ColourPresets.COLOUR_HEATMAP_HEALTH_DOWNED);
            colors["heat-map-health-defeated"] = ser_ImColor(ColourPresets.COLOUR_HEATMAP_HEALTH_DEFEATED);
            colors["heat-map-health-shroud-necromancer"] = ser_ImColor(ColourPresets.COLOUR_HEATMAP_SHROUD_NECROMANCER);
            colors["heat-map-health-shroud-specter"] = ser_ImColor(ColourPresets.COLOUR_HEATMAP_SHROUD_SPECTER);
            colors["heat-map-barrier"] = ser_ImColor(ColourPresets.COLOUR_HEATMAP_BARRIER);
            colors["heat-map-hovered"] = ser_ImColor(ColourPresets.COLOUR_HEATMAP_HOVERED);
            
            /** TODO: Move into separate `Border` config. */
            colors["border"] = ser_ImColor(BorderPresets.COLOUR_BORDER);
        }

        settings["text"] = ser_TextStyle_t(ConfigText);
        settings["icon-duration"] = ser_IconText_t(ConfigIconDuration);
        settings["icon-stacks"] = ser_IconText_t(ConfigIconStacks);

        return settings;
    }

    json jsonSerialiseLayouts(const LayoutConfig_t& layoutConfig)
    {
        json layout = json::object();

        /* General */
        layout["name"] = layoutConfig.name;
        layout["colors"] = layoutConfig.colors;
        layout["position"] = ser_Position_t(layoutConfig.position);
        layout["layout"] = ser_Layout_t(layoutConfig.layout);

        /* Indicators */
        layout["indicators"] = json::array();
        for (auto& indicator : layoutConfig.indicators)
        {
            layout["indicators"].push_back(ser_Indicator_t(indicator));
        }

        return layout;
    }

    void LoadLayout(const std::filesystem::path& aFilePath)
    {
        json layout = json::object();

        if (jsonParse(aFilePath, layout))
        {
            LayoutConfig_t layoutConfig{};

            if (!layout.is_null())
            {
                layoutConfig.id = g_LayoutEditor.GenerateUID();
    
                dser_BasicType(layout["name"], layoutConfig.name);
                dser_BasicType(layout["colors"], layoutConfig.colors);
                dser_Position_t(layout["position"], layoutConfig.position);
                dser_Layout_t(layout["layout"], layoutConfig.layout);
    
                auto& indicators = layout["indicators"];
                if (!indicators.is_null() && indicators.is_array())
                {
                    for (auto& indicator_json : indicators)
                    {
                        if (!indicator_json.is_null())
                        {
                            Indicator_t indicator{};
                            dser_Indicator_t(indicator_json, indicator);
                            indicator.id = g_LayoutEditor.GenerateUID();

                            layoutConfig.indicators.push_back(indicator);
                        }
                    }
                }
            }
    
            g_LayoutManager.Insert(aFilePath, layoutConfig);
            s_LastSavedLayouts[aFilePath] = jsonSerialiseLayouts(layoutConfig);
        }
    }

    void SaveLayout(const std::filesystem::path& aFilePath)
    {
        auto& layoutConfig = g_LayoutManager.GetLayoutFromFilePath(aFilePath);
        json layout = jsonSerialiseLayouts(layoutConfig);
        s_LastSavedLayouts[aFilePath] = layout;

        /* write updated configuration to disk */
        Mutex.lock();
        {
            /* save to settings.json */
            std::ofstream file(aFilePath);
            file << layout.dump(1, '\t') << std::endl;
            file.close();
        }
        Mutex.unlock();
    }

} // namespace Settings

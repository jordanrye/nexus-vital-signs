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

    bool jsonParse(const std::filesystem::path& aFilePath, json& jObject);

    void dser_TextConfig_t(json& object, TextConfig_t& config)
    {
        if (!object.is_null())
        {
            dser_BasicType(object["font-type"], config.fontType);
            dser_BasicType(object["font"], config.font);
            dser_BasicType(object["font-size-type"], config.fontSizeType);
            dser_BasicType(object["font-size"], config.fontSize);
            dser_ImColor(object["color"], config.color);
            dser_BasicType(object["shadow"], config.shadow);
            dser_ImColor(object["shadow-color"], config.shadowColor);
            dser_BasicType(object["outline"], config.outline);
            dser_ImColor(object["outline-color"], config.outlineColor);
        }
    }

    void dser_IconTextConfig_t(json& object, IconTextConfig_t& config)
    {
        if (!object.is_null())
        {
            dser_TextConfig_t(object, config);
            dser_Position_t(object["position"], config.position);
        }
    }

    json ser_TextConfig_t(const TextConfig_t& config)
    {
        json object = json::object();
        object["font-type"] = config.fontType;
        object["font"] = config.font;
        object["font-size-type"] = config.fontSizeType;
        object["font-size"] = config.fontSize;
        object["color"] = ser_ImColor(config.color);
        object["shadow"] = config.shadow;
        object["shadow-color"] = ser_ImColor(config.shadowColor);
        object["outline"] = config.outline;
        object["outline-color"] = ser_ImColor(config.outlineColor);
        return object;
    }

    json ser_IconTextConfig_t(const IconTextConfig_t& config)
    {
        json object = ser_TextConfig_t(config);
        object["position"] = ser_Position_t(config.position);
        return object;
    }

    void Load(const std::filesystem::path& aSettingsPath)
    {
        json settings = json::object();

        /* load settings */
        if (jsonParse(aSettingsPath, settings))
        {
            if (!settings.is_null())
            {
                if (!settings["general"].is_null())
                {
                    dser_GeneralConfig_t(settings["general"], ConfigGeneral);
                }

                /* colors settings */
                if (!settings["colors"].is_null())
                {
                    auto& colors = settings["colors"];

                    /* Simple Colours */
                    dser_ImColor(colors["background"], ColourPresets.COLOUR_BACKGROUND);
                    dser_ImColor(colors["health"], ColourPresets.COLOUR_HEALTH);
                    dser_ImColor(colors["health-downed"], ColourPresets.COLOUR_HEALTH_DOWNED);
                    dser_ImColor(colors["health-defeated"], ColourPresets.COLOUR_HEALTH_DEFEATED);
                    dser_ImColor(colors["health-shroud-necromancer"], ColourPresets.COLOUR_SHROUD_NECROMANCER);
                    dser_ImColor(colors["health-shroud-specter"], ColourPresets.COLOUR_SHROUD_SPECTER);
                    dser_ImColor(colors["barrier"], ColourPresets.COLOUR_BARRIER);
                    dser_ImColor(colors["hovered"], ColourPresets.COLOUR_HOVERED);

                    /** TODO: Move into separate `Border` config. */
                    dser_ImColor(colors["border"], BorderPresets.COLOUR_BORDER);
                }

                if (!settings["text"].is_null())
                {
                    dser_TextConfig_t(settings["text"], ConfigText);
                }

                if (!settings["icon-duration"].is_null())
                {
                    dser_IconTextConfig_t(settings["icon-duration"], ConfigIconDuration);
                }

                if (!settings["icon-stacks"].is_null())
                {
                    dser_IconTextConfig_t(settings["icon-stacks"], ConfigIconStacks);
                }
            }
        }
    }

    void Save(const std::filesystem::path& aSettingsPath)
    {
        json settings = json::object();

        settings["general"] = ser_GeneralConfig_t(ConfigGeneral);

        /* colors settings */
        settings["colors"] = json::object();
        {
            auto& colors = settings["colors"];

            /* Simple Colours  */
            colors["background"] = ser_ImColor(ColourPresets.COLOUR_BACKGROUND);
            colors["health"] = ser_ImColor(ColourPresets.COLOUR_HEALTH);
            colors["health-downed"] = ser_ImColor(ColourPresets.COLOUR_HEALTH_DOWNED);
            colors["health-defeated"] = ser_ImColor(ColourPresets.COLOUR_HEALTH_DEFEATED);
            colors["health-shroud-necromancer"] = ser_ImColor(ColourPresets.COLOUR_SHROUD_NECROMANCER);
            colors["health-shroud-specter"] = ser_ImColor(ColourPresets.COLOUR_SHROUD_SPECTER);
            colors["barrier"] = ser_ImColor(ColourPresets.COLOUR_BARRIER);
            colors["hovered"] = ser_ImColor(ColourPresets.COLOUR_HOVERED);
            
            /** TODO: Move into separate `Border` config. */
            colors["border"] = ser_ImColor(BorderPresets.COLOUR_BORDER);
        }

        settings["text"] = ser_TextConfig_t(ConfigText);
        settings["icon-duration"] = ser_IconTextConfig_t(ConfigIconDuration);
        settings["icon-stacks"] = ser_IconTextConfig_t(ConfigIconStacks);

        /* write updated configuration to disk */
        Mutex.lock();
        {
            /* save to settings.json */
            std::ofstream file(aSettingsPath);
            file << settings.dump(1, '\t') << std::endl;
            file.close();
        }
        Mutex.unlock();
    }

    void LoadLayout(const std::filesystem::path& layoutFilePath)
    {
        json layout = json::object();

        if (jsonParse(layoutFilePath, layout))
        {
            LayoutConfig_t layoutConfig{};

            if (!layout.is_null())
            {
                layoutConfig.id = g_LayoutEditor.GenerateUID();
    
                dser_BasicType(layout["name"], layoutConfig.name);
                dser_BasicType(layout["colors"], layoutConfig.colors);
                dser_Position_t(layout["position"], layoutConfig.position);
                dser_Layout_t(layout["layout"], layoutConfig.layout);
                // dser_Behaviour_t(layout["behaviour"], layoutConfig.behaviour);
    
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
    
            g_LayoutManager.Insert(layoutFilePath, layoutConfig);
        }
    }

    void SaveLayout(const std::filesystem::path& layoutFilePath)
    {
        json layout = json::object();
        auto& layoutConfig = g_LayoutManager.GetLayoutFromFilePath(layoutFilePath);

        /* General */
        layout["name"] = layoutConfig.name;
        layout["colors"] = layoutConfig.colors;
        layout["position"] = ser_Position_t(layoutConfig.position);
        layout["layout"] = ser_Layout_t(layoutConfig.layout);
        // layout["behaviour"] = ser_Behaviour_t(layoutConfig.behaviour);

        /* Indicators */
        layout["indicators"] = json::array();
        for (auto& indicator : layoutConfig.indicators)
        {
            layout["indicators"].push_back(ser_Indicator_t(indicator));
        }

        /* write updated configuration to disk */
        Mutex.lock();
        {
            /* save to settings.json */
            std::ofstream file(layoutFilePath);
            file << layout.dump(1, '\t') << std::endl;
            file.close();
        }
        Mutex.unlock();
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

} // namespace Settings

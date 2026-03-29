#include "ui_common.h"

#include <filesystem>
#include <map>

#include "nexus/Nexus.h"

#include "shared.h"
#include "utilities.h"

namespace UI {

    ImColor GetBackgroundColour(const ColourPresets_t& config, const std::string& palette)
    {
        if (palette == "Generic")
        {
            return config.COLOUR_BACKGROUND;
        }
        else if (palette == "Profession")
        {
            return config.COLOUR_PROF_BACKGROUND;
        }
    }

    ImColor GetHealthColour(const ColourPresets_t& config, const std::string& palette, VitalSignsDataLink::E_HEALTH_TYPE healthType, VitalSignsDataLink::EProfession profession)
    {
        ImColor colour{};

        if (palette == "Generic")
        {
            switch (healthType)
            {
                case VitalSignsDataLink::E_HEALTH_ALIVE:
                    colour = config.COLOUR_HEALTH;
                    break;
                case VitalSignsDataLink::E_HEALTH_DOWNED:
                    colour = config.COLOUR_HEALTH_DOWNED;
                    break;
                case VitalSignsDataLink::E_HEALTH_DEFEATED:
                    colour = config.COLOUR_HEALTH_DEFEATED;
                    break;
                case VitalSignsDataLink::E_HEALTH_SHROUD_NECROMANCER:
                    colour = config.COLOUR_SHROUD_NECROMANCER;
                    break;
                case VitalSignsDataLink::E_HEALTH_SHROUD_SPECTER:
                    colour = config.COLOUR_SHROUD_SPECTER;
                    break;
            }
        }
        else if (palette == "Profession")
        {
            float h, s, v;

            switch (profession)
            {
                case VitalSignsDataLink::EProfession::Elementalist:
                    colour = config.COLOUR_PROF_HEALTH_ELEMENTALIST;
                    break;
                case VitalSignsDataLink::EProfession::Engineer:
                    colour = config.COLOUR_PROF_HEALTH_ENGINEER;
                    break;
                case VitalSignsDataLink::EProfession::Guardian:
                    colour = config.COLOUR_PROF_HEALTH_GUARDIAN;
                    break;
                case VitalSignsDataLink::EProfession::Mesmer:
                    colour = config.COLOUR_PROF_HEALTH_MESMER;
                    break;
                case VitalSignsDataLink::EProfession::Necromancer:
                    colour = config.COLOUR_PROF_HEALTH_NECROMANCER;
                    break;
                case VitalSignsDataLink::EProfession::Ranger:
                    colour = config.COLOUR_PROF_HEALTH_RANGER;
                    break;
                case VitalSignsDataLink::EProfession::Revenant:
                    colour = config.COLOUR_PROF_HEALTH_REVENANT;
                    break;
                case VitalSignsDataLink::EProfession::Thief:
                    colour = config.COLOUR_PROF_HEALTH_THIEF;
                    break;
                case VitalSignsDataLink::EProfession::Warrior:
                    colour = config.COLOUR_PROF_HEALTH_WARRIOR;
                    break;
            }

            switch (healthType)
            {
                case VitalSignsDataLink::E_HEALTH_DOWNED:
                    colour = config.COLOUR_HEALTH_DOWNED;
                    break;
                case VitalSignsDataLink::E_HEALTH_DEFEATED:
                    colour = config.COLOUR_HEALTH_DEFEATED;
                    break;
                case VitalSignsDataLink::E_HEALTH_SHROUD_NECROMANCER:
                    ImGui::ColorConvertRGBtoHSV(colour.Value.x, colour.Value.y, colour.Value.z, h, s, v);
                    v *= 0.5f;
                    v = max(v, 0.0f);
                    v = min(v, 255.0f);
                    ImGui::ColorConvertHSVtoRGB(h, s, v, colour.Value.x, colour.Value.y, colour.Value.z);
                    break;
                case VitalSignsDataLink::E_HEALTH_SHROUD_SPECTER:
                    ImGui::ColorConvertRGBtoHSV(colour.Value.x, colour.Value.y, colour.Value.z, h, s, v);
                    v *= 0.5f;
                    v = max(v, 0.0f);
                    v = min(v, 255.0f);
                    ImGui::ColorConvertHSVtoRGB(h, s, v, colour.Value.x, colour.Value.y, colour.Value.z);
                    break;
            }
        }

        return colour;
    }

    ImColor GetBarrierColour(const ColourPresets_t& config, const std::string& palette)
    {
        if (palette == "Generic")
        {
            return config.COLOUR_BARRIER;
        }
        else if (palette == "Profession")
        {
            return config.COLOUR_PROF_BARRIER;
        }
    }

    Texture* GetOrCreateTexture(const std::string& textureSource, const std::string& texturePath)
    {
        Texture* texture = APIDefs->Textures.Get(texturePath.c_str());

        // Create the Texture if it does not exist.
        if (texture == nullptr && !texturePath.empty())
        {
            if (textureSource == "File")
            {
                if (texturePath != "")
                {
                    if (std::filesystem::path(texturePath).is_relative())
                    {
                        texture = APIDefs->Textures.GetOrCreateFromFile(texturePath.c_str(), (GameDir / texturePath).string().c_str());
                    }
                    else
                    {
                        texture = APIDefs->Textures.GetOrCreateFromFile(texturePath.c_str(), texturePath.c_str());
                    }
                }
            }
            else if (textureSource == "URL")
            {
                if (texturePath != "")
                {
                    std::string url = texturePath;
                    std::string scheme = url_utils::getScheme(url);
                    std::string domain = url_utils::getDomain(url);
                    std::string path = url_utils::getPath(url);

                    if ((scheme.length() != 0) && (domain.length() != 0) && (path.length() != 0))
                    {
                        texture = APIDefs->Textures.GetOrCreateFromURL(url.c_str(), std::string(scheme + domain).c_str(), path.c_str());
                    }
                }
            }
        }

        return texture;
    }

} // namespace UI

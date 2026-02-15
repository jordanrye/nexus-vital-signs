#include "ui_common.h"

#include <filesystem>
#include <map>

#include "nexus/Nexus.h"

#include "shared.h"
#include "utilities.h"

namespace UI {

    ImColor GetHealthColour(const ColourPresets_t& config, VitalSignsData::E_HEALTH_TYPE healthType)
    {
        ImColor colour{};

        switch (healthType)
        {
            case VitalSignsData::E_HEALTH_ALIVE:
                colour = config.COLOUR_HEALTH;
                break;
            case VitalSignsData::E_HEALTH_DOWNED:
                colour = config.COLOUR_HEALTH_DOWNED;
                break;
            case VitalSignsData::E_HEALTH_DEFEATED:
                colour = config.COLOUR_HEALTH_DEFEATED;
                break;
            case VitalSignsData::E_HEALTH_SHROUD_NECROMANCER:
                colour = config.COLOUR_SHROUD_NECROMANCER;
                break;
            case VitalSignsData::E_HEALTH_SHROUD_SPECTER:
                colour = config.COLOUR_SHROUD_SPECTER;
                break;
        }

        return colour;
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

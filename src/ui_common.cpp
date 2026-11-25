#include "ui_common.h"

#include <filesystem>
#include <map>

#include "nexus/Nexus.h"

#include "shared.h"
#include "utilities.h"

namespace UI {

    ImColor GetBackgroundColour(const ColourPresets_t& config, const std::string& palette, VitalSignsDataLink::EHealthType healthType)
    {
        if (palette == "Default")
        {
            return (healthType == VitalSignsDataLink::EHealthType::Unknown) ? config.COLOUR_BACKGROUND_UNKNOWN : config.COLOUR_BACKGROUND;
        }
        else if (palette == "Profession")
        {
            return (healthType == VitalSignsDataLink::EHealthType::Unknown) ? config.COLOUR_PROF_BACKGROUND_UNKNOWN : config.COLOUR_PROF_BACKGROUND;
        }
        else if (palette == "Heat Map")
        {
            return (healthType == VitalSignsDataLink::EHealthType::Unknown) ? config.COLOUR_HEATMAP_BACKGROUND_UNKNOWN : config.COLOUR_HEATMAP_BACKGROUND;
        }
        else if (palette == "Role")
        {
            return (healthType == VitalSignsDataLink::EHealthType::Unknown) ? config.COLOUR_ROLE_BACKGROUND_UNKNOWN : config.COLOUR_ROLE_BACKGROUND;
        }
        return ImColor();
    }

    ImColor GetHealthColour(const ColourPresets_t& config, const std::string& palette, VitalSignsDataLink::EHealthType healthType, float health, VitalSignsDataLink::EProfession profession, VitalSignsDataLink::EPrimaryRole primaryRole)
    {
        ImColor colour{};

        if (palette == "Default")
        {
            switch (healthType)
            {
                case VitalSignsDataLink::EHealthType::Alive:
                    colour = config.COLOUR_HEALTH;
                    break;
                case VitalSignsDataLink::EHealthType::Downed:
                    colour = config.COLOUR_HEALTH_DOWNED;
                    break;
                case VitalSignsDataLink::EHealthType::Defeated:
                    colour = config.COLOUR_HEALTH_DEFEATED;
                    break;
                case VitalSignsDataLink::EHealthType::ShroudNecromancer:
                    colour = config.COLOUR_SHROUD_NECROMANCER;
                    break;
                case VitalSignsDataLink::EHealthType::ShroudSpecter:
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
                case VitalSignsDataLink::EHealthType::Downed:
                    colour = config.COLOUR_PROF_HEALTH_DOWNED;
                    break;
                case VitalSignsDataLink::EHealthType::Defeated:
                    colour = config.COLOUR_PROF_HEALTH_DEFEATED;
                    break;
                case VitalSignsDataLink::EHealthType::ShroudNecromancer:
                    colour = config.COLOUR_PROF_SHROUD_NECROMANCER;
                    break;
                case VitalSignsDataLink::EHealthType::ShroudSpecter:
                    colour = config.COLOUR_PROF_SHROUD_SPECTER;
                    break;
            }
        }
        else if (palette == "Heat Map")
        {
            switch (healthType)
            {
                case VitalSignsDataLink::EHealthType::Downed:
                    colour = config.COLOUR_HEATMAP_HEALTH_DOWNED;
                    break;
                case VitalSignsDataLink::EHealthType::Defeated:
                    colour = config.COLOUR_HEATMAP_HEALTH_DEFEATED;
                    break;
                case VitalSignsDataLink::EHealthType::ShroudNecromancer:
                    colour = config.COLOUR_HEATMAP_SHROUD_NECROMANCER;
                    break;
                case VitalSignsDataLink::EHealthType::ShroudSpecter:
                    colour = config.COLOUR_HEATMAP_SHROUD_SPECTER;
                    break;
                case VitalSignsDataLink::EHealthType::Alive:
                default:
                {
                    ImVec4 c1, c2;
                    float t = 0.0f;
                    float pct = health;
                    if (pct >= 0.75f) {
                        c1 = config.COLOUR_HEATMAP_HEALTH_75.Value;
                        c2 = config.COLOUR_HEATMAP_HEALTH_100.Value;
                        t = (pct - 0.75f) / 0.25f;
                    } else if (pct >= 0.5f) {
                        c1 = config.COLOUR_HEATMAP_HEALTH_50.Value;
                        c2 = config.COLOUR_HEATMAP_HEALTH_75.Value;
                        t = (pct - 0.5f) / 0.25f;
                    } else if (pct >= 0.25f) {
                        c1 = config.COLOUR_HEATMAP_HEALTH_25.Value;
                        c2 = config.COLOUR_HEATMAP_HEALTH_50.Value;
                        t = (pct - 0.25f) / 0.25f;
                    } else {
                        c1 = config.COLOUR_HEATMAP_HEALTH_0.Value;
                        c2 = config.COLOUR_HEATMAP_HEALTH_25.Value;
                        t = pct / 0.25f;
                    }
                    
                    float h1, s1, v1;
                    ImGui::ColorConvertRGBtoHSV(c1.x, c1.y, c1.z, h1, s1, v1);
                    float h2, s2, v2;
                    ImGui::ColorConvertRGBtoHSV(c2.x, c2.y, c2.z, h2, s2, v2);
                    
                    float h = h1 + (h2 - h1) * t;
                    float s = s1 + (s2 - s1) * t;
                    float v = v1 + (v2 - v1) * t;
                    
                    ImGui::ColorConvertHSVtoRGB(h, s, v, colour.Value.x, colour.Value.y, colour.Value.z);
                    colour.Value.w = c1.w + (c2.w - c1.w) * t;
                    break;
                }
            }
        }
        else if (palette == "Role")
        {
            switch (healthType)
            {
                case VitalSignsDataLink::EHealthType::Downed:
                    colour = config.COLOUR_ROLE_HEALTH_DOWNED;
                    break;
                case VitalSignsDataLink::EHealthType::Defeated:
                    colour = config.COLOUR_ROLE_HEALTH_DEFEATED;
                    break;
                case VitalSignsDataLink::EHealthType::ShroudNecromancer:
                    colour = config.COLOUR_ROLE_SHROUD_NECROMANCER;
                    break;
                case VitalSignsDataLink::EHealthType::ShroudSpecter:
                    colour = config.COLOUR_ROLE_SHROUD_SPECTER;
                    break;
                case VitalSignsDataLink::EHealthType::Alive:
                default:
                {
                    switch (primaryRole)
                    {
                        case VitalSignsDataLink::EPrimaryRole::Damage:
                            colour = config.COLOUR_ROLE_HEALTH_DAMAGE;
                            break;
                        case VitalSignsDataLink::EPrimaryRole::DamageAlacrity:
                        case VitalSignsDataLink::EPrimaryRole::DamageQuickness:
                            colour = config.COLOUR_ROLE_HEALTH_DAMAGE_BOON;
                            break;
                        case VitalSignsDataLink::EPrimaryRole::HealerAlacrity:
                        case VitalSignsDataLink::EPrimaryRole::HealerQuickness:
                            colour = config.COLOUR_ROLE_HEALTH_HEALER_BOON;
                            break;
                        case VitalSignsDataLink::EPrimaryRole::Flexible:
                        case VitalSignsDataLink::EPrimaryRole::None:
                        default:
                            colour = config.COLOUR_ROLE_HEALTH_DAMAGE;
                            break;
                    }
                    break;
                }
            }
        }

        return colour;
    }

    ImColor GetBarrierColour(const ColourPresets_t& config, const std::string& palette)
    {
        if (palette == "Default")
        {
            return config.COLOUR_BARRIER;
        }
        else if (palette == "Profession")
        {
            return config.COLOUR_PROF_BARRIER;
        }
        else if (palette == "Heat Map")
        {
            return config.COLOUR_HEATMAP_BARRIER;
        }
        else if (palette == "Role")
        {
            return config.COLOUR_ROLE_BARRIER;
        }
        return ImColor();
    }

    bool HasGameIcon(const Trigger_t& trigger)
    {
        if ((trigger.category != "Boons") && (trigger.category != "Conditions"))
        {
            return false;
        }

        return VitalSignsDataLink::hasEffectIcon(trigger.effect);
    }

    Texture GetOrCreateTexture(const Icon_t& icon)
    {
        Texture texture{};

        if (icon.source == "Game")
        {
            /* The game's icon is resolved every frame, it may move to another device texture */
            if ((nullptr != VitalsData) && HasGameIcon(icon.trigger))
            {
                texture = VitalsData->getEffectIconTexture(icon.trigger.effect);
            }

            return texture;
        }

        if (!APIDefs || icon.path.empty())
        {
            return texture;
        }

        Texture* nexusTexture = APIDefs->Textures.Get(icon.path.c_str());

        // Create the Texture if it does not exist.
        if (nexusTexture == nullptr)
        {
            if (icon.source == "File")
            {
                if (std::filesystem::path(icon.path).is_relative())
                {
                    nexusTexture = APIDefs->Textures.GetOrCreateFromFile(icon.path.c_str(), (GameDir / icon.path).string().c_str());
                }
                else
                {
                    nexusTexture = APIDefs->Textures.GetOrCreateFromFile(icon.path.c_str(), icon.path.c_str());
                }
            }
            else if (icon.source == "URL")
            {
                std::string url = icon.path;
                std::string scheme = url_utils::getScheme(url);
                std::string domain = url_utils::getDomain(url);
                std::string path = url_utils::getPath(url);

                if ((scheme.length() != 0) && (domain.length() != 0) && (path.length() != 0))
                {
                    nexusTexture = APIDefs->Textures.GetOrCreateFromURL(url.c_str(), std::string(scheme + domain).c_str(), path.c_str());
                }
            }
        }

        if (nexusTexture != nullptr)
        {
            texture = *nexusTexture;
        }

        return texture;
    }

    Texture* GetOrCreateTextureFromResource(const char* identifier, unsigned resourceId)
    {
        Texture* texture = nullptr;

        if (APIDefs)
        {
            texture = APIDefs->Textures.GetOrCreateFromResource(identifier, resourceId, hSelf);
        }

        return texture;
    }

    StyleScope::StyleScope(const StylePreset_t& style)
    {
        if (style.colorsSource == "Custom colors")
        {
            PushColor(ImGuiCol_Text, style.text);
            PushColor(ImGuiCol_TextDisabled, style.textDisabled);
            PushColor(ImGuiCol_WindowBg, style.windowBackground);
            PushColor(ImGuiCol_PopupBg, style.windowBackground);
            PushColor(ImGuiCol_Border, style.border);
            PushColor(ImGuiCol_Separator, style.separator);
            PushColor(ImGuiCol_FrameBg, style.frameBackground);
            PushColor(ImGuiCol_FrameBgHovered, style.frameBackgroundHovered);
            PushColor(ImGuiCol_FrameBgActive, style.frameBackgroundActive);
            PushColor(ImGuiCol_Button, style.button);
            PushColor(ImGuiCol_ButtonHovered, style.buttonHovered);
            PushColor(ImGuiCol_ButtonActive, style.buttonActive);
            PushColor(ImGuiCol_CheckMark, style.checkMark);
            PushColor(ImGuiCol_Header, style.selectable);
            PushColor(ImGuiCol_HeaderHovered, style.selectableHovered);
            PushColor(ImGuiCol_HeaderActive, style.selectableActive);
        }

        if (style.sizesSource == "Custom sizes")
        {
            PushVar(ImGuiStyleVar_WindowPadding, style.windowPadding);
            PushVar(ImGuiStyleVar_WindowRounding, style.windowRounding);
            PushVar(ImGuiStyleVar_PopupRounding, style.windowRounding);
            PushVar(ImGuiStyleVar_WindowBorderSize, style.windowBorderSize);
            PushVar(ImGuiStyleVar_PopupBorderSize, style.windowBorderSize);
            PushVar(ImGuiStyleVar_FramePadding, style.framePadding);
            PushVar(ImGuiStyleVar_FrameRounding, style.frameRounding);
            PushVar(ImGuiStyleVar_ItemSpacing, style.itemSpacing);
        }

        const ImFont* defaultFont = ImGui::GetIO().FontDefault ? ImGui::GetIO().FontDefault : ImGui::GetFont();
        std::string fontFilePath = ""; // Nexus font
        m_fontSize = defaultFont->FontSize; // Nexus font size

        if (style.fontSource == "Custom font")
        {
            fontFilePath = style.font;
            if (style.fontSize > 0.0f) m_fontSize = style.fontSize;
        }

        /* Null for the Nexus font, or while a custom font is loading */
        ImFont* font = utils::font::GetFont(fontFilePath, m_fontSize);

        if (font)
        {
            ImGui::PushFont(font);
            m_isFontPushed = true;
        }
    }

    StyleScope::~StyleScope()
    {
        if (m_isFontPushed)
        {
            ImGui::PopFont();
        }

        ImGui::PopStyleVar(m_varCount);
        ImGui::PopStyleColor(m_colorCount);
    }

    void StyleScope::ApplyFontScale() const
    {
        const float fontBaseSize = ImGui::GetFont()->FontSize;

        if (fontBaseSize > 0.0f)
        {
            ImGui::SetWindowFontScale(m_fontSize / fontBaseSize);
        }
    }

    void StyleScope::PushColor(const ImGuiCol index, const ImColor& color)
    {
        ImGui::PushStyleColor(index, color.Value);
        m_colorCount++;
    }

    void StyleScope::PushVar(const ImGuiStyleVar index, const float value)
    {
        ImGui::PushStyleVar(index, value);
        m_varCount++;
    }

    void StyleScope::PushVar(const ImGuiStyleVar index, const ImVec2& value)
    {
        ImGui::PushStyleVar(index, value);
        m_varCount++;
    }

} // namespace UI

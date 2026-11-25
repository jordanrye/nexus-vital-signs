#ifndef VS_UI_COMMON_H
#define VS_UI_COMMON_H

#include "data_link/data_link.h"

#include "addon_types.h"

namespace UI {

    const int SQUAD_MEMBER_LIMIT = 50; // max number of player frames
    const int SQUAD_SUBGROUP_LIMIT = 15; // max number of subgroups
    const int BOON_LIMIT = 12; // max number of boons
    const int CONDITION_LIMIT = 14; // max number of conditions

    ImColor GetBackgroundColour(const ColourPresets_t& config, const std::string& palette, VitalSignsDataLink::EHealthType healthType);
    ImColor GetHealthColour(const ColourPresets_t& config, const std::string& palette, VitalSignsDataLink::EHealthType healthType, float health, VitalSignsDataLink::EProfession profession, VitalSignsDataLink::EPrimaryRole primaryRole);
    ImColor GetBarrierColour(const ColourPresets_t& config, const std::string& palette);

    bool HasGameIcon(const Trigger_t& trigger);

    /* The icon's texture from its "File", "URL" or "Game" source. Empty
     * until it has loaded, valid for the current frame only. */
    Texture GetOrCreateTexture(const Icon_t& icon);
    Texture* GetOrCreateTextureFromResource(const char* identifier, unsigned resourceId);

    /**
     * Applies a style preset to the ImGui windows drawn while it is in scope.
     * Constructed before `ImGui::Begin` (as the window's colours and sizes are
     * read there), and destroyed after `ImGui::End`. Only "Custom" groups are
     * pushed, the rest keep the default Nexus style.
     * 
     * @note Popups opened inside the window inherit the style, so the popup
     * background, rounding, and border size follow the parent's style.
     *
     * @note The font is resolved like the grid's text: "Nexus font" is the
     * current font at the default font's size. A custom font with no file is
     * the current font at the custom size. ImGui widgets have no font size
     * argument, unlike `AddText`, so `ApplyFontScale` scales the window's font
     * to the size instead.
     */
    class StyleScope
    {
    public:
        explicit StyleScope(const StylePreset_t& style);
        ~StyleScope();

        StyleScope(const StyleScope&) = delete;
        StyleScope& operator=(const StyleScope&) = delete;

        /* Called after each `Begin`, `BeginPopup` and `BeginCombo`, as each
         * opens its own window. The scale is 1 unless the font is not loaded
         * at the size (i.e., the Nexus font at a custom size). */
        void ApplyFontScale() const;

    private:
        void PushColor(const ImGuiCol index, const ImColor& color);
        void PushVar(const ImGuiStyleVar index, const float value);
        void PushVar(const ImGuiStyleVar index, const ImVec2& value);

        int m_colorCount = 0;
        int m_varCount = 0;
        bool m_isFontPushed = false;
        float m_fontSize = 0.0f;
    };

} // namespace UI

#endif /* VS_UI_COMMON_H */

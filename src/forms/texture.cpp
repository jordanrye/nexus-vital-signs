#include "forms.h"

#include "ui_common.h"

void form_Texture(Icon_t& icon)
{
    static const char* textureSourceOptions[] {
        "File",
        "URL",
        "Game",
    };

    const bool hasGameIcon = UI::HasGameIcon(icon.trigger);

    /* The trigger is set above the texture, so a change to one without a game
     * icon falls back to "File" before the source is shown. */
    if ((icon.source == "Game") && !hasGameIcon)
    {
        icon.source = "File";
        icon.path = "";
    }

    /* Unknown sources are shown as "File", as they are drawn. */
    std::string currentSource = "File";
    if (icon.source == "URL") currentSource = "URL";
    else if (icon.source == "Game") currentSource = "Game";

    // Source
    if (ImGui::BeginCombo("Source##Tex", currentSource.c_str()))
    {
        for (const char* option : textureSourceOptions)
        {
            const bool isSelected = (currentSource == option);
            const bool isUnavailable = (std::string(option) == "Game") && !hasGameIcon;
            const ImGuiSelectableFlags flags = isUnavailable ? ImGuiSelectableFlags_Disabled : ImGuiSelectableFlags_None;

            if (ImGui::Selectable(option, isSelected, flags) && !isSelected)
            {
                icon.source = option;
                icon.path = "";
            }

            if (isUnavailable && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
            {
                ImGui::SetTooltip("No game icon available for this trigger.");
            }

            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    // Path
    if (currentSource == "Game")
    {
        /* The game icon is found from the trigger. There is no path. */
    }
    else if (currentSource == "URL")
    {
        ImGui::InputText("Filepath##TexFile", &icon.path);
    }
    else
    {
        ImGui::ButtonFile("Filepath", icon.path, IconsDir.string(), GameDir.string());
    }
}

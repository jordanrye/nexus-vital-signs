#include "forms.h"

void form_StyleFontPreset(StylePreset_t& style)
{
    static const char* fontOptions[] = {
        "Nexus font",
        "Custom font"
    };

    int option = 0; // Default to "Nexus font"
    if (style.fontSource == "Nexus font") option = 0;
    else if (style.fontSource == "Custom font") option = 1;

    if (ImGui::Combo("Font##WIDGET_FONT_TYPE", &option, fontOptions, IM_ARRAYSIZE(fontOptions)) && (option == 1))
    {
        style.font = "";
        style.fontSize = ImGui::GetIO().FontDefault->FontSize;
    }

    style.fontSource = fontOptions[option];

    if (option == 1)
    {
        /* Left empty, the Nexus font is used at the custom size */
        ImGui::ButtonFile("Font File", style.font, FontsDir.string(), GameDir.string(), L"All Files (*.*)\0*.*\0TrueType Font (*.ttf)\0*.ttf\0OpenType Font (*.otf)\0*.otf\0");
        ImGui::InputFloat("Font Size##WIDGET_FONT_SIZE", &style.fontSize, 1.f, 1.f, "%.1f");
    }
}

void form_StyleColorsPreset(StylePreset_t& style)
{
    static const char* colorsOptions[] = {
        "Nexus colors",
        "Custom colors"
    };

    int option = 0; // Default to "Nexus colors"
    if (style.colorsSource == "Nexus colors") option = 0;
    else if (style.colorsSource == "Custom colors") option = 1;

    if (ImGui::Combo("Colors##WIDGET_COLORS_TYPE", &option, colorsOptions, IM_ARRAYSIZE(colorsOptions)) && (option == 1))
    {
        const ImVec4* nexusColors = ImGui::GetStyle().Colors;

        style.text = nexusColors[ImGuiCol_Text];
        style.textDisabled = nexusColors[ImGuiCol_TextDisabled];
        style.windowBackground = nexusColors[ImGuiCol_WindowBg];
        style.border = nexusColors[ImGuiCol_Border];
        style.separator = nexusColors[ImGuiCol_Separator];
        style.frameBackground = nexusColors[ImGuiCol_FrameBg];
        style.frameBackgroundHovered = nexusColors[ImGuiCol_FrameBgHovered];
        style.frameBackgroundActive = nexusColors[ImGuiCol_FrameBgActive];
        style.button = nexusColors[ImGuiCol_Button];
        style.buttonHovered = nexusColors[ImGuiCol_ButtonHovered];
        style.buttonActive = nexusColors[ImGuiCol_ButtonActive];
        style.checkMark = nexusColors[ImGuiCol_CheckMark];
        style.selectable = nexusColors[ImGuiCol_Header];
        style.selectableHovered = nexusColors[ImGuiCol_HeaderHovered];
        style.selectableActive = nexusColors[ImGuiCol_HeaderActive];
    }

    style.colorsSource = colorsOptions[option];

    if (option == 1)
    {
        const ImGuiColorEditFlags flags = ImGuiColorEditFlags_AlphaPreviewHalf;

        ImGui::ColorEdit4("Text##WIDGET_COLOR", &(style.text.Value.x), flags);
        ImGui::ColorEdit4("Text Disabled##WIDGET_COLOR", &(style.textDisabled.Value.x), flags);
        ImGui::ColorEdit4("Window Background##WIDGET_COLOR", &(style.windowBackground.Value.x), flags);
        ImGui::ColorEdit4("Border##WIDGET_COLOR", &(style.border.Value.x), flags);
        ImGui::ColorEdit4("Separator##WIDGET_COLOR", &(style.separator.Value.x), flags);
        ImGui::ColorEdit4("Frame Background##WIDGET_COLOR", &(style.frameBackground.Value.x), flags);
        ImGui::ColorEdit4("Frame Background (Hovered)##WIDGET_COLOR", &(style.frameBackgroundHovered.Value.x), flags);
        ImGui::ColorEdit4("Frame Background (Active)##WIDGET_COLOR", &(style.frameBackgroundActive.Value.x), flags);
        ImGui::ColorEdit4("Button##WIDGET_COLOR", &(style.button.Value.x), flags);
        ImGui::ColorEdit4("Button (Hovered)##WIDGET_COLOR", &(style.buttonHovered.Value.x), flags);
        ImGui::ColorEdit4("Button (Active)##WIDGET_COLOR", &(style.buttonActive.Value.x), flags);
        ImGui::ColorEdit4("Check Mark##WIDGET_COLOR", &(style.checkMark.Value.x), flags);
        ImGui::ColorEdit4("Selectable##WIDGET_COLOR", &(style.selectable.Value.x), flags);
        ImGui::ColorEdit4("Selectable (Hovered)##WIDGET_COLOR", &(style.selectableHovered.Value.x), flags);
        ImGui::ColorEdit4("Selectable (Active)##WIDGET_COLOR", &(style.selectableActive.Value.x), flags);
    }
}

void form_StyleSizesPreset(StylePreset_t& style)
{
    static const char* sizesOptions[] = {
        "Nexus sizes",
        "Custom sizes"
    };

    int option = 0; // Default to "Nexus sizes"
    if (style.sizesSource == "Nexus sizes") option = 0;
    else if (style.sizesSource == "Custom sizes") option = 1;

    if (ImGui::Combo("Sizes##WIDGET_SIZES_TYPE", &option, sizesOptions, IM_ARRAYSIZE(sizesOptions)) && (option == 1))
    {
        const ImGuiStyle& nexusStyle = ImGui::GetStyle();

        style.windowPadding = nexusStyle.WindowPadding;
        style.windowRounding = nexusStyle.WindowRounding;
        style.windowBorderSize = nexusStyle.WindowBorderSize;
        style.framePadding = nexusStyle.FramePadding;
        style.frameRounding = nexusStyle.FrameRounding;
        style.itemSpacing = nexusStyle.ItemSpacing;
    }

    style.sizesSource = sizesOptions[option];

    if (option == 1)
    {
        ImGui::SliderFloat2("Window Padding##WIDGET_SIZE", &(style.windowPadding.x), 0.0f, 20.0f, "%.0f");
        ImGui::SliderFloat("Window Rounding##WIDGET_SIZE", &style.windowRounding, 0.0f, 12.0f, "%.0f");
        ImGui::SliderFloat("Window Border Size##WIDGET_SIZE", &style.windowBorderSize, 0.0f, 4.0f, "%.0f");
        ImGui::SliderFloat2("Frame Padding##WIDGET_SIZE", &(style.framePadding.x), 0.0f, 20.0f, "%.0f");
        ImGui::SliderFloat("Frame Rounding##WIDGET_SIZE", &style.frameRounding, 0.0f, 12.0f, "%.0f");
        ImGui::SliderFloat2("Item Spacing##WIDGET_SIZE", &(style.itemSpacing.x), 0.0f, 20.0f, "%.0f");
    }
}

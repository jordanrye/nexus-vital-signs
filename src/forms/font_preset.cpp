#include "forms.h"

void form_FontPreset(std::string& fontSource, std::string& fontPath)
{
    static const char* fontOptions[] = {
        "Nexus font",
        "Custom font"
    };

    int option = 0; // Default to "Nexus font"
    if (fontSource == "Nexus font") option = 0;
    else if (fontSource == "Custom font") option = 1;

    ImGui::Combo("Font##TEXT_FONT_TYPE", &option, fontOptions, IM_ARRAYSIZE(fontOptions));
    fontSource = fontOptions[option];

    switch (option)
    {
        case 0:
            fontPath = "";
            break;
        case 1:
            ImGui::ButtonFile("Font File", fontPath, FontsDir.string(), GameDir.string(), L"All Files (*.*)\0*.*\0TrueType Font (*.ttf)\0*.ttf\0OpenType Font (*.otf)\0*.otf\0");
            break;
    }
}

void form_FontSizePreset(std::string& fontSource, float& fontSize)
{
    static const char* fontSizeOptions[] = {
        "Nexus font size",
        "Custom font size"
    };

    int option = 0; // Default to "Nexus font size"
    if (fontSource == "Nexus font size") option = 0;
    else if (fontSource == "Custom font size") option = 1;

    ImGui::Combo("Font Size##TEXT_FONT_SIZE_TYPE", &option, fontSizeOptions, IM_ARRAYSIZE(fontSizeOptions));
    fontSource = fontSizeOptions[option];

    switch (option)
    {
        case 0:
            fontSize = ImGui::GetIO().FontDefault->FontSize;
            break;
        case 1:
            ImGui::InputFloat("Size##TEXT_FONT_SIZE_VALUE", &fontSize, 1.f, 1.f, "%.1f");
            break;
    }
}

void form_FontDecoratorPreset(bool& isShadow, ImColor& colourShadow, bool& isOutline, ImColor& colourOutline)
{
    ImGui::Checkbox("Shadow##TEXT_SHADOW", &isShadow);
    if (isShadow)
    {
        ImGui::SameLine();
        ImGui::ColorEdit4("Shadow Color##TEXT_SHADOW_COLOR", (float*)&colourShadow, ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_NoInputs);
    }

    ImGui::Checkbox("Outline##TEXT_OUTLINE", &isOutline);
    if (isOutline)
    {
        ImGui::SameLine();
        ImGui::ColorEdit4("Outline Color##TEXT_OUTLINE_COLOR", (float*)&colourOutline, ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_NoInputs);
    }
}

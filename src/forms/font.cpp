#include "forms.h"

void form_Font(std::string& fontSource, std::string& fontFilePath)
{
    static const char* fontOptions[] = {
        "Default font",
        "Nexus font",
        "Custom font"
    };

    int option = 0; // Default to "Default font"
    if (fontSource == "Default font") option = 0;
    if (fontSource == "Nexus font") option = 1;
    else if (fontSource == "Custom font") option = 2;

    ImGui::Combo("Font##TEXT_FONT_TYPE", &option, fontOptions, IM_ARRAYSIZE(fontOptions));
    fontSource = fontOptions[option];

    switch (option)
    {
        case 0:
            fontFilePath = "";
            break;
        case 1:
            fontFilePath = "";
            break;
        case 2:
            ImGui::ButtonFile("Font File", fontFilePath, FontsDir.string(), GameDir.string(), L"All Files (*.*)\0*.*\0TrueType Font (*.ttf)\0*.ttf\0OpenType Font (*.otf)\0*.otf\0");
            break;
    }
}

void form_FontSize(std::string& fontSource, float& fontSize)
{
    static const char* fontSizeOptions[] = {
        "Default font size",
        "Nexus font size",
        "Custom font size"
    };

    int option = 0; // Default to "Default font size"
    if (fontSource == "Default font size") option = 0;
    if (fontSource == "Nexus font size") option = 1;
    else if (fontSource == "Custom font size") option = 2;

    ImGui::Combo("Font Size##TEXT_FONT_SIZE_TYPE", &option, fontSizeOptions, IM_ARRAYSIZE(fontSizeOptions));
    fontSource = fontSizeOptions[option];

    switch (option)
    {
        case 0:
            fontSize = 0.f;
            break;
        case 1:
            fontSize = 0.f;
            break;
        case 2:
            ImGui::InputFloat("Size##TEXT_FONT_SIZE_VALUE", &fontSize, 1.f, 1.f, "%.1f");
            break;
    }
}

void form_FontColour(std::string& source, ImColor& colour)
{
    static const char* colorOptions[] = {
        "Default color",
        "Custom color"
    };

    int option = 0; // Default to "Default color"
    if (source == "Default color") option = 0;
    if (source == "Custom color") option = 1;

    ImGui::Combo("Color##TEXT_COLOR_TYPE", &option, colorOptions, IM_ARRAYSIZE(colorOptions));
    source = colorOptions[option];

    if (option == 1)
    {
        ImGui::ColorEdit4("Color##TEXT_COLOR_VALUE", (float*)&colour, ImGuiColorEditFlags_AlphaPreviewHalf);
    }
}

void form_FontDecorator(std::string& source, bool& isShadow, ImColor& colourShadow, bool& isOutline, ImColor& colourOutline)
{
    static const char* fontDecoratorOptions[] = {
        "Default decorators",
        "Custom decorators"
    };

    int option = 0; // Default to "Default decorators"
    if (source == "Default decorators") option = 0;
    if (source == "Custom decorators") option = 1;

    ImGui::Combo("Decorators##TEXT_DECORATORS_TYPE", &option, fontDecoratorOptions, IM_ARRAYSIZE(fontDecoratorOptions));
    source = fontDecoratorOptions[option];

    if (option == 1)
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

}

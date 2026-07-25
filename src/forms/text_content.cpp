#include "forms.h"

void form_TextContent(std::string& textType, std::string& textCustom)
{
    static const char* textTypeOptions[] = {
        "Character name",
        "Account name",
        "Nickname",
        "Health percentage",
        "Custom text"
    };

    int option = 0; // Default to "Character name"
    if (textType == "Character name") option = 0;
    else if (textType == "Account name") option = 1;
    else if (textType == "Nickname") option = 2;
    else if (textType == "Health percentage") option = 3;
    else if (textType == "Custom text") option = 4;

    ImGui::Combo("Text##TEXT_CONTENT", &option, textTypeOptions, IM_ARRAYSIZE(textTypeOptions));
    textType = textTypeOptions[option];

    if (textType == "Custom text")
    {
        ImGui::InputText("Custom Text##TEXT_CUSTOM", &textCustom);
    }
}

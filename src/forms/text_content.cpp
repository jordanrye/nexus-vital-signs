#include "forms.h"

void form_TextContent(std::string& textType, std::string& textCustom)
{
    static const char* textTypeOptions[] = {
        "Character name",
        "Health percentage",
        "Custom text"
    };

    int option = 0; // Default to "Character name"
    if (textType == "Character name") option = 0;
    if (textType == "Health percentage") option = 1;
    else if (textType == "Custom text") option = 2;

    ImGui::Combo("Text##TEXT_CONTENT", &option, textTypeOptions, IM_ARRAYSIZE(textTypeOptions));
    textType = textTypeOptions[option];

    if (textType == "Custom text")
    {
        static char inputBuff_TextCustom[MAX_PATH] = "";
        memset(inputBuff_TextCustom, 0, sizeof(inputBuff_TextCustom));
        strcpy_s(inputBuff_TextCustom, sizeof(inputBuff_TextCustom), textCustom.c_str());

        if (ImGui::InputText("Custom Text##TEXT_CUSTOM", inputBuff_TextCustom, IM_ARRAYSIZE(inputBuff_TextCustom))) {
            textCustom = inputBuff_TextCustom;
        }
    }
}

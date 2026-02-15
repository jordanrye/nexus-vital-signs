#include "forms.h"

void form_Texture(std::string& textureSource, std::string& texturePath)
{
    static char inputBuff_TexturePath[MAX_PATH];
    static const char* textureSourceOptions[] {
        "File",
        "URL"
    };

    memset(inputBuff_TexturePath, 0, MAX_PATH);
    strcpy_s(inputBuff_TexturePath, MAX_PATH, texturePath.c_str());

    int option = 0; // Default to "File"
    if (textureSource == "File") option = 0;
    else if (textureSource == "URL") option = 1;

    // Source
    int prevOption = option;
    if (ImGui::Combo("Source##Tex", &option, textureSourceOptions, IM_ARRAYSIZE(textureSourceOptions)))
    {
        textureSource = textureSourceOptions[option];

        if (option != prevOption)
        {
            texturePath = "";
        }
    }

    // Path
    switch (option)
    {
        case 0:
            // File
            ImGui::ButtonFile("Filepath", texturePath, IconsDir.string(), GameDir.string());
            break;
        case 1:
            // URL
            if (ImGui::InputText("Filepath##TexFile", inputBuff_TexturePath, IM_ARRAYSIZE(inputBuff_TexturePath)))
            {
                texturePath = inputBuff_TexturePath;
            }
            break;
    }
    
}

#include "forms.h"

void form_TextLimit(std::string& limitType, float& limitValue)
{
    static const char* limitTypeOptions[] = {
        "None",
        "Character limit",
        "Percentage width"
    };

    int option = 0;
    if (limitType == "Character limit") option = 1;
    else if (limitType == "Percentage width") option = 2;

    ImGui::Combo("Width Limit##TEXT_WIDTH_TYPE", &option, limitTypeOptions, IM_ARRAYSIZE(limitTypeOptions));
    limitType = limitTypeOptions[option];

    switch (option)
    {
        case 0:
            break;
        case 1:
            {
                int val = static_cast<int>(limitValue);
                if (ImGui::InputInt("Value##TEXT_LIMIT_VALUE", &val))
                {
                    limitValue = static_cast<float>(val);
                }
                break;
            }
        case 2:
            ImGui::InputFloat("Value##TEXT_LIMIT_VALUE", &limitValue, 1.f, 1.f, "%.1f");
            break;
    }
}

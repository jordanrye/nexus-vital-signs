#include "forms.h"

void form_SelectIndicatorType(std::string& indicatorType, bool isDisabled)
{
    static const char* indicatorTypeOptions[] {
        "Icon",
        "Icon List",
        "Group",
        "Border",
        "Colour",
        "Highlight",
        "Text"
    };

    int option = 0; // Default to "Icon"
    if (indicatorType == "Icon") option = 0;
    else if (indicatorType == "Icon List") option = 1;
    else if (indicatorType == "Group") option = 2;
    else if (indicatorType == "Border") option = 3;
    else if (indicatorType == "Colour") option = 4;
    else if (indicatorType == "Highlight") option = 5;
    else if (indicatorType == "Text") option = 6;

    if (isDisabled) { ImGui::BeginDisabled(); }
    ImGui::Combo("Type##", &option, indicatorTypeOptions, IM_ARRAYSIZE(indicatorTypeOptions));
    indicatorType = indicatorTypeOptions[option];
    if (isDisabled) { ImGui::EndDisabled(); }
}

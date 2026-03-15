#include "forms.h"

void form_SelectLayoutType(std::string& layoutType, bool isDisabled)
{
    static const char* layoutTypeOptions[] {
        "Grid",
        // "Radial"
    };

    int option = 0; // Default to "Grid"
    if (layoutType == "Grid") option = 0;
    // else if (layoutType == "Radial") option = 1;

    if (isDisabled) { ImGui::BeginDisabled(); }
    ImGui::Combo("Type##", &option, layoutTypeOptions, IM_ARRAYSIZE(layoutTypeOptions));
    layoutType = layoutTypeOptions[option];
    if (isDisabled) { ImGui::EndDisabled(); }
}

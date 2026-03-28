#include "forms.h"

void form_Visibility(std::string& visibility)
{
    static const char* visibilityOptions[] {
        "Always show",
        "Hide out of combat"
    };

    int option = 0; // Default to "Always show"
    if (visibility == "Always show") option = 0;
    else if (visibility == "Hide out of combat") option = 1;

    ImGui::Combo("Visibility", &option, visibilityOptions, IM_ARRAYSIZE(visibilityOptions));
    visibility = visibilityOptions[option];
}
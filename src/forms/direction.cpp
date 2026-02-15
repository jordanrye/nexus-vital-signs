#include "forms.h"

void form_Direction(std::string& direction)
{
    static const char* directionOptions[] {
        "Left-to-right",
        "Top-to-bottom",
        "Right-to-left",
        "Bottom-to-top"
    };

    static int option = 0; // Default to "Left-to-right"
    if (direction == "Left-to-right") option = 0;
    else if (direction == "Top-to-bottom") option = 1;
    else if (direction == "Right-to-left") option = 2;
    else if (direction == "Bottom-to-top") option = 3;

    ImGui::Combo("Direction##Direction", &option, directionOptions, IM_ARRAYSIZE(directionOptions));
    direction = directionOptions[option];
}

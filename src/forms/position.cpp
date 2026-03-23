#include "forms.h"

void form_Position(Position_t& position)
{
    static const char* anchorPointOptions[] {
        "Top-left",
        "Top-centre",
        "Top-right",
        "Centre-left",
        "Centre",
        "Centre-right",
        "Bottom-left",
        "Bottom-centre",
        "Bottom-right"
    };

    int anchorPoint = 4; // Default to "Centre"
    if (position.anchor == "Top-left") anchorPoint = 0;
    else if (position.anchor == "Top-centre") anchorPoint = 1;
    else if (position.anchor == "Top-right") anchorPoint = 2;
    else if (position.anchor == "Centre-left") anchorPoint = 3;
    else if (position.anchor == "Centre") anchorPoint = 4;
    else if (position.anchor == "Centre-right") anchorPoint = 5;
    else if (position.anchor == "Bottom-left") anchorPoint = 6;
    else if (position.anchor == "Bottom-centre") anchorPoint = 7;
    else if (position.anchor == "Bottom-right") anchorPoint = 8;

    ImGui::Combo("Anchor##Position", &anchorPoint, anchorPointOptions, IM_ARRAYSIZE(anchorPointOptions));
    position.anchor = anchorPointOptions[anchorPoint];
    ImGui::InputInt("Offset X##Position", &position.offset.x, 1);
    ImGui::InputInt("Offset Y##Position", &position.offset.y, 1);
}

void form_Position(std::string& source, Position_t& position)
{
    static const char* positionOptions[] = {
        "Default position",
        "Custom position"
    };

    int option = 0; // Default to "Default position"
    if (source == "Default position") option = 0;
    else if (source == "Custom position") option = 1;

    ImGui::Combo("Position##POSITION_TYPE", &option, positionOptions, IM_ARRAYSIZE(positionOptions));
    source = positionOptions[option];

    if (option == 1)
    {
        form_Position(position);
    }
}

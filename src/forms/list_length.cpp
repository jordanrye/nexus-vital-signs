#include "forms.h"

void form_ListLength(std::string& listLength)
{
    static const char* listLengthOptions[] {
        "Dynamic",
        "Static"
    };

    static int option = 0; // Default to "Dynamic"
    if (listLength == "Dynamic") option = 0;
    else if (listLength == "Static") option = 1;

    ImGui::Combo("Length##", &option, listLengthOptions, IM_ARRAYSIZE(listLengthOptions));
    listLength = listLengthOptions[option];
}

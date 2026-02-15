#include "forms.h"

void form_ListSpacing(int& listSpacing)
{
    ImGui::InputInt("Spacing##", &listSpacing, 1);
}

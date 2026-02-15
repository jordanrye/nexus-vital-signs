#include "forms.h"

void form_Size(Size_t& size)
{
    // Icon size
    ImGui::InputInt("Size X##Size", &size.width, 1);
    ImGui::InputInt("Size Y##Size", &size.height, 1);
}

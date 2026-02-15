#include "forms.h"

void form_SelectLayout(const std::vector<std::string>& layouts, std::string& selectedLayout)
{
    if (ImGui::BeginCombo("Layout", selectedLayout.c_str()))
    {
        for (const auto& layoutName : layouts)
        {
            bool is_selected = (selectedLayout == layoutName);
            if (ImGui::Selectable(layoutName.c_str(), is_selected))
            {
                selectedLayout = layoutName;
            }
            if (is_selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
}

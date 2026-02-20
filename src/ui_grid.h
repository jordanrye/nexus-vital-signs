#ifndef VS_UI_GRID_H
#define VS_UI_GRID_H

#include "interface/vital_signs_interface.h"
#include "imgui/imgui.h"

#include "layout_types.h"
#include "ui_common.h"

namespace UI::Grid {

    bool BeginGridMenu(const char* name, const LayoutConfig_t& layout, const ColourPresets_t& colours, const BorderPresets_t& borders, bool isActive);
    void EndGridMenu();
    bool GridMenuItem(const char* name, VitalSignsData::EProfession profession, VitalSignsData::ESpecialisation specialisation, float health, VitalSignsData::E_HEALTH_TYPE healthType, float barrier, VitalSignsData::Effects_t& effects);

} // namespace UI::Grid

#endif /* VS_UI_GRID_H */

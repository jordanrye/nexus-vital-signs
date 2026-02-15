#ifndef VS_UI_RADIAL_H
#define VS_UI_RADIAL_H

#include "interface/vital_signs_interface.h"
#include "imgui/imgui.h"

#include "layout_types.h"
#include "ui_common.h"

namespace UI::Radial {

    bool BeginRadialMenu(const char* name, const Position_t& position, const Layout_t& layout, const ColourPresets_t& colours, bool isActive);
    void EndRadialMenu();
    bool RadialMenuItem(const char* name, float health, VitalSignsData::E_HEALTH_TYPE healthType, float barrier, VitalSignsData::Effects_t& effects);

} // namespace UI::Radial

#endif /* VS_UI_RADIAL_H */

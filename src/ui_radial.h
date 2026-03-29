#ifndef VS_UI_RADIAL_H
#define VS_UI_RADIAL_H

#include "data_link/data_link.h"
#include "imgui/imgui.h"

#include "layout_types.h"
#include "ui_common.h"

namespace UI::Radial {

    bool BeginRadialMenu(const char* name, const Position_t& position, const Layout_t& layout, const std::string& palette, const ColourPresets_t& colours, bool isActive);
    void EndRadialMenu();
    bool RadialMenuItem(const VitalSignsDataLink::UserData_t& userData);

} // namespace UI::Radial

#endif /* VS_UI_RADIAL_H */

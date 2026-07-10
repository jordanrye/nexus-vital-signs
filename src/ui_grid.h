#ifndef VS_UI_GRID_H
#define VS_UI_GRID_H

#include "data_link/data_link.h"
#include "imgui/imgui.h"

#include "layout_types.h"
#include "ui_common.h"

namespace UI::Grid {

    bool BeginGridMenu(const char* name, LayoutConfig_t& layout, const ColourPresets_t& colours, const BorderPresets_t& borders, bool isActive);
    void EndGridMenu();
    bool GridMenuItem(const VitalSignsDataLink::UserData_t& userData);

} // namespace UI::Grid

#endif /* VS_UI_GRID_H */

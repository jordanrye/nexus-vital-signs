#pragma once

#include <string>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

namespace ImGui
{
    bool Tooltip();
    void TooltipGeneric(const char* str, ...);

    void TextPadded(const char* str, float paddingX, float paddingY);
    
    void TextWrappedPadded(const char* str, float paddingX, float paddingY);
    void TextWrappedDisabled(const char* str);

    bool SelectablePadded(const char* label, bool selected, float paddingX, float paddingY);

    void BeginGroupPanel(const char* name, const ImVec2& size);
    void EndGroupPanel();

    void BeginDisabled();
    void EndDisabled();

    bool ButtonArrow(const char* label, ImGuiDir direction, bool disabled);
    bool ButtonCross(const char* label);
    bool ButtonPlus(const char* label);
    void ButtonFile(const char* label, std::string& filePath, std::string targetDir, std::string hiddenSubstr = "", const wchar_t* filter = L"All Files (*.*)\0*.*\0Supported Files (*.png, *.jpg, *.jpeg)\0*.png;*.jpg;*.jpeg\0PNG (*.png)\0*.png\0JPEG (*.jpg, *.jpeg)\0*.jpg;*.jpeg\0");

    void AddRectFilledGradientV(ImDrawList* draw_list, const ImVec2& p_min, const ImVec2& p_max, ImU32 col_1, ImU32 col_2, float rounding, ImDrawCornerFlags rounding_corners);
    void AddRectFilledGradientH(ImDrawList* draw_list, const ImVec2& p_min, const ImVec2& p_max, ImU32 col_1, ImU32 col_2, float rounding, ImDrawCornerFlags rounding_corners);

} // namespace ImGui

#include "imgui_extensions.h"

#include <Windows.h>
#include <filesystem>
#include <thread>

#include "utilities.h"

namespace ImGui
{
    bool Tooltip()
    {
        bool hovered = ImGui::IsItemHovered();
        if (hovered)
        {
            ImGui::BeginTooltip();
        }
        return hovered;
    }

    void TooltipGeneric(const char* str, ...)
    {
        if (ImGui::Tooltip())
        {
            ImGui::Text(str);
            ImGui::EndTooltip();
        }
    }

    void TextPadded(const char* str, float paddingX, float paddingY)
    {
        ImVec2 textSize = ImGui::CalcTextSize(str);
        ImVec2 cursorStart = ImGui::GetCursorPos();
        ImGui::InvisibleButton("##TextPadded", textSize + ImVec2(paddingX * 2, paddingY * 2));
        ImVec2 cursorFinal = ImGui::GetCursorPos();
        ImGui::SetCursorPos(cursorStart + ImVec2(paddingX, paddingY));
        ImGui::Text(str);
        ImGui::SetCursorPos(cursorFinal);
    }

    void TextWrappedPadded(const char* str, float paddingX, float paddingY)
    {
        ImVec2 textSize = ImGui::CalcTextSize(str);
        ImVec2 cursorStart = ImGui::GetCursorPos();
        ImGui::InvisibleButton("##TextWrappedPadded", textSize + ImVec2(paddingX * 2, paddingY * 2));
        ImVec2 cursorFinal = ImGui::GetCursorPos();
        ImGui::SetCursorPos(cursorStart + ImVec2(paddingX, paddingY));
        ImGui::TextWrapped(str);
        ImGui::SetCursorPos(cursorFinal);
    }

    void TextWrappedDisabled(const char* str)
    {
        ImGui::BeginDisabled();
        ImGui::TextWrapped(str);
        ImGui::EndDisabled();
    }

    bool SelectablePadded(const char* label, bool selected, float paddingX, float paddingY)
    {
        //ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, {paddingX, paddingY});
        return ImGui::Selectable(label, selected);
        //ImGui::PopStyleVar(1);
    }

    static ImVector<ImRect> s_GroupPanelLabelStack;

    void BeginGroupPanel(const char* name, const ImVec2& size)
    {
        ImGui::BeginGroup();

        auto cursorPos = ImGui::GetCursorScreenPos();
        auto itemSpacing = ImGui::GetStyle().ItemSpacing;
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

        auto frameHeight = ImGui::GetFrameHeight();
        ImGui::BeginGroup();

        ImVec2 effectiveSize = size;
        if (size.x < 0.0f)
            effectiveSize.x = ImGui::GetContentRegionAvailWidth();
        else
            effectiveSize.x = size.x;
        ImGui::Dummy(ImVec2(effectiveSize.x, 0.0f));

        ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
        ImGui::SameLine(0.0f, 0.0f);
        ImGui::BeginGroup();
        ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
        ImGui::SameLine(0.0f, 0.0f);
        ImGui::Text(name);
        auto labelMin = ImGui::GetItemRectMin();
        auto labelMax = ImGui::GetItemRectMax();
        ImGui::SameLine(0.0f, 0.0f);
        ImGui::Dummy(ImVec2(0.0, frameHeight + itemSpacing.y));
        ImGui::BeginGroup();

        //ImGui::GetWindowDrawList()->AddRect(labelMin, labelMax, IM_COL32(255, 0, 255, 255));

        ImGui::PopStyleVar(2);

#if IMGUI_VERSION_NUM >= 17301
        ImGui::GetCurrentWindow()->ContentRegionRect.Max.x -= frameHeight * 0.5f;
        ImGui::GetCurrentWindow()->WorkRect.Max.x -= frameHeight * 0.5f;
        ImGui::GetCurrentWindow()->InnerRect.Max.x -= frameHeight * 0.5f;
#else
        ImGui::GetCurrentWindow()->ContentsRegionRect.Max.x -= frameHeight * 0.5f;
#endif
        ImGui::GetCurrentWindow()->Size.x -= frameHeight;

        auto itemWidth = ImGui::CalcItemWidth();
        ImGui::PushItemWidth(ImMax(0.0f, itemWidth - frameHeight));

        s_GroupPanelLabelStack.push_back(ImRect(labelMin, labelMax));
    }

    void EndGroupPanel()
    {
        ImGui::PopItemWidth();

        auto itemSpacing = ImGui::GetStyle().ItemSpacing;

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

        auto frameHeight = ImGui::GetFrameHeight();

        ImGui::EndGroup();

        //ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(0, 255, 0, 64), 4.0f);

        ImGui::EndGroup();

        ImGui::SameLine(0.0f, 0.0f);
        ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
        ImGui::Dummy(ImVec2(0.0, frameHeight - frameHeight * 0.5f - itemSpacing.y));

        ImGui::EndGroup();

        auto itemMin = ImGui::GetItemRectMin();
        auto itemMax = ImGui::GetItemRectMax();
        //ImGui::GetWindowDrawList()->AddRectFilled(itemMin, itemMax, IM_COL32(255, 0, 0, 64), 4.0f);

        auto labelRect = s_GroupPanelLabelStack.back();
        s_GroupPanelLabelStack.pop_back();

        ImVec2 halfFrame = ImVec2(frameHeight * 0.25f, frameHeight) * 0.5f;
        ImRect frameRect = ImRect(itemMin + halfFrame, itemMax - ImVec2(halfFrame.x, 0.0f));
        labelRect.Min.x -= itemSpacing.x;
        labelRect.Max.x += itemSpacing.x;
        for (int i = 0; i < 4; ++i)
        {
            switch (i)
            {
                // left half-plane
                case 0: ImGui::PushClipRect(ImVec2(-FLT_MAX, -FLT_MAX), ImVec2(labelRect.Min.x, FLT_MAX), true); break;
                    // right half-plane
                case 1: ImGui::PushClipRect(ImVec2(labelRect.Max.x, -FLT_MAX), ImVec2(FLT_MAX, FLT_MAX), true); break;
                    // top
                case 2: ImGui::PushClipRect(ImVec2(labelRect.Min.x, -FLT_MAX), ImVec2(labelRect.Max.x, labelRect.Min.y), true); break;
                    // bottom
                case 3: ImGui::PushClipRect(ImVec2(labelRect.Min.x, labelRect.Max.y), ImVec2(labelRect.Max.x, FLT_MAX), true); break;
            }

            ImGui::GetWindowDrawList()->AddRect(
                frameRect.Min, frameRect.Max,
                ImColor(ImGui::GetStyleColorVec4(ImGuiCol_Separator)),
                halfFrame.x);

            ImGui::PopClipRect();
        }

        ImGui::PopStyleVar(2);

#if IMGUI_VERSION_NUM >= 17301
        ImGui::GetCurrentWindow()->ContentRegionRect.Max.x += frameHeight * 0.5f;
        ImGui::GetCurrentWindow()->WorkRect.Max.x += frameHeight * 0.5f;
        ImGui::GetCurrentWindow()->InnerRect.Max.x += frameHeight * 0.5f;
#else
        ImGui::GetCurrentWindow()->ContentsRegionRect.Max.x += frameHeight * 0.5f;
#endif
        ImGui::GetCurrentWindow()->Size.x += frameHeight;

        ImGui::Dummy(ImVec2(0.0f, 0.0f));

        ImGui::EndGroup();
    }

    void BeginDisabled()
    {
        // Push a style var to dim the widgets visually
        // This makes the widgets appear grayed out
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);

        // Push the disabled item flag. This is the core mechanism
        // that tells ImGui to treat subsequent items as non-interactable.
        // This flag was available in v1.80.
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
    }

    // Call this after drawing disabled widgets to re-enable interaction and restore alpha
    void EndDisabled()
    {
        // Pop the disabled item flag
        ImGui::PopItemFlag();

        // Pop the style var to restore original alpha
        ImGui::PopStyleVar();
    }

    bool ButtonArrow(const char* label, ImGuiDir direction, bool disabled)
	{
		if (disabled)
		{
			PushItemFlag(ImGuiItemFlags_Disabled, true);
			PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.33f);
			ArrowButton(label, direction);
			PopItemFlag();
			PopStyleVar();
			return false;
		}
		
		return ImGui::ArrowButton(label, direction);
	}

    bool ButtonCross(const char* label)
	{
		float sz = GetFrameHeight();
		ImVec2 size = ImVec2(sz, sz);

		ImGuiButtonFlags flags = ImGuiButtonFlags_None;

		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		const ImGuiID id = window->GetID(label);
		const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
		const float default_size = GetFrameHeight();
		ItemSize(size, (size.y >= default_size) ? g.Style.FramePadding.y : -1.0f);
		if (!ItemAdd(bb, id))
			return false;

		if (window->DC.ItemFlags & ImGuiItemFlags_ButtonRepeat)
			flags |= ImGuiButtonFlags_Repeat;

		bool hovered, held;
		bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

		// Render
		const ImU32 bg_col = GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
		const ImU32 text_col = GetColorU32(ImGuiCol_Text);
		RenderNavHighlight(bb, id);
		RenderFrame(bb.Min, bb.Max, bg_col, true, g.Style.FrameRounding);
		
		ImU32 col = GetColorU32(held ? ImGuiCol_ButtonActive : ImGuiCol_ButtonHovered);
		ImVec2 center = bb.GetCenter();

		float cross_extent = g.FontSize * 0.5f * 0.7071f - 1.0f;
		ImU32 cross_col = GetColorU32(ImGuiCol_Text);
		center -= ImVec2(0.5f, 0.5f);
		window->DrawList->AddLine(center + ImVec2(+cross_extent, +cross_extent), center + ImVec2(-cross_extent, -cross_extent), cross_col, 1.0f);
		window->DrawList->AddLine(center + ImVec2(+cross_extent, -cross_extent), center + ImVec2(-cross_extent, +cross_extent), cross_col, 1.0f);

		return pressed;
	}

    bool ButtonPlus(const char* label)
	{
		float sz = GetFrameHeight();
		ImVec2 size = ImVec2(sz, sz);

		ImGuiButtonFlags flags = ImGuiButtonFlags_None;

		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		const ImGuiID id = window->GetID(label);
		const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
		const float default_size = GetFrameHeight();
		ItemSize(size, (size.y >= default_size) ? g.Style.FramePadding.y : -1.0f);
		if (!ItemAdd(bb, id))
			return false;

		if (window->DC.ItemFlags & ImGuiItemFlags_ButtonRepeat)
			flags |= ImGuiButtonFlags_Repeat;

		bool hovered, held;
		bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

		// Render
		const ImU32 bg_col = GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
		const ImU32 text_col = GetColorU32(ImGuiCol_Text);
		RenderNavHighlight(bb, id);
		RenderFrame(bb.Min, bb.Max, bg_col, true, g.Style.FrameRounding);
		
		ImU32 col = GetColorU32(held ? ImGuiCol_ButtonActive : ImGuiCol_ButtonHovered);
		ImVec2 center = bb.GetCenter();

		float plus_extent = g.FontSize * 0.5f * 0.7071f - 1.f;
		ImU32 plus_col = GetColorU32(ImGuiCol_Text);
		center -= ImVec2(0.5f, 0.5f);
		window->DrawList->AddLine(center + ImVec2(-plus_extent, 0.f), center + ImVec2(+plus_extent, 0.f), plus_col, 1.0f);
		window->DrawList->AddLine(center + ImVec2(0.f, -plus_extent), center + ImVec2(0.f, +plus_extent), plus_col, 1.0f);

		return pressed;
	}

    void ButtonFile(const char* label, std::string& filePath, std::string targetDir, std::string hiddenSubstr, const wchar_t* filter)
    {
        ImGui::PushID(label);
        if (ImGui::Button(filePath.c_str(), ImVec2(ImGui::CalcItemWidth(), 0)))
        {
            std::thread([=, &filePath] {
                OPENFILENAME ofn{};
                TCHAR szFile[MAX_PATH]{};
                TCHAR initialDir[MAX_PATH]{};

                std::wstring wIconsDir(targetDir.begin(), targetDir.end());
                swprintf_s(initialDir, MAX_PATH, L"%s", wIconsDir.c_str());

                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = static_cast<HWND>(nullptr);
                ofn.lpstrFile = szFile;
                ofn.nMaxFile = sizeof(szFile);
                ofn.lpstrFilter = filter;
                ofn.nFilterIndex = 2;
                ofn.lpstrInitialDir = initialDir;
                ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

                if (GetOpenFileName(&ofn) == TRUE)
                {
                    // Get file path
                    std::wstring wFilePath(ofn.lpstrFile);
                    std::string selectedFilePath(wFilePath.begin(), wFilePath.end());
                    
                    // Get substr to hide
                    std::wstring wGameDir(hiddenSubstr.begin(), hiddenSubstr.end());
                    std::string pathToRemove = hiddenSubstr + "\\";

                    filePath = string_utils::replace_substr(selectedFilePath, pathToRemove, "");
                }
            }).detach();
        }
        ImGui::SameLine();
        ImGui::TextPadded(label, 0.f, ImGui::GetStyle().ItemSpacing.y); /** FIXME: Padding does not vertically centre label. */ 
        ImGui::PopID();
    }

    void AddRectFilledGradientV(ImDrawList* draw_list, const ImVec2& p_min, const ImVec2& p_max, ImU32 col_1, ImU32 col_2, float rounding, ImDrawCornerFlags rounding_corners)
    {
        // Draw a placeholder rectangle and record the vertices.
        int vtx_idx_begin = draw_list->VtxBuffer.Size;
        draw_list->AddRectFilled(p_min, p_max, IM_COL32_WHITE, rounding, rounding_corners);
        int vtx_idx_end = draw_list->VtxBuffer.Size;

        // Manually shade the vertices
        ImDrawVert* vert_start = draw_list->VtxBuffer.Data + vtx_idx_begin;
        ImDrawVert* vert_end = draw_list->VtxBuffer.Data + vtx_idx_end;
        
        // Deconstruct colour channels for interpolation
        ImVec4 c0 = ImGui::ColorConvertU32ToFloat4(col_1);
        ImVec4 c1 = ImGui::ColorConvertU32ToFloat4(col_2);

        // Calculate gradient axis (vertical)
        float gradient_height = p_max.y - p_min.y;
        
        for (ImDrawVert* vert = vert_start; vert < vert_end; vert++) 
        {
            // Calculate interpolation factor 't' based on vertex Y position
            float t = ImClamp((vert->pos.y - p_min.y) / gradient_height, 0.0f, 1.0f);
            
            // Linear interpolation of all channels
            ImVec4 interpolated_col = ImLerp(c0, c1, t);
            vert->col = ImGui::ColorConvertFloat4ToU32(interpolated_col);
        }
    }

    void AddRectFilledGradientH(ImDrawList* draw_list, const ImVec2& p_min, const ImVec2& p_max, ImU32 col_1, ImU32 col_2, float rounding, ImDrawCornerFlags rounding_corners)
    {
        // Draw a placeholder rectangle and record the vertices.
        int vtx_idx_begin = draw_list->VtxBuffer.Size;
        draw_list->AddRectFilled(p_min, p_max, IM_COL32_WHITE, rounding, rounding_corners);
        int vtx_idx_end = draw_list->VtxBuffer.Size;

        // Manually shade the vertices
        ImDrawVert* vert_start = draw_list->VtxBuffer.Data + vtx_idx_begin;
        ImDrawVert* vert_end = draw_list->VtxBuffer.Data + vtx_idx_end;

        // Deconstruct colour channels for interpolation
        ImVec4 c0 = ImGui::ColorConvertU32ToFloat4(col_1);
        ImVec4 c1 = ImGui::ColorConvertU32ToFloat4(col_2);

        // Calculate gradient axis (horizontal)
        float gradient_width = p_max.x - p_min.x;

        for (ImDrawVert* vert = vert_start; vert < vert_end; vert++)
        {
            // Calculate interpolation factor 't' based on vertex X position
            float t = ImClamp((vert->pos.x - p_min.x) / gradient_width, 0.0f, 1.0f);
            
            // Linear interpolation of all channels
            ImVec4 interpolated_col = ImLerp(c0, c1, t);
            vert->col = ImGui::ColorConvertFloat4ToU32(interpolated_col);
        }
    }

} // namespace ImGui

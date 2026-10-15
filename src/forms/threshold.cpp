#include "forms.h"

void form_Threshold(float& percentage, bool& stretchToFitHeight, Line_t& line)
{
    ImGui::SliderFloat("Percentage##THRESHOLD_PERCENT", &percentage, 0.0f, 100.0f, "%.1f%%");
    if (percentage < 0.0f) percentage = 0.0f;
    if (percentage > 100.0f) percentage = 100.0f;

    ImGui::Checkbox("Fit Length to Height##THRESHOLD_STRETCH_H", &stretchToFitHeight);
    if (!stretchToFitHeight)
    {
        ImGui::InputInt("Length##THRESHOLD_LINE_LEN", &line.length);
        if (line.length < 1) line.length = 1;
    }

    ImGui::InputInt("Thickness##THRESHOLD_THICKNESS", &line.thickness);
    if (line.thickness < 1) line.thickness = 1;

    static const char* styleOptions[] { "Solid", "Dotted", "Dashed" };
    int sOpt = 0;
    if (line.style == "Dotted") sOpt = 1;
    else if (line.style == "Dashed") sOpt = 2;
    if (ImGui::Combo("Style##THRESHOLD_STYLE", &sOpt, styleOptions, IM_ARRAYSIZE(styleOptions)))
    {
        line.style = styleOptions[sOpt];
    }

    ImGui::ColorEdit4("Color##THRESHOLD_COLOR", (float*)&line.color, ImGuiColorEditFlags_AlphaPreviewHalf);
}

#include "forms.h"

void form_BarDataSource(std::string& dataSource, std::string& dataEffect, float& dataMaxValue)
{
    static const char* dataSourceOptions[] = { 
        "Health percentage", 
        "Barrier percentage", 
        "Shroud percentage", 
        "Effect duration", 
        "Effect stacks" 
    };
    int dsOpt = 0;
    for (int i = 0; i < IM_ARRAYSIZE(dataSourceOptions); ++i)
    {
        if (dataSource == dataSourceOptions[i]) { dsOpt = i; break; }
    }
    if (ImGui::Combo("Source##BAR_DS", &dsOpt, dataSourceOptions, IM_ARRAYSIZE(dataSourceOptions)))
    {
        dataSource = dataSourceOptions[dsOpt];
    }
    
    if (dataSource == "Effect duration" || dataSource == "Effect stacks")
    {
        static const char* effectCategoryOptions[] {
            "Boons",
            "Conditions"
        };

        static const char* boonsEffectOptions[] {
            "Aegis",
            "Alacrity",
            "Fury",
            "Might",
            "Protection",
            "Quickness",
            "Regeneration",
            "Resistance",
            "Resolution",
            "Stability",
            "Swiftness",
            "Vigor"
        };

        static const char* conditionsEffectOptions[] {
            "Bleeding",
            "Blinded",
            "Burning",
            "Chilled",
            "Confusion",
            "Crippled",
            "Fear",
            "Immobilized",
            "Poisoned",
            "Slow",
            "Taunt",
            "Torment",
            "Vulnerability",
            "Weakness"
        };

        // Determine current category based on dataEffect
        bool isCondition = false;
        for (int i = 0; i < IM_ARRAYSIZE(conditionsEffectOptions); ++i)
        {
            if (dataEffect == conditionsEffectOptions[i])
            {
                isCondition = true;
                break;
            }
        }

        int categorySelection = isCondition ? 1 : 0;
        if (ImGui::Combo("Category##BAR_EFF_CAT", &categorySelection, effectCategoryOptions, IM_ARRAYSIZE(effectCategoryOptions)))
        {
            dataEffect = (categorySelection == 1) ? conditionsEffectOptions[0] : boonsEffectOptions[0];
        }

        if (categorySelection == 0)
        {
            int boonSelection = 0;
            for (int i = 0; i < IM_ARRAYSIZE(boonsEffectOptions); ++i)
            {
                if (dataEffect == boonsEffectOptions[i])
                {
                    boonSelection = i;
                    break;
                }
            }
            if (ImGui::Combo("Effect##BAR_EFF_NAME", &boonSelection, boonsEffectOptions, IM_ARRAYSIZE(boonsEffectOptions)))
            {
                dataEffect = boonsEffectOptions[boonSelection];
            }
        }
        else
        {
            int condSelection = 0;
            for (int i = 0; i < IM_ARRAYSIZE(conditionsEffectOptions); ++i)
            {
                if (dataEffect == conditionsEffectOptions[i])
                {
                    condSelection = i;
                    break;
                }
            }
            if (ImGui::Combo("Effect##BAR_EFF_NAME", &condSelection, conditionsEffectOptions, IM_ARRAYSIZE(conditionsEffectOptions)))
            {
                dataEffect = conditionsEffectOptions[condSelection];
            }
        }

        if (dataEffect.empty())
        {
            dataEffect = (categorySelection == 1) ? conditionsEffectOptions[0] : boonsEffectOptions[0];
        }

        // Default dataMaxValue if not set
        if (dataMaxValue <= 0.0f)
        {
            dataMaxValue = (dataSource == "Effect duration") ? 30.0f : 25.0f;
        }

        ImGui::InputFloat("Max Value##BAR_MAX_VAL", &dataMaxValue);
        if (dataMaxValue < 0.0f) dataMaxValue = 0.0f;
    }
}

void form_BarProperties(Size_t& size, ImColor& backgroundColor, ImColor& foregroundColor, int& rounding, int& borderThickness, ImColor& borderColor)
{
    form_Size(size);
    ImGui::ColorEdit4("Background Color##BAR_BG", (float*)&backgroundColor, ImGuiColorEditFlags_AlphaPreviewHalf);
    ImGui::ColorEdit4("Foreground Color##BAR_FG", (float*)&foregroundColor, ImGuiColorEditFlags_AlphaPreviewHalf);
    ImGui::InputInt("Rounding##BAR_ROUND", &rounding);
    if (rounding < 0) rounding = 0;
    ImGui::InputInt("Border Thickness##BAR_BORDER_T", &borderThickness);
    if (borderThickness < 0) borderThickness = 0;
    
    if (borderThickness > 0)
    {
        ImGui::ColorEdit4("Border Color##BAR_BORDER_C", (float*)&borderColor, ImGuiColorEditFlags_AlphaPreviewHalf);
    }
}

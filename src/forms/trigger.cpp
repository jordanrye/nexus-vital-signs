#include "forms.h"

void form_Trigger(Trigger_t& trigger)
{
    static const char* triggerCategoryOptions[] {
        "<Inherit From Parent>", 
        "Boons",
        "Conditions",
        "Auras",
        "Control Effects",
        "Raid Mechanics",
    };
    static const char* triggerBoonsEffectOptions[] {
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
    static const char* triggerConditionsEffectOptions[] {
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
    static const char* triggerAurasEffectOptions[] {
        "Chaos Aura",
        "Dark Aura",
        "Fire Aura",
        "Frost Aura",
        "Light Aura",
        "Magnetic Aura",
        "Shocking Aura"
    };
    static const char* triggerThresholdOptions[] {
        "Present",
        "Absent",
        "More than",
        "Less than",
        "Exactly",
        "Not",
        "Between"
    };

    ImGui::PushID("Trigger");
    {
        // Trigger
        int triggerCategory = 0; // Default to "<Inherit From Parent>"
        if (trigger.category == "<Inherit From Parent>") triggerCategory = 0;
        else if (trigger.category == "Boons") triggerCategory = 1;
        else if (trigger.category == "Conditions") triggerCategory = 2;
        else if (trigger.category == "Auras") triggerCategory = 3;
        else if (trigger.category == "Control Effects") triggerCategory = 4;
        else if (trigger.category == "Raid Mechanics") triggerCategory = 5;
        ImGui::Combo("Type", &triggerCategory, triggerCategoryOptions, IM_ARRAYSIZE(triggerCategoryOptions));
        trigger.category = triggerCategoryOptions[triggerCategory];
    
        if ("<Inherit From Parent>" == trigger.category)
        {
            trigger.effect = "<Inherit From Parent>";
            trigger.threshold = "<Inherit From Parent>";
        }
        else if ("Boons" == trigger.category)
        {
            int triggerEffect = 0; // Default to "Aegis"
            if (trigger.effect == "Aegis") triggerEffect = 0;
            else if (trigger.effect == "Alacrity") triggerEffect = 1;
            else if (trigger.effect == "Fury") triggerEffect = 2;
            else if (trigger.effect == "Might") triggerEffect = 3;
            else if (trigger.effect == "Protection") triggerEffect = 4;
            else if (trigger.effect == "Quickness") triggerEffect = 5;
            else if (trigger.effect == "Regeneration") triggerEffect = 6;
            else if (trigger.effect == "Resistance") triggerEffect = 7;
            else if (trigger.effect == "Resolution") triggerEffect = 8;
            else if (trigger.effect == "Stability") triggerEffect = 9;
            else if (trigger.effect == "Swiftness") triggerEffect = 10;
            else if (trigger.effect == "Vigor") triggerEffect = 11;
    
            ImGui::Combo("Effect", &triggerEffect, triggerBoonsEffectOptions, IM_ARRAYSIZE(triggerBoonsEffectOptions));
            trigger.effect = triggerBoonsEffectOptions[triggerEffect];
        }
        else if ("Conditions" == trigger.category)
        {
            int triggerEffect = 0; // Default to "Bleeding"
            if (trigger.effect == "Bleeding") triggerEffect = 0;
            else if (trigger.effect == "Blinded") triggerEffect = 1;
            else if (trigger.effect == "Burning") triggerEffect = 2;
            else if (trigger.effect == "Chilled") triggerEffect = 3;
            else if (trigger.effect == "Confusion") triggerEffect = 4;
            else if (trigger.effect == "Crippled") triggerEffect = 5;
            else if (trigger.effect == "Fear") triggerEffect = 6;
            else if (trigger.effect == "Immobilized") triggerEffect = 7;
            else if (trigger.effect == "Poisoned") triggerEffect = 8;
            else if (trigger.effect == "Slow") triggerEffect = 9;
            else if (trigger.effect == "Taunt") triggerEffect = 10;
            else if (trigger.effect == "Torment") triggerEffect = 11;
            else if (trigger.effect == "Vulnerability") triggerEffect = 12;
            else if (trigger.effect == "Weakness") triggerEffect = 13;
    
            ImGui::Combo("Effect", &triggerEffect, triggerConditionsEffectOptions, IM_ARRAYSIZE(triggerConditionsEffectOptions));
            trigger.effect = triggerConditionsEffectOptions[triggerEffect];
        }
        else if ("Auras" == trigger.category)
        {
            int triggerEffect = 0; // Default to "Chaos Aura"
            if (trigger.effect == "Chaos Aura") triggerEffect = 0;
            else if (trigger.effect == "Dark Aura") triggerEffect = 1;
            else if (trigger.effect == "Fire Aura") triggerEffect = 2;
            else if (trigger.effect == "Frost Aura") triggerEffect = 3;
            else if (trigger.effect == "Light Aura") triggerEffect = 4;
            else if (trigger.effect == "Magnetic Aura") triggerEffect = 5;
            else if (trigger.effect == "Shocking Aura") triggerEffect = 6;

            ImGui::Combo("Effect", &triggerEffect, triggerAurasEffectOptions, IM_ARRAYSIZE(triggerAurasEffectOptions));
            trigger.effect = triggerAurasEffectOptions[triggerEffect];
        }
    
        if ("<Inherit From Parent>" != trigger.effect)
        {
            int triggerThreshold = 0; // Default to "Present"
            if (trigger.threshold == "Present") triggerThreshold = 0;
            else if (trigger.threshold == "Absent") triggerThreshold = 1;
            else if (trigger.threshold == "More than") triggerThreshold = 2;
            else if (trigger.threshold == "Less than") triggerThreshold = 3;
            else if (trigger.threshold == "Exactly") triggerThreshold = 4;
            else if (trigger.threshold == "Not") triggerThreshold = 5;
            else if (trigger.threshold == "Between") triggerThreshold = 6;
    
            ImGui::Combo("Threshold", &triggerThreshold, triggerThresholdOptions, IM_ARRAYSIZE(triggerThresholdOptions));
            trigger.threshold = triggerThresholdOptions[triggerThreshold];
        }
    }
    ImGui::PopID();
}
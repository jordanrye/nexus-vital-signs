#include "forms.h"

void form_Trigger(Trigger_t& trigger)
{
    static const char* triggerCategoryOptions[] {
        "<Inherit From Parent>", 
        "Boons",
        "Conditions",
        "Professions",
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
        "Vigor",
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
        "Weakness",
    };
    static const char* triggerProfessionOptions[] {
        "Elementalist",
        "Elementalist: Tempest",
        "Elementalist: Weaver",
        "Elementalist: Catalyst",
        "Elementalist: Evoker",
        "Engineer",
        "Engineer: Scrapper",
        "Engineer: Holosmith",
        "Engineer: Mechanist",
        "Engineer: Amalgam",
        "Guardian",
        "Guardian: Dragonhunter",
        "Guardian: Firebrand",
        "Guardian: Willbender",
        "Guardian: Luminary",
        "Mesmer",
        "Mesmer: Chronomancer",
        "Mesmer: Mirage",
        "Mesmer: Virtuoso",
        "Mesmer: Troubadour",
        "Necromancer",
        "Necromancer: Reaper",
        "Necromancer: Scourge",
        "Necromancer: Harbinger",
        "Necromancer: Ritualist",
        "Ranger",
        "Ranger: Druid",
        "Ranger: Soulbeast",
        "Ranger: Untamed",
        "Ranger: Galeshot",
        "Revenant",
        "Revenant: Herald",
        "Revenant: Renegade",
        "Revenant: Vindicator",
        "Revenant: Conduit",
        "Thief",
        "Thief: Daredevil",
        "Thief: Deadeye",
        "Thief: Specter",
        "Thief: Antiquary",
        "Warrior",
        "Warrior: Berserker",
        "Warrior: Spellbreaker",
        "Warrior: Bladesworn",
        "Warrior: Paragon",
    };
    static const char* triggerConditionOptions[] {
        "Status: Active",
        "Status: Inactive",
        "Duration: Less Than",
        "Duration: More Than",
        "Stacks: Less Than",
        "Stacks: More Than",
        "Stacks: Between",
    };

    ImGui::PushID("Trigger");
    {
        int triggerCategory = 0; // Default to "<Inherit From Parent>"
        if (trigger.category == "<Inherit From Parent>") triggerCategory = 0;
        else if (trigger.category == "Boons") triggerCategory = 1;
        else if (trigger.category == "Conditions") triggerCategory = 2;
        else if (trigger.category == "Professions") triggerCategory = 3;
        ImGui::Combo("Category", &triggerCategory, triggerCategoryOptions, IM_ARRAYSIZE(triggerCategoryOptions));
        trigger.category = triggerCategoryOptions[triggerCategory];
    
        if ("<Inherit From Parent>" == trigger.category)
        {
            trigger.effect = "<Inherit From Parent>";
            trigger.condition = "<Inherit From Parent>";
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
        else if ("Professions" == trigger.category)
        {
            int triggerEffect = 0;             
            for (int i = 0; i < IM_ARRAYSIZE(triggerProfessionOptions); i++)
            {
                if (trigger.effect == triggerProfessionOptions[i])
                {
                    triggerEffect = i;
                    break;
                }
            }
            ImGui::Combo("Effect", &triggerEffect, triggerProfessionOptions, IM_ARRAYSIZE(triggerProfessionOptions));
            trigger.effect = triggerProfessionOptions[triggerEffect];
        }

        if (("<Inherit From Parent>" != trigger.category) && ("Professions" != trigger.category))
        {
            int triggerCondition = 0; // Default to "Status: Active"
            if (trigger.condition == "Status: Active") triggerCondition = 0;
            else if (trigger.condition == "Status: Inactive") triggerCondition = 1;
            else if (trigger.condition == "Duration: Less Than") triggerCondition = 2;
            else if (trigger.condition == "Duration: More Than") triggerCondition = 3;
            else if (trigger.condition == "Stacks: Less Than") triggerCondition = 4;
            else if (trigger.condition == "Stacks: More Than") triggerCondition = 5;
            else if (trigger.condition == "Stacks: Between") triggerCondition = 6;
    
            ImGui::Combo("Condition", &triggerCondition, triggerConditionOptions, IM_ARRAYSIZE(triggerConditionOptions));
            trigger.condition = triggerConditionOptions[triggerCondition];

            if (trigger.condition != "Status: Active" && trigger.condition != "Status: Inactive")
            {
                if (trigger.condition == "Stacks: Between")
                {
                    ImGui::InputFloat("Threshold (Min)", &trigger.threshold, 1.f, 1.f, "%.1f");
                    ImGui::InputFloat("Threshold (Max)", &trigger.thresholdMax, 1.f, 1.f, "%.1f");
                }
                else
                {
                    ImGui::InputFloat("Threshold", &trigger.threshold, 1.f, 1.f, "%.1f");
                }
            }
        }
    }
    ImGui::PopID();
}
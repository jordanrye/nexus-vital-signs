#include "forms.h"

void form_IconTextTriggerDuration(IconTextTrigger_t& trigger, bool showSource)
{
    if (showSource) ImGui::BeginGroupPanel("Trigger", ImVec2(ImGui::GetContentRegionMax().x, 0.f));

    int sourceSelection = 1;
    if (showSource)
    {
        static const char* sourceOptions[] = { "Default trigger", "Custom trigger" };
        sourceSelection = (trigger.source == "Custom trigger") ? 1 : 0;
        if (ImGui::Combo("Source##TRIGGER_SOURCE", &sourceSelection, sourceOptions, IM_ARRAYSIZE(sourceOptions)))
            trigger.source = sourceOptions[sourceSelection];
    }

    if (sourceSelection == 1)
    {
        static const char* conditionOptions[] = { "<Inherit From Parent>", "Duration: Less Than", "Duration: More Than" };
        int conditionSelection = 0;
        if (trigger.condition == "Duration: Less Than") conditionSelection = 1;
        else if (trigger.condition == "Duration: More Than") conditionSelection = 2;

        if (ImGui::Combo("Condition##TRIGGER_CONDITION", &conditionSelection, conditionOptions, IM_ARRAYSIZE(conditionOptions)))
            trigger.condition = conditionOptions[conditionSelection];

        if (conditionSelection == 1 || conditionSelection == 2)
        {
            ImGui::InputFloat("Threshold##TRIGGER_THRESHOLD", &trigger.threshold, 0.1f, 1.0f, "%.1f");
        }
    }

    if (showSource) ImGui::EndGroupPanel();
}

void form_IconTextTriggerStacks(IconTextTrigger_t& trigger, bool showSource)
{
    if (showSource) ImGui::BeginGroupPanel("Trigger", ImVec2(ImGui::GetContentRegionMax().x, 0.f));

    int sourceSelection = 1;
    if (showSource)
    {
        static const char* sourceOptions[] = { "Default trigger", "Custom trigger" };
        sourceSelection = (trigger.source == "Custom trigger") ? 1 : 0;
        if (ImGui::Combo("Source##TRIGGER_SOURCE", &sourceSelection, sourceOptions, IM_ARRAYSIZE(sourceOptions)))
            trigger.source = sourceOptions[sourceSelection];
    }

    if (sourceSelection == 1)
    {
        static const char* conditionOptions[] = { "<Inherit From Parent>", "Stacks: Less Than", "Stacks: More Than", "Stacks: Between" };
        int conditionSelection = 0;
        if (trigger.condition == "Stacks: Less Than") conditionSelection = 1;
        else if (trigger.condition == "Stacks: More Than") conditionSelection = 2;
        else if (trigger.condition == "Stacks: Between") conditionSelection = 3;

        if (ImGui::Combo("Condition##TRIGGER_CONDITION", &conditionSelection, conditionOptions, IM_ARRAYSIZE(conditionOptions)))
            trigger.condition = conditionOptions[conditionSelection];

        if (conditionSelection == 1 || conditionSelection == 2)
        {
            ImGui::InputFloat("Threshold##TRIGGER_THRESHOLD", &trigger.threshold, 1.0f, 5.0f, "%.0f");
        }
        else if (conditionSelection == 3)
        {
            ImGui::InputFloat("Min##TRIGGER_THRESHOLD", &trigger.threshold, 1.0f, 5.0f, "%.0f");
            ImGui::InputFloat("Max##TRIGGER_THRESHOLD_MAX", &trigger.thresholdMax, 1.0f, 5.0f, "%.0f");
        }
    }

    if (showSource) ImGui::EndGroupPanel();
}
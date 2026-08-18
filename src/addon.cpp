#include "addon.h"

#include <functional>
#include <map>
#include <string>
#include <sstream>
#include <queue>

#include "data_link/data_link.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui_extensions.h"
#include "imgui_memory_view.h"

#include "forms/forms.h"
#include "imgui_tree_view.h"
#include "settings.h"
#include "shared.h"
#include "ui_common.h"
#include "ui_grid.h"
#include "ui_radial.h"
#include "utils_deletion.h"

bool isValidGameState()
{
    if (NexusLink && MumbleLink && MumbleIdentity)
    {
        if (NexusLink->IsGameplay && MumbleLink->Context.IsGameFocused && !MumbleLink->Context.IsMapOpen && !MumbleLink->Context.IsTextboxFocused)
        {
            return true;
        }
    }

    return false;
}

bool isInCombat()
{
    if (MumbleLink)
    {
        if (MumbleLink->Context.IsInCombat)
        {
            return true;
        }
    }

    return false;
}

namespace Addon {

    /* Global state */
    static bool isRadialMenuActive = false;
    static bool isPreviewModeActive = false;
    bool isSquadManagerActive = false;

    /* Option tabs */
    void OptionsGeneral();
    void OptionsLayoutEditor();
    void OptionsPresets();

    /* Layout Editor utilities */
    void RegisterLayoutEditorViews();
    void RegisterPresetConfigViews();

    void KeybindHandler(const char* aIdentifier, bool aIsRelease)
    {
        if (strcmp(aIdentifier, "KB_HEALING_RADIAL") == 0)
        {
            if (isValidGameState())
            {
                if (!aIsRelease)
                {
                    isRadialMenuActive = true;
                }
                else
                {
                    isRadialMenuActive = false;
                }
            }
            else
            {
                isRadialMenuActive = false;
            }
        }
        else if (strcmp(aIdentifier, "KB_VS_TOGGLE_SQUAD_MANAGER") == 0)
        {
            if (isValidGameState())
            {
                if (!aIsRelease)
                {
                    isSquadManagerActive = true;
                }
                else
                {
                    isSquadManagerActive = false;
                }
            }
            else
            {
                isSquadManagerActive = false;
            }
        }
    }

    bool IsFrameHidden(std::string visibility)
    {
        if (visibility == "Hide out of combat" && !isInCombat())
        {
            return true;
        }

        return false;
    }

    void Render()
    {
        if (isPreviewModeActive || !NexusLink || !NexusLink->IsGameplay || !VitalsData || !VitalsData->isAvailable())
        {
            isPreviewModeActive = false;
            return;
        }

        const VitalSignsDataLink::E_GROUP_TYPE groupType = VitalsData->getGroupType();
        std::string activeLayoutName = ConfigGeneral.soloLayout;
        LayoutConfig_t* activeLayout = nullptr;
        bool hideSelf = false;
        bool hideSubgroups = false;
        std::vector<int>* activeHiddenSubgroups = nullptr;

        switch (groupType)
        {
            case VitalSignsDataLink::E_GROUP_PARTY:
                VitalsData->setPartyFrameVisibility(!ConfigGeneral.isHiddenNativeParty);
                if (IsFrameHidden(ConfigGeneral.partyVisibility))
                {
                    return;
                }
                activeLayoutName = ConfigGeneral.partyLayout;
                hideSelf = ConfigGeneral.isHiddenSelfParty;
                activeHiddenSubgroups = &ConfigGeneral.hiddenSubgroupsParty;
                break;
            case VitalSignsDataLink::E_GROUP_SQUAD_10:
                VitalsData->setSquadFrameVisibility(!ConfigGeneral.isHiddenNativeRaid);
                if (IsFrameHidden(ConfigGeneral.raidVisibility))
                {
                    return;
                }
                activeLayoutName = ConfigGeneral.raidLayout;
                hideSelf = ConfigGeneral.isHiddenSelfRaid;
                hideSubgroups = ConfigGeneral.isHiddenSubgroupsRaid;
                activeHiddenSubgroups = &ConfigGeneral.hiddenSubgroupsRaid;
                break;
            case VitalSignsDataLink::E_GROUP_SQUAD_50:
                VitalsData->setSquadFrameVisibility(!ConfigGeneral.isHiddenNativeSquad);
                if (IsFrameHidden(ConfigGeneral.squadVisibility))
                {
                    return;
                }
                activeLayoutName = ConfigGeneral.squadLayout;
                hideSelf = ConfigGeneral.isHiddenSelfSquad;
                hideSubgroups = ConfigGeneral.isHiddenSubgroupsSquad;
                activeHiddenSubgroups = &ConfigGeneral.hiddenSubgroupsSquad;
                break;
            case VitalSignsDataLink::E_GROUP_NONE:
            default:
                activeLayoutName = ConfigGeneral.soloLayout;
                activeHiddenSubgroups = nullptr;
                break;
        }

        for (auto& layout : g_LayoutManager.GetAllLayouts())
        {
            if (layout.second.name == activeLayoutName)
            {
                activeLayout = &layout.second;
                break;
            }
        }

        if (!activeLayout)
        {
            return; // No active layout found, nothing to render
        }

        if ("Grid" == activeLayout->layout.type)
        {
            if (UI::Grid::BeginGridMenu("VitalSigns##Grid", *activeLayout, ColourPresets, BorderPresets, activeHiddenSubgroups, true /** TODO: isInCombat() */))
            {
                auto clientId = VitalsData->getClientId();
                auto clientSubgroupId = VitalsData->getUserData(clientId).SubgroupId;

                for (const auto &subgroup : VitalsData->getUsers())
                {
                    if (hideSubgroups && subgroup.first != clientSubgroupId)
                    {
                        continue;
                    }

                    for (const auto &user : subgroup.second)
                    {
                        if (hideSelf && user == clientId)
                        {
                            continue;
                        }

                        auto userData = VitalsData->getUserData(user);
            
                        if (UI::Grid::GridMenuItem(userData))
                        {
                            VitalsData->setLockedSelection(user);
                        }
                    }
                }
    
                UI::Grid::EndGridMenu();
            }
        }
        else if ("Radial" == activeLayout->layout.type)
        {
            if (UI::Radial::BeginRadialMenu("VitalSigns##Radial", activeLayout->position, activeLayout->layout, activeLayout->colors, ColourPresets, isRadialMenuActive))
            {
                auto clientId = VitalsData->getClientId();
                auto clientSubgroupId = VitalsData->getUserData(clientId).SubgroupId;

                for (const auto &subgroup : VitalsData->getUsers())
                {
                    if (hideSubgroups && subgroup.first != clientSubgroupId)
                    {
                        continue;
                    }

                    for (const auto &user : subgroup.second)
                    {
                        if (hideSelf && user == clientId)
                        {
                            continue;
                        }

                        auto userData = VitalsData->getUserData(user);

                        if (UI::Radial::RadialMenuItem(userData))
                        {
                            VitalsData->setLockedSelection(user);
                        }
                    }
                }

                UI::Radial::EndRadialMenu();
            }
            
            /** TODO: Implement `if (snap_cursor_to_menu_position)` */
            {
                static POINT prevCursorPosition{};
                static bool prevIsRadialMenuActive = false;
    
                /* Check if state has changed */
                if (prevIsRadialMenuActive != isRadialMenuActive)
                {
                    if (isRadialMenuActive)
                    {
                        GetCursorPos(&prevCursorPosition);
    
                        /* Set cursor position */
                        SetCursorPos((ImGui::GetIO().DisplaySize.x / 2.f + activeLayout->position.offset.x), (ImGui::GetIO().DisplaySize.y / 2.f + activeLayout->position.offset.y));
                    }
                    else
                    {
                        /* Restore cursor position */
                        SetCursorPos(prevCursorPosition.x, prevCursorPosition.y);
                    }
                }
    
                prevIsRadialMenuActive = isRadialMenuActive;
            }
        }
    }

    void Options()
    {
        if (ImGui::BeginTabBar("SettingsTabBar"))
        {
            if (ImGui::BeginTabItem("General"))
            {
                OptionsGeneral();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Layout Editor"))
            {
                OptionsLayoutEditor();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Presets"))
            {
                OptionsPresets();
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }        
    }

    void OptionsGeneral()
    {
        // Generate layout list
        std::vector<std::string> layoutNames = { "" };
        for (const auto& layout : g_LayoutManager.GetAllLayouts())
        {
            layoutNames.push_back(layout.second.name);
        }

        ImGui::PushID("PartyFrames");
        {
            ImGui::TextDisabled("Party Frames (5 Players)");
            ImGui::Separator();
            form_SelectLayout(layoutNames, ConfigGeneral.partyLayout);
            form_Visibility(ConfigGeneral.partyVisibility);
            ImGui::Checkbox("Hide self", &ConfigGeneral.isHiddenSelfParty);
            ImGui::Checkbox("Hide native frames", &ConfigGeneral.isHiddenNativeParty);
        }
        ImGui::PopID();
        
        ImGui::PushID("RaidFrames");
        {
            ImGui::TextDisabled("Raid Frames (10 Players)");
            ImGui::Separator();
            form_SelectLayout(layoutNames, ConfigGeneral.raidLayout);
            form_Visibility(ConfigGeneral.raidVisibility);
            ImGui::Checkbox("Hide self", &ConfigGeneral.isHiddenSelfRaid);
            ImGui::Checkbox("Hide subgroups", &ConfigGeneral.isHiddenSubgroupsRaid);
            ImGui::Checkbox("Hide native frames", &ConfigGeneral.isHiddenNativeRaid);
        }
        ImGui::PopID();

        ImGui::PushID("SquadFrames");
        {
            ImGui::TextDisabled("Squad Frames (50 Players)");
            ImGui::Separator();
            form_SelectLayout(layoutNames, ConfigGeneral.squadLayout);
            form_Visibility(ConfigGeneral.squadVisibility);
            ImGui::Checkbox("Hide self", &ConfigGeneral.isHiddenSelfSquad);
            ImGui::Checkbox("Hide subgroups", &ConfigGeneral.isHiddenSubgroupsSquad);
            ImGui::Checkbox("Hide native frames", &ConfigGeneral.isHiddenNativeSquad);
        }
        ImGui::PopID();
        
        if (Settings::IsDirtySettings())
        {
            Settings::SaveSettings();
        }
    }

    void OptionsLayoutEditor()
    {
        static bool isInitialised = false;
        bool isOpenPopup = false;

        if (!isInitialised)
        {
            RegisterLayoutEditorViews();
            isInitialised = true;
        }

        g_LayoutEditor.UpdateTreeViewCreateText("Create New Layout");
        g_LayoutEditor.UpdateTreeViewSaveText(Settings::IsDirtyLayouts() ? "Save Layouts*" : "Save Layouts");

        /* Render Layout Editor */
        g_LayoutEditor.RenderTreeView(
            [&isOpenPopup]() { isOpenPopup = true; },
            [](TreeNodeUID id) { return g_LayoutManager.Delete(id) || g_LayoutManager.DeleteIndicator(id); },
            [](TreeNodeUID id, const std::string& name, const std::string& type) { g_LayoutManager.AddIndicator(id, name, type); },
            [](TreeNodeUID id, size_t oldIdx, size_t newIdx) { g_LayoutManager.ReorderIndicators(id, oldIdx, newIdx); },
            []() { Settings::SaveAllLayouts(); },
            []() { Settings::LoadAllLayouts(); RegisterLayoutEditorViews(); }
        );
        g_LayoutEditor.RenderContentView();

        if (isOpenPopup)
        {
            ImGui::OpenPopup("Create a new layout");
        }

        if (ImGui::BeginPopupModal("Create a new layout", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            static char inputBuff_Name[MAX_PATH] = "";
            static std::string inputStr_Type = "Grid";
            static bool createFromTemplate = false;

            ImGui::InputText("Name", inputBuff_Name, IM_ARRAYSIZE(inputBuff_Name));
            form_SelectLayoutType(inputStr_Type);
            ImGui::Checkbox("Create using default template", &createFromTemplate);

            if (ImGui::Button("Cancel", ImVec2(100, 0)))
            {
                // Reset state and close
                memset(inputBuff_Name, 0, sizeof(inputBuff_Name));
                createFromTemplate = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Button, (ImU32)ImColor(38, 128, 20));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImU32)ImColor(48, 160, 25));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImU32)ImColor(30, 100, 15));
            
            if (ImGui::Button("Create", ImVec2(100, 0)))
            {
                g_LayoutManager.Create(std::string(inputBuff_Name), inputStr_Type, createFromTemplate, PacksDir);
                memset(inputBuff_Name, 0, sizeof(inputBuff_Name));
                createFromTemplate = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::PopStyleColor(3);

            ImGui::EndPopup();
        }

        /* Render Layout Preview */
        if (g_LayoutEditor.GetActiveNode() != TreeNodeUID::NONE)
        {
            isPreviewModeActive = true;

            if (LayoutConfig_t* layout = g_LayoutManager.GetLayoutFromNodeID(g_LayoutEditor.GetActiveNode()))
            {
                layout->previewNodeId = g_LayoutEditor.GetActiveNode();
                
                if (UI::Grid::BeginGridMenu("Preview##Grid", *layout, ColourPresets, BorderPresets, nullptr, true))
                {
                    VitalSignsDataLink::Effects_t dummyEffects{};
                    VitalSignsDataLink::UserId_t dummyUserId{};
                    VitalSignsDataLink::SubgroupId_t dummySubgroupId = 1;
                    int previewCount = 0;
                    auto AddPreviewItem = [&](
                        VitalSignsDataLink::ESquadRole squadRole,
                        const char* accountName,
                        const char* characterName, 
                        const char* nickname, 
                        bool isInInstance,
                        VitalSignsDataLink::EProfession profession, 
                        VitalSignsDataLink::ESpecialisation specialisation, 
                        float health, 
                        VitalSignsDataLink::E_HEALTH_TYPE healthType, 
                        float barrier) 
                    {
                        VitalSignsDataLink::UserData_t user;
                        user.SubgroupId = (previewCount / 5) + 1;
                        user.SquadRole = squadRole;
                        user.AccountName = accountName;
                        user.CharacterName = characterName;
                        user.Nickname = nickname;
                        user.IsInInstance = isInInstance;
                        user.Profession = profession;
                        user.Specialisation = specialisation;
                        user.HealthType = healthType;
                        user.Health = VitalSignsDataLink::Resource_t(health, 1.0f);
                        user.Barrier = VitalSignsDataLink::Resource_t(barrier, 1.0f);
                        user.Shroud = VitalSignsDataLink::Resource_t(health, 1.0f);
                        user.Effects = dummyEffects;
                        UI::Grid::GridMenuItem(user);
                        previewCount++;
                    };

                    AddPreviewItem(VitalSignsDataLink::ESquadRole::Commander, "Preview.0001", "Preview 1", "", true, VitalSignsDataLink::EProfession::Elementalist, VitalSignsDataLink::ESpecialisation::None, 1.0f, VitalSignsDataLink::E_HEALTH_ALIVE, 0.0f);
                    AddPreviewItem(VitalSignsDataLink::ESquadRole::Lieutenant, "Preview.0002", "Preview 2", "", true, VitalSignsDataLink::EProfession::Engineer, VitalSignsDataLink::ESpecialisation::None, 0.75f, VitalSignsDataLink::E_HEALTH_ALIVE, 0.5f);
                    AddPreviewItem(VitalSignsDataLink::ESquadRole::None, "Preview.0003", "Preview 3", "My Nickname 3", true, VitalSignsDataLink::EProfession::Guardian, VitalSignsDataLink::ESpecialisation::None, 0.5f, VitalSignsDataLink::E_HEALTH_ALIVE, 0.25f);
                    AddPreviewItem(VitalSignsDataLink::ESquadRole::None, "Preview.0004", "Preview 4", "My Nickname 4", true, VitalSignsDataLink::EProfession::Mesmer, VitalSignsDataLink::ESpecialisation::None, 0.25f, VitalSignsDataLink::E_HEALTH_ALIVE, 0.0f);
                    AddPreviewItem(VitalSignsDataLink::ESquadRole::None, "Preview.0005", "Preview 5", "", true, VitalSignsDataLink::EProfession::Necromancer, VitalSignsDataLink::ESpecialisation::None, 1.0f, VitalSignsDataLink::E_HEALTH_ALIVE, 0.5f);
                    AddPreviewItem(VitalSignsDataLink::ESquadRole::None, "Preview.0006", "Preview 6", "", true, VitalSignsDataLink::EProfession::Necromancer, VitalSignsDataLink::ESpecialisation::None, 0.75f, VitalSignsDataLink::E_HEALTH_SHROUD_NECROMANCER, 0.25f);
                    AddPreviewItem(VitalSignsDataLink::ESquadRole::None, "Preview.0007", "Preview 7", "", true, VitalSignsDataLink::EProfession::Ranger, VitalSignsDataLink::ESpecialisation::None, 0.75f, VitalSignsDataLink::E_HEALTH_ALIVE, 0.0f);
                    AddPreviewItem(VitalSignsDataLink::ESquadRole::None, "Preview.0008", "Preview 8", "", true, VitalSignsDataLink::EProfession::Revenant, VitalSignsDataLink::ESpecialisation::None, 0.5f, VitalSignsDataLink::E_HEALTH_ALIVE, 0.0f);
                    AddPreviewItem(VitalSignsDataLink::ESquadRole::None, "Preview.0009", "Preview 9", "", true, VitalSignsDataLink::EProfession::Thief, VitalSignsDataLink::ESpecialisation::None, 0.5f, VitalSignsDataLink::E_HEALTH_ALIVE, 0.25f);
                    AddPreviewItem(VitalSignsDataLink::ESquadRole::None, "Preview.0010", "Preview 10", "", false, VitalSignsDataLink::EProfession::Warrior, VitalSignsDataLink::ESpecialisation::None, 0.75f, VitalSignsDataLink::E_HEALTH_ALIVE, 0.25f);
                    AddPreviewItem(VitalSignsDataLink::ESquadRole::None, "Preview.0011", "Preview 11", "", true, VitalSignsDataLink::EProfession::Thief, VitalSignsDataLink::ESpecialisation::ThiefSpecter, 0.5f, VitalSignsDataLink::E_HEALTH_ALIVE, 0.25f);
                    AddPreviewItem(VitalSignsDataLink::ESquadRole::None, "Preview.0012", "Preview 12", "", true, VitalSignsDataLink::EProfession::Thief, VitalSignsDataLink::ESpecialisation::ThiefSpecter, 0.75f, VitalSignsDataLink::E_HEALTH_SHROUD_SPECTER, 0.25f);
                    AddPreviewItem(VitalSignsDataLink::ESquadRole::None, "Preview.0013", "Preview 13", "", true, VitalSignsDataLink::EProfession::Thief, VitalSignsDataLink::ESpecialisation::ThiefSpecter, 0.5f, VitalSignsDataLink::E_HEALTH_DOWNED, 0.25f);
                    AddPreviewItem(VitalSignsDataLink::ESquadRole::None, "Preview.0014", "Preview 14", "", true, VitalSignsDataLink::EProfession::Thief, VitalSignsDataLink::ESpecialisation::ThiefSpecter, 0.25f, VitalSignsDataLink::E_HEALTH_DEFEATED, 0.0f);
                    AddPreviewItem(VitalSignsDataLink::ESquadRole::None, "Preview.0015", "Preview 15", "", true, VitalSignsDataLink::EProfession::Thief, VitalSignsDataLink::ESpecialisation::ThiefSpecter, 0.75f, VitalSignsDataLink::E_HEALTH_ALIVE, 0.25f);
                    UI::Grid::EndGridMenu();
                }

                layout->previewNodeId = TreeNodeUID::NONE;
            }
        }
    }

    void OptionsPresets()
    {
        static bool isInitialised = false;

        if (!isInitialised)
        {
            RegisterPresetConfigViews();
            isInitialised = true;
        }

        g_PresetConfig.UpdateTreeViewSaveText(Settings::IsDirtyPresets() ? "Save Presets*" : "Save Presets");

        g_PresetConfig.RenderTreeView(nullptr, nullptr, nullptr, nullptr, Settings::SavePresets, Settings::LoadPresets);
        g_PresetConfig.RenderContentView();
    }

    // void OptionsColors()
    // {
    //     ImGui::BeginGroupPanel("Effects", ImVec2(inputWidth, 0.f));
    //     {
    //         /** TODO: Move into `Indicator` config. */
    //         ImGui::ColorEdit4("boons (1)##COLOUR_BOONS_1", &(ColourPresets.COLOUR_BOONS_1.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
    //         ImGui::ColorEdit4("boons (2)##COLOUR_BOONS_2", &(ColourPresets.COLOUR_BOONS_2.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
    //         ImGui::ColorEdit4("conditions (1)##COLOUR_CONDITIONS_1", &(ColourPresets.COLOUR_CONDITIONS_1.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
    //         ImGui::ColorEdit4("conditions (2)##COLOUR_CONDITIONS_2", &(ColourPresets.COLOUR_CONDITIONS_2.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
    //     }
    //     ImGui::EndGroupPanel();
    // }

    void ContentViewEmpty()
    {
        ImGui::TextDisabled("No element selected.");
        ImGui::Separator();
        ImGui::TextWrappedDisabled("Click an item in the navigation menu to configure it, or press the \"Create New Layout\" button in the bottom-left.");
    }

    void ContentViewGeneral(std::string& name, std::string& colors, Position_t& position, Layout_t& layout)
    {
        static const char* colorsOptions[] {
            "Default Color Palette",
            "Profession Color Palette",
            "Heat Map Color Palette"
        };

        static int layoutSelection = 0; // Default to "Radial"
        if (layout.type == "Radial") layoutSelection = 0;
        else if (layout.type == "Grid") layoutSelection = 1;
        
        ImGui::TextDisabled("General");
        ImGui::Separator();
        {
            if (ImGui::InputText("Name", &name))
            {
                g_LayoutEditor.UpdateNodeLabel(g_LayoutEditor.GetActiveNode(), name);
            }
            form_SelectLayoutType(layout.type, true);
        }

        ImGui::TextDisabled("Position");
        ImGui::Separator();
        {
            form_Position(position);
        }

        ImGui::TextDisabled("Colors");
        ImGui::Separator();
        {
            static int colorsSelection = 0; // Default to "Centre"
            if (colors == "Default") colorsSelection = 0;
            else if (colors == "Profession") colorsSelection = 1;
            else if (colors == "Heat Map") colorsSelection = 2;
            // else if (colors == "Role") colorsSelection = -1;
            // else if (colors == "Party") colorsSelection = -1;

            if (ImGui::Combo("Color Palette", &colorsSelection, colorsOptions, IM_ARRAYSIZE(colorsOptions)))
            {
                if (colorsSelection == 0) colors = "Default";
                else if (colorsSelection == 1) colors = "Profession";
                else if (colorsSelection == 2) colors = "Heat Map";
            }
        }

        /* Radial*/
        if (0U == layoutSelection)
        {
            ImGui::TextDisabled("Layout");
            ImGui::Separator();
            {
                ImGui::InputFloat("Radius (Inner)##RADIUS_MIN", &layout.radial.sectorRadiusInner, 5.f, 10.f, "%.2f");
                ImGui::InputFloat("Radius (Outer)##RADIUS_MAX", &layout.radial.sectorRadiusOuter, 5.f, 10.f, "%.2f");
                ImGui::SliderInt("Items (Min)##ITEMS_MIN", &layout.radial.sectorCountMin, 2, 10);
                ImGui::SliderInt("Items (Max)##ITEMS_MAX", &layout.radial.sectorCountMax, 4, UI::SQUAD_MEMBER_LIMIT);
                if (ImGui::InputInt("Item Spacing##ITEM_INNER_SPACING", &layout.radial.itemSpacing, 1, 2, ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    const float MAX_SPACING = (layout.radial.sectorRadiusOuter - layout.radial.sectorRadiusInner) / 2;
                    if (layout.radial.itemSpacing > MAX_SPACING)
                    {
                        layout.radial.itemSpacing = MAX_SPACING;
                    }
                    else if (layout.radial.itemSpacing < 0)
                    {
                        layout.radial.itemSpacing = 0;
                    }
                }
                if (ImGui::InputInt("Item Border##ITEM_BORDER", &layout.itemBorder, 1, 2, ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    const float MAX_BORDER = (layout.radial.sectorRadiusOuter - layout.radial.sectorRadiusInner) / 2;
                    if (layout.itemBorder > MAX_BORDER)
                    {
                        layout.itemBorder = MAX_BORDER;
                    }
                    else if (layout.itemBorder < 0)
                    {
                        layout.itemBorder = 0;
                    }
                }
            }
        }

        /* Grid */
        if (1U == layoutSelection)
        {
            ImGui::TextDisabled("Grid Properties");
            ImGui::Separator();
            {
                form_Direction(layout.grid.frameDirection, "Frame Direction");
                ImGui::SameLine();
                ImGui::TextDisabled("(?)");
                ImGui::TooltipGeneric(
                    "Determines whether players in the same\n" \
                    "subgroup are positioned in a columnar or\n" \
                    "row arrangement.");

                form_Direction(layout.grid.squadDirection, "Squad Direction");
                ImGui::SameLine();
                ImGui::TextDisabled("(?)");
                ImGui::TooltipGeneric(
                    "Determines where Subgroup 2 is positioned\n" \
                    "relative to Subgroup 1.");

                if (ImGui::InputInt("Max Rows##MAX_ROWS", &layout.grid.maxRows, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    if (layout.grid.maxRows < 1)
                    {
                        layout.grid.maxRows = 1;
                    }
                }
                
                if (ImGui::InputInt("Max Columns##MAX_COLUMNS", &layout.grid.maxColumns, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    if (layout.grid.maxColumns < 1)
                    {
                        layout.grid.maxColumns = 1;
                    }
                }
                
                if (ImGui::InputInt("Spacing (Horizontal)##CELL_SPACING_HORIZONTAL", &layout.grid.spacingHorizontal, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    if (layout.grid.spacingHorizontal < -layout.itemBorder)
                    {
                        layout.grid.spacingHorizontal = -layout.itemBorder;
                    }
                }
                
                if (ImGui::InputInt("Spacing (Vertical)##CELL_SPACING_VERTICAL", &layout.grid.spacingVertical, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    if (layout.grid.spacingVertical < -layout.itemBorder)
                    {
                        layout.grid.spacingVertical = -layout.itemBorder;
                    }
                }
            }

            ImGui::TextDisabled("Frame Properties");
            ImGui::Separator();
            {
                if (ImGui::InputInt("Width##CELL_WIDTH", &layout.grid.cellWidth, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    if (layout.grid.cellWidth < 0)
                    {
                        layout.grid.cellWidth = 0;
                    }
                }
                if (ImGui::InputInt("Height##CELL_HEIGHT", &layout.grid.cellHeight, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    if (layout.grid.cellHeight < 0)
                    {
                        layout.grid.cellHeight = 0;
                    }
                }
                if (ImGui::InputInt("Rounding##CELL_ROUNDING", &layout.grid.cellRounding, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    if (layout.grid.cellRounding < 0)
                    {
                        layout.grid.cellRounding = 0;
                    }
                }
                if (ImGui::InputInt("Border##CELL_BORDER", &layout.itemBorder, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    if (layout.itemBorder < 0)
                    {
                        layout.itemBorder = 0;
                    }
                }
            }

            ImGui::TextDisabled("Advanced");
            ImGui::Separator();
            {
                if (ImGui::CollapsingHeader("Subgroup Headers"))
                {
                    ImGui::BeginGroupPanel("General", ImVec2(ImGui::GetContentRegionMax().x, 0.f));
                    {
                        static const char* visibilityOptions[] = { "Always show", "Show on hover", "Hidden" };
                        int vOpt = 0;
                        if (layout.grid.subgroupHeader.visibility == "Always show") vOpt = 0;
                        else if (layout.grid.subgroupHeader.visibility == "Show on hover") vOpt = 1;
                        else if (layout.grid.subgroupHeader.visibility == "Hidden") vOpt = 2;
                        if (ImGui::Combo("Visibility##SQUAD_MANAGER_VIS", &vOpt, visibilityOptions, IM_ARRAYSIZE(visibilityOptions)))
                        {
                            layout.grid.subgroupHeader.visibility = visibilityOptions[vOpt];
                        }
                        
                        static const char* typeOptions[] = { "Badge", "Bracket", "Divider" };
                        int tOpt = 0;
                        if (layout.grid.subgroupHeader.type == "Badge") tOpt = 0;
                        else if (layout.grid.subgroupHeader.type == "Bracket") tOpt = 1;
                        else if (layout.grid.subgroupHeader.type == "Divider") tOpt = 2;
                        if (ImGui::Combo("Type##SQUAD_MANAGER_TYPE", &tOpt, typeOptions, IM_ARRAYSIZE(typeOptions)))
                        {
                            layout.grid.subgroupHeader.type = typeOptions[tOpt];
                        }
                    }
                    ImGui::EndGroupPanel();

                    ImGui::BeginGroupPanel("Position", ImVec2(ImGui::GetContentRegionMax().x, 0.f));
                    {
                        if (layout.grid.subgroupHeader.type == "Badge" || layout.grid.subgroupHeader.type == "Bracket")
                        {
                            std::vector<const char*> anchorOptions;
                            if (layout.grid.squadDirection == "Left-to-right" || layout.grid.squadDirection == "Right-to-left")
                            {
                                anchorOptions = { "Top", "Bottom" };
                                if (layout.grid.subgroupHeader.anchor != "Top" && layout.grid.subgroupHeader.anchor != "Bottom") layout.grid.subgroupHeader.anchor = "Top";
                            }
                            else
                            {
                                anchorOptions = { "Left", "Right" };
                                if (layout.grid.subgroupHeader.anchor != "Left" && layout.grid.subgroupHeader.anchor != "Right") layout.grid.subgroupHeader.anchor = "Left";
                            }
                            
                            int aOpt = 0;
                            for (size_t i = 0; i < anchorOptions.size(); ++i)
                            {
                                if (layout.grid.subgroupHeader.anchor == anchorOptions[i]) aOpt = (int)i;
                            }
                            
                            if (ImGui::Combo("Anchor##SQUAD_MANAGER_ANCHOR", &aOpt, anchorOptions.data(), (int)anchorOptions.size()))
                            {
                                layout.grid.subgroupHeader.anchor = anchorOptions[aOpt];
                            }
                        }
                        else if (layout.grid.subgroupHeader.type == "Divider")
                        {
                            std::vector<const char*> alignOptions;
                            if (layout.grid.squadDirection == "Left-to-right" || layout.grid.squadDirection == "Right-to-left")
                            {
                                alignOptions = { "Top", "Centre", "Bottom" };
                                if (layout.grid.subgroupHeader.divider.alignment != "Top" && layout.grid.subgroupHeader.divider.alignment != "Bottom") layout.grid.subgroupHeader.divider.alignment = "Centre";
                            }
                            else
                            {
                                alignOptions = { "Left", "Centre", "Right" };
                                if (layout.grid.subgroupHeader.divider.alignment != "Left" && layout.grid.subgroupHeader.divider.alignment != "Right") layout.grid.subgroupHeader.divider.alignment = "Centre";
                            }
                            int alignOpt = 0;
                            for (size_t i = 0; i < alignOptions.size(); ++i)
                            {
                                if (layout.grid.subgroupHeader.divider.alignment == alignOptions[i]) alignOpt = (int)i;
                            }
                            if (ImGui::Combo("Alignment##SQUAD_MANAGER_DIV_ALIGN", &alignOpt, alignOptions.data(), (int)alignOptions.size()))
                            {
                                layout.grid.subgroupHeader.divider.alignment = alignOptions[alignOpt];
                            }
                        }
                        ImGui::InputInt("Offset X##SQUAD_MANAGER_OFFSET_X", &layout.grid.subgroupHeader.offset.x);
                        ImGui::InputInt("Offset Y##SQUAD_MANAGER_OFFSET_Y", &layout.grid.subgroupHeader.offset.y);
                    }
                    ImGui::EndGroupPanel();

                    if (layout.grid.subgroupHeader.type == "Badge")
                    {
                        ImGui::BeginGroupPanel("Badge Properties", ImVec2(ImGui::GetContentRegionMax().x, 0.f));
                        {
                            static const char* shapeOptions[] = { "Rectangle", "Corner Ribbon", "Texture" };
                            int sOpt = 0;
                            if (layout.grid.subgroupHeader.badge.shape == "Corner Ribbon") sOpt = 1;
                            else if (layout.grid.subgroupHeader.badge.shape == "Texture") sOpt = 2;
                            if (ImGui::Combo("Shape##SQUAD_MANAGER_SHAPE", &sOpt, shapeOptions, IM_ARRAYSIZE(shapeOptions)))
                            {
                                layout.grid.subgroupHeader.badge.shape = shapeOptions[sOpt];
                            }
    
                            if (!layout.grid.subgroupHeader.stretchToFitWidth) {
                                ImGui::InputInt("Width##SQUAD_MANAGER_DIM_W", &layout.grid.subgroupHeader.badge.rectangle.dimensions.width);
                            }
                            ImGui::Checkbox("Stretch to Fit Width##SQUAD_MANAGER_STRETCH_W", &layout.grid.subgroupHeader.stretchToFitWidth);
    
                            if (!layout.grid.subgroupHeader.stretchToFitHeight) {
                                ImGui::InputInt("Height##SQUAD_MANAGER_DIM_H", &layout.grid.subgroupHeader.badge.rectangle.dimensions.height);
                            }
                            ImGui::Checkbox("Stretch to Fit Height##SQUAD_MANAGER_STRETCH_H", &layout.grid.subgroupHeader.stretchToFitHeight);
    
                            ImGui::ColorEdit4("Background Color##SQUAD_MANAGER_BG", (float*)&layout.grid.subgroupHeader.badge.rectangle.color, ImGuiColorEditFlags_AlphaPreviewHalf);
    
                            ImGui::InputInt("Rounding##SQUAD_MANAGER_ROUNDING", &layout.grid.subgroupHeader.badge.rectangle.rounding);
                            if (layout.grid.subgroupHeader.badge.rectangle.rounding < 0) layout.grid.subgroupHeader.badge.rectangle.rounding = 0;
    
                            ImGui::InputInt("Border Thickness##SQUAD_MANAGER_BORDER_T", &layout.grid.subgroupHeader.badge.rectangle.borderThickness);
                            if (layout.grid.subgroupHeader.badge.rectangle.borderThickness < 0) layout.grid.subgroupHeader.badge.rectangle.borderThickness = 0;
                            
                            if (layout.grid.subgroupHeader.badge.rectangle.borderThickness > 0)
                            {
                                ImGui::ColorEdit4("Border Color##SQUAD_MANAGER_BORDER_C", (float*)&layout.grid.subgroupHeader.badge.rectangle.borderColor, ImGuiColorEditFlags_AlphaPreviewHalf);
                            }
                        }
                        ImGui::EndGroupPanel();
                    }
                    else if (layout.grid.subgroupHeader.type == "Bracket")
                    {
                        ImGui::BeginGroupPanel("Bracket Properties", ImVec2(ImGui::GetContentRegionMax().x, 0.f));
                        {
                            ImGui::ColorEdit4("Color##SQUAD_MANAGER_BRKT_C", (float*)&layout.grid.subgroupHeader.bracket.line.color, ImGuiColorEditFlags_AlphaPreviewHalf);
                            ImGui::InputInt("Thickness##SQUAD_MANAGER_BRKT_THICK", &layout.grid.subgroupHeader.bracket.line.thickness);
                            ImGui::InputInt("Margin##SQUAD_MANAGER_BRKT_MARGIN", &layout.grid.subgroupHeader.bracket.margin);
                            ImGui::InputInt("Arm Length (Outer)##SQUAD_MANAGER_BRKT_LIP", &layout.grid.subgroupHeader.bracket.outerArmLength);
                            ImGui::InputInt("Arm Length (Inner)##SQUAD_MANAGER_BRKT_CLIP", &layout.grid.subgroupHeader.bracket.innerArmLength);
                            ImGui::Checkbox("Shadow##SQUAD_MANAGER_BRKT_SHDW", &layout.grid.subgroupHeader.bracket.shadow);
                            if (layout.grid.subgroupHeader.bracket.shadow)
                            {
                                ImGui::SameLine();
                                ImGui::ColorEdit4("Shadow Color##SQUAD_MANAGER_BRKT_SHDW_C", (float*)&layout.grid.subgroupHeader.bracket.shadowColor, ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_NoInputs);
                            }
                            ImGui::Checkbox("Outline##SQUAD_MANAGER_BRKT_OUTL", &layout.grid.subgroupHeader.bracket.outline);
                            if (layout.grid.subgroupHeader.bracket.outline)
                            {
                                ImGui::SameLine();
                                ImGui::ColorEdit4("Outline Color##SQUAD_MANAGER_BRKT_OUTL_C", (float*)&layout.grid.subgroupHeader.bracket.outlineColor, ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_NoInputs);
                            }
                        }
                        ImGui::EndGroupPanel();
                    }
                    else if (layout.grid.subgroupHeader.type == "Divider")
                    {
                        ImGui::BeginGroupPanel("Divider Properties", ImVec2(ImGui::GetContentRegionMax().x, 0.f));
                        {
                            static const char* divOptions[] = { "Solid", "Dotted", "Dashed", "Rectangle", "Texture" };
                            int dOpt = 0;
                            if (layout.grid.subgroupHeader.divider.line.style == "Dotted") dOpt = 1;
                            else if (layout.grid.subgroupHeader.divider.line.style == "Dashed") dOpt = 2;
                            else if (layout.grid.subgroupHeader.divider.line.style == "Rectangle") dOpt = 3;
                            else if (layout.grid.subgroupHeader.divider.line.style == "Texture") dOpt = 4;
                            if (ImGui::Combo("Divider Style##SQUAD_MANAGER_DIVIDER", &dOpt, divOptions, IM_ARRAYSIZE(divOptions)))
                            {
                                layout.grid.subgroupHeader.divider.line.style = divOptions[dOpt];
                            }
    
                            ImGui::InputInt("Spacing##SQUAD_MANAGER_DIV_SPACING", &layout.grid.subgroupHeader.divider.spacing);
    
                            bool isRectOrTex = (layout.grid.subgroupHeader.divider.line.style == "Rectangle" || layout.grid.subgroupHeader.divider.line.style == "Texture");
                            bool isVerticalSquad = (layout.grid.squadDirection == "Left-to-right" || layout.grid.squadDirection == "Right-to-left");
                            bool isStretchingLine = isVerticalSquad ? layout.grid.subgroupHeader.divider.stretchToFitHeight : layout.grid.subgroupHeader.divider.stretchToFitWidth;
                            
                            if (!isRectOrTex) {
                                if (!isStretchingLine) {
                                    ImGui::InputInt("Line Length##SQUAD_MANAGER_DIV_LEN", &layout.grid.subgroupHeader.divider.line.length);
                                }
                            } else {
                                if (!layout.grid.subgroupHeader.divider.stretchToFitWidth) {
                                    ImGui::InputInt("Width##SQUAD_MANAGER_DIV_DIM_W", &layout.grid.subgroupHeader.divider.rectangle.dimensions.width);
                                }
                            }
    
                            if (isRectOrTex || !isVerticalSquad) {
                                ImGui::Checkbox("Stretch to Fit Width##SQUAD_MANAGER_DIV_STRETCH_W", &layout.grid.subgroupHeader.divider.stretchToFitWidth);
                            }
    
                            if (!isRectOrTex) {
                                ImGui::InputInt("Line Thickness##SQUAD_MANAGER_DIV_THICK", &layout.grid.subgroupHeader.divider.line.thickness);
                            } else {
                                if (!layout.grid.subgroupHeader.divider.stretchToFitHeight) {
                                    ImGui::InputInt("Height##SQUAD_MANAGER_DIV_DIM_H", &layout.grid.subgroupHeader.divider.rectangle.dimensions.height);
                                }
                            }
    
                            if (isRectOrTex || isVerticalSquad) {
                                ImGui::Checkbox("Stretch to Fit Height##SQUAD_MANAGER_DIV_STRETCH_H", &layout.grid.subgroupHeader.divider.stretchToFitHeight);
                            }
    
                            if (!isRectOrTex) {
                                ImGui::ColorEdit4("Divider Color##SQUAD_MANAGER_DIV_C", (float*)&layout.grid.subgroupHeader.divider.line.color, ImGuiColorEditFlags_AlphaPreviewHalf);
                            } else {
                                ImGui::ColorEdit4("Background Color##SQUAD_MANAGER_DIV_C", (float*)&layout.grid.subgroupHeader.divider.rectangle.color, ImGuiColorEditFlags_AlphaPreviewHalf);
                                
                                ImGui::InputInt("Rounding##SQUAD_MANAGER_DIV_ROUNDING", &layout.grid.subgroupHeader.divider.rectangle.rounding);
                                if (layout.grid.subgroupHeader.divider.rectangle.rounding < 0) layout.grid.subgroupHeader.divider.rectangle.rounding = 0;
                                
                                ImGui::InputInt("Border Thickness##SQUAD_MANAGER_DIV_BORDER_T", &layout.grid.subgroupHeader.divider.rectangle.borderThickness);
                                if (layout.grid.subgroupHeader.divider.rectangle.borderThickness < 0) layout.grid.subgroupHeader.divider.rectangle.borderThickness = 0;
                                
                                if (layout.grid.subgroupHeader.divider.rectangle.borderThickness > 0)
                                {
                                    ImGui::ColorEdit4("Border Color##SQUAD_MANAGER_DIV_BORDER_C", (float*)&layout.grid.subgroupHeader.divider.rectangle.borderColor, ImGuiColorEditFlags_AlphaPreviewHalf);
                                }
                            }
                        }
                        ImGui::EndGroupPanel();
                    }

                    ImGui::BeginGroupPanel("Text Properties", ImVec2(ImGui::GetContentRegionMax().x, 0.f));
                    {
                        ImGui::PushID("SQUAD_MANAGER_LABEL");
                        form_Position(layout.grid.subgroupHeader.labelPosition);
                        form_Font(layout.grid.subgroupHeader.textStyle.fontSource, layout.grid.subgroupHeader.textStyle.font);
                        form_FontSize(layout.grid.subgroupHeader.textStyle.fontSizeSource, layout.grid.subgroupHeader.textStyle.fontSize);
                        form_FontColour(layout.grid.subgroupHeader.textStyle.colorSource, layout.grid.subgroupHeader.textStyle.color);
                        form_FontDecorator(layout.grid.subgroupHeader.textStyle.decoratorSource, layout.grid.subgroupHeader.textStyle.shadow, layout.grid.subgroupHeader.textStyle.shadowColor, layout.grid.subgroupHeader.textStyle.outline, layout.grid.subgroupHeader.textStyle.outlineColor);
                        ImGui::PopID();
                    }
                    ImGui::EndGroupPanel();
                }
            }
        }
    }

    void ContentViewIndicator(Indicator_t& indicator)
    {
        ImGui::TextDisabled("General");
        ImGui::Separator();
        {
            if (ImGui::Checkbox("Enabled", &indicator.enabled))
            {
                g_LayoutEditor.UpdateNodeEnabled(g_LayoutEditor.GetActiveNode(), indicator.enabled);
            }
            if (ImGui::InputText("Name", &indicator.name))
            {
                g_LayoutEditor.UpdateNodeLabel(g_LayoutEditor.GetActiveNode(), indicator.name);
            }
            form_SelectIndicatorType(indicator.type, true);
        }

        if ("Icon" == indicator.type)
        {
            ImGui::TextDisabled("Position");
            ImGui::Separator();
            {
                form_Position(indicator.iconSingle.position);
            }

            ImGui::TextDisabled("Icon");
            ImGui::Separator();
            {
                form_Texture(indicator.iconSingle.icon.source, indicator.iconSingle.icon.path);
                form_Size(indicator.iconSingle.size);
                
                ImGui::PushID("DurationText");
                if (indicator.iconSingle.showDuration)
                {
                    if (ImGui::CheckableCollapsingHeader("Duration", &(indicator.iconSingle.showDuration)))
                    {
                        ImGui::BeginGroupPanel("Position", ImVec2(ImGui::GetContentRegionMax().x, 0.f));
                        {
                            form_Position(indicator.iconSingle.durationText.positionSource, indicator.iconSingle.durationText.position);
                        }
                        ImGui::EndGroupPanel();
                        
                        form_IconTextFormatDuration(indicator.iconSingle.durationText.textFormatSource, indicator.iconSingle.durationText.textFormatPrecision);

                        ImGui::BeginGroupPanel("Text Style", ImVec2(ImGui::GetContentRegionMax().x, 0.f));
                        {
                            form_Font(indicator.iconSingle.durationText.textStyle.fontSource, indicator.iconSingle.durationText.textStyle.font);
                            form_FontSize(indicator.iconSingle.durationText.textStyle.fontSizeSource, indicator.iconSingle.durationText.textStyle.fontSize);
                            form_FontColour(indicator.iconSingle.durationText.textStyle.colorSource, indicator.iconSingle.durationText.textStyle.color);
                            form_FontDecorator(
                                indicator.iconSingle.durationText.textStyle.decoratorSource, 
                                indicator.iconSingle.durationText.textStyle.shadow, 
                                indicator.iconSingle.durationText.textStyle.shadowColor, 
                                indicator.iconSingle.durationText.textStyle.outline, 
                                indicator.iconSingle.durationText.textStyle.outlineColor);
                        }
                        ImGui::EndGroupPanel();

                        form_IconTextTriggerDuration(indicator.iconSingle.durationText.trigger);
                    }
                }
                else
                {
                    ImGui::Checkbox("Show Duration", &indicator.iconSingle.showDuration);
                }
                ImGui::PopID();
                
                ImGui::PushID("StacksText");
                if (indicator.iconSingle.showStacks)
                {
                    if (ImGui::CheckableCollapsingHeader("Stacks", &(indicator.iconSingle.showStacks)))
                    {
                        ImGui::BeginGroupPanel("Position", ImVec2(ImGui::GetContentRegionMax().x, 0.f));
                        {
                            form_Position(indicator.iconSingle.stacksText.positionSource, indicator.iconSingle.stacksText.position);
                        }
                        ImGui::EndGroupPanel();
    
                        ImGui::BeginGroupPanel("Text Style", ImVec2(ImGui::GetContentRegionMax().x, 0.f));
                        {
                            form_Font(indicator.iconSingle.stacksText.textStyle.fontSource, indicator.iconSingle.stacksText.textStyle.font);
                            form_FontSize(indicator.iconSingle.stacksText.textStyle.fontSizeSource, indicator.iconSingle.stacksText.textStyle.fontSize);
                            form_FontColour(indicator.iconSingle.stacksText.textStyle.colorSource, indicator.iconSingle.stacksText.textStyle.color);
                            form_FontDecorator(
                                indicator.iconSingle.stacksText.textStyle.decoratorSource, 
                                indicator.iconSingle.stacksText.textStyle.shadow, 
                                indicator.iconSingle.stacksText.textStyle.shadowColor, 
                                indicator.iconSingle.stacksText.textStyle.outline, 
                                indicator.iconSingle.stacksText.textStyle.outlineColor);
                        }
                        ImGui::EndGroupPanel();

                        form_IconTextTriggerStacks(indicator.iconSingle.stacksText.trigger);
                    }
                }
                else
                {
                    ImGui::Checkbox("Show Stacks", &indicator.iconSingle.showStacks);
                }
                ImGui::PopID();
            }
            
            ImGui::TextDisabled("Trigger");
            ImGui::Separator();
            {
                form_Trigger(indicator.iconSingle.icon.trigger);
            }
        }

        if ("Icon List" == indicator.type)
        {
            ImGui::TextDisabled("Position");
            ImGui::Separator();
            {
                form_Position(indicator.iconList.position);
            }

            ImGui::TextDisabled("List Properties");
            ImGui::Separator();
            {
                form_ListLength(indicator.iconList.listLength);
                form_Direction(indicator.iconList.listDirection);
                form_ListSpacing(indicator.iconList.listSpacing);
            }
            
            ImGui::TextDisabled("Shared Icon Properties");
            ImGui::Separator();
            {
                form_Size(indicator.iconList.size);

                ImGui::PushID("DurationText");
                if (indicator.iconList.showDuration)
                {
                    if (ImGui::CheckableCollapsingHeader("Duration", &(indicator.iconList.showDuration)))
                    {
                        ImGui::BeginGroupPanel("Position", ImVec2(ImGui::GetContentRegionMax().x, 0.f));
                        {
                            form_Position(indicator.iconList.durationText.positionSource, indicator.iconList.durationText.position);
                        }
                        ImGui::EndGroupPanel();

                        form_IconTextFormatDuration(indicator.iconList.durationText.textFormatSource, indicator.iconList.durationText.textFormatPrecision);

                        ImGui::BeginGroupPanel("Text Style", ImVec2(ImGui::GetContentRegionMax().x, 0.f));
                        {
                            form_Font(indicator.iconList.durationText.textStyle.fontSource, indicator.iconList.durationText.textStyle.font);
                            form_FontSize(indicator.iconList.durationText.textStyle.fontSizeSource, indicator.iconList.durationText.textStyle.fontSize);
                            form_FontColour(indicator.iconList.durationText.textStyle.colorSource, indicator.iconList.durationText.textStyle.color);
                            form_FontDecorator(
                                indicator.iconList.durationText.textStyle.decoratorSource, 
                                indicator.iconList.durationText.textStyle.shadow, 
                                indicator.iconList.durationText.textStyle.shadowColor, 
                                indicator.iconList.durationText.textStyle.outline, 
                                indicator.iconList.durationText.textStyle.outlineColor);   
                        }
                        ImGui::EndGroupPanel();

                        form_IconTextTriggerDuration(indicator.iconList.durationText.trigger);
                    }
                }
                else
                {
                    ImGui::Checkbox("Show Duration", &indicator.iconList.showDuration);
                }
                ImGui::PopID();

                ImGui::PushID("StacksText");
                if (indicator.iconList.showStacks)
                {
                    if (ImGui::CheckableCollapsingHeader("Stacks", &(indicator.iconList.showStacks)))
                    {
                        ImGui::BeginGroupPanel("Position", ImVec2(ImGui::GetContentRegionMax().x, 0.f));
                        {
                            form_Position(indicator.iconList.stacksText.positionSource, indicator.iconList.stacksText.position);
                        }
                        ImGui::EndGroupPanel();
                        
                        ImGui::BeginGroupPanel("Text Style", ImVec2(ImGui::GetContentRegionMax().x, 0.f));
                        {
                            form_Font(indicator.iconList.stacksText.textStyle.fontSource, indicator.iconList.stacksText.textStyle.font);
                            form_FontSize(indicator.iconList.stacksText.textStyle.fontSizeSource, indicator.iconList.stacksText.textStyle.fontSize);
                            form_FontColour(indicator.iconList.stacksText.textStyle.colorSource, indicator.iconList.stacksText.textStyle.color);
                            form_FontDecorator(
                                indicator.iconList.stacksText.textStyle.decoratorSource, 
                                indicator.iconList.stacksText.textStyle.shadow, 
                                indicator.iconList.stacksText.textStyle.shadowColor, 
                                indicator.iconList.stacksText.textStyle.outline, 
                                indicator.iconList.stacksText.textStyle.outlineColor);
                        }
                        ImGui::EndGroupPanel();

                        form_IconTextTriggerStacks(indicator.iconList.stacksText.trigger);
                    }
                }
                else
                {
                    ImGui::Checkbox("Show Stacks", &indicator.iconList.showStacks);
                }
                ImGui::PopID();
            }

            ImGui::TextDisabled("Icon List");
            ImGui::Separator();
            {
                int idx = 0;
                DeletionQueue queueDelete;

                for (auto& icon : indicator.iconList.list)
                {

                    bool isOpen = ImGui::CollapsingHeader((icon.trigger.effect + "###" + std::to_string(static_cast<int>(icon.id)) + std::to_string(idx)).c_str(), ImGuiTreeNodeFlags_AllowItemOverlap);
                    float buttonSpacing = ImGui::GetItemRectSize().y + ImGui::GetStyle().ItemSpacing.x; /* button width + item spacing */

                    ImGui::SameLine(ImGui::GetContentRegionMax().x - (1 * buttonSpacing));
                    if (ImGui::ButtonCross(("x##" + std::to_string(static_cast<int>(icon.id)) + "_" + std::to_string(idx)).c_str()))
                    {
                        queueDelete.Queue(idx);
                    }
                    
                    ImGui::SameLine(ImGui::GetContentRegionMax().x - (2 * buttonSpacing));
                    if (ImGui::ButtonArrow(("MoveDown##" + std::to_string(static_cast<int>(icon.id)) + "_" + std::to_string(idx)).c_str(), ImGuiDir_Down, idx == indicator.iconList.list.size()-1))
                    {
                        std::swap(indicator.iconList.list[idx], indicator.iconList.list[idx+1]);
                    }

                    ImGui::SameLine(ImGui::GetContentRegionMax().x - (3 * buttonSpacing));
                    if (ImGui::ButtonArrow(("MoveUp##" + std::to_string(static_cast<int>(icon.id)) + "_" + std::to_string(idx)).c_str(), ImGuiDir_Up, idx == 0))
                    {
                        std::swap(indicator.iconList.list[idx], indicator.iconList.list[idx-1]);
                    }

                    if (isOpen)
                    {
                        ImGui::PushID(idx);

                        ImGui::BeginGroupPanel("Icon Properties", ImVec2(ImGui::GetContentRegionMax().x, 0.f));
                        {
                            form_Texture(icon.source, icon.path);
                        }
                        ImGui::EndGroupPanel();

                        ImGui::BeginGroupPanel("Trigger", ImVec2(ImGui::GetContentRegionMax().x, 0.f));
                        {
                            form_Trigger(icon.trigger);
                        }
                        ImGui::EndGroupPanel();

                        ImGui::PopID();
                    }

                    idx++;
                }

                if (ImGui::Button(("Add Icon##" + std::to_string(static_cast<int>(indicator.id)) + "_" + std::to_string(idx)).c_str()))
                {
                    Icon_t temp{};
                    temp.id = g_LayoutEditor.GenerateUID();
                    indicator.iconList.list.push_back(temp);
                }
                
                // Delete Icon
                queueDelete.Apply([&](int idx) {
                    indicator.iconList.list.erase(indicator.iconList.list.begin() + idx);
                });
            }
        }

        if ("Group" == indicator.type)
        {
            ImGui::TextDisabled("Group Properties");
            ImGui::Separator();
            {
                ImGui::Checkbox("Priority Group", &indicator.group.priorityGroup);
                ImGui::SameLine();
                ImGui::TextDisabled("(?)");
                ImGui::TooltipGeneric("Only the first triggered item in the\n" \
                    "group will be rendered.");
            }
            
            ImGui::TextDisabled("Trigger");
            ImGui::Separator();
            {
                form_Trigger(indicator.group.trigger);
            }
        }

        if ("Border" == indicator.type)
        {
            ImGui::TextDisabled("Border Properties");
            ImGui::Separator();
            {
                ImGui::ColorEdit4("Color##BORDER_COLOR", (float*)&indicator.border.color, ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::InputInt("Thickness##BORDER_THICKNESS", &indicator.border.thickness, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue);
            }
            
            ImGui::TextDisabled("Trigger");
            ImGui::Separator();
            {
                form_Trigger(indicator.border.trigger);
            }
        }

        if ("Colour" == indicator.type)
        {
            ImGui::TextDisabled("Color Properties");
            ImGui::Separator();
            {
                ImGui::ColorEdit4("Color##COLOUR_COLOR", (float*)&indicator.colour.color, ImGuiColorEditFlags_AlphaPreviewHalf);
            }
            
            ImGui::TextDisabled("Trigger");
            ImGui::Separator();
            {
                form_Trigger(indicator.colour.trigger);
            }
        }

        if ("Glow" == indicator.type)
        {
            static const char* positionOptions[] {
                "Inner",
                "Outer"
            };

            ImGui::TextDisabled("Glow Properties");
            ImGui::Separator();
            {
                ImGui::ColorEdit4("Color##GLOW_COLOR", (float*)&indicator.glow.color, ImGuiColorEditFlags_AlphaPreviewHalf);
                
                int positionSelection = 0; // Default to "Inner"
                if (indicator.glow.position == "Inner") positionSelection = 0;
                else if (indicator.glow.position == "Outer") positionSelection = 1;

                if (ImGui::Combo("Position##GLOW_POSITION", &positionSelection, positionOptions, IM_ARRAYSIZE(positionOptions)))
                {
                    indicator.glow.position = positionOptions[positionSelection];
                }

                static const char* thicknessOptions[] {
                    "Pixels",
                    "Percentage"
                };

                int thicknessSelection = 0; // Default to "Pixels"
                if (indicator.glow.thicknessType == "Pixels") thicknessSelection = 0;
                else if (indicator.glow.thicknessType == "Percentage") thicknessSelection = 1;

                if (ImGui::Combo("Thickness Type##GLOW_THICKNESS_TYPE", &thicknessSelection, thicknessOptions, IM_ARRAYSIZE(thicknessOptions)))
                {
                    indicator.glow.thicknessType = thicknessOptions[thicknessSelection];
                }

                ImGui::SliderFloat("Thickness##GLOW_THICKNESS", &indicator.glow.thickness, 1.0f, 100.0f, "%.0f");
                ImGui::SliderFloat("Hardness##GLOW_HARDNESS", &indicator.glow.hardness, 0.0f, 1.0f, "%.2f");

                ImGui::Text("Directions");
                ImGui::Checkbox("Top##GLOW_TOP", &indicator.glow.directionTop);
                ImGui::SameLine();
                ImGui::Checkbox("Bottom##GLOW_BOTTOM", &indicator.glow.directionBottom);
                ImGui::SameLine();
                ImGui::Checkbox("Left##GLOW_LEFT", &indicator.glow.directionLeft);
                ImGui::SameLine();
                ImGui::Checkbox("Right##GLOW_RIGHT", &indicator.glow.directionRight);
            }
            
            ImGui::TextDisabled("Trigger");
            ImGui::Separator();
            {
                form_Trigger(indicator.glow.trigger);
            }
        }

        if ("Text" == indicator.type)
        {
            ImGui::TextDisabled("Position");
            ImGui::Separator();
            {
                form_Position(indicator.text.position);
            }

            ImGui::TextDisabled("Text");
            ImGui::Separator();
            {
                form_TextContent(indicator.text.textContent, indicator.text.textCustom);
            }

            ImGui::TextDisabled("Text Style");
            ImGui::Separator();
            {
                form_Font(indicator.text.fontType, indicator.text.font);
                form_FontSize(indicator.text.fontSizeType, indicator.text.fontSize);
                form_FontColour(indicator.text.colorType, indicator.text.color);
                form_FontDecorator(indicator.text.decoratorsType, indicator.text.shadow, indicator.text.shadowColor, indicator.text.outline, indicator.text.outlineColor);
                form_TextLimit(indicator.text.widthType, indicator.text.widthValue);
            }
            
            ImGui::TextDisabled("Trigger");
            ImGui::Separator();
            {
                form_Trigger(indicator.text.trigger);
            }
        }
    }

    void RegisterLayoutEditorViewsDFS(std::vector<TreeNode>& nodes, const std::vector<Indicator_t>& indicators)
    {
        for (const auto& indicator : indicators)
        {
            const TreeNodeUID indicator_id = indicator.id;

            /* Register "indicator" content view */
            g_LayoutEditor.RegisterContentView(indicator_id, [indicator_id]() {
                if (Indicator_t* ind = g_LayoutManager.GetIndicator(indicator_id))
                {
                    ContentViewIndicator(*ind);
                }
            });

            std::vector<TreeNode> children;
            if (indicator.type == "Group")
            {
                RegisterLayoutEditorViewsDFS(children, indicator.group.indicators);
            }

            nodes.push_back({
                indicator.id, 
                indicator.name, 
                indicator.type,
                std::move(children),
                ((indicator.type == "Group") ? TreeNodeType::BRANCH : TreeNodeType::LEAF),
                indicator.enabled
            });
        }
    }

    void RegisterLayoutEditorViews()
    {
        /* Clear previous data */
        g_LayoutEditor.Clear();
        g_LayoutEditor.UpdateTreeViewHeader("Select Item...");
        
        /* Register static content */
        g_LayoutEditor.RegisterContentView(TreeNodeUID::NONE, ContentViewEmpty);

        /* Register dynamic content */
        for (auto& layout_pair : g_LayoutManager.GetAllLayouts())
        {
            const auto layout_path = layout_pair.first;
            std::vector<TreeNode> children;

            /* Register root-level "layout" content view */
            g_LayoutEditor.RegisterContentView(layout_pair.second.id, [layout_path]() {
                auto& layout = g_LayoutManager.GetLayoutFromFilePath(layout_path);
                ContentViewGeneral(layout.name, layout.colors, layout.position, layout.layout);
            });
            
            RegisterLayoutEditorViewsDFS(children, layout_pair.second.indicators);

            /* Add layout to the menu */
            g_LayoutEditor.AppendNode(TreeNodeUID::NONE, layout_pair.second.id, layout_pair.second.name, layout_pair.second.layout.type, TreeNodeType::ROOT, std::move(children));
        }
    }

    void RegisterPresetConfigViews()
    {
        g_PresetConfig.Clear();
        g_PresetConfig.UpdateTreeViewHeader("Select Preset...");

        auto AddPresetItem = [](TreeNodeUID parentId, const std::string& type, const std::string& name, ContentRenderer&& renderer) {
            TreeNodeUID id = g_PresetConfig.GenerateUID();
            g_PresetConfig.AppendNode(parentId, id, name, type, TreeNodeType::LEAF);
            g_PresetConfig.RegisterContentView(id, std::move(renderer));
        };

        TreeNodeUID coloursBranchId = g_PresetConfig.GenerateUID();
        g_PresetConfig.AppendNode(TreeNodeUID::NONE, coloursBranchId, "Colors", "", TreeNodeType::BRANCH);

        AddPresetItem(coloursBranchId, "Colors", "Default", []() {
            ImGui::PushID("Colors/Default");
            {
                ImGui::TextDisabled("Color Properties");
                ImGui::Separator();
                ImGui::ColorEdit4("Background", &(ColourPresets.COLOUR_BACKGROUND.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::ColorEdit4("Background (Unknown)", &(ColourPresets.COLOUR_BACKGROUND_UNKNOWN.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::ColorEdit4("Health (Alive)", &(ColourPresets.COLOUR_HEALTH.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::ColorEdit4("Health (Downed)", &(ColourPresets.COLOUR_HEALTH_DOWNED.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::ColorEdit4("Health (Defeated)", &(ColourPresets.COLOUR_HEALTH_DEFEATED.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::ColorEdit4("Shroud (Necromancer)", &(ColourPresets.COLOUR_SHROUD_NECROMANCER.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::ColorEdit4("Shroud (Specter)", &(ColourPresets.COLOUR_SHROUD_SPECTER.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::ColorEdit4("Barrier", &(ColourPresets.COLOUR_BARRIER.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::ColorEdit4("Hovered", &(ColourPresets.COLOUR_HOVERED.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
            }
            ImGui::PopID();
        });

        AddPresetItem(coloursBranchId, "Colors", "Profession", []() {
            ImGui::PushID("Colors/Profession");
            {
                ImGui::TextDisabled("Color Properties");
                ImGui::Separator();
                ImGui::ColorEdit4("Background", &(ColourPresets.COLOUR_PROF_BACKGROUND.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::ColorEdit4("Background (Unknown)", &(ColourPresets.COLOUR_PROF_BACKGROUND_UNKNOWN.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::ColorEdit4("Health (Elementalist)", &(ColourPresets.COLOUR_PROF_HEALTH_ELEMENTALIST.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::ColorEdit4("Health (Engineer)", &(ColourPresets.COLOUR_PROF_HEALTH_ENGINEER.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::ColorEdit4("Health (Guardian)", &(ColourPresets.COLOUR_PROF_HEALTH_GUARDIAN.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::ColorEdit4("Health (Mesmer)", &(ColourPresets.COLOUR_PROF_HEALTH_MESMER.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::ColorEdit4("Health (Necromancer)", &(ColourPresets.COLOUR_PROF_HEALTH_NECROMANCER.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::ColorEdit4("Health (Ranger)", &(ColourPresets.COLOUR_PROF_HEALTH_RANGER.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::ColorEdit4("Health (Revenant)", &(ColourPresets.COLOUR_PROF_HEALTH_REVENANT.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::ColorEdit4("Health (Thief)", &(ColourPresets.COLOUR_PROF_HEALTH_THIEF.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::ColorEdit4("Health (Warrior)", &(ColourPresets.COLOUR_PROF_HEALTH_WARRIOR.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::ColorEdit4("Health (Downed)", &(ColourPresets.COLOUR_PROF_HEALTH_DOWNED.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::ColorEdit4("Health (Defeated)", &(ColourPresets.COLOUR_PROF_HEALTH_DEFEATED.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::ColorEdit4("Shroud (Necromancer)", &(ColourPresets.COLOUR_PROF_SHROUD_NECROMANCER.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::ColorEdit4("Shroud (Specter)", &(ColourPresets.COLOUR_PROF_SHROUD_SPECTER.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::ColorEdit4("Barrier", &(ColourPresets.COLOUR_PROF_BARRIER.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::ColorEdit4("Hovered", &(ColourPresets.COLOUR_PROF_HOVERED.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
            }
            ImGui::PopID();
        });

        AddPresetItem(coloursBranchId, "Colors", "Heat Map", []() {
            ImGui::PushID("Colors/HeatMap");
            {
                ImGui::TextDisabled("Color Properties");
                ImGui::Separator();
                ImGui::ColorEdit4("Background", &(ColourPresets.COLOUR_HEATMAP_BACKGROUND.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::ColorEdit4("Background (Unknown)", &(ColourPresets.COLOUR_HEATMAP_BACKGROUND_UNKNOWN.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::ColorEdit4("Health (Alive: 100%)", &(ColourPresets.COLOUR_HEATMAP_HEALTH_100.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::ColorEdit4("Health (Alive: 75%)", &(ColourPresets.COLOUR_HEATMAP_HEALTH_75.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::ColorEdit4("Health (Alive: 50%)", &(ColourPresets.COLOUR_HEATMAP_HEALTH_50.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::ColorEdit4("Health (Alive: 25%)", &(ColourPresets.COLOUR_HEATMAP_HEALTH_25.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::ColorEdit4("Health (Alive: 0%)", &(ColourPresets.COLOUR_HEATMAP_HEALTH_0.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::ColorEdit4("Health (Downed)", &(ColourPresets.COLOUR_HEATMAP_HEALTH_DOWNED.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::ColorEdit4("Health (Defeated)", &(ColourPresets.COLOUR_HEATMAP_HEALTH_DEFEATED.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::ColorEdit4("Shroud (Necromancer)", &(ColourPresets.COLOUR_HEATMAP_SHROUD_NECROMANCER.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::ColorEdit4("Shroud (Specter)", &(ColourPresets.COLOUR_HEATMAP_SHROUD_SPECTER.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::ColorEdit4("Barrier", &(ColourPresets.COLOUR_HEATMAP_BARRIER.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::ColorEdit4("Hovered", &(ColourPresets.COLOUR_HEATMAP_HOVERED.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
            }
            ImGui::PopID();
        });

        TreeNodeUID frameStatesBranchId = g_PresetConfig.GenerateUID();
        g_PresetConfig.AppendNode(TreeNodeUID::NONE, frameStatesBranchId, "Frame States", "", TreeNodeType::BRANCH);

        auto AddFrameStateItem = [&](const std::string& name, FrameStatePreset_t& preset) {
            AddPresetItem(frameStatesBranchId, "Frame States", name, [name, &preset]() {
                ImGui::PushID(("FrameStates/" + name).c_str());
                {
                    ImGui::TextDisabled("Border Override");
                    ImGui::Separator();
                    ImGui::Checkbox("Enable Border", &preset.borderOverride);
                    if (preset.borderOverride)
                    {
                        ImGui::ColorEdit4("Color##BORDER_COLOR", (float*)&preset.border.color, ImGuiColorEditFlags_AlphaPreviewHalf);
                        ImGui::InputInt("Thickness##BORDER_THICKNESS", &preset.border.thickness, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue);
                    }

                    ImGui::TextDisabled("Overlay");
                    ImGui::Separator();
                    ImGui::Checkbox("Enable Overlay", &preset.overlayEnabled);
                    if (preset.overlayEnabled)
                    {
                        ImGui::ColorEdit4("Color##OVERLAY_COLOR", (float*)&preset.overlayColor, ImGuiColorEditFlags_AlphaPreviewHalf);
                    }

                    ImGui::TextDisabled("Glow Effect");
                    ImGui::Separator();
                    ImGui::Checkbox("Enable Glow", &preset.glowEnabled);
                    if (preset.glowEnabled)
                    {
                        ImGui::ColorEdit4("Color##GLOW_COLOR", (float*)&preset.glow.color, ImGuiColorEditFlags_AlphaPreviewHalf);
                        
                        static const char* positionOptions[] {
                            "Inner",
                            "Outer"
                        };
                        
                        int positionSelection = 0; // Default to "Inner"
                        if (preset.glow.position == "Inner") positionSelection = 0;
                        else if (preset.glow.position == "Outer") positionSelection = 1;

                        if (ImGui::Combo("Position##GLOW_POSITION", &positionSelection, positionOptions, IM_ARRAYSIZE(positionOptions)))
                        {
                            preset.glow.position = positionOptions[positionSelection];
                        }
                        
                        ImGui::Checkbox("Top", &preset.glow.directionTop); ImGui::SameLine();
                        ImGui::Checkbox("Bottom", &preset.glow.directionBottom); ImGui::SameLine();
                        ImGui::Checkbox("Left", &preset.glow.directionLeft); ImGui::SameLine();
                        ImGui::Checkbox("Right", &preset.glow.directionRight);
                        
                        static const char* thicknessOptions[] {
                            "Pixels",
                            "Percentage"
                        };

                        int thicknessSelection = 0; // Default to "Pixels"
                        if (preset.glow.thicknessType == "Pixels") thicknessSelection = 0;
                        else if (preset.glow.thicknessType == "Percentage") thicknessSelection = 1;

                        if (ImGui::Combo("Thickness Type##GLOW_THICKNESS_TYPE", &thicknessSelection, thicknessOptions, IM_ARRAYSIZE(thicknessOptions)))
                        {
                            preset.glow.thicknessType = thicknessOptions[thicknessSelection];
                        }

                        ImGui::SliderFloat("Thickness##GLOW_THICKNESS", &preset.glow.thickness, 0.0f, 100.0f, "%.0f");
                        ImGui::SliderFloat("Hardness##GLOW_HARDNESS", &preset.glow.hardness, 0.0f, 1.0f, "%.2f");
                    }
                }
                ImGui::PopID();
            });
        };

        AddFrameStateItem("Hovered", FrameStatePresets.hovered);
        AddFrameStateItem("Selected", FrameStatePresets.selected);
        AddFrameStateItem("Self", FrameStatePresets.self);
        AddFrameStateItem("Commander", FrameStatePresets.commander);

        TreeNodeUID borderBranchId = g_PresetConfig.GenerateUID();
        g_PresetConfig.AppendNode(TreeNodeUID::NONE, borderBranchId, "Borders", "", TreeNodeType::BRANCH);

        AddPresetItem(borderBranchId, "Borders", "Default", []() {
            ImGui::PushID("Borders/Default");
            {
                ImGui::TextDisabled("Border Properties");
                ImGui::Separator();
                ImGui::ColorEdit4("Default", &(BorderPresets.COLOUR_BORDER.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::BeginDisabled();
                ImGui::ColorEdit4("Hovered", &(BorderPresets.COLOUR_BORDER_HOVERED.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::ColorEdit4("Selected", &(BorderPresets.COLOUR_BORDER_SELECTED.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::ColorEdit4("Self", &(BorderPresets.COLOUR_BORDER_SELF.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::EndDisabled();
            }
            ImGui::PopID();
        });

        TreeNodeUID textBranchId = g_PresetConfig.GenerateUID();
        g_PresetConfig.AppendNode(TreeNodeUID::NONE, textBranchId, "Text", "", TreeNodeType::BRANCH);

        AddPresetItem(textBranchId, "Text", "General", []() {
            ImGui::PushID("Text/General");
            {
                ImGui::TextDisabled("Text Style");
                ImGui::Separator();
                form_FontPreset(ConfigText.fontSource, ConfigText.font);
                form_FontSizePreset(ConfigText.fontSizeSource, ConfigText.fontSize);
                ImGui::ColorEdit4("Color##TEXT_COLOR", (float*)&ConfigText.color, ImGuiColorEditFlags_AlphaPreviewHalf);
                form_FontDecoratorPreset(ConfigText.shadow, ConfigText.shadowColor, ConfigText.outline, ConfigText.outlineColor);
            }
            ImGui::PopID();
        });

        AddPresetItem(textBranchId, "Text", "Icon (Duration)", []() {
            ImGui::PushID("Text/Icon/Duration");
            {
                ImGui::TextDisabled("Position");
                ImGui::Separator();
                form_Position(ConfigIconDuration.position);

                ImGui::TextDisabled("Text");
                ImGui::Separator();
                std::string dummySource;
                form_IconTextFormatDuration(dummySource, ConfigIconDuration.textFormatPrecision, false);

                ImGui::TextDisabled("Text Style");
                ImGui::Separator();
                form_FontPreset(ConfigIconDuration.textStyle.fontSource, ConfigIconDuration.textStyle.font);
                form_FontSizePreset(ConfigIconDuration.textStyle.fontSizeSource, ConfigIconDuration.textStyle.fontSize);
                ImGui::ColorEdit4("Color##ICON_DURATION_COLOR", (float*)&ConfigIconDuration.textStyle.color, ImGuiColorEditFlags_AlphaPreviewHalf);
                form_FontDecoratorPreset(ConfigIconDuration.textStyle.shadow, ConfigIconDuration.textStyle.shadowColor, ConfigIconDuration.textStyle.outline, ConfigIconDuration.textStyle.outlineColor);

                ImGui::TextDisabled("Trigger");
                ImGui::Separator();
                form_IconTextTriggerDuration(ConfigIconDuration.trigger, false);
            }
            ImGui::PopID();
        });

        AddPresetItem(textBranchId, "Text", "Icon (Stacks)", []() {
            ImGui::PushID("Text/Icon/Stacks");
            {
                ImGui::TextDisabled("Position");
                ImGui::Separator();
                form_Position(ConfigIconStacks.position);

                ImGui::TextDisabled("Text Style");
                ImGui::Separator();
                form_FontPreset(ConfigIconStacks.textStyle.fontSource, ConfigIconStacks.textStyle.font);
                form_FontSizePreset(ConfigIconStacks.textStyle.fontSizeSource, ConfigIconStacks.textStyle.fontSize);
                ImGui::ColorEdit4("Color##ICON_STACKS_COLOR", (float*)&ConfigIconStacks.textStyle.color, ImGuiColorEditFlags_AlphaPreviewHalf);
                form_FontDecoratorPreset(ConfigIconStacks.textStyle.shadow, ConfigIconStacks.textStyle.shadowColor, ConfigIconStacks.textStyle.outline, ConfigIconStacks.textStyle.outlineColor);

                ImGui::TextDisabled("Trigger");
                ImGui::Separator();
                form_IconTextTriggerStacks(ConfigIconStacks.trigger, false);
            }
            ImGui::PopID();
        });
    }

} // namespace Addon

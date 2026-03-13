#include "addon.h"

#include <functional>
#include <map>
#include <string>
#include <sstream>
#include <queue>

#include "interface/vital_signs_interface.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui_extensions.h"
#include "imgui_memory_view.h"

#include "forms/forms.h"
#include "imgui_tree_view.h"
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
    }

    void Render()
    {
        if (isPreviewModeActive || !NexusLink->IsGameplay || !VitalsData->isAvailable())
        {
            isPreviewModeActive = false;
            return;
        }

        const VitalSignsData::E_GROUP_TYPE groupType = VitalsData->getGroupType();
        std::string activeLayoutName = ConfigGeneral.soloLayout;
        LayoutConfig_t* activeLayout = nullptr;

        switch (groupType)
        {
            case VitalSignsData::E_GROUP_PARTY:
                activeLayoutName = ConfigGeneral.partyLayout;
                break;
            case VitalSignsData::E_GROUP_SQUAD_10:
                activeLayoutName = ConfigGeneral.raidLayout;
                break;
            case VitalSignsData::E_GROUP_SQUAD_50:
                activeLayoutName = ConfigGeneral.squadLayout;
                break;
            case VitalSignsData::E_GROUP_NONE:
            default:
                activeLayoutName = ConfigGeneral.soloLayout;
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
            if (UI::Grid::BeginGridMenu("VitalSigns##Grid", *activeLayout, ColourPresets, BorderPresets, true /** TODO: isInCombat() */))
            {
                for (auto &user : VitalsData->getUsers())
                {
                    auto userData = VitalsData->getUserData(user);
    
                    if (UI::Grid::GridMenuItem(userData))
                    {
                        VitalsData->setLockedSelection(user);
                    }
                }
    
                UI::Grid::EndGridMenu();
            }
        }
        else if ("Radial" == activeLayout->layout.type)
        {
            if (UI::Radial::BeginRadialMenu("VitalSigns##Radial", activeLayout->position, activeLayout->layout, activeLayout->colors, ColourPresets, isRadialMenuActive))
            {
                for (auto &user : VitalsData->getUsers())
                {
                    auto userData = VitalsData->getUserData(user);

                    if (UI::Radial::RadialMenuItem(userData))
                    {
                        VitalsData->setLockedSelection(user);
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
            ImGui::BeginDisabled();            
            ImGui::Checkbox("Hide native frames", &ConfigGeneral.isHiddenNativeParty);
            ImGui::EndDisabled();
        }
        ImGui::PopID();
        
        ImGui::PushID("RaidFrames");
        {
            ImGui::TextDisabled("Raid Frames (10 Players)");
            ImGui::Separator();
            form_SelectLayout(layoutNames, ConfigGeneral.raidLayout);
            ImGui::BeginDisabled();            
            ImGui::Checkbox("Hide native frames", &ConfigGeneral.isHiddenNativeRaid);
            ImGui::EndDisabled();
        }
        ImGui::PopID();

        ImGui::PushID("SquadFrames");
        {
            ImGui::TextDisabled("Squad Frames (50 Players)");
            ImGui::Separator();
            form_SelectLayout(layoutNames, ConfigGeneral.squadLayout);
            ImGui::BeginDisabled();            
            ImGui::Checkbox("Hide native frames", &ConfigGeneral.isHiddenNativeSquad);
            ImGui::EndDisabled();
        }
        ImGui::PopID();
    }

    void OptionsLayoutEditor()
    {
        static bool isInitialised = false;

        if (!isInitialised)
        {
            RegisterLayoutEditorViews();
            isInitialised = true;
        }

        /* Render Layout Editor */
        g_LayoutEditor.RenderTreeView(
            [](TreeNodeUID id) { return g_LayoutManager.Delete(id) || g_LayoutManager.DeleteIndicator(id); },
            [](TreeNodeUID id, const std::string& name, const std::string& type) { g_LayoutManager.AddIndicator(id, name, type); },
            [](TreeNodeUID id, size_t oldIdx, size_t newIdx) { g_LayoutManager.ReorderIndicators(id, oldIdx, newIdx); }
        );
        g_LayoutEditor.RenderContentView();

        /* Render Layout Preview */
        if (g_LayoutEditor.GetActiveNode() != TreeNodeUID::NONE)
        {
            isPreviewModeActive = true;

            if (LayoutConfig_t* layout = g_LayoutManager.GetLayoutFromNodeID(g_LayoutEditor.GetActiveNode()))
            {
                layout->previewNodeId = g_LayoutEditor.GetActiveNode();
                
                if (UI::Grid::BeginGridMenu("Preview##Grid", *layout, ColourPresets, BorderPresets, true))
                {
                    VitalSignsData::Effects_t dummyEffects{};
                    auto AddPreviewItem = [&](const char* name, VitalSignsData::EProfession prof, VitalSignsData::ESpecialisation spec, float hp, VitalSignsData::E_HEALTH_TYPE hpType, float barrier) {
                        VitalSignsData::UserData_t user;
                        user.CharacterName = name;
                        user.Profession = prof;
                        user.Specialisation = spec;
                        user.HealthType = hpType;
                        user.Health = VitalSignsData::Resource_t(hp, 1.0f);
                        user.Barrier = VitalSignsData::Resource_t(barrier, 1.0f);
                        user.Shroud = VitalSignsData::Resource_t(hp, 1.0f);
                        user.Effects = dummyEffects;
                        UI::Grid::GridMenuItem(user);
                    };

                    AddPreviewItem("Preview 1", VitalSignsData::EProfession::Elementalist, VitalSignsData::ESpecialisation::None, 1.0f, VitalSignsData::E_HEALTH_ALIVE, 0.0f);
                    AddPreviewItem("Preview 2", VitalSignsData::EProfession::Engineer, VitalSignsData::ESpecialisation::None, 0.75f, VitalSignsData::E_HEALTH_ALIVE, 0.5f);
                    AddPreviewItem("Preview 3", VitalSignsData::EProfession::Guardian, VitalSignsData::ESpecialisation::None, 0.5f, VitalSignsData::E_HEALTH_ALIVE, 0.25f);
                    AddPreviewItem("Preview 4", VitalSignsData::EProfession::Mesmer, VitalSignsData::ESpecialisation::None, 0.25f, VitalSignsData::E_HEALTH_ALIVE, 0.0f);
                    AddPreviewItem("Preview 5", VitalSignsData::EProfession::Necromancer, VitalSignsData::ESpecialisation::None, 1.0f, VitalSignsData::E_HEALTH_ALIVE, 0.5f);
                    AddPreviewItem("Preview 6", VitalSignsData::EProfession::Necromancer, VitalSignsData::ESpecialisation::None, 0.75f, VitalSignsData::E_HEALTH_SHROUD_NECROMANCER, 0.25f);
                    AddPreviewItem("Preview 7", VitalSignsData::EProfession::Ranger, VitalSignsData::ESpecialisation::None, 0.75f, VitalSignsData::E_HEALTH_ALIVE, 0.0f);
                    AddPreviewItem("Preview 8", VitalSignsData::EProfession::Revenant, VitalSignsData::ESpecialisation::None, 0.5f, VitalSignsData::E_HEALTH_ALIVE, 0.0f);
                    AddPreviewItem("Preview 9", VitalSignsData::EProfession::Thief, VitalSignsData::ESpecialisation::None, 0.5f, VitalSignsData::E_HEALTH_ALIVE, 0.25f);
                    AddPreviewItem("Preview 10", VitalSignsData::EProfession::Warrior, VitalSignsData::ESpecialisation::None, 0.75f, VitalSignsData::E_HEALTH_ALIVE, 0.25f);
                    AddPreviewItem("Preview 11", VitalSignsData::EProfession::Thief, VitalSignsData::ESpecialisation::ThiefSpecter, 0.5f, VitalSignsData::E_HEALTH_ALIVE, 0.25f);
                    AddPreviewItem("Preview 12", VitalSignsData::EProfession::Thief, VitalSignsData::ESpecialisation::ThiefSpecter, 0.75f, VitalSignsData::E_HEALTH_SHROUD_SPECTER, 0.25f);
                    AddPreviewItem("Preview 13", VitalSignsData::EProfession::Thief, VitalSignsData::ESpecialisation::ThiefSpecter, 0.5f, VitalSignsData::E_HEALTH_DOWNED, 0.25f);
                    AddPreviewItem("Preview 14", VitalSignsData::EProfession::Thief, VitalSignsData::ESpecialisation::ThiefSpecter, 0.25f, VitalSignsData::E_HEALTH_DEFEATED, 0.0f);
                    AddPreviewItem("Preview 15", VitalSignsData::EProfession::Thief, VitalSignsData::ESpecialisation::ThiefSpecter, 0.75f, VitalSignsData::E_HEALTH_ALIVE, 0.25f);
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

        g_PresetConfig.RenderTreeView(nullptr, nullptr, nullptr);
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
        ImGui::TextWrappedDisabled("Click an item in the navigation menu to configure it, or press the button below to create a new layout.");

        ImGui::NewLine();

        if (ImGui::Button("Create a new layout"))
        {
            ImGui::OpenPopup("Create a new layout");
        }

        static char inputBuff_Name[MAX_PATH] = "";
        static std::string inputStr_Type = "Grid";
        static bool createFromTemplate = false;

        if (ImGui::BeginPopupModal("Create a new layout", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
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
    }

    void ContentViewGeneral(std::string& name, std::string& colors, Position_t& position, Layout_t& layout)
    {
        static const char* colorsOptions[] {
            "Generic Color Palette",
            "Role Color Palette",
            "Profession Color Palette",
            "Party Color Palette",
        };
        static const char* layoutOptions[] {
            "Radial",
            "Grid"
        };
        static const char* visibilityOptions[] {
            "Always visible",
            "Visible while in combat",
            "Visible while activated"
        };
        static const char* cellDirectionOptions[] {
            "Left-to-right",
            "Top-to-bottom",
            "Right-to-left",
            "Bottom-to-top"
        };

        static char inputBuff[MAX_PATH];

        static int layoutSelection = 0; // Default to "Radial"
        if (layout.type == "Radial") layoutSelection = 0;
        else if (layout.type == "Grid") layoutSelection = 1;
        
        ImGui::TextDisabled("General");
        ImGui::Separator();
        {
            memset(inputBuff, 0, MAX_PATH);
            strcpy_s(inputBuff, MAX_PATH, name.c_str());

            if (ImGui::InputText("Name", inputBuff, IM_ARRAYSIZE(inputBuff)))
            {
                name = inputBuff;
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
            if (colors == "Generic") colorsSelection = 0;
            else if (colors == "Role") colorsSelection = 1;
            else if (colors == "Profession") colorsSelection = 1;
            else if (colors == "Party") colorsSelection = 3;

            if (ImGui::Combo("Color Palette", &colorsSelection, colorsOptions, IM_ARRAYSIZE(colorsOptions)))
            {
                if (colorsSelection == 0) colors = "Generic";
                else if (colorsSelection == 1) colors = "Profession";
            }
        }

        ImGui::TextDisabled("Layout");
        ImGui::Separator();
        {
            /* Radial*/
            if (0U == layoutSelection)
            {
                ImGui::InputFloat("Radius (Inner)##RADIUS_MIN", &layout.radial.sectorRadiusInner, 5.f, 10.f, "%.2f");
                ImGui::InputFloat("Radius (Outer)##RADIUS_MAX", &layout.radial.sectorRadiusOuter, 5.f, 10.f, "%.2f");
                ImGui::SliderInt("Items (Min)##ITEMS_MIN", &layout.radial.sectorCountMin, 2, 10);
                ImGui::SliderInt("Items (Max)##ITEMS_MAX", &layout.radial.sectorCountMax, 4, UI::SQUAD_MEMBER_LIMIT);
                if (ImGui::InputInt("Item Spacing##ITEM_INNER_SPACING", &layout.itemSpacing, 1, 2))
                {
                    const float MAX_SPACING = (layout.radial.sectorRadiusOuter - layout.radial.sectorRadiusInner) / 2;
                    if (layout.itemSpacing > MAX_SPACING)
                    {
                        layout.itemSpacing = MAX_SPACING;
                    }
                    else if (layout.itemSpacing < 0)
                    {
                        layout.itemSpacing = 0;
                    }
                }
                if (ImGui::InputInt("Item Border##ITEM_BORDER", &layout.itemBorder, 1, 2))
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
            
            /* Grid */
            if (1U == layoutSelection)
            {
                form_Direction(layout.grid.cellDirection);

                std::string label = ((layout.grid.cellDirection == "Top-to-bottom" || layout.grid.cellDirection == "Bottom-to-top") ? "Row" : "Column");
                if (ImGui::SliderInt(std::string("Max Cells Per " + label + "##CELL_DIRECTION_MAX").c_str(), &layout.grid.cellDirectionMax, 1, UI::SQUAD_MEMBER_LIMIT))
                {
                    if (layout.grid.cellDirectionMax < 1)
                    {
                        layout.grid.cellDirectionMax = 1;
                    }
                }

                if (ImGui::SliderInt("Max Cells##CELL_MAX", &layout.grid.cellMax, 1, UI::SQUAD_MEMBER_LIMIT))
                {
                    if (layout.grid.cellMax < 1)
                    {
                        layout.grid.cellMax = 1;
                    }
                    if (layout.grid.cellMax > UI::SQUAD_MEMBER_LIMIT)
                    {
                        layout.grid.cellMax = UI::SQUAD_MEMBER_LIMIT;
                    }
                }
                
                if (ImGui::InputInt("Cell Width##CELL_WIDTH", &layout.grid.cellWidth, 1))
                {
                    if (layout.grid.cellWidth < 0)
                    {
                        layout.grid.cellWidth = 0;
                    }
                }
                if (ImGui::InputInt("Cell Height##CELL_HEIGHT", &layout.grid.cellHeight, 1))
                {
                    if (layout.grid.cellHeight < 0)
                    {
                        layout.grid.cellHeight = 0;
                    }
                }
                if (ImGui::InputInt("Cell Rounding##CELL_ROUNDING", &layout.grid.cellRounding, 1))
                {
                    if (layout.grid.cellRounding < 0)
                    {
                        layout.grid.cellRounding = 0;
                    }
                }
                if (ImGui::InputInt("Cell Border##CELL_BORDER", &layout.itemBorder, 1))
                {
                    if (layout.itemBorder < 0)
                    {
                        layout.itemBorder = 0;
                    }
                }
                if (ImGui::InputInt("Cell Spacing##CELL_SPACING", &layout.itemSpacing, 1))
                {
                    if (layout.itemSpacing < -layout.itemBorder)
                    {
                        layout.itemSpacing = -layout.itemBorder;
                    }
                }
            }
        }

        ImGui::TextDisabled("Behaviour");
        ImGui::Separator();
        {
            ImGui::Combo("Visibility##VISIBILITY", &layout.visibility, visibilityOptions, IM_ARRAYSIZE(visibilityOptions));
            if ((0 == layout.visibility) || (1 == layout.visibility))
            {
                ImGui::SliderFloat("Opacity (Inactive)##INACTIVE_OPACITY", &layout.inactiveOpacity, 0.f, 1.f);
            }
        }
    }

    void ContentViewIndicator(Indicator_t& indicator)
    {
        ImGui::TextDisabled("General");
        ImGui::Separator();
        {
            static char inputBuff_Name[MAX_PATH];
            memset(inputBuff_Name, 0, MAX_PATH);
            strcpy_s(inputBuff_Name, MAX_PATH, indicator.name.c_str());

            ImGui::Checkbox("Enabled", &indicator.enabled);
            if (ImGui::InputText("Name", inputBuff_Name, IM_ARRAYSIZE(inputBuff_Name)))
            {
                indicator.name = inputBuff_Name;
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
                ImGui::Checkbox("Show Duration", &indicator.iconSingle.showDuration);
                ImGui::Checkbox("Show Stacks", &indicator.iconSingle.showStacks);
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
                ImGui::Checkbox("Show Duration", &indicator.iconList.showDuration);
                ImGui::Checkbox("Show Stacks", &indicator.iconList.showStacks);
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
                        ImGui::TextDisabled("Icon Properties");
                        ImGui::Separator();
                        {
                            form_Texture(icon.source, icon.path);
                        }

                        ImGui::TextDisabled("Trigger");
                        ImGui::Separator();
                        {
                            form_Trigger(icon.trigger);
                        }
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
                ImGui::InputInt("Thickness##BORDER_THICKNESS", &indicator.border.thickness);
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

        if ("Highlight" == indicator.type)
        {
            static const char* directionOptions[] {
                "Top",
                "Bottom",
                "Left",
                "Right"
            };

            ImGui::TextDisabled("Highlight Properties");
            ImGui::Separator();
            {
                ImGui::ColorEdit4("Color##HIGHLIGHT_COLOR", (float*)&indicator.highlight.color, ImGuiColorEditFlags_AlphaPreviewHalf);
                
                int directionSelection = 1; // Default to "Bottom"
                if (indicator.highlight.position == "Top") directionSelection = 0;
                else if (indicator.highlight.position == "Bottom") directionSelection = 1;
                else if (indicator.highlight.position == "Left") directionSelection = 2;
                else if (indicator.highlight.position == "Right") directionSelection = 3;

                if (ImGui::Combo("Position##HIGHLIGHT_POSITION", &directionSelection, directionOptions, IM_ARRAYSIZE(directionOptions)))
                {
                    indicator.highlight.position = directionOptions[directionSelection];
                }

                ImGui::SliderFloat("Size (%)##HIGHLIGHT_SIZE", &indicator.highlight.size, 0.0f, 100.0f, "%.0f");
            }
            
            ImGui::TextDisabled("Trigger");
            ImGui::Separator();
            {
                form_Trigger(indicator.highlight.trigger);
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

            ImGui::TextDisabled("Text Properties");
            ImGui::Separator();
            {
                form_Font(indicator.text.fontType, indicator.text.fontPath);
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
                (indicator.type == "Group") ? TreeNodeType::BRANCH : TreeNodeType::LEAF
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

        AddPresetItem(coloursBranchId, "Colors", "Simple", []() {
            ImGui::PushID("Colors/Simple");
            {
                ImGui::TextDisabled("Color Properties");
                ImGui::Separator();
                ImGui::ColorEdit4("Background", &(ColourPresets.COLOUR_BACKGROUND.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
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
                ImGui::ColorEdit4("Elementalist", &(ColourPresets.COLOUR_PROF_HEALTH_ELEMENTALIST.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::ColorEdit4("Engineer", &(ColourPresets.COLOUR_PROF_HEALTH_ENGINEER.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::ColorEdit4("Guardian", &(ColourPresets.COLOUR_PROF_HEALTH_GUARDIAN.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::ColorEdit4("Mesmer", &(ColourPresets.COLOUR_PROF_HEALTH_MESMER.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::ColorEdit4("Necromancer", &(ColourPresets.COLOUR_PROF_HEALTH_NECROMANCER.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::ColorEdit4("Ranger", &(ColourPresets.COLOUR_PROF_HEALTH_RANGER.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::ColorEdit4("Revenant", &(ColourPresets.COLOUR_PROF_HEALTH_REVENANT.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::ColorEdit4("Thief", &(ColourPresets.COLOUR_PROF_HEALTH_THIEF.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::ColorEdit4("Warrior", &(ColourPresets.COLOUR_PROF_HEALTH_WARRIOR.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::ColorEdit4("Barrier", &(ColourPresets.COLOUR_PROF_BARRIER.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
                ImGui::ColorEdit4("Hovered", &(ColourPresets.COLOUR_PROF_HOVERED.Value.x), ImGuiColorEditFlags_AlphaPreviewHalf);
            }
            ImGui::PopID();
        });

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
                ImGui::TextDisabled("Text Properties");
                ImGui::Separator();
                form_FontPreset(ConfigText.fontType, ConfigText.fontPath);
                form_FontSizePreset(ConfigText.fontSizeType, ConfigText.fontSize);
                ImGui::ColorEdit4("Color##TEXT_COLOR", (float*)&ConfigText.color, ImGuiColorEditFlags_AlphaPreviewHalf);
                form_FontDecoratorPreset(ConfigText.shadow, ConfigText.shadowColor, ConfigText.outline, ConfigText.outlineColor);
            }
            ImGui::PopID();
        });

        AddPresetItem(textBranchId, "Text", "Icon (Duration)", []() {
            ImGui::PushID("Text/Icon/Duration");
            {
                ImGui::TextDisabled("Text Properties");
                ImGui::Separator();
                form_FontPreset(ConfigIconDuration.fontType, ConfigIconDuration.fontPath);
                form_FontSizePreset(ConfigIconDuration.fontSizeType, ConfigIconDuration.fontSize);
                ImGui::ColorEdit4("Color##ICON_DURATION_COLOR", (float*)&ConfigIconDuration.color, ImGuiColorEditFlags_AlphaPreviewHalf);
                form_FontDecoratorPreset(ConfigIconDuration.shadow, ConfigIconDuration.shadowColor, ConfigIconDuration.outline, ConfigIconDuration.outlineColor);
                
                ImGui::TextDisabled("Position");
                ImGui::Separator();
                form_Position(ConfigIconDuration.position);
            }
            ImGui::PopID();
        });

        AddPresetItem(textBranchId, "Text", "Icon (Stacks)", []() {
            ImGui::PushID("Text/Icon/Stacks");
            {
                ImGui::TextDisabled("Text Properties");
                ImGui::Separator();
                form_FontPreset(ConfigIconStacks.fontType, ConfigIconStacks.fontPath);
                form_FontSizePreset(ConfigIconStacks.fontSizeType, ConfigIconStacks.fontSize);
                ImGui::ColorEdit4("Color##ICON_STACKS_COLOR", (float*)&ConfigIconStacks.color, ImGuiColorEditFlags_AlphaPreviewHalf);
                form_FontDecoratorPreset(ConfigIconStacks.shadow, ConfigIconStacks.shadowColor, ConfigIconStacks.outline, ConfigIconStacks.outlineColor);
                
                ImGui::TextDisabled("Position");
                ImGui::Separator();
                form_Position(ConfigIconStacks.position);
            }
            ImGui::PopID();
        });
    }

} // namespace Addon

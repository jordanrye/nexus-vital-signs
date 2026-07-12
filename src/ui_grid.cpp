#include "ui_grid.h"

#include <algorithm>
#include <vector>

#include "imgui_extensions.h"

#include "addon.h"
#include "shared.h"
#include "ui_common.h"
#include "utilities.h"

inline ImVec4 _ImVec4(float p)
{
    return ImVec4(p, p, p, p);
}

namespace UI::Grid {

    static struct GridContext_t
    {
        // Configuration
        LayoutConfig_t* layoutConfig;
        std::vector<int>* hiddenSubgroups; /** TODO: Global sorting and filtering */
        ColourPresets_t colourPresets;
        BorderPresets_t borderPresets;

        // Menu state
        bool isActive;
        bool isItemPending = false;
        bool isClose;
        ImVec2 menuPosition;
        int index;
        int indexHovered;
        
        // Item state
        bool isValid[SQUAD_MEMBER_LIMIT];
        VitalSignsDataLink::UserData_t userData[SQUAD_MEMBER_LIMIT];
        VitalSignsDataLink::UserId_t userIdHovered;

        // Checkbox state
        struct CheckboxData_t {
            ImVec2 position;
            int subgroupId;
        };
        std::vector<CheckboxData_t> headerCheckboxes;
    } context;

    struct GridDrawProperties_t 
    {
        ImVec2 position;
        int rows;
        int columns;
    };

    struct CellDrawProperties_t 
    {
        ImVec2 size;
        ImVec4 padding;
        float spacing;
        int rounding;
    };

    struct DrawProperties_t 
    {
        ImVec2 position;
        float width;
        float height;
        int rounding;
        ImDrawCornerFlags roundingCorners;
    };

    bool IsItemHovered(DrawProperties_t& properties)
    {
        bool isHovered = false;

        if (context.isActive)
        {
            /* Get cell region */
            ImVec2 r_min(properties.position);
            ImVec2 r_max(r_min.x + properties.width, r_min.y + properties.height);

            /* Determine hovered status */
            if (ImGui::IsMouseHoveringRect(r_min, r_max, false))
            {
                isHovered = true;
            }
        }

        return isHovered;
    }

    ImVec2 CalcItemPosition(const DrawProperties_t& properties, const ImVec2& itemSize, const std::string& anchor, const Coordinate_t& offset)
    {
        ImVec2 position{};

        if (anchor == "Top-left")
        {
            position.x = properties.position.x + offset.x;
            position.y = properties.position.y + offset.y;
        }
        else if (anchor == "Top-centre")
        {
            position.x = properties.position.x + (properties.width / 2) - (itemSize.x * 0.5f) + offset.x;
            position.y = properties.position.y + offset.y;
        }
        else if (anchor == "Top-right")
        {
            position.x = properties.position.x + properties.width - itemSize.x + offset.x;
            position.y = properties.position.y + offset.y;
        }
        else if (anchor == "Centre-left")
        {
            position.x = properties.position.x + offset.x;
            position.y = properties.position.y + (properties.height / 2) - (itemSize.y * 0.5f) + offset.y;
        }
        else if (anchor == "Centre")
        {
            position.x = properties.position.x + (properties.width / 2) - (itemSize.x * 0.5f) + offset.x;
            position.y = properties.position.y + (properties.height / 2) - (itemSize.y * 0.5f) + offset.y;
        }
        else if (anchor == "Centre-right")
        {
            position.x = properties.position.x + properties.width - itemSize.x + offset.x;
            position.y = properties.position.y + (properties.height / 2) - (itemSize.y * 0.5f) + offset.y;
        }
        else if (anchor == "Bottom-left")
        {
            position.x = properties.position.x + offset.x;
            position.y = properties.position.y + properties.height - itemSize.y + offset.y;
        }
        else if (anchor == "Bottom-centre")
        {
            position.x = properties.position.x + (properties.width / 2) - (itemSize.x * 0.5f) + offset.x;
            position.y = properties.position.y + properties.height - itemSize.y + offset.y;
        }
        else if (anchor == "Bottom-right")
        {
            position.x = properties.position.x + properties.width - itemSize.x + offset.x;
            position.y = properties.position.y + properties.height - itemSize.y + offset.y;
        }

        return position;
    }

    void DrawCell(ImDrawList * const drawList, DrawProperties_t properties, ImColor colour)
    {
        ImVec2 p_min(properties.position.x, properties.position.y);
        ImVec2 p_max(p_min.x + properties.width, p_min.y + properties.height);

        drawList->AddRectFilled(p_min, p_max, colour, properties.rounding, properties.roundingCorners);
    }

    void DrawBorder(ImDrawList * const drawList, DrawProperties_t properties, ImColor colour, float thickness)
    {
        /* Calculate an offset so that the border is positioned inside the cell. */
        float offset = thickness / 2.f;
 
        ImVec2 p_min(properties.position.x + offset, properties.position.y + offset);
        ImVec2 p_max(properties.position.x + properties.width - offset, properties.position.y + properties.height - offset);

        drawList->AddRect(p_min, p_max, colour, properties.rounding, properties.roundingCorners, thickness);
    }

    void DrawIconText(ImDrawList* const drawList, const ImVec2& iconPosition, const ImVec2& iconSize, const char* text, const IconText_t& config, const IconText_t& globalConfig)
    {
        std::string fontFilePath = ""; // Nexus font
        if (config.textStyle.fontSource == "Default font") fontFilePath = globalConfig.textStyle.font;
        else if (config.textStyle.fontSource == "Custom font") fontFilePath = config.textStyle.font;

        float fontSize = ImGui::GetIO().FontDefault->FontSize;
        if (config.textStyle.fontSizeSource == "Default font size") fontSize = globalConfig.textStyle.fontSize;
        else if (config.textStyle.fontSizeSource == "Custom font size") fontSize = config.textStyle.fontSize;

        ImFont* font = utils::font::GetFont(fontFilePath, fontSize);

        if (font)
        {
            ImGui::PushFont(font);
        }

        ImVec2 textSize = ImGui::CalcTextSize(text);

        DrawProperties_t iconProps;
        iconProps.position = iconPosition;
        iconProps.width = iconSize.x;
        iconProps.height = iconSize.y;
        // rounding ignored

        std::string anchor = (config.positionSource == "Custom position") ? config.position.anchor : globalConfig.position.anchor;
        Coordinate_t offset = (config.positionSource == "Custom position") ? config.position.offset : globalConfig.position.offset;

        ImVec2 textPos = CalcItemPosition(iconProps, textSize, anchor, offset);

        ImColor color = (config.textStyle.colorSource == "Custom color") ? config.textStyle.color : globalConfig.textStyle.color;
        bool useShadow = (config.textStyle.decoratorSource == "Custom decorators") ? config.textStyle.shadow : globalConfig.textStyle.shadow;
        ImColor shadowColor = (config.textStyle.decoratorSource == "Custom decorators") ? config.textStyle.shadowColor : globalConfig.textStyle.shadowColor;
        bool useOutline = (config.textStyle.decoratorSource == "Custom decorators") ? config.textStyle.outline : globalConfig.textStyle.outline;
        ImColor outlineColor = (config.textStyle.decoratorSource == "Custom decorators") ? config.textStyle.outlineColor : globalConfig.textStyle.outlineColor;

        if (useShadow) drawList->AddText(font, fontSize, textPos + ImVec2(1, 1), shadowColor, text);
        if (useOutline) {
            drawList->AddText(font, fontSize, textPos + ImVec2(-1, 0), outlineColor, text);
            drawList->AddText(font, fontSize, textPos + ImVec2(1, 0), outlineColor, text);
            drawList->AddText(font, fontSize, textPos + ImVec2(0, -1), outlineColor, text);
            drawList->AddText(font, fontSize, textPos + ImVec2(0, 1), outlineColor, text);
        }
        drawList->AddText(font, fontSize, textPos, color, text);

        if (font)
        {
            ImGui::PopFont();
        }
    }

    void DrawIcon(ImDrawList * const drawList, DrawProperties_t properties, Texture* icon, const Size_t& size, std::string anchor, Coordinate_t offset, bool showDuration, float duration, const IconText_t& durationConfig, bool showStacks, unsigned int stacks, const IconText_t& stacksConfig)
    {
        if (icon != nullptr)
        {
            ImVec2 iconSize = ImVec2((float)size.width, (float)size.height);
            ImVec2 iconPosition = CalcItemPosition(properties, iconSize, anchor, offset);

            drawList->AddImage(icon->Resource, iconPosition, ImVec2(iconPosition.x + iconSize.x, iconPosition.y + iconSize.y));

            bool isDurationVisible = showDuration;
            if (isDurationVisible)
            {
                std::string condition = (durationConfig.trigger.source == "Custom trigger") ? durationConfig.trigger.condition : ConfigIconDuration.trigger.condition;
                float threshold = (durationConfig.trigger.source == "Custom trigger") ? durationConfig.trigger.threshold : ConfigIconDuration.trigger.threshold;

                if (condition == "Duration: Less Than" && (duration / 1000.f) >= threshold)
                {
                    isDurationVisible = false;
                }
                else if (condition == "Duration: More Than" && (duration / 1000.f) <= threshold)
                {
                    isDurationVisible = false;
                }
            }

            if (isDurationVisible)
            {
                int precision = (durationConfig.textFormatSource == "Custom precision") ? durationConfig.textFormatPrecision : ConfigIconDuration.textFormatPrecision;

                char buf[32];
                if (precision <= 0)
                {
                    sprintf_s(buf, "%.0f", std::ceil(duration / 1000.f));
                }
                else
                {
                    char fmt[16];
                    sprintf_s(fmt, "%%.%df", precision);
                    sprintf_s(buf, fmt, (duration / 1000.f));
                }
                DrawIconText(drawList, iconPosition, iconSize, buf, durationConfig, ConfigIconDuration);
            }

            bool isStacksVisible = showStacks && (stacks > 1U);
            if (isStacksVisible)
            {
                std::string condition = (stacksConfig.trigger.source == "Custom trigger") ? stacksConfig.trigger.condition : ConfigIconStacks.trigger.condition;
                float threshold = (stacksConfig.trigger.source == "Custom trigger") ? stacksConfig.trigger.threshold : ConfigIconStacks.trigger.threshold;
                float thresholdMax = (stacksConfig.trigger.source == "Custom trigger") ? stacksConfig.trigger.thresholdMax : ConfigIconStacks.trigger.thresholdMax;

                if (condition == "Stacks: Less Than" && stacks >= threshold)
                {
                    isStacksVisible = false;
                }
                else if (condition == "Stacks: More Than" && stacks <= threshold)
                {
                    isStacksVisible = false;
                }
                else if (condition == "Stacks: Between" && (stacks < threshold || stacks > thresholdMax))
                {
                    isStacksVisible = false;
                }
            }

            if (isStacksVisible)
            {
                char buf[32];
                sprintf_s(buf, "%u", stacks);
                DrawIconText(drawList, iconPosition, iconSize, buf, stacksConfig, ConfigIconStacks);
            }
        }
    }

    static void CalcGridDimensions(int itemCount, const GridProperties_t& gridLayout, int& outRows, int& outColumns)
    {
        int requiredSubgroups = ImMax(1, (itemCount + gridLayout.cellDirectionMax - 1) / gridLayout.cellDirectionMax);

        // Reserve an extra subgroup block if the user is currently holding a valid drag-and-drop player payload
        const ImGuiPayload* payload = ImGui::GetDragDropPayload();
        if (payload && payload->IsDataType("VS_USER_ID") && (requiredSubgroups < gridLayout.rowColMax + 1))
        {
            requiredSubgroups++;
        }

        outRows = 1;
        outColumns = 1;

        // Internal growth (cells)
        if (gridLayout.cellDirection == "Left-to-right" || gridLayout.cellDirection == "Right-to-left") outColumns = gridLayout.cellDirectionMax;
        else outRows = gridLayout.cellDirectionMax;

        // External growth (subgroups)
        if (gridLayout.squadDirection == "Left-to-right" || gridLayout.squadDirection == "Right-to-left") outColumns *= requiredSubgroups;
        else outRows *= requiredSubgroups;
    }

    DrawProperties_t CalcDrawProperties(float width, float height, const CellDrawProperties_t& cellDraw, ImDrawCornerFlags cellRoundingCorners, const GridDrawProperties_t& gridDraw, int index)
    {
        // Layout configuration
        const int cellDirectionMax = context.layoutConfig->layout.grid.cellDirectionMax;
        const std::string& direction = context.layoutConfig->layout.grid.cellDirection;
        const std::string& squadDirection = context.layoutConfig->layout.grid.squadDirection;

        // Target indices
        const int subgroup_index = index / cellDirectionMax;
        const int member_index = index % cellDirectionMax;

        // Determine relative dimensions for the layout block
        int cellCols = (direction == "Left-to-right" || direction == "Right-to-left") ? cellDirectionMax : 1;
        int cellRows = (direction == "Top-to-bottom" || direction == "Bottom-to-top") ? cellDirectionMax : 1;
        
        int squadCols = gridDraw.columns / cellCols;
        int squadRows = gridDraw.rows / cellRows;

        int indexColumn = 0;
        int indexRow = 0;

        // Apply squad offset (external growth)
        // Shifts the entire subgroup relative to previous subgroups based on the squad direction
        if (squadDirection == "Left-to-right") indexColumn += subgroup_index * cellCols;
        else if (squadDirection == "Right-to-left") indexColumn += (squadCols - 1 - subgroup_index) * cellCols;
        else if (squadDirection == "Top-to-bottom") indexRow += subgroup_index * cellRows;
        else if (squadDirection == "Bottom-to-top") indexRow += (squadRows - 1 - subgroup_index) * cellRows;

        // Apply cell offset (internal growth)
        // Shifts the individual member relative to its subgroup origin based on the cell direction
        if (direction == "Left-to-right") indexColumn += member_index;
        else if (direction == "Right-to-left") indexColumn += (cellCols - 1 - member_index);
        else if (direction == "Top-to-bottom") indexRow += member_index;
        else if (direction == "Bottom-to-top") indexRow += (cellRows - 1 - member_index);

        // Calculate the offset based on row/column index and padding/spacing setup
        float offsetRow = indexRow * (cellDraw.size.y + cellDraw.spacing + cellDraw.padding.x);
        float offsetColumn = indexColumn * (cellDraw.size.x + cellDraw.spacing + cellDraw.padding.w) + (indexColumn * cellDraw.padding.y) - (indexColumn * cellDraw.padding.w);
        ImVec2 cellOffset(offsetColumn, offsetRow);

        // Finalise draw properties using the grid's anchor position
        DrawProperties_t properties{};
        properties.position.x = gridDraw.position.x + cellOffset.x + cellDraw.padding.w;
        properties.position.y = gridDraw.position.y + cellOffset.y + cellDraw.padding.x;
        properties.width = width;
        properties.height = height;
        properties.rounding = cellDraw.rounding;
        properties.roundingCorners = cellRoundingCorners;

        return properties;
    }

    bool BeginGridMenu(const char* name, LayoutConfig_t& layout, const ColourPresets_t& colours, const BorderPresets_t& borders, std::vector<int>* hiddenSubgroups, bool isActive)
    {
        bool isOpen = false;

        if (isActive || context.isItemPending)
        {
            context.layoutConfig = &layout;
            context.hiddenSubgroups = hiddenSubgroups;
            context.colourPresets = colours;
            context.borderPresets = borders;
            context.headerCheckboxes.clear();

            context.isActive = isActive;
            context.isClose = false;

            // Cache layout settings locally to keep calculations readable
            const auto& gridLayout = context.layoutConfig->layout.grid;
            const auto& posConfig = context.layoutConfig->position;

            // Calculate maximum rendered bounds (in columns/rows)
            int rows, columns;
            CalcGridDimensions(context.index, gridLayout, rows, columns);

            // Convert abstract rows/cols to absolute pixel dimensions
            float menuWidth = (float)(columns * gridLayout.cellWidth + (columns - 1) * context.layoutConfig->layout.itemSpacing);
            float menuHeight = (float)(rows * gridLayout.cellHeight + (rows - 1) * context.layoutConfig->layout.itemSpacing);
            
            DrawProperties_t displayProps{};
            displayProps.position = ImVec2(0.f, 0.f);
            displayProps.width = ImGui::GetIO().DisplaySize.x;
            displayProps.height = ImGui::GetIO().DisplaySize.y;

            // Determine the ImGui window pivot mapping for correct directional scaling against the anchor
            ImVec2 pivot(0.0f, 0.0f);
            if (posConfig.anchor == "Top-centre")         pivot = ImVec2(0.5f, 0.0f);
            else if (posConfig.anchor == "Top-right")     pivot = ImVec2(1.0f, 0.0f);
            else if (posConfig.anchor == "Centre-left")   pivot = ImVec2(0.0f, 0.5f);
            else if (posConfig.anchor == "Centre")        pivot = ImVec2(0.5f, 0.5f);
            else if (posConfig.anchor == "Centre-right")  pivot = ImVec2(1.0f, 0.5f);
            else if (posConfig.anchor == "Bottom-left")   pivot = ImVec2(0.0f, 1.0f);
            else if (posConfig.anchor == "Bottom-centre") pivot = ImVec2(0.5f, 1.0f);
            else if (posConfig.anchor == "Bottom-right")  pivot = ImVec2(1.0f, 1.0f);

            // Resolve the screen coordinates from the selected anchor strings and user offsets
            ImVec2 screenPos = CalcItemPosition(displayProps, ImVec2(0.0f, 0.0f), posConfig.anchor, posConfig.offset);

            ImGui::SetNextWindowPos(screenPos, ImGuiCond_Always, pivot);
            ImGui::SetNextWindowSize(ImVec2(menuWidth, menuHeight));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

            if (ImGui::Begin(name, nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing))
            {
                context.menuPosition = ImGui::GetWindowPos();

                ImGui::PushID(name); // Popped in `EndGridMenu`

                if (!context.isActive)
                {
                    // Disable interaction for everything within this block
                    ImGui::BeginDisabled();

                    context.isItemPending = false;
                }
                
                context.index = 0;
                memset(context.isValid, 0, sizeof(context.isValid));

                isOpen = true;
            }
            else
            {
                ImGui::End();
            }

            ImGui::PopStyleVar();
        }

        return isOpen;
    }

    static bool IsPreviewActive()
    {
        return (context.layoutConfig->previewNodeId != TreeNodeUID::NONE);
    }

    static bool IsTriggerMet(const Trigger_t& trigger, VitalSignsDataLink::UserData_t& userData, bool isPreview = false)
    {
        if (isPreview)
        {
            return true;
        }

        if (IsPreviewActive())
        {
            return false;
        }

        if (trigger.category == "<Inherit From Parent>")
        {
            return true;
        }

        if ((trigger.category == "Professions") && (trigger.effect == VitalSignsDataLink::getProfessionString(userData.Profession, userData.Specialisation)))
        {
            return true;
        }

        if (trigger.category == "Health")
        {
            bool statusMatch = false;
            if (trigger.effect == "Alive" && userData.HealthType == VitalSignsDataLink::E_HEALTH_ALIVE) statusMatch = true;
            else if (trigger.effect == "Downed" && userData.HealthType == VitalSignsDataLink::E_HEALTH_DOWNED) statusMatch = true;
            else if (trigger.effect == "Defeated" && userData.HealthType == VitalSignsDataLink::E_HEALTH_DEFEATED) statusMatch = true;
            else if (trigger.effect == "Shroud (Necromancer)" && userData.HealthType == VitalSignsDataLink::E_HEALTH_SHROUD_NECROMANCER) statusMatch = true;
            else if (trigger.effect == "Shroud (Specter)" && userData.HealthType == VitalSignsDataLink::E_HEALTH_SHROUD_SPECTER) statusMatch = true;

            float healthRatio = userData.GetHealthRatio() * 100.0f; // Scale to match UI values (0-100)

            if ((trigger.condition == "Status: Active" && statusMatch) ||
                (trigger.condition == "Status: Inactive" && !statusMatch) ||
                (trigger.condition == "Threshold: Less Than" && statusMatch && healthRatio < trigger.threshold) ||
                (trigger.condition == "Threshold: More Than" && statusMatch && healthRatio > trigger.threshold) ||
                (trigger.condition == "Threshold: Between" && statusMatch && healthRatio >= trigger.threshold && healthRatio <= trigger.thresholdMax))
            {
                return true;
            }
            return false;
        }

        auto currStacks = userData.Effects[trigger.effect].stacks;
        auto currDuration = userData.Effects[trigger.effect].duration / 1000.0f;

        if ((trigger.condition == "Status: Active" && currStacks >= 1U) ||
            (trigger.condition == "Status: Inactive" && currStacks == 0U) ||
            (trigger.condition == "Duration: Less Than" && currDuration < trigger.threshold) ||
            (trigger.condition == "Duration: More Than" && currDuration > trigger.threshold) ||
            (trigger.condition == "Stacks: Less Than" && currStacks < trigger.threshold) ||
            (trigger.condition == "Stacks: More Than" && currStacks > trigger.threshold) ||
            (trigger.condition == "Stacks: Between" && currStacks >= trigger.threshold && currStacks <= trigger.thresholdMax))
        {
            return true;
        }

        return false;
    }

    static bool HasDescendant(const Indicator_t& indicator, TreeNodeUID targetId)
    {
        if (indicator.id == targetId) return true;
        if (indicator.type == "Group")
        {
            for (const auto& child : indicator.group.indicators)
            {
                if (HasDescendant(child, targetId)) return true;
            }
        }
        else if (indicator.type == "Icon List")
        {
            for (const auto& icon : indicator.iconList.list)
            {
                if (icon.id == targetId) return true;
            }
        }
        return false;
    }

    static bool ProcessIndicatorsDFS(const std::vector<Indicator_t>& indicators, VitalSignsDataLink::UserData_t& userData, bool isPriorityList, bool isParentPreviewed, ImColor* outColor, const Indicator_t** outBorder, std::vector<std::pair<const Indicator_t*, bool>>& outDrawables)
    {
        bool isAnyApplied = false;
        
        for (const auto& indicator : indicators)
        {
            bool isCurrentPreviewed = (indicator.id == context.layoutConfig->previewNodeId);
            bool isPreviewed = isCurrentPreviewed || (indicator.enabled && isParentPreviewed);
            bool isTraversalForced = !isPreviewed && IsPreviewActive() && HasDescendant(indicator, context.layoutConfig->previewNodeId);

            if (indicator.enabled || isPreviewed || isTraversalForced)
            {
                bool isApplied = false;

                if (indicator.type == "Group")
                {
                    if (IsTriggerMet(indicator.group.trigger, userData, isPreviewed) || isTraversalForced)
                    {
                        if (ProcessIndicatorsDFS(indicator.group.indicators, userData, indicator.group.priorityGroup, isPreviewed, outColor, outBorder, outDrawables))
                        {
                            isApplied = true;
                        }
                    }
                }
                else if (indicator.type == "Colour")
                {
                    if (IsTriggerMet(indicator.colour.trigger, userData, isPreviewed) || isTraversalForced)
                    {
                        if (outColor) *outColor = indicator.colour.color;
                        isApplied = true;
                    }
                }
                else if (indicator.type == "Border")
                {
                    if (IsTriggerMet(indicator.border.trigger, userData, isPreviewed) || isTraversalForced)
                    {
                        if (outBorder) *outBorder = &indicator;
                        isApplied = true;
                    }
                }
                else if (indicator.type == "Highlight")
                {
                    if (IsTriggerMet(indicator.highlight.trigger, userData, isPreviewed) || isTraversalForced)
                    {
                        outDrawables.push_back({ &indicator, isPreviewed });
                        isApplied = true;
                    }
                }
                else if (indicator.type == "Icon")
                {
                    if (IsTriggerMet(indicator.iconSingle.icon.trigger, userData, isPreviewed) || isTraversalForced)
                    {
                        outDrawables.push_back({ &indicator, isPreviewed });
                        isApplied = true;
                    }
                }
                else if (indicator.type == "Icon List")
                {
                    for (const auto& icon : indicator.iconList.list)
                    {
                        bool isIconForced = isPreviewed || (icon.id == context.layoutConfig->previewNodeId);

                        if (IsTriggerMet(icon.trigger, userData, isIconForced))
                        {
                            isApplied = true;
                            break;
                        }
                    }

                    if (isApplied) 
                    {
                        outDrawables.push_back({ &indicator, isPreviewed });
                    }
                }
                else if (indicator.type == "Text")
                {
                    if (IsTriggerMet(indicator.text.trigger, userData, isPreviewed) || isTraversalForced)
                    {
                        outDrawables.push_back({ &indicator, isPreviewed });
                        isApplied = true;
                    }
                }
    
                if (isApplied)
                {
                    isAnyApplied = true;

                    if (isPriorityList)
                    {
                        /* Prune branch from DFS traversal. */
                        break;
                    }
                }
            }

        }

        return isAnyApplied;
    }

    static void DrawIndicator(ImDrawList* const drawList, const DrawProperties_t& parentProperties, const DrawProperties_t& contentProperties, const Indicator_t* indicator, VitalSignsDataLink::UserData_t& userData, bool isPreviewed)
    {
        if (indicator->type == "Highlight")
        {
            const float sizeRatio = indicator->highlight.size / 100.0f;
            ImVec2 p_min, p_max;
            ImDrawCornerFlags roundingCorners = ImDrawCornerFlags_None;
            ImU32 colour_start, colour_end;
            
            if (indicator->highlight.position == "Top")
            {
                float height = contentProperties.height * sizeRatio;
                p_min = ImVec2((contentProperties.position.x + 1), (contentProperties.position.y + 1));
                p_max = ImVec2((p_min.x + contentProperties.width - 1), (p_min.y + height - 1));
                roundingCorners = (sizeRatio >= 1.0f) ? ImDrawCornerFlags_All : ImDrawCornerFlags_Top;
                colour_start = indicator->highlight.color;
                colour_end = ImColor(0, 0, 0, 0);
                ImGui::AddRectFilledGradientV(drawList, p_min, p_max, colour_start, colour_end, static_cast<float>(contentProperties.rounding), roundingCorners);
            }
            else if (indicator->highlight.position == "Bottom")
            {
                float height = contentProperties.height * sizeRatio;
                p_min = ImVec2((contentProperties.position.x + 1), (contentProperties.position.y + contentProperties.height - height + 1));
                p_max = ImVec2((p_min.x + contentProperties.width - 1), (p_min.y + height - 1));
                roundingCorners = (sizeRatio >= 1.0f) ? ImDrawCornerFlags_All : ImDrawCornerFlags_Bot;
                colour_start = ImColor(0, 0, 0, 0);
                colour_end = indicator->highlight.color;
                ImGui::AddRectFilledGradientV(drawList, p_min, p_max, colour_start, colour_end, static_cast<float>(contentProperties.rounding), roundingCorners);
            }
            else if (indicator->highlight.position == "Left")
            {
                float width = contentProperties.width * sizeRatio;
                p_min = ImVec2((contentProperties.position.x + 1), (contentProperties.position.y + 1));
                p_max = ImVec2((p_min.x + width - 1), (p_min.y + contentProperties.height - 1));
                roundingCorners = (sizeRatio >= 1.0f) ? ImDrawCornerFlags_All : ImDrawCornerFlags_Left;
                colour_start = indicator->highlight.color;
                colour_end = ImColor(0, 0, 0, 0);
                ImGui::AddRectFilledGradientH(drawList, p_min, p_max, colour_start, colour_end, static_cast<float>(contentProperties.rounding), roundingCorners);
            }
            else if (indicator->highlight.position == "Right")
            {
                float width = contentProperties.width * sizeRatio;
                p_min = ImVec2((contentProperties.position.x + contentProperties.width - width + 1), (contentProperties.position.y + 1));
                p_max = ImVec2((contentProperties.position.x + contentProperties.width - 1), (p_min.y + contentProperties.height - 1));
                roundingCorners = (sizeRatio >= 1.0f) ? ImDrawCornerFlags_All : ImDrawCornerFlags_Right;
                colour_start = ImColor(0, 0, 0, 0);
                colour_end = indicator->highlight.color;
                ImGui::AddRectFilledGradientH(drawList, p_min, p_max, colour_start, colour_end, static_cast<float>(contentProperties.rounding), roundingCorners);
            }
        }
        else if (indicator->type == "Icon")
        {
            Texture* texture = UI::GetOrCreateTexture(indicator->iconSingle.icon.source, indicator->iconSingle.icon.path);
            float duration = userData.Effects[indicator->iconSingle.icon.trigger.effect].duration;
            unsigned int stacks = userData.Effects[indicator->iconSingle.icon.trigger.effect].stacks;
            DrawIcon(drawList, parentProperties, texture, indicator->iconSingle.size, indicator->iconSingle.position.anchor, indicator->iconSingle.position.offset, indicator->iconSingle.showDuration, duration, indicator->iconSingle.durationText, indicator->iconSingle.showStacks, stacks, indicator->iconSingle.stacksText);
        }
        else if (indicator->type == "Icon List")
        {
            ImVec2 iconSpacing(0, 0);

            if (indicator->iconList.listDirection == "Left-to-right")
            {
                iconSpacing.x = indicator->iconList.size.width + indicator->iconList.listSpacing;
            }
            else if (indicator->iconList.listDirection == "Top-to-bottom")
            {
                iconSpacing.y = indicator->iconList.size.height + indicator->iconList.listSpacing;
            }
            else if (indicator->iconList.listDirection == "Right-to-left")
            {
                iconSpacing.x = -(indicator->iconList.size.width + indicator->iconList.listSpacing);
            }
            else if (indicator->iconList.listDirection == "Bottom-to-top")
            {
                iconSpacing.y = -(indicator->iconList.size.height + indicator->iconList.listSpacing);
            }

            int i = 0;
            for (const auto& icon : indicator->iconList.list)
            {
                if (IsTriggerMet(icon.trigger, userData, isPreviewed))
                {
                    Coordinate_t iconOffset((iconSpacing.x * i + indicator->iconList.position.offset.x), (iconSpacing.y * i + indicator->iconList.position.offset.y));
                        
                    Texture* texture = UI::GetOrCreateTexture(icon.source, icon.path);
                    float duration = userData.Effects[icon.trigger.effect].duration;
                    unsigned int stacks = userData.Effects[icon.trigger.effect].stacks;
                    DrawIcon(drawList, parentProperties, texture, indicator->iconList.size, indicator->iconList.position.anchor, iconOffset, indicator->iconList.showDuration, duration, indicator->iconList.durationText, indicator->iconList.showStacks, stacks, indicator->iconList.stacksText);

                    if (indicator->iconList.listLength == "Dynamic")
                    {
                        i++;
                    }
                }

                if (indicator->iconList.listLength == "Static")
                {
                    i++;
                }
            }
        }
        else if (indicator->type == "Text")
        {
            std::string text;
            if (indicator->text.textContent == "Character name")
            {
                text = userData.GetDisplayName();
            } 
            else if (indicator->text.textContent == "Account name")
            {
                text = userData.GetAccountName();
            }
            else if (indicator->text.textContent == "Nickname")
            {
                text = userData.GetNickname();
            }
            else if (indicator->text.textContent == "Health percentage")
            {
                text = std::to_string(max(static_cast<int>(userData.GetHealthRatio() * 100.0f), 0)) + "%";
            } 
            else
            {
                text = indicator->text.textCustom;
            }

            std::string fontFilePath = ""; // Nexus font
            if (indicator->text.fontType == "Default font")
            {
                fontFilePath = ConfigText.font;
            }
            else if (indicator->text.fontType == "Custom font")
            {
                fontFilePath = indicator->text.font;
            }

            float fontSize = ImGui::GetIO().FontDefault->FontSize;
            if (indicator->text.fontSizeType == "Default font size")
            {
                fontSize = ConfigText.fontSize;
            }
            else if (indicator->text.fontSizeType == "Custom font size")
            {
                fontSize = indicator->text.fontSize;
            }

            ImFont* font = utils::font::GetFont(fontFilePath, fontSize);

            if (indicator->text.widthType == "Character limit")
            {
                if (text.length() > (size_t)indicator->text.widthValue)
                {
                    text = text.substr(0, (size_t)indicator->text.widthValue);
                }
            } 
            else if (indicator->text.widthType == "Percentage width")
            {
                float maxWidth = contentProperties.width * (indicator->text.widthValue / 100.0f);
                ImFont* activeFont = font ? font : ImGui::GetFont();
                const char* text_end = text.c_str() + text.length();
                const char* remaining = nullptr;
                activeFont->CalcTextSizeA(fontSize, maxWidth, 0.0f, text.c_str(), text_end, &remaining);

                if (remaining && remaining < text_end)
                {
                    text.resize(remaining - text.c_str());
                }
            }

            if (font) ImGui::PushFont(font);
            ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
            if (font) ImGui::PopFont();

            ImVec2 pos = CalcItemPosition(parentProperties, textSize, indicator->text.position.anchor, indicator->text.position.offset);

            ImColor color = (indicator->text.colorType == "Custom color") ? indicator->text.color : ConfigText.color;
            bool useShadow = (indicator->text.decoratorsType == "Custom decorators") ? indicator->text.shadow : ConfigText.shadow;
            ImColor shadowColor = (indicator->text.decoratorsType == "Custom decorators") ? indicator->text.shadowColor : ConfigText.shadowColor;
            bool useOutline = (indicator->text.decoratorsType == "Custom decorators") ? indicator->text.outline : ConfigText.outline;
            ImColor outlineColor = (indicator->text.decoratorsType == "Custom decorators") ? indicator->text.outlineColor : ConfigText.outlineColor;

            if (useShadow)
            {
                drawList->AddText(font, fontSize, ImVec2(pos.x + 1, pos.y + 1), shadowColor, text.c_str());
            }

            if (useOutline)
            {
                drawList->AddText(font, fontSize, ImVec2(pos.x - 1, pos.y), outlineColor, text.c_str());
                drawList->AddText(font, fontSize, ImVec2(pos.x + 1, pos.y), outlineColor, text.c_str());
                drawList->AddText(font, fontSize, ImVec2(pos.x, pos.y - 1), outlineColor, text.c_str());
                drawList->AddText(font, fontSize, ImVec2(pos.x, pos.y + 1), outlineColor, text.c_str());
            }
                    
            drawList->AddText(font, fontSize, pos, color, text.c_str());
        }
    }

    void EndGridMenu()
    {
        const auto clientId = VitalsData->getClientId();
        
        /// TODO: Encapsulate this function and add additional sort orders (e.g., role).
        /* Sort players in each subgroup by squad rank */
        int currentIdx = 0;
        while (currentIdx < context.index)
        {
            if (!context.isValid[currentIdx])
            {
                currentIdx++;
                continue;
            }

            auto currentSubgroupId = context.userData[currentIdx].SubgroupId;
            
            std::vector<VitalSignsDataLink::UserData_t> validUsers;
            std::vector<int> subgroupIndices;
            
            int scanIdx = currentIdx;
            while (scanIdx < context.index)
            {
                if (context.isValid[scanIdx])
                {
                    if (context.userData[scanIdx].SubgroupId == currentSubgroupId)
                    {
                        validUsers.push_back(context.userData[scanIdx]);
                        subgroupIndices.push_back(scanIdx);
                    }
                    else
                    {
                        break;
                    }
                }
                scanIdx++;
            }

            std::stable_sort(validUsers.begin(), validUsers.end(), [clientId](const VitalSignsDataLink::UserData_t& a, const VitalSignsDataLink::UserData_t& b) {
                auto getPriority = [clientId](const VitalSignsDataLink::UserData_t& user) -> int {
                    if (user.SquadRole == VitalSignsDataLink::ESquadRole::Commander) return 0;
                    if (user.SquadRole == VitalSignsDataLink::ESquadRole::Lieutenant) return 1;
                    if (user.UserId == clientId) return 2;
                    return 3;
                };
                return getPriority(a) < getPriority(b);
            });

            for (size_t i = 0; i < subgroupIndices.size(); i++)
            {
                context.userData[subgroupIndices[i]] = validUsers[i];
            }

            currentIdx = scanIdx;
        }

        /* Grid properties */
        GridDrawProperties_t gridDrawProperties;
        gridDrawProperties.position = context.menuPosition;

        const auto& gridLayout = context.layoutConfig->layout.grid;
        const int cellDirectionMax = gridLayout.cellDirectionMax;

        CalcGridDimensions(context.index, gridLayout, gridDrawProperties.rows, gridDrawProperties.columns);

        /* Frame properties */
        CellDrawProperties_t frameDrawProperties;
        frameDrawProperties.size = ImVec2(
            max(1.0f, static_cast<float>(context.layoutConfig->layout.grid.cellWidth - (2 * context.layoutConfig->layout.itemBorder))), 
            max(1.0f, static_cast<float>(context.layoutConfig->layout.grid.cellHeight - (2 * context.layoutConfig->layout.itemBorder))));
        frameDrawProperties.padding = _ImVec4(static_cast<float>(context.layoutConfig->layout.itemBorder));
        frameDrawProperties.spacing = static_cast<float>(context.layoutConfig->layout.itemSpacing + context.layoutConfig->layout.itemBorder);
        frameDrawProperties.rounding = ImMax(context.layoutConfig->layout.grid.cellRounding, context.layoutConfig->layout.grid.cellRounding - context.layoutConfig->layout.itemBorder);

        /* Frame border properties */
        CellDrawProperties_t borderDrawProperties;
        borderDrawProperties.size = ImVec2(
            static_cast<float>(context.layoutConfig->layout.grid.cellWidth), 
            static_cast<float>(context.layoutConfig->layout.grid.cellHeight));
        borderDrawProperties.padding = _ImVec4(0.f);
        borderDrawProperties.spacing = static_cast<float>(context.layoutConfig->layout.itemSpacing);
        borderDrawProperties.rounding = context.layoutConfig->layout.grid.cellRounding;

        /**
         * @note The state for `indexHovered` is reset in `EndGridMenu` because
         * `GridMenuItem` (which is called prior to `EndGridMenu`) relies on the
         * result from the previous frame.
         **/
        context.indexHovered = -1;

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->PushClipRectFullScreen(); // Required to prevent clipping

        int lastPopulatedGroupIndex = (context.index == 0) ? -1 : ((context.index - 1) / cellDirectionMax);
        lastPopulatedGroupIndex = ImMin(lastPopulatedGroupIndex, context.layoutConfig->layout.grid.rowColMax - 1);

        /* Map the new drop target column */
        int newGroupIndex = lastPopulatedGroupIndex + 1;
        if ((newGroupIndex < context.layoutConfig->layout.grid.rowColMax) && 
            ((newGroupIndex * cellDirectionMax) < SQUAD_MEMBER_LIMIT))
        {
            lastPopulatedGroupIndex = newGroupIndex;
        }
        
        auto getSubgroupId = [&](int idx) -> VitalSignsDataLink::SubgroupId_t {
            return (idx == newGroupIndex) ? static_cast<VitalSignsDataLink::SubgroupId_t>(-1) : context.userData[idx * cellDirectionMax].SubgroupId;
        };

        /* Draw drop targets and highlights */
        int groupIndex = 0;
        ImGui::PushID("DragAndDrop");
        while (groupIndex <= lastPopulatedGroupIndex)
        {
            VitalSignsDataLink::SubgroupId_t droppedSubgroupId = getSubgroupId(groupIndex);
            int startGroupIndex = groupIndex;
            while (groupIndex <= lastPopulatedGroupIndex && getSubgroupId(groupIndex) == droppedSubgroupId)
            {
                groupIndex++;
            }
            int endGroupIndex = groupIndex - 1;

            DrawProperties_t firstCellProps = CalcDrawProperties(borderDrawProperties.size.x, borderDrawProperties.size.y, borderDrawProperties, ImDrawCornerFlags_All, gridDrawProperties, startGroupIndex * cellDirectionMax);
            DrawProperties_t lastCellProps = CalcDrawProperties(borderDrawProperties.size.x, borderDrawProperties.size.y, borderDrawProperties, ImDrawCornerFlags_All, gridDrawProperties, endGroupIndex * cellDirectionMax + cellDirectionMax - 1);
            
            ImVec2 p_min(ImMin(firstCellProps.position.x, lastCellProps.position.x), ImMin(firstCellProps.position.y, lastCellProps.position.y));
            ImVec2 p_max(ImMax(firstCellProps.position.x + firstCellProps.width, lastCellProps.position.x + lastCellProps.width), ImMax(firstCellProps.position.y + firstCellProps.height, lastCellProps.position.y + lastCellProps.height));
            
            /// TODO: Make this more elegant 
            p_min.x -= (context.layoutConfig->layout.itemSpacing / 2.f);
            p_min.y -= (context.layoutConfig->layout.itemSpacing / 2.f);
            p_max.x += (context.layoutConfig->layout.itemSpacing / 2.f);
            p_max.y += (context.layoutConfig->layout.itemSpacing / 2.f);

            ImGui::SetCursorScreenPos(p_min);
            ImGui::PushID(groupIndex);
            ImGui::InvisibleButton("SubgroupTarget", ImVec2(p_max.x - p_min.x, p_max.y - p_min.y));
            ImGui::SetItemAllowOverlap();

            // Squad Manager: Subgroup header
            if (Addon::isSquadManagerActive)
            {
                std::string headerText = (droppedSubgroupId == static_cast<VitalSignsDataLink::SubgroupId_t>(-1)) ? "New Subgroup" : "Subgroup " + std::to_string(droppedSubgroupId + 1);
                
                bool isHidden = false;
                if (context.hiddenSubgroups)
                {
                    isHidden = std::find(context.hiddenSubgroups->begin(), context.hiddenSubgroups->end(), droppedSubgroupId) != context.hiddenSubgroups->end();
                }

                if (isHidden)
                {
                    /// FIXME: Opacity not working for hidden subgroups
                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.4f);
                }

                ImFont* font = nullptr;
                if (ConfigText.fontSource != "Nexus font" && ConfigText.fontSource != "Default font")
                {
                    font = utils::font::GetFont(ConfigText.font, ConfigText.fontSize);
                }

                if (font) ImGui::PushFont(font);
                ImVec2 text_size = ImGui::CalcTextSize(headerText.c_str());

                float frameWidth = (float)context.layoutConfig->layout.grid.cellWidth;
                float frameHeight = (float)context.layoutConfig->layout.grid.cellHeight;
                float spacing = (float)context.layoutConfig->layout.itemSpacing;
                std::string cellDir = context.layoutConfig->layout.grid.cellDirection;

                float headerWidth, headerHeight;
                ImVec2 header_p_min;

                if (cellDir == "Top-to-bottom")
                {
                    headerWidth = frameWidth;
                    headerHeight = text_size.y * 3.0f;
                    header_p_min = ImVec2(firstCellProps.position.x, firstCellProps.position.y - headerHeight - spacing);
                }
                else if (cellDir == "Bottom-to-top")
                {
                    headerWidth = frameWidth;
                    headerHeight = text_size.y * 3.0f;
                    header_p_min = ImVec2(firstCellProps.position.x, firstCellProps.position.y + frameHeight + spacing);
                }
                else if (cellDir == "Left-to-right")
                {
                    headerHeight = frameHeight;
                    headerWidth = text_size.x + 20.0f;
                    header_p_min = ImVec2(firstCellProps.position.x - headerWidth - spacing, firstCellProps.position.y);
                }
                else if (cellDir == "Right-to-left")
                {
                    headerHeight = frameHeight;
                    headerWidth = text_size.x + 20.0f;
                    header_p_min = ImVec2(firstCellProps.position.x + frameWidth + spacing, firstCellProps.position.y);
                }
                else
                {
                    headerWidth = frameWidth;
                    headerHeight = text_size.y * 3.0f;
                    header_p_min = ImVec2(firstCellProps.position.x, firstCellProps.position.y - headerHeight - spacing);
                }

                ImVec2 header_p_max = ImVec2(header_p_min.x + headerWidth, header_p_min.y + headerHeight);

                ImU32 col_solid = ImColor(0, 0, 0, 255);
                ImU32 col_trans = ImColor(0, 0, 0, 0);

                if (cellDir == "Top-to-bottom")
                {
                    ImGui::AddRectFilledGradientV(drawList, header_p_min, header_p_max, col_trans, col_solid, context.layoutConfig->layout.grid.cellRounding, ImDrawCornerFlags_All);
                }
                else if (cellDir == "Bottom-to-top")
                {
                    ImGui::AddRectFilledGradientV(drawList, header_p_min, header_p_max, col_solid, col_trans, context.layoutConfig->layout.grid.cellRounding, ImDrawCornerFlags_All);
                }
                else if (cellDir == "Left-to-right")
                {
                    ImGui::AddRectFilledGradientH(drawList, header_p_min, header_p_max, col_trans, col_solid, context.layoutConfig->layout.grid.cellRounding, ImDrawCornerFlags_All);
                }
                else if (cellDir == "Right-to-left")
                {
                    ImGui::AddRectFilledGradientH(drawList, header_p_min, header_p_max, col_solid, col_trans, context.layoutConfig->layout.grid.cellRounding, ImDrawCornerFlags_All);
                }
                else
                {
                    ImGui::AddRectFilledGradientV(drawList, header_p_min, header_p_max, col_trans, col_solid, context.layoutConfig->layout.grid.cellRounding, ImDrawCornerFlags_All);
                }

                ImVec2 text_pos = ImVec2(header_p_min.x + (headerWidth - text_size.x) * 0.5f, header_p_min.y + (headerHeight - text_size.y) * 0.5f);

                if (ConfigText.shadow)
                {
                    drawList->AddText(font, ConfigText.fontSize, ImVec2(text_pos.x + 1, text_pos.y + 1), ConfigText.shadowColor, headerText.c_str());
                }
                if (ConfigText.outline)
                {
                    drawList->AddText(font, ConfigText.fontSize, ImVec2(text_pos.x - 1, text_pos.y), ConfigText.outlineColor, headerText.c_str());
                    drawList->AddText(font, ConfigText.fontSize, ImVec2(text_pos.x + 1, text_pos.y), ConfigText.outlineColor, headerText.c_str());
                    drawList->AddText(font, ConfigText.fontSize, ImVec2(text_pos.x, text_pos.y - 1), ConfigText.outlineColor, headerText.c_str());
                    drawList->AddText(font, ConfigText.fontSize, ImVec2(text_pos.x, text_pos.y + 1), ConfigText.outlineColor, headerText.c_str());
                }

                drawList->AddText(font, ConfigText.fontSize, text_pos, ConfigText.color, headerText.c_str());
                if (font) ImGui::PopFont();

                if (context.hiddenSubgroups)
                {
                    float checkboxSize = ImGui::GetFrameHeight();
                    ImVec2 checkbox_pos = ImVec2(text_pos.x + text_size.x + 8.0f, text_pos.y + (text_size.y - checkboxSize) * 0.5f);
                    context.headerCheckboxes.push_back({checkbox_pos, droppedSubgroupId});
                }

                if (isHidden)
                {
                    ImGui::PopStyleVar();
                }
            }

            // Squad Manager: Drag-and-drop (drop)
            if (Addon::isSquadManagerActive && ImGui::BeginDragDropTarget())
            {
                drawList->AddRectFilled(p_min, p_max, ImColor(255, 255, 255, 64), context.layoutConfig->layout.grid.cellRounding);

                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("VS_USER_ID", ImGuiDragDropFlags_AcceptNoDrawDefaultRect))
                {
                    if (payload->IsDelivery())
                    {
                        VitalSignsDataLink::UserId_t draggedUserId = *(const VitalSignsDataLink::UserId_t*)payload->Data;
                        VitalsData->setSubgroup(draggedUserId, droppedSubgroupId);
                    }    
                }
                ImGui::EndDragDropTarget();
            }
            ImGui::PopID();
        }
        ImGui::PopID();

        std::vector<std::pair<const Indicator_t*, bool>> drawables;

        /* Draw items */
        for (int i = 0; i < context.index; i++)
        {
            if (!context.isValid[i])
            {
                continue;
            }

            // Skip rendering items outside the bounds
            int current_s = i / context.layoutConfig->layout.grid.cellDirectionMax;
            if (current_s >= context.layoutConfig->layout.grid.rowColMax)
            {
                continue; 
            }

            VitalSignsDataLink::UserData_t& userData = context.userData[i];

            bool isHidden = false;
            if (context.hiddenSubgroups)
            {
                isHidden = std::find(context.hiddenSubgroups->begin(), context.hiddenSubgroups->end(), userData.SubgroupId) != context.hiddenSubgroups->end();
            }

            if (isHidden)
            {
                /// FIXME: Opacity not working for hidden subgroups
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.4f);
            }

            ImGui::PushID(i);
            {
                drawables.clear();

                DrawProperties_t parentProperties = CalcDrawProperties(frameDrawProperties.size.x, frameDrawProperties.size.y, frameDrawProperties, ImDrawCornerFlags_All, gridDrawProperties, i);
    
                /* Invisible button (creates clickable region) */
                ImGui::SetCursorScreenPos(parentProperties.position);
                ImGui::InvisibleButton("", ImVec2(parentProperties.width, parentProperties.height));

                if (ImGui::BeginPopupContextItem())
                {
                    // const auto clientId = VitalsData->getClientId();
                    const auto clientRole = VitalsData->getUserData(clientId).SquadRole;
                    const auto clientMap = VitalsData->getUserData(clientId).MapName;

                    ImGui::TextDisabled(userData.GetAccountName().c_str());
                    ImGui::Text(userData.GetDisplayName().c_str());

                    if (clientId != userData.UserId)
                    {
                        ImGui::Separator();
                        if (ImGui::Button("Whisper")) { VitalsData->whisperUser(userData.UserId); }
                        if (ImGui::Button("Send Mail")) { VitalsData->sendMail(userData.AccountName); }
                        if (userData.IsInInstance)
                        {
                            if (ImGui::Button("Inspect Cosmetics")) { VitalsData->inspectCosmetics(userData.UserId); }
                        }
                        if (clientMap == userData.MapName && !userData.IsInInstance)
                        {
                            if (ImGui::Button("Join in Instance")) { VitalsData->joinUserInstance(userData.UserId); }
                        }

                        ImGui::Separator();
                        if (userData.ContactType != VitalSignsDataLink::EContactType::Friend)
                        {
                            if (ImGui::Button("Add Friend")) { VitalsData->addFriend(userData.UserId); }
                        }
                        else
                        {
                            if (ImGui::Button("Remove Friend")) { VitalsData->removeFriend(userData.UserId); }
                        }
                        if (userData.ContactType != VitalSignsDataLink::EContactType::Blocked)
                        {
                            if (ImGui::Button("Block")) { VitalsData->blockUser(userData.UserId); }
                        }
                        else
                        {
                            if (ImGui::Button("Unblock")) { VitalsData->unblockUser(userData.UserId); }
                        }
                    }

                    if ((VitalsData->getGroupType() == VitalSignsDataLink::E_GROUP_SQUAD_10) ||
                        (VitalsData->getGroupType() == VitalSignsDataLink::E_GROUP_SQUAD_50))
                    {
                        ImGui::Separator();
                        if (clientId != userData.UserId)
                        {
                            if (clientRole == VitalSignsDataLink::ESquadRole::Commander)
                            {
                                if (userData.IsCommanderUnlocked)
                                {
                                    if (ImGui::Button("Appoint Squad Leader")) { VitalsData->setCommander(userData.UserId); }
                                }
                                if (userData.SquadRole == VitalSignsDataLink::ESquadRole::None)
                                {
                                    if (ImGui::Button("Appoint Lieutenant")) { VitalsData->setLieutenant(userData.UserId, true); }
                                }
                                if (userData.SquadRole == VitalSignsDataLink::ESquadRole::Lieutenant)
                                {
                                    if (ImGui::Button("Demote Lieutenant")) { VitalsData->setLieutenant(userData.UserId, false); }
                                }
                            }
                            if (clientRole == VitalSignsDataLink::ESquadRole::Commander || clientRole == VitalSignsDataLink::ESquadRole::Lieutenant)
                            {
                                if (ImGui::Button("Kick from Squad")) { VitalsData->kickUser(userData.UserId); }
                            }
                        }
                        if (ImGui::Button("Leave Squad")) { VitalsData->leaveGroup(); }
                    }

                    if (VitalsData->getGroupType() == VitalSignsDataLink::E_GROUP_PARTY)
                    {
                        ImGui::Separator();
                        if (clientId != userData.UserId)
                        {
                            if (ImGui::Button("Kick from Party")) { VitalsData->kickUser(userData.UserId); }
                        }
                        if (ImGui::Button("Leave Party")) { VitalsData->leaveGroup(); }
                    }

                    if (clientId != userData.UserId)
                    {
                        ImGui::Separator();
                        if (ImGui::Button("Report")) { VitalsData->reportUser(userData.UserId); }
                    }

                    ImGui::EndPopup();
                }

                const bool isHovered = IsItemHovered(parentProperties);
                const ImColor backgroundColour = GetBackgroundColour(context.colourPresets, context.layoutConfig->colors);
                ImColor healthColour = GetHealthColour(context.colourPresets, context.layoutConfig->colors, userData.HealthType, userData.GetHealthRatio(), userData.Profession);
                const ImColor barrierColour = GetBarrierColour(context.colourPresets, context.layoutConfig->colors);
                const Indicator_t* borderStyle = nullptr;
    
                /* Process indicators (returns a list of drawable items) */
                ProcessIndicatorsDFS(context.layoutConfig->indicators, userData, false, false, &healthColour, &borderStyle, drawables);
                if ((context.layoutConfig->previewNodeId != TreeNodeUID::NONE) && (context.layoutConfig->previewNodeId == context.layoutConfig->id))
                {
                    /* Preview mode */
                    ProcessIndicatorsDFS(context.layoutConfig->indicators, userData, false, true, &healthColour, &borderStyle, drawables);
                }
    
                /* Border */
                DrawProperties_t borderProps = CalcDrawProperties(borderDrawProperties.size.x, borderDrawProperties.size.y, borderDrawProperties, ImDrawCornerFlags_All, gridDrawProperties, i);
                float borderThickness = 0.0f;
                if (borderStyle)
                {
                    if (borderStyle->border.thickness >= 1)
                    {
                        borderThickness = (float)borderStyle->border.thickness;
                        DrawBorder(drawList, borderProps, borderStyle->border.color, borderThickness);
                    }
                }
                else if (context.layoutConfig->layout.itemBorder >= 1)
                {
                    borderThickness = (float)context.layoutConfig->layout.itemBorder;
                    DrawBorder(drawList, borderProps, context.borderPresets.COLOUR_BORDER, borderThickness);
                }
    
                /* Content properties */
                DrawProperties_t contentProps = borderProps;
                contentProps.position.x += borderThickness;
                contentProps.position.y += borderThickness;
                contentProps.width -= 2.0f * borderThickness;
                contentProps.height -= 2.0f * borderThickness;
                
                /* Background */
                DrawCell(drawList, contentProps, backgroundColour);
                
                /* Health */
                float health = userData.GetHealthRatio();
                float barrier = userData.GetBarrierRatio();

                if (health > 0.001f)
                {
                    float healthWidth = contentProps.width * health;
                    ImDrawCornerFlags roundingCorners = ((contentProps.width - healthWidth) < (float)contentProps.rounding) ? ImDrawCornerFlags_All : ImDrawCornerFlags_Left;
                    DrawProperties_t properties = contentProps;
                    properties.width = healthWidth;
                    properties.roundingCorners = roundingCorners;
    
                    DrawCell(drawList, properties, healthColour);
    
                    if (isHovered)
                    {
                        DrawCell(drawList, properties, context.colourPresets.COLOUR_HOVERED);
                    }
                }
                
                /* Barrier */
                if (barrier > 0.001f)
                {
                    float barrierWidth = contentProps.width * barrier;
                    DrawProperties_t properties = contentProps;
                    properties.width = barrierWidth;
    
                    if ((barrier + health) > 0.999f)
                    {
                        /* Overflow barrier */
                        properties.position.x += contentProps.width - barrierWidth;
                        properties.roundingCorners = ImDrawCornerFlags_Right;
                        DrawCell(drawList, properties, barrierColour);
                    }
                    else
                    {
                        /* Padding barrier */
                        float healthWidth = contentProps.width * health;
                        ImDrawCornerFlags roundingCorners = ((healthWidth + barrierWidth) < (contentProps.width - (float)contentProps.rounding)) ? ImDrawCornerFlags_None : ImDrawCornerFlags_Right;
                        properties.position.x += healthWidth;
                        properties.roundingCorners = roundingCorners;
                        DrawCell(drawList, properties, barrierColour);
                    }
                }
    
                /* Indicators */
                for (const auto& pair : drawables)
                {
                    DrawIndicator(drawList, parentProperties, contentProps, pair.first, userData, pair.second);
                }

                /* Squad Manager: Drag-and-drop (drag) */
                if (Addon::isSquadManagerActive && ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
                {
                    ImGui::SetDragDropPayload("VS_USER_ID", &userData.UserId, sizeof(userData.UserId));
                    ImGui::Text("%s", userData.GetDisplayName().c_str());
                    ImGui::EndDragDropSource();
                }
    
                if (isHovered)
                {
                    context.indexHovered = i;
                    context.userIdHovered = userData.UserId;
    
                    context.isItemPending = true;
                }
            }
            ImGui::PopID();
            
            if (isHidden)
            {
                ImGui::PopStyleVar();
            }
        }

        drawList->PopClipRect();

        // Squad Manager: Move and re-size
        if (Addon::isSquadManagerActive)
        {
            // Compute first cell for move grip
            DrawProperties_t firstCell = CalcDrawProperties(frameDrawProperties.size.x, frameDrawProperties.size.y, frameDrawProperties, ImDrawCornerFlags_All, gridDrawProperties, 0);

            // Compute last possible cell for resize grip based on current rowColMax and cellDirectionMax
            int max_index = (context.layoutConfig->layout.grid.rowColMax * context.layoutConfig->layout.grid.cellDirectionMax) - 1;
            DrawProperties_t lastCell = CalcDrawProperties(frameDrawProperties.size.x, frameDrawProperties.size.y, frameDrawProperties, ImDrawCornerFlags_All, gridDrawProperties, max_index);

            ImDrawList* fgDrawList = ImGui::GetForegroundDrawList();

            auto drawDashedLine = [&](ImVec2 p1, ImVec2 p2, ImU32 col, float thickness, float dash_length, float gap_length) {
                float dx = p2.x - p1.x;
                float dy = p2.y - p1.y;
                float length = std::sqrt(dx*dx + dy*dy);
                if (length == 0.0f) return;
                float nx = dx / length;
                float ny = dy / length;
                for (float t = 0.0f; t < length; t += dash_length + gap_length) {
                    float t_end = ImMin(t + dash_length, length);
                    fgDrawList->AddLine(ImVec2(p1.x + nx * t, p1.y + ny * t), ImVec2(p1.x + nx * t_end, p1.y + ny * t_end), col, thickness);
                }
            };

            ImVec2 border_min = ImVec2(firstCell.position.x - 2.0f, firstCell.position.y - 2.0f);
            ImVec2 border_max = ImVec2(lastCell.position.x + lastCell.width + 2.0f, lastCell.position.y + lastCell.height + 2.0f);
            ImU32 dash_col = ImColor(200, 200, 200, 150);
            drawDashedLine(border_min, ImVec2(border_max.x, border_min.y), dash_col, 1.0f, 4.0f, 4.0f);
            drawDashedLine(ImVec2(border_max.x, border_min.y), border_max, dash_col, 1.0f, 4.0f, 4.0f);
            drawDashedLine(border_max, ImVec2(border_min.x, border_max.y), dash_col, 1.0f, 4.0f, 4.0f);
            drawDashedLine(ImVec2(border_min.x, border_max.y), border_min, dash_col, 1.0f, 4.0f, 4.0f);

            // Move Grip
            float moveGripWidth = 22.0f;
            float moveGripHeight = 24.0f;
            ImVec2 moveGripMin;
            std::string cellDir = context.layoutConfig->layout.grid.cellDirection;
            if (cellDir == "Top-to-bottom") {
                // Header is above, place grip to the left of the dotted line's top-left corner
                moveGripMin = ImVec2(border_min.x - moveGripWidth - 6.0f, border_min.y);
            } else if (cellDir == "Left-to-right") {
                // Header is to the left, place grip above the dotted line's top-left corner
                moveGripMin = ImVec2(border_min.x, border_min.y - moveGripHeight - 6.0f);
            } else {
                // Top-left is free, place diagonally
                moveGripMin = ImVec2(border_min.x - moveGripWidth - 6.0f, border_min.y - moveGripHeight - 6.0f);
            }
            
            ImGui::SetNextWindowPos(moveGripMin);
            ImGui::SetNextWindowSize(ImVec2(moveGripWidth, moveGripHeight));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            ImGui::Begin("##MoveGripWindow", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
            ImGui::InvisibleButton("##MoveGripBtn", ImVec2(moveGripWidth, moveGripHeight));
            if (ImGui::IsItemActive())
            {
                context.layoutConfig->position.offset.x += ImGui::GetIO().MouseDelta.x;
                context.layoutConfig->position.offset.y += ImGui::GetIO().MouseDelta.y;
            }
            // Draw move grip
            fgDrawList->AddRectFilled(moveGripMin, ImVec2(moveGripMin.x + moveGripWidth, moveGripMin.y + moveGripHeight), ImColor(100, 100, 100, 200), 2.0f);
            for (int r = 0; r < 2; ++r) {
                for (int c = 0; c < 3; ++c) {
                    fgDrawList->AddCircleFilled(ImVec2(moveGripMin.x + 7.0f + r * 8.0f, moveGripMin.y + 6.0f + c * 6.0f), 2.0f, ImColor(200, 200, 200, 255));
                }
            }
            ImGui::End();
            ImGui::PopStyleVar();

            // Resize Grip
            static ImVec2 resize_drag_accum(0.0f, 0.0f);
            
            float resizeGripSize = 24.0f;
            ImVec2 resizeGripMax(lastCell.position.x + lastCell.width + 8.0f, lastCell.position.y + lastCell.height + 8.0f);
            ImVec2 resizeGripMin(resizeGripMax.x - resizeGripSize, resizeGripMax.y - resizeGripSize);
            
            ImGui::SetNextWindowPos(resizeGripMin);
            ImGui::SetNextWindowSize(ImVec2(resizeGripSize, resizeGripSize));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            ImGui::Begin("##ResizeGripWindow", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
            ImGui::InvisibleButton("##ResizeGripBtn", ImVec2(resizeGripSize, resizeGripSize));
            if (ImGui::IsItemActive())
            {
                resize_drag_accum.x += ImGui::GetIO().MouseDelta.x;
                resize_drag_accum.y += ImGui::GetIO().MouseDelta.y;

                float cellTotalWidth = frameDrawProperties.size.x + frameDrawProperties.spacing + frameDrawProperties.padding.w;
                float cellTotalHeight = frameDrawProperties.size.y + frameDrawProperties.spacing + frameDrawProperties.padding.x;

                if (std::abs(resize_drag_accum.x) > cellTotalWidth)
                {
                    int drag_units = resize_drag_accum.x / cellTotalWidth;
                    if (context.layoutConfig->layout.grid.cellDirection == "Left-to-right" || context.layoutConfig->layout.grid.cellDirection == "Right-to-left") {
                        context.layoutConfig->layout.grid.cellDirectionMax = ImMax(1, context.layoutConfig->layout.grid.cellDirectionMax + drag_units);
                    } else {
                        context.layoutConfig->layout.grid.rowColMax = ImMax(1, ImMin((int)UI::SQUAD_MEMBER_LIMIT, context.layoutConfig->layout.grid.rowColMax + drag_units));
                    }
                    resize_drag_accum.x -= drag_units * cellTotalWidth;
                }
                
                if (std::abs(resize_drag_accum.y) > cellTotalHeight)
                {
                    int drag_units = resize_drag_accum.y / cellTotalHeight;
                    if (context.layoutConfig->layout.grid.cellDirection == "Top-to-bottom" || context.layoutConfig->layout.grid.cellDirection == "Bottom-to-top") {
                        context.layoutConfig->layout.grid.cellDirectionMax = ImMax(1, context.layoutConfig->layout.grid.cellDirectionMax + drag_units);
                    } else {
                        context.layoutConfig->layout.grid.rowColMax = ImMax(1, ImMin((int)UI::SQUAD_MEMBER_LIMIT, context.layoutConfig->layout.grid.rowColMax + drag_units));
                    }
                    resize_drag_accum.y -= drag_units * cellTotalHeight;
                }
            }
            else
            {
                resize_drag_accum = ImVec2(0.0f, 0.0f); // reset when not dragging
            }
            
            // Draw resize grip
            ImU32 resizeCol = ImGui::IsItemHovered() ? ImColor(130, 130, 130, 255) : ImColor(100, 100, 100, 200);
            auto drawRoundedTriangle = [&](ImVec2 min, ImVec2 max, float r, ImU32 col) {
                float d = r * 2.41421356f;
                if (max.x - min.x < 2.0f * d || max.y - min.y < 2.0f * d) {
                    fgDrawList->AddTriangleFilled(ImVec2(min.x, max.y), max, ImVec2(max.x, min.y), col);
                    return;
                }
                fgDrawList->PathClear();
                fgDrawList->PathArcTo(ImVec2(max.x - r, max.y - r), r, 0.0f, IM_PI / 2.0f, 5);
                fgDrawList->PathArcTo(ImVec2(min.x + d, max.y - r), r, IM_PI / 2.0f, IM_PI * 1.25f, 5);
                fgDrawList->PathArcTo(ImVec2(max.x - r, min.y + d), r, IM_PI * 1.25f, IM_PI * 2.0f, 5);
                fgDrawList->PathFillConvex(col);
            };
            drawRoundedTriangle(resizeGripMin, resizeGripMax, 2.0f, resizeCol);
            
            ImU32 lineCol = ImGui::IsItemHovered() ? ImColor(255, 255, 255, 255) : ImColor(200, 200, 200, 255);
            for (int i = 1; i <= 3; ++i) {
                float offset = i * 4.0f + 3.0f;
                ImVec2 p1(resizeGripMax.x - 4.0f, resizeGripMax.y - offset);
                ImVec2 p2(resizeGripMax.x - offset, resizeGripMax.y - 4.0f);
                fgDrawList->AddLine(p1, p2, lineCol, 1.5f);
            }
            ImGui::End();
            ImGui::PopStyleVar();
        }

        if (ImGui::IsWindowHovered() && (context.indexHovered == -1))
        {
            /* Allow mouse inputs to pass through to the game */
            ImGui::GetIO().WantCaptureMouse = false;
        }

        if (context.isClose || !context.isActive)
        {
            context.indexHovered = -1;
            
            if (!context.isActive)
            {
                ImGui::EndDisabled();
            }
        }

        ImGui::PopID(); // Pushed in `BeginGridMenu`
        ImGui::End(); // Begin in `BeginGridMenu`
        
        // Squad Manager: Visibility Checkboxes
        if (Addon::isSquadManagerActive && context.hiddenSubgroups)
        {
            for (const auto& cb : context.headerCheckboxes)
            {
                ImGui::SetNextWindowPos(cb.position);
                float checkboxSize = ImGui::GetFrameHeight();
                ImGui::SetNextWindowSize(ImVec2(checkboxSize * 2.0f, checkboxSize));
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
                
                std::string windowName = "##VisibilityGripWindow" + std::to_string(cb.subgroupId);
                ImGui::Begin(windowName.c_str(), nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
                
                bool isVisible = std::find(context.hiddenSubgroups->begin(), context.hiddenSubgroups->end(), cb.subgroupId) == context.hiddenSubgroups->end();
                
                ImGui::PushID(cb.subgroupId);
                if (ImGui::Checkbox("##Visibility", &isVisible))
                {
                    if (isVisible)
                    {
                        context.hiddenSubgroups->erase(std::remove(context.hiddenSubgroups->begin(), context.hiddenSubgroups->end(), cb.subgroupId), context.hiddenSubgroups->end());
                    }
                    else
                    {
                        context.hiddenSubgroups->push_back(cb.subgroupId);
                    }
                }
                ImGui::PopID();
                
                ImGui::End();
                ImGui::PopStyleVar();
            }
        }
    }

    bool GridMenuItem(const VitalSignsDataLink::UserData_t& userData)
    {
        bool isHidden = false;
        if (context.hiddenSubgroups)
        {
            isHidden = std::find(context.hiddenSubgroups->begin(), context.hiddenSubgroups->end(), userData.SubgroupId) != context.hiddenSubgroups->end();
        }

        if (isHidden && !Addon::isSquadManagerActive)
        {
            return false; // Skip entirely, don't even allocate space
        }

        bool isSelected = false;

        int cellDirectionMax = context.layoutConfig->layout.grid.cellDirectionMax;
        
        if ((context.index > 0) && (context.userData[context.index - 1].SubgroupId != userData.SubgroupId))
        {
            if (context.index % cellDirectionMax != 0)
            {
                context.index = ((context.index / cellDirectionMax) + 1) * cellDirectionMax;
            }
        }

        int current_s = context.index / cellDirectionMax;

        if (context.index < SQUAD_MEMBER_LIMIT)
        {
            context.isValid[context.index] = true;
            context.userData[context.index] = userData;
    
            /**
             * @note Check if the item is clicked. Compare `UserId` against `userIdHovered` 
             * instead of checking `context.index == context.indexHovered` to prevent a bug
             * where sorting the grid inside `EndGridMenu` decouples the user's insertion slot
             * (`context.index`) from their rendered display slot (`context.indexHovered`).
             */
            if ((context.indexHovered != -1) && (userData.UserId == context.userIdHovered) && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                isSelected = true;
                context.isClose = true;
            }
    
            context.index++;
        }

        return isSelected;
    }

} // namespace UI::Grid

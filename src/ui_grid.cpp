#include "ui_grid.h"

#include "imgui_extensions.h"
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
        LayoutConfig_t layoutConfig;
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
        std::string charName[SQUAD_MEMBER_LIMIT];
        VitalSignsData::EProfession profession[SQUAD_MEMBER_LIMIT];
        VitalSignsData::ESpecialisation specialisation[SQUAD_MEMBER_LIMIT];
        float health[SQUAD_MEMBER_LIMIT];
        VitalSignsData::E_HEALTH_TYPE healthType[SQUAD_MEMBER_LIMIT];
        float barrier[SQUAD_MEMBER_LIMIT];
        VitalSignsData::Effects_t effects[SQUAD_MEMBER_LIMIT];
    } context;

    struct GridDrawProperties_t 
    {
        ImVec2 position;
        int rows;
        int columns;
        std::string direction;
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
            /* Get mouse position */
            const ImVec2 mousePos = ImGui::GetIO().MousePos;
            
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

    void DrawIconText(ImDrawList* const drawList, const ImVec2& iconPosition, const ImVec2& iconSize, const std::string& text, const IconTextConfig_t& config)
    {
        std::string fontPath = (config.fontType == "Custom font") ? config.fontPath : "";
        float fontSize = (config.fontSizeType == "Custom font size") ? config.fontSize : ImGui::GetIO().FontDefault->FontSize;

        ImFont* font = utils::font::GetFont(fontPath, fontSize);

        if (font)
        {
            ImGui::PushFont(font);
            drawList->PushTextureID(font->ContainerAtlas->TexID);
        }

        ImVec2 textSize = ImGui::CalcTextSize(text.c_str());

        DrawProperties_t iconProps;
        iconProps.position = iconPosition;
        iconProps.width = iconSize.x;
        iconProps.height = iconSize.y;
        // rounding ignored

        ImVec2 textPos = CalcItemPosition(iconProps, textSize, config.position.anchor, config.position.offset);

        if (config.shadow) drawList->AddText(font, fontSize, textPos + ImVec2(1, 1), config.shadowColor, text.c_str());
        if (config.outline) {
            drawList->AddText(font, fontSize, textPos + ImVec2(-1, 0), config.outlineColor, text.c_str());
            drawList->AddText(font, fontSize, textPos + ImVec2(1, 0), config.outlineColor, text.c_str());
            drawList->AddText(font, fontSize, textPos + ImVec2(0, -1), config.outlineColor, text.c_str());
            drawList->AddText(font, fontSize, textPos + ImVec2(0, 1), config.outlineColor, text.c_str());
        }
        drawList->AddText(font, fontSize, textPos, config.color, text.c_str());

        if (font) drawList->PopTextureID();
        if (font) ImGui::PopFont();
    }

    void DrawIcon(ImDrawList * const drawList, DrawProperties_t properties, Texture* icon, const Size_t& size, std::string anchor, Coordinate_t offset, bool showDuration, float duration, bool showStacks, unsigned int stacks)
    {
        if (icon != nullptr)
        {
            ImVec2 iconSize = ImVec2((float)size.width, (float)size.height);
            ImVec2 iconPosition = CalcItemPosition(properties, iconSize, anchor, offset);

            drawList->AddImage(icon->Resource, iconPosition, ImVec2(iconPosition.x + iconSize.x, iconPosition.y + iconSize.y));

            if (showDuration)
            {
                char buf[32];
                sprintf_s(buf, "%.1f", (duration / 1000.f));
                DrawIconText(drawList, iconPosition, iconSize, buf, ConfigIconDuration);
            }

            if (showStacks && (stacks > 1U))
            {
                char buf[32];
                sprintf_s(buf, "%u", stacks);
                DrawIconText(drawList, iconPosition, iconSize, buf, ConfigIconStacks);
            }
        }
    }

    DrawProperties_t CalcDrawProperties(float width, float height, const CellDrawProperties_t& cellDraw, ImDrawCornerFlags cellRoundingCorners, const GridDrawProperties_t& gridDraw, int index)
    {
        int indexRow = 0;
        int indexColumn = 0;

        if (gridDraw.direction == "Top-to-bottom")
        {
            if (gridDraw.rows > 0)
            {
                indexRow = index % gridDraw.rows;
                indexColumn = index / gridDraw.rows;
            }
        }
        else if (gridDraw.direction == "Right-to-left")
        {
            if (gridDraw.columns > 0)
            {
                indexColumn = (gridDraw.columns - 1) - (index % gridDraw.columns);
                indexRow = index / gridDraw.columns;
            }
        }
        else if (gridDraw.direction == "Bottom-to-top")
        {
            if (gridDraw.rows > 0)
            {
                indexRow = (gridDraw.rows - 1) - (index % gridDraw.rows);
                indexColumn = index / gridDraw.rows;
            }
        }
        else
        {
            /* Default: Left-to-right */
            if (gridDraw.columns > 0)
            {
                indexRow = index / gridDraw.columns;
                indexColumn = index % gridDraw.columns;
            }
        }

        float offsetRow = indexRow * (cellDraw.size.y + cellDraw.spacing + cellDraw.padding.x);
        float offsetColumn = indexColumn * (cellDraw.size.x + cellDraw.spacing + cellDraw.padding.w) + (indexColumn * cellDraw.padding.y) - (indexColumn * cellDraw.padding.w);
        ImVec2 cellOffset(offsetColumn, offsetRow);

        DrawProperties_t properties{};
        properties.position.x = gridDraw.position.x + cellOffset.x + cellDraw.padding.w;
        properties.position.y = gridDraw.position.y + cellOffset.y + cellDraw.padding.x;
        properties.width = width;
        properties.height = height;
        properties.rounding = cellDraw.rounding;
        properties.roundingCorners = cellRoundingCorners;

        return properties;
    }

    bool BeginGridMenu(const char* name, const LayoutConfig_t& layout, const ColourPresets_t& colours, const BorderPresets_t& borders, bool isActive)
    {
        bool isOpen = false;
        bool isAlwaysDisplayed = (layout.layout.visibility == 0);

        if (isActive || isAlwaysDisplayed || context.isItemPending)
        {
            context.layoutConfig = layout;
            context.colourPresets = colours;
            context.borderPresets = borders;

            context.isActive = isActive;
            context.isClose = false;

            context.menuPosition = ImVec2(
                (ImGui::GetIO().DisplaySize.x / 2.f + context.layoutConfig.position.offset.x), 
                (ImGui::GetIO().DisplaySize.y / 2.f + context.layoutConfig.position.offset.y));
            
            float menuWidth = (float)(context.layoutConfig.layout.grid.columnCount * context.layoutConfig.layout.grid.cellWidth + (context.layoutConfig.layout.grid.columnCount - 1) * context.layoutConfig.layout.itemSpacing);
            float menuHeight = (float)(context.layoutConfig.layout.grid.rowCount * context.layoutConfig.layout.grid.cellHeight + (context.layoutConfig.layout.grid.rowCount - 1) * context.layoutConfig.layout.itemSpacing);
            
            ImGui::SetNextWindowPos(context.menuPosition);
            ImGui::SetNextWindowSize(ImVec2(menuWidth, menuHeight));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

            if (ImGui::Begin(name, nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar))
            {
                ImGui::PushID(name); // Popped in `EndGridMenu`

                if (!context.isActive)
                {
                    // Disable interaction for everything within this block
                    ImGui::BeginDisabled();

                    // Override opacity
                    context.colourPresets.COLOUR_BACKGROUND.Value.w *= context.layoutConfig.layout.inactiveOpacity;
                    context.colourPresets.COLOUR_HEALTH.Value.w *= context.layoutConfig.layout.inactiveOpacity;
                    context.colourPresets.COLOUR_HEALTH_DOWNED.Value.w *= context.layoutConfig.layout.inactiveOpacity;
                    context.colourPresets.COLOUR_HEALTH_DEFEATED.Value.w *= context.layoutConfig.layout.inactiveOpacity;
                    context.colourPresets.COLOUR_SHROUD_NECROMANCER.Value.w *= context.layoutConfig.layout.inactiveOpacity;
                    context.colourPresets.COLOUR_SHROUD_SPECTER.Value.w *= context.layoutConfig.layout.inactiveOpacity;
                    context.colourPresets.COLOUR_BARRIER.Value.w *= context.layoutConfig.layout.inactiveOpacity;
                    context.colourPresets.COLOUR_HOVERED.Value.w *= context.layoutConfig.layout.inactiveOpacity;

                    context.borderPresets.COLOUR_BORDER.Value.w *= context.layoutConfig.layout.inactiveOpacity;

                    context.isItemPending = false;
                }
                
                context.index = 0;

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
        return (context.layoutConfig.previewNodeId != TreeNodeUID::NONE);
    }

    static bool IsTriggerMet(const Trigger_t& trigger, int userIndex, bool isPreview = false)
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

        if ((trigger.category == "Professions") && (trigger.effect == VitalSignsData::getProfessionString(context.profession[userIndex], context.specialisation[userIndex])))
        {
            return true;
        }

        auto currStacks = context.effects[userIndex][trigger.effect].stacks;
        auto currDuration = context.effects[userIndex][trigger.effect].duration / 1000.0f;

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

    static bool ProcessIndicatorsDFS(const std::vector<Indicator_t>& indicators, int userIndex, bool isPriorityList, bool isParentPreviewed, ImColor* outColor, const Indicator_t** outBorder, std::vector<std::pair<const Indicator_t*, bool>>& outDrawables)
    {
        bool isAnyApplied = false;
        
        for (const auto& indicator : indicators)
        {
            bool isCurrentPreviewed = (indicator.id == context.layoutConfig.previewNodeId);
            bool isPreviewed = isCurrentPreviewed || (indicator.enabled && isParentPreviewed);
            bool isTraversalForced = !isPreviewed && IsPreviewActive() && HasDescendant(indicator, context.layoutConfig.previewNodeId);

            if (indicator.enabled || isPreviewed || isTraversalForced)
            {
                bool isApplied = false;

                if (indicator.type == "Group")
                {
                    if (IsTriggerMet(indicator.group.trigger, userIndex, isPreviewed) || isTraversalForced)
                    {
                        if (ProcessIndicatorsDFS(indicator.group.indicators, userIndex, indicator.group.priorityGroup, isPreviewed, outColor, outBorder, outDrawables))
                        {
                            isApplied = true;
                        }
                    }
                }
                else if (indicator.type == "Colour")
                {
                    if (IsTriggerMet(indicator.colour.trigger, userIndex, isPreviewed) || isTraversalForced)
                    {
                        if (outColor) *outColor = indicator.colour.color;
                        isApplied = true;
                    }
                }
                else if (indicator.type == "Border")
                {
                    if (IsTriggerMet(indicator.border.trigger, userIndex, isPreviewed) || isTraversalForced)
                    {
                        if (outBorder) *outBorder = &indicator;
                        isApplied = true;
                    }
                }
                else if (indicator.type == "Highlight")
                {
                    if (IsTriggerMet(indicator.highlight.trigger, userIndex, isPreviewed) || isTraversalForced)
                    {
                        outDrawables.push_back({ &indicator, isPreviewed });
                        isApplied = true;
                    }
                }
                else if (indicator.type == "Icon")
                {
                    if (IsTriggerMet(indicator.iconSingle.icon.trigger, userIndex, isPreviewed) || isTraversalForced)
                    {
                        outDrawables.push_back({ &indicator, isPreviewed });
                        isApplied = true;
                    }
                }
                else if (indicator.type == "Icon List")
                {
                    for (const auto& icon : indicator.iconList.list)
                    {
                        bool isIconForced = isPreviewed || (icon.id == context.layoutConfig.previewNodeId);

                        if (IsTriggerMet(icon.trigger, userIndex, isIconForced))
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
                    if (IsTriggerMet(indicator.text.trigger, userIndex, isPreviewed) || isTraversalForced)
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

    static void DrawIndicator(ImDrawList* const drawList, const DrawProperties_t& parentProperties, const DrawProperties_t& contentProperties, const Indicator_t* indicator, int userIndex, bool isPreviewed)
    {
        if (indicator->type == "Highlight")
        {
            ImVec2 p_min, p_max;
            ImDrawCornerFlags roundingCorners = ImDrawCornerFlags_None;
            ImU32 colour_top, colour_bottom;
            
            float heightRatio = indicator->highlight.height / 100.0f;
            float height = contentProperties.height * heightRatio;

            if (indicator->highlight.direction == "Top")
            {
                p_min = ImVec2((contentProperties.position.x + 1), (contentProperties.position.y + 1));
                p_max = ImVec2((p_min.x + contentProperties.width - 1), (p_min.y + height - 1));
                roundingCorners = (heightRatio >= 1.0f) ? ImDrawCornerFlags_All : ImDrawCornerFlags_Top;
                colour_top = indicator->highlight.color;
                colour_bottom = ImColor(0, 0, 0, 0);
            }
            else // if (indicator->highlight.direction == "Bottom")
            {
                p_min = ImVec2((contentProperties.position.x + 1), (contentProperties.position.y + contentProperties.height - height + 1));
                p_max = ImVec2((p_min.x + contentProperties.width - 1), (p_min.y + height - 1));
                roundingCorners = (heightRatio >= 1.0f) ? ImDrawCornerFlags_All : ImDrawCornerFlags_Bot;
                colour_top = ImColor(0, 0, 0, 0);
                colour_bottom = indicator->highlight.color;
            }

            ImGui::AddRectFilledGradient(drawList, p_min, p_max, colour_top, colour_bottom, static_cast<float>(contentProperties.rounding), roundingCorners);
        }
        else if (indicator->type == "Icon")
        {
            Texture* texture = UI::GetOrCreateTexture(indicator->iconSingle.icon.source, indicator->iconSingle.icon.path);
            float duration = context.effects[userIndex][indicator->iconSingle.icon.trigger.effect].duration;
            unsigned int stacks = context.effects[userIndex][indicator->iconSingle.icon.trigger.effect].stacks;
            DrawIcon(drawList, parentProperties, texture, indicator->iconSingle.size, indicator->iconSingle.position.anchor, indicator->iconSingle.position.offset, indicator->iconSingle.showDuration, duration, indicator->iconSingle.showStacks, stacks);
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
                if (IsTriggerMet(icon.trigger, userIndex, isPreviewed))
                {
                    Coordinate_t iconOffset((iconSpacing.x * i + indicator->iconList.position.offset.x), (iconSpacing.y * i + indicator->iconList.position.offset.y));
                        
                    Texture* texture = UI::GetOrCreateTexture(icon.source, icon.path);
                    float duration = context.effects[userIndex][icon.trigger.effect].duration;
                    unsigned int stacks = context.effects[userIndex][icon.trigger.effect].stacks;
                    DrawIcon(drawList, parentProperties, texture, indicator->iconList.size, indicator->iconList.position.anchor, iconOffset, indicator->iconList.showDuration, duration, indicator->iconList.showStacks, stacks);

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
                text = context.charName[userIndex];
            } 
            else if (indicator->text.textContent == "Health percentage")
            {
                text = std::to_string(max(static_cast<int>(context.health[userIndex] * 100.0f), 0)) + "%";
            } 
            else
            {
                text = indicator->text.textCustom;
            }

            std::string fontPath = ""; // Nexus font
            if (indicator->text.fontType == "Default font")
            {
                fontPath = ConfigText.fontPath;
            }
            else if (indicator->text.fontType == "Custom font")
            {
                fontPath = indicator->text.fontPath;
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

            ImFont* font = utils::font::GetFont(fontPath, fontSize);

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

            bool pushTexture = (font != nullptr);
            if (pushTexture)
            {
                drawList->PushTextureID(font->ContainerAtlas->TexID);
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

            if (pushTexture)
            {
                drawList->PopTextureID();
            }
        }
    }

    void EndGridMenu()
    {
        /* Grid properties */
        GridDrawProperties_t gridDrawProperties;
        gridDrawProperties.position = context.menuPosition;
        gridDrawProperties.rows = context.layoutConfig.layout.grid.rowCount;
        gridDrawProperties.columns = context.layoutConfig.layout.grid.columnCount;
        gridDrawProperties.direction = context.layoutConfig.layout.grid.cellDirection;

        /* Frame properties */
        CellDrawProperties_t frameDrawProperties;
        frameDrawProperties.size = ImVec2(
            (float)(context.layoutConfig.layout.grid.cellWidth - (2 * context.layoutConfig.layout.itemBorder)), 
            (float)(context.layoutConfig.layout.grid.cellHeight - (2 * context.layoutConfig.layout.itemBorder)));
        frameDrawProperties.padding = _ImVec4((float)context.layoutConfig.layout.itemBorder);
        frameDrawProperties.spacing = (float)(context.layoutConfig.layout.itemSpacing + context.layoutConfig.layout.itemBorder);
        frameDrawProperties.rounding = ImMax(context.layoutConfig.layout.grid.cellRounding, context.layoutConfig.layout.grid.cellRounding - context.layoutConfig.layout.itemBorder);
        
        /* Frame border properties */
        CellDrawProperties_t borderDrawProperties;
        borderDrawProperties.size = ImVec2(
            (float)context.layoutConfig.layout.grid.cellWidth, 
            (float)context.layoutConfig.layout.grid.cellHeight);
        borderDrawProperties.padding = _ImVec4(0.f);
        borderDrawProperties.spacing = (float)context.layoutConfig.layout.itemSpacing;
        borderDrawProperties.rounding = context.layoutConfig.layout.grid.cellRounding;

        /**
         * @note The state for `indexHovered` is reset in `EndGridMenu` because
         * `GridMenuItem` (which is called prior to `EndGridMenu`) relies on the
         * result from the previous frame.
         **/
        context.indexHovered = -1;

        ImDrawList* drawList = ImGui::GetForegroundDrawList();
        
        /* Draw items */
        for (int i = 0; i < context.index; i++)
        {
            ImGui::PushID(i);
            {
                std::vector<std::pair<const Indicator_t*, bool>> drawables;

                DrawProperties_t parentProperties = CalcDrawProperties(frameDrawProperties.size.x, frameDrawProperties.size.y, frameDrawProperties, ImDrawCornerFlags_All, gridDrawProperties, i);
    
                /* Invisible button (creates clickable region) */
                ImGui::SetCursorScreenPos(parentProperties.position);
                ImGui::InvisibleButton("", ImVec2(parentProperties.width, parentProperties.height));

                const bool isHovered = IsItemHovered(parentProperties);
                const ImColor backgroundColour = GetBackgroundColour(context.colourPresets, context.layoutConfig.colors);
                ImColor healthColour = GetHealthColour(context.colourPresets, context.layoutConfig.colors, context.healthType[i], context.profession[i]);
                const ImColor barrierColour = GetBarrierColour(context.colourPresets, context.layoutConfig.colors);
                const Indicator_t* borderStyle = nullptr;
    
                /* Process indicators (returns a list of drawable items) */
                ProcessIndicatorsDFS(context.layoutConfig.indicators, i, false, false, &healthColour, &borderStyle, drawables);
                if ((context.layoutConfig.previewNodeId != TreeNodeUID::NONE) && (context.layoutConfig.previewNodeId == context.layoutConfig.id))
                {
                    /* Preview mode */
                    ProcessIndicatorsDFS(context.layoutConfig.indicators, i, false, true, &healthColour, &borderStyle, drawables);
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
                else if (context.layoutConfig.layout.itemBorder >= 1)
                {
                    borderThickness = (float)context.layoutConfig.layout.itemBorder;
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
                if (context.health[i] > 0.001f)
                {
                    float healthWidth = contentProps.width * context.health[i];
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
                if (context.barrier[i] > 0.001f)
                {
                    float barrierWidth = contentProps.width * context.barrier[i];
                    DrawProperties_t properties = contentProps;
                    properties.width = barrierWidth;
    
                    if ((context.barrier[i] + context.health[i]) > 0.999f)
                    {
                        /* Overflow barrier */
                        properties.position.x += contentProps.width - barrierWidth;
                        properties.roundingCorners = ImDrawCornerFlags_Right;
                        DrawCell(drawList, properties, barrierColour);
                    }
                    else
                    {
                        /* Padding barrier */
                        float healthWidth = contentProps.width * context.health[i];
                        ImDrawCornerFlags roundingCorners = ((healthWidth + barrierWidth) < (contentProps.width - (float)contentProps.rounding)) ? ImDrawCornerFlags_None : ImDrawCornerFlags_Right;
                        properties.position.x += healthWidth;
                        properties.roundingCorners = roundingCorners;
                        DrawCell(drawList, properties, barrierColour);
                    }
                }
    
                /* Indicators */
                for (const auto& pair : drawables)
                {
                    DrawIndicator(drawList, parentProperties, contentProps, pair.first, i, pair.second);
                }
    
                if (isHovered)
                {
                    context.indexHovered = i;
    
                    context.isItemPending = true;
                }
            }
            ImGui::PopID();
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
    }

    bool GridMenuItem(const VitalSignsData::UserData_t& userData)
    {
        bool isSelected = false;

        if (context.index < (context.layoutConfig.layout.grid.rowCount * context.layoutConfig.layout.grid.columnCount))
        {
            std::string name = (userData.CharacterName.empty() ? userData.AccountName : userData.CharacterName);
            float health = ((userData.Health.Max > 0.0f) ? (userData.Health.Current / userData.Health.Max) : 0.0f);
            float barrier = ((userData.Health.Max > 0.0f) ? (userData.Barrier.Current / userData.Health.Max) : 0.0f);
            if ((VitalSignsData::E_HEALTH_SHROUD_NECROMANCER == userData.HealthType) ||
                (VitalSignsData::E_HEALTH_SHROUD_SPECTER == userData.HealthType))
            {
                health = ((userData.Shroud.Max > 0.0f) ? (userData.Shroud.Current / userData.Shroud.Max) : 0.0f);
            }

            context.charName[context.index] = name;
            context.profession[context.index] = userData.Profession;
            context.specialisation[context.index] = userData.Specialisation;
            context.health[context.index] = health;
            context.healthType[context.index] = userData.HealthType;
            context.barrier[context.index] = barrier;
            context.effects[context.index] = userData.Effects;
    
            if ((context.index == context.indexHovered) && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                isSelected = true;
                context.isClose = true;
            }
    
            context.index++;
        }

        return isSelected;
    }

} // namespace UI::Grid

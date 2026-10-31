#include "ui_grid.h"

#include <algorithm>
#include <vector>

#include "data_link/data_link.h"
#include "imgui/imgui.h"
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

    constexpr int MAX_GRID_CELLS = SQUAD_MEMBER_LIMIT * SQUAD_SUBGROUP_LIMIT;

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
        bool isValid[MAX_GRID_CELLS];
        VitalSignsDataLink::UserData_t userData[MAX_GRID_CELLS];
        VitalSignsDataLink::UserId_t userIdHovered;
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
        ImVec2 spacing;
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
        float offset = ImMax(0.0f, (thickness - 1.0f) / 2.0f);
        float rounding = (properties.rounding > 0.0f) ? ImMax(0.0f, properties.rounding - offset) : 0.0f;
 
        ImVec2 p_min(properties.position.x + offset, properties.position.y + offset);
        ImVec2 p_max(properties.position.x + properties.width - offset, properties.position.y + properties.height - offset);

        drawList->AddRect(p_min, p_max, colour, rounding, properties.roundingCorners, thickness);
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

        ImVec2 textPosition = CalcItemPosition(iconProps, textSize, anchor, offset);

        ImColor color = (config.textStyle.colorSource == "Custom color") ? config.textStyle.color : globalConfig.textStyle.color;
        bool useShadow = (config.textStyle.decoratorSource == "Custom decorators") ? config.textStyle.shadow : globalConfig.textStyle.shadow;
        ImColor shadowColor = (config.textStyle.decoratorSource == "Custom decorators") ? config.textStyle.shadowColor : globalConfig.textStyle.shadowColor;
        bool useOutline = (config.textStyle.decoratorSource == "Custom decorators") ? config.textStyle.outline : globalConfig.textStyle.outline;
        ImColor outlineColor = (config.textStyle.decoratorSource == "Custom decorators") ? config.textStyle.outlineColor : globalConfig.textStyle.outlineColor;

        if (useShadow) drawList->AddText(font, fontSize, textPosition + ImVec2(1, 1), shadowColor, text);
        if (useOutline) {
            drawList->AddText(font, fontSize, textPosition + ImVec2(-1, 0), outlineColor, text);
            drawList->AddText(font, fontSize, textPosition + ImVec2(1, 0), outlineColor, text);
            drawList->AddText(font, fontSize, textPosition + ImVec2(0, -1), outlineColor, text);
            drawList->AddText(font, fontSize, textPosition + ImVec2(0, 1), outlineColor, text);
        }
        drawList->AddText(font, fontSize, textPosition, color, text);

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
        int cellDirectionMax = (gridLayout.frameDirection == "Left-to-right" || gridLayout.frameDirection == "Right-to-left") ? gridLayout.maxColumns : gridLayout.maxRows;
        int requiredSubgroups = ImMax(1, (itemCount + cellDirectionMax - 1) / cellDirectionMax);

        // Reserve an extra subgroup block if the user is currently holding a valid drag-and-drop player payload
        int squadMax = (gridLayout.squadDirection == "Left-to-right" || gridLayout.squadDirection == "Right-to-left") ? gridLayout.maxColumns : gridLayout.maxRows;
        const ImGuiPayload* payload = ImGui::GetDragDropPayload();
        if (payload && payload->IsDataType("VS_USER_ID") && (requiredSubgroups < squadMax + 1))
        {
            requiredSubgroups++;
        }

        outRows = 1;
        outColumns = 1;

        // Internal growth (cells)
        if (gridLayout.frameDirection == "Left-to-right" || gridLayout.frameDirection == "Right-to-left") outColumns = cellDirectionMax;
        else outRows = cellDirectionMax;

        // External growth (subgroups)
        if (gridLayout.squadDirection == "Left-to-right" || gridLayout.squadDirection == "Right-to-left") outColumns *= requiredSubgroups;
        else outRows *= requiredSubgroups;
    }

    float CalcDividerSizeForLayout(const LayoutConfig_t& config, const std::string& squadDirection, const std::string& frameDirection)
    {
        const auto& shProps = config.layout.grid.subgroupHeader;
        if (shProps.position != "Internal") return 0.0f;
        
        bool isHorizontal = (squadDirection == "Top-to-bottom" || squadDirection == "Bottom-to-top");
        bool isVerticalFrame = (frameDirection == "Top-to-bottom" || frameDirection == "Bottom-to-top");
        bool isRectOrTex = (shProps.type == "Rectangle");

        float textHeight = 0.0f;
        float textWidth = 0.0f;
        
        if (shProps.visibility != "Hidden") {
            const auto& textStyle = shProps.textStyle;
            float fontSize = ImGui::GetIO().FontDefault->FontSize;
            if (textStyle.fontSizeSource == "Default font size")
            {
                fontSize = (float)ConfigText.fontSize;
            }
            else if (textStyle.fontSizeSource == "Custom font size")
            {
                fontSize = (float)textStyle.fontSize;
            }

            std::string fontSource = (textStyle.fontSource == "Default font") ? ConfigText.fontSource : textStyle.fontSource;
            std::string fontName = (textStyle.fontSource == "Default font") ? ConfigText.font : textStyle.font;
            ImFont* font = nullptr;
            if (fontSource != "Nexus font" && fontSource != "Default font") font = utils::font::GetFont(fontName, fontSize);
            
            ImFont* activeFont = font ? font : ImGui::GetFont();
            
            std::vector<VitalSignsDataLink::SubgroupId_t> activeSubgroups;
            for (int i = 0; i < context.index; i++) {
                if (context.isValid[i]) {
                    auto subgroupId = context.userData[i].SubgroupId;
                    if (std::find(activeSubgroups.begin(), activeSubgroups.end(), subgroupId) == activeSubgroups.end()) {
                        activeSubgroups.push_back(subgroupId);
                    }
                }
            }
            
            if (activeSubgroups.empty())
            {
                const char* text = "10";
                ImVec2 textSize = activeFont->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, text);
                textHeight = textSize.y;
                textWidth = textSize.x;
            } 
            else
            {
                for (auto subgroupId : activeSubgroups)
                {
                    std::string str = std::to_string(subgroupId);
                    ImVec2 textSize = activeFont->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, str.c_str());
                    textHeight = ImMax(textHeight, textSize.y);
                    textWidth = ImMax(textWidth, textSize.x);
                }
            }
        }
        
        float padX = ImGui::GetStyle().FramePadding.x * 4.0f;
        float padY = ImGui::GetStyle().FramePadding.y * 2.0f + 2.0f;
        
        if (isHorizontal) {
            if (isRectOrTex) {
                return shProps.stretchToFitHeight ? (isVerticalFrame ? 0.0f : textHeight + padY) : (float)shProps.rectangle.dimensions.height;
            } else {
                return ImMax((float)shProps.line.thickness, textHeight);
            }
        } else {
            if (isRectOrTex) {
                return shProps.stretchToFitWidth ? (isVerticalFrame ? textWidth + padX : 0.0f) : (float)shProps.rectangle.dimensions.width;
            } else {
                return ImMax((float)shProps.line.thickness, textWidth);
            }
        }
    }

    DrawProperties_t CalcDrawProperties(float width, float height, const CellDrawProperties_t& cellDraw, ImDrawCornerFlags cellRoundingCorners, const GridDrawProperties_t& gridDraw, int index)
    {
        // Layout configuration
        const std::string& direction = context.layoutConfig->layout.grid.frameDirection;
        const int cellDirectionMax = (direction == "Left-to-right" || direction == "Right-to-left") ? context.layoutConfig->layout.grid.maxColumns : context.layoutConfig->layout.grid.maxRows;
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
        float offsetRow = indexRow * (cellDraw.size.y + cellDraw.spacing.y + cellDraw.padding.x);
        float offsetColumn = indexColumn * (cellDraw.size.x + cellDraw.spacing.x + cellDraw.padding.w) + (indexColumn * cellDraw.padding.y) - (indexColumn * cellDraw.padding.w);

        // Apply divider spacing
        if (context.layoutConfig->layout.grid.subgroupHeader.position == "Internal")
        {
            if ((context.layoutConfig->layout.grid.subgroupHeader.visibility == "Always show") ||  
                (context.layoutConfig->layout.grid.subgroupHeader.visibility == "Show on hover") ||
                (Addon::isSquadManagerActive))
            {
                float dividerSpacing = (float)context.layoutConfig->layout.grid.subgroupHeader.spacing;
                float dividerSize = CalcDividerSizeForLayout(*context.layoutConfig, squadDirection, direction);
                float gridSpacing = (squadDirection == "Top-to-bottom" || squadDirection == "Bottom-to-top") ? context.layoutConfig->layout.grid.spacingVertical : context.layoutConfig->layout.grid.spacingHorizontal;
                float totalSpacing = dividerSpacing + dividerSize + gridSpacing;
                
                int uniqueSubgroupCount = 0;
                int totalUniqueSubgroups = 1;
                if (context.index > 0 && context.isValid[0])
                {
                    VitalSignsDataLink::SubgroupId_t lastId = context.userData[0].SubgroupId;
                    for (int i = 1; i < context.index; i++)
                    {
                        if (context.isValid[i] && context.userData[i].SubgroupId != lastId)
                        {
                            totalUniqueSubgroups++;
                            if (i <= index) uniqueSubgroupCount++;
                            lastId = context.userData[i].SubgroupId;
                        }
                    }
                    if (index >= context.index)
                    {
                        int lastGroupIndex = (context.index - 1) / cellDirectionMax;
                        int currentGroupIndex = index / cellDirectionMax;
                        
                        if (currentGroupIndex > lastGroupIndex)
                        {
                            uniqueSubgroupCount = totalUniqueSubgroups;
                            totalUniqueSubgroups++;
                        }
                    }
                }
                else
                {
                    totalUniqueSubgroups = 0;
                }
    
                if (squadDirection == "Top-to-bottom") offsetRow += uniqueSubgroupCount * totalSpacing;
                else if (squadDirection == "Bottom-to-top") offsetRow += (totalUniqueSubgroups - 1 - uniqueSubgroupCount) * totalSpacing;
                else if (squadDirection == "Left-to-right") offsetColumn += uniqueSubgroupCount * totalSpacing;
                else if (squadDirection == "Right-to-left") offsetColumn += (totalUniqueSubgroups - 1 - uniqueSubgroupCount) * totalSpacing;
            }
        }

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

            context.isActive = isActive;
            context.isClose = false;

            // Cache layout settings locally to keep calculations readable
            const auto& gridLayout = context.layoutConfig->layout.grid;
            const auto& posConfig = context.layoutConfig->position;

            // Calculate maximum rendered bounds (in columns/rows)
            int rows, columns;
            CalcGridDimensions(context.index, gridLayout, rows, columns);

            // Convert abstract rows/cols to absolute pixel dimensions
            float menuWidth = (float)(columns * gridLayout.cellWidth + (columns - 1) * context.layoutConfig->layout.grid.spacingHorizontal);
            float menuHeight = (float)(rows * gridLayout.cellHeight + (rows - 1) * context.layoutConfig->layout.grid.spacingVertical);
            
            if (context.layoutConfig->layout.grid.subgroupHeader.position == "Internal")
            {
                if ((context.layoutConfig->layout.grid.subgroupHeader.visibility == "Always show") ||
                    (context.layoutConfig->layout.grid.subgroupHeader.visibility == "Show on hover") ||
                    (Addon::isSquadManagerActive))
                {
                    float dividerSpacing = (float)context.layoutConfig->layout.grid.subgroupHeader.spacing;
                    float dividerSize = CalcDividerSizeForLayout(*context.layoutConfig, gridLayout.squadDirection, gridLayout.frameDirection);
                    float gridSpacing = (gridLayout.squadDirection == "Top-to-bottom" || gridLayout.squadDirection == "Bottom-to-top") ? context.layoutConfig->layout.grid.spacingVertical : context.layoutConfig->layout.grid.spacingHorizontal;
                    float totalSpacing = dividerSpacing + dividerSize + gridSpacing;
    
                    int totalUniqueSubgroups = 1;
                    if (context.index > 0 && context.isValid[0])
                    {
                        VitalSignsDataLink::SubgroupId_t lastId = context.userData[0].SubgroupId;
                        for (int i = 1; i < context.index; i++)
                        {
                            if (context.isValid[i] && context.userData[i].SubgroupId != lastId)
                            {
                                totalUniqueSubgroups++;
                                lastId = context.userData[i].SubgroupId;
                            }
                        }
                    }
                    else
                    {
                        totalUniqueSubgroups = 0;
                    }

                    if (totalUniqueSubgroups > 1)
                    {
                        if (gridLayout.squadDirection == "Top-to-bottom" || gridLayout.squadDirection == "Bottom-to-top") menuHeight += (totalUniqueSubgroups - 1) * totalSpacing;
                        else menuWidth += (totalUniqueSubgroups - 1) * totalSpacing;
                    }
                }
            }
            
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

        if (trigger.category == "Party/Squad Roles")
        {
            if (trigger.effect == "Self" && VitalsData && userData.UserId == VitalsData->getClientId()) return true;
            if (trigger.effect == "Commander" && userData.SquadRole == VitalSignsDataLink::ESquadRole::Commander) return true;
            if (trigger.effect == "Lieutenant" && userData.SquadRole == VitalSignsDataLink::ESquadRole::Lieutenant) return true;
            if (trigger.effect == "Ready Check (Not Ready)" && VitalsData && VitalsData->isReadyCheckActive() && !userData.IsReady) return true;
            if (trigger.effect == "Ready Check (Ready)" && VitalsData && VitalsData->isReadyCheckActive() && userData.IsReady) return true;
            if (VitalsData && (trigger.effect == "Vote Kick (Not Voted)" || trigger.effect == "Vote Kick (Voted)"))
            {
                /* The user is the votee, and the effect reflects whether the client has voted to kick them */
                const bool isVoted = (trigger.effect == "Vote Kick (Voted)");
                for (const auto& kickVote : VitalsData->getKickVotes())
                {
                    if (kickVote.VoteeId == userData.UserId) return (kickVote.HasClientVoted == isVoted);
                }
            }
            if (trigger.effect == "Homestead Owner" && userData.IsHomesteadOwner) return true;
            return false;
        }

        if (trigger.category == "Targeting")
        {
            if (trigger.effect == "Hovered" && userData.IsHovered) return true;
            if (trigger.effect == "Selected" && userData.IsSelected) return true;
            if (trigger.effect == "Personal Target" && userData.IsPersonalTarget) return true;
            if (trigger.effect == "Party Target" && userData.IsPartyTarget) return true;
            return false;
        }

        if (trigger.category == "Assigned Roles")
        {
            if (trigger.effect == "Damage" && userData.PrimaryRole == VitalSignsDataLink::EPrimaryRole::Damage) return true;
            if (trigger.effect == "Damage (Alacrity)" && userData.PrimaryRole == VitalSignsDataLink::EPrimaryRole::DamageAlacrity) return true;
            if (trigger.effect == "Damage (Quickness)" && userData.PrimaryRole == VitalSignsDataLink::EPrimaryRole::DamageQuickness) return true;
            if (trigger.effect == "Healer (Alacrity)" && userData.PrimaryRole == VitalSignsDataLink::EPrimaryRole::HealerAlacrity) return true;
            if (trigger.effect == "Healer (Quickness)" && userData.PrimaryRole == VitalSignsDataLink::EPrimaryRole::HealerQuickness) return true;
            if (trigger.effect == "Flexible" && userData.PrimaryRole == VitalSignsDataLink::EPrimaryRole::Flexible) return true;
            if (trigger.effect == "Tank" && userData.SecondaryRole == VitalSignsDataLink::ESecondaryRole::Tank) return true;
            if (trigger.effect == "Kiter" && userData.SecondaryRole == VitalSignsDataLink::ESecondaryRole::Kiter) return true;
            if (trigger.effect == "Utility" && userData.SecondaryRole == VitalSignsDataLink::ESecondaryRole::Utility) return true;
            return false;
        }

        if (trigger.category == "Squad Markers")
        {
            if (trigger.effect == "Arrow" && userData.SquadMarker == VitalSignsDataLink::ESquadMarker::Arrow) return true;
            if (trigger.effect == "Circle" && userData.SquadMarker == VitalSignsDataLink::ESquadMarker::Circle) return true;
            if (trigger.effect == "Heart" && userData.SquadMarker == VitalSignsDataLink::ESquadMarker::Heart) return true;
            if (trigger.effect == "Square" && userData.SquadMarker == VitalSignsDataLink::ESquadMarker::Square) return true;
            if (trigger.effect == "Star" && userData.SquadMarker == VitalSignsDataLink::ESquadMarker::Star) return true;
            if (trigger.effect == "Spiral" && userData.SquadMarker == VitalSignsDataLink::ESquadMarker::Spiral) return true;
            if (trigger.effect == "Triangle" && userData.SquadMarker == VitalSignsDataLink::ESquadMarker::Triangle) return true;
            if (trigger.effect == "X" && userData.SquadMarker == VitalSignsDataLink::ESquadMarker::X) return true;
            return false;
        }

        if (trigger.category == "Health")
        {
            bool statusMatch = false;
            if (trigger.effect == "Alive" && userData.HealthType == VitalSignsDataLink::E_HEALTH_ALIVE) statusMatch = true;
            else if (trigger.effect == "Downed" && userData.HealthType == VitalSignsDataLink::E_HEALTH_DOWNED) statusMatch = true;
            else if (trigger.effect == "Defeated" && userData.HealthType == VitalSignsDataLink::E_HEALTH_DEFEATED) statusMatch = true;
            else if (trigger.effect == "Shroud (Necromancer)" && userData.HealthType == VitalSignsDataLink::E_HEALTH_SHROUD_NECROMANCER) statusMatch = true;
            else if (trigger.effect == "Shroud (Specter)" && userData.HealthType == VitalSignsDataLink::E_HEALTH_SHROUD_SPECTER) statusMatch = true;
            else if (trigger.effect == "Unknown" && userData.HealthType == VitalSignsDataLink::E_HEALTH_UNKNOWN) statusMatch = true;

            const bool isShrouded = (userData.HealthType == VitalSignsDataLink::E_HEALTH_SHROUD_NECROMANCER || userData.HealthType == VitalSignsDataLink::E_HEALTH_SHROUD_SPECTER);
            float ratio = (isShrouded && (trigger.effect == "Shroud (Necromancer)" || trigger.effect == "Shroud (Specter)")) ? userData.GetShroudRatio() : userData.GetHealthRatio();
            float healthRatio = ratio * 100.0f; // Scale to match UI values (0-100)

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
                else if (indicator.type == "Glow")
                {
                    if (IsTriggerMet(indicator.glow.trigger, userData, isPreviewed) || isTraversalForced)
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
                else if (indicator.type == "Bar")
                {
                    if (IsTriggerMet(indicator.bar.trigger, userData, isPreviewed) || isTraversalForced)
                    {
                        outDrawables.push_back({ &indicator, isPreviewed });
                        isApplied = true;
                    }
                }
                else if (indicator.type == "Threshold")
                {
                    if (IsTriggerMet(indicator.threshold.trigger, userData, isPreviewed) || isTraversalForced)
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
    
    void DrawGlow(ImDrawList* const drawList, const DrawProperties_t& properties, const GlowIndicator_t& glow)
    {
        ImVec2 p_min = properties.position;
        ImVec2 p_max = ImVec2(p_min.x + properties.width, p_min.y + properties.height);
        
        float actualThickness = glow.thickness;
        if (glow.thicknessType == "Percentage")
        {
            actualThickness = properties.height * (glow.thickness / 100.0f);
        }

        int steps = (int)actualThickness;
        if (steps <= 0) return;

        ImColor baseCol = glow.color;
        float r = baseCol.Value.x;
        float g = baseCol.Value.y;
        float b = baseCol.Value.z;
        float a = baseCol.Value.w;

        ImVec2 clip_min = p_min;
        ImVec2 clip_max = p_max;
        
        if (glow.position == "Outer")
        {
            if (glow.directionLeft) clip_min.x -= (actualThickness + 2.0f);
            if (glow.directionTop) clip_min.y -= (actualThickness + 2.0f);
            if (glow.directionRight) clip_max.x += (actualThickness + 2.0f);
            if (glow.directionBottom) clip_max.y += (actualThickness + 2.0f);
        }

        drawList->PushClipRect(clip_min, clip_max, true);

        for (int i = 0; i < steps; i++)
        {
            float progress = (float)i / steps;
            float stepAlpha = a * (1.0f - progress * (1.0f - glow.hardness));
            
            if (glow.position == "Outer")
            {
                float offset = (float)(i + 1);
                ImVec2 step_min = p_min;
                ImVec2 step_max = p_max;
                
                step_min.x -= offset;
                step_max.x += offset;
                step_min.y -= offset;
                step_max.y += offset;
                
                drawList->AddRect(step_min, step_max, ImColor(r, g, b, stepAlpha), properties.rounding + offset, properties.roundingCorners, 1.0f);
            }
            else
            {
                float offset = (float)i;
                ImVec2 step_min = p_min;
                ImVec2 step_max = p_max;
                
                step_min.x = glow.directionLeft ? (step_min.x + offset) : (step_min.x - actualThickness - 2.0f);
                step_max.x = glow.directionRight ? (step_max.x - offset) : (step_max.x + actualThickness + 2.0f);
                step_min.y = glow.directionTop ? (step_min.y + offset) : (step_min.y - actualThickness - 2.0f);
                step_max.y = glow.directionBottom ? (step_max.y - offset) : (step_max.y + actualThickness + 2.0f);
                
                drawList->AddRect(step_min, step_max, ImColor(r, g, b, stepAlpha), ImMax(0.0f, properties.rounding - offset), properties.roundingCorners, 1.0f);
            }
        }
        
        drawList->PopClipRect();
    }

    static void DrawIndicator(ImDrawList* const drawList, const DrawProperties_t& parentProperties, const DrawProperties_t& contentProperties, const Indicator_t* indicator, VitalSignsDataLink::UserData_t& userData, bool isPreviewed)
    {
        if (indicator->type == "Icon")
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
        else if (indicator->type == "Bar")
        {
            float fillRatio = 0.0f;
            if (indicator->bar.dataSource == "Health percentage")
            {
                fillRatio = userData.GetHealthRatio();
            }
            else if (indicator->bar.dataSource == "Barrier percentage")
            {
                /* Maximum barrier is equal to half of the user's maximum health */
                fillRatio = userData.GetBarrierRatio() * 2.0f;
            }
            else if (indicator->bar.dataSource == "Shroud percentage")
            {
                fillRatio = userData.GetShroudRatio();
            }
            else if (indicator->bar.dataSource == "Effect duration")
            {
                if (indicator->bar.dataMaxValue > 0.0f)
                {
                    fillRatio = (userData.Effects[indicator->bar.dataEffect].duration / 1000.0f) / indicator->bar.dataMaxValue;
                }
            }
            else if (indicator->bar.dataSource == "Effect stacks")
            {
                if (indicator->bar.dataMaxValue > 0.0f)
                {
                    fillRatio = (float)userData.Effects[indicator->bar.dataEffect].stacks / indicator->bar.dataMaxValue;
                }
            }
            fillRatio = ImClamp(fillRatio, 0.0f, 1.0f);

            ImVec2 barSize = ImVec2((float)indicator->bar.size.width, (float)indicator->bar.size.height);
            ImVec2 barPosition = CalcItemPosition(parentProperties, barSize, indicator->bar.position.anchor, indicator->bar.position.offset);

            // Floor the position to ensure crisp 1px borders
            barPosition.x = std::floor(barPosition.x);
            barPosition.y = std::floor(barPosition.y);

            DrawProperties_t barProps;
            barProps.position = barPosition;
            barProps.width = barSize.x;
            barProps.height = barSize.y;
            barProps.rounding = indicator->bar.rounding;
            barProps.roundingCorners = ImDrawCornerFlags_All;

            float borderThickness = (float)indicator->bar.borderThickness;

            DrawProperties_t bgProps = barProps;
            if (borderThickness > 0.0f)
            {
                bgProps.position.x += borderThickness;
                bgProps.position.y += borderThickness;
                bgProps.width -= borderThickness * 2.0f;
                bgProps.height -= borderThickness * 2.0f;
                bgProps.rounding = ImMax(0.0f, bgProps.rounding - borderThickness);
            }

            // Background
            DrawCell(drawList, bgProps, indicator->bar.backgroundColor);

            // Fill
            if (fillRatio > 0.0f)
            {
                DrawProperties_t fillProps = bgProps;
                fillProps.width *= fillRatio;
                fillProps.roundingCorners = (fillProps.width < (bgProps.width - bgProps.rounding)) ? ImDrawCornerFlags_Left : ImDrawCornerFlags_All;
                
                DrawCell(drawList, fillProps, indicator->bar.foregroundColor);
            }

            // Border
            if (indicator->bar.borderThickness > 0)
            {
                DrawBorder(drawList, barProps, indicator->bar.borderColor, (float)indicator->bar.borderThickness);
            }
        }
    }

    void DrawTextWithDecorators(ImDrawList* drawList, ImFont* font, float fontSize, ImVec2 pos, ImColor color, bool useShadow, ImColor shadowColor, bool useOutline, ImColor outlineColor, const std::string& text)
    {
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

    void DrawDashedLine(ImDrawList* drawList, const ImVec2& p1, const ImVec2& p2, ImU32 color, float thickness, float dashLen, float gapLen)
    {
        bool isHorizontal = (p1.y == p2.y);
        float totalLen = isHorizontal ? (p2.x - p1.x) : (p2.y - p1.y);
        for (float t = 0.0f; t < totalLen; t += dashLen + gapLen)
        {
            float currentDashLen = ImMin(dashLen, totalLen - t);
            if (isHorizontal) {
                ImVec2 rMin = ImVec2(p1.x + t, p1.y - thickness * 0.5f);
                ImVec2 rMax = ImVec2(p1.x + t + currentDashLen, p1.y + thickness * 0.5f);
                drawList->AddRectFilled(rMin, rMax, color);
            } else {
                ImVec2 rMin = ImVec2(p1.x - thickness * 0.5f, p1.y + t);
                ImVec2 rMax = ImVec2(p1.x + thickness * 0.5f, p1.y + t + currentDashLen);
                drawList->AddRectFilled(rMin, rMax, color);
            }
        }
    }

    void DrawThreshold(ImDrawList* const drawList, const DrawProperties_t& properties, const ThresholdIndicator_t& threshold)
    {
        float clampedPct = ImClamp(threshold.percentage, 0.0f, 100.0f);
        float xPos = std::floor(properties.position.x + (properties.width * (clampedPct / 100.0f)));

        float height = threshold.stretchToFitHeight ? properties.height : (float)threshold.line.length;
        if (height <= 0.0f) return;

        float yPos = std::floor(properties.position.y + (properties.height - height) * 0.5f);
        float thickness = ImMax(1.0f, (float)threshold.line.thickness);
        float xOffset = std::floor(thickness * 0.5f);

        if (threshold.line.style == "Dashed")
        {
            DrawDashedLine(drawList, ImVec2(xPos, yPos), ImVec2(xPos, yPos + height), threshold.line.color, thickness, 4.0f, 4.0f);
        }
        else if (threshold.line.style == "Dotted")
        {
            DrawDashedLine(drawList, ImVec2(xPos, yPos), ImVec2(xPos, yPos + height), threshold.line.color, thickness, thickness, 2.0f);
        }
        else
        {
            ImVec2 rMin = ImVec2(xPos - xOffset, yPos);
            ImVec2 rMax = ImVec2(xPos - xOffset + thickness, yPos + height);
            drawList->AddRectFilled(rMin, rMax, threshold.line.color);
        }
    }

    void CalcRectangleGeometry(const SubgroupHeaderProperties_t& shProps, const ImVec2& grid_p_min, const ImVec2& grid_p_max, const ImVec2& text_size, const std::string& frameDirection, float spacingVertical, float spacingHorizontal, ImVec2& out_p_min, ImVec2& out_p_max, float& out_width, float& out_height)
    {
        ImVec2 offset = ImVec2((float)shProps.offset.x, (float)shProps.offset.y);
        float padX = ImGui::GetStyle().FramePadding.x * 4.0f;
        float padY = ImGui::GetStyle().FramePadding.y * 2.0f + 2.0f;
        bool isVerticalFrame = (frameDirection == "Top-to-bottom" || frameDirection == "Bottom-to-top");

        if (shProps.stretchToFitWidth) {
            out_width = isVerticalFrame ? (grid_p_max.x - grid_p_min.x) : (text_size.x + padX);
        } else {
            out_width = (float)shProps.rectangle.dimensions.width;
        }
        
        if (shProps.stretchToFitHeight) {
            out_height = isVerticalFrame ? (text_size.y + padY) : (grid_p_max.y - grid_p_min.y);
        } else {
            out_height = (float)shProps.rectangle.dimensions.height;
        }
        
        if (shProps.anchor == "Top") {
            out_p_min = ImVec2(grid_p_min.x, grid_p_min.y - spacingVertical - out_height - shProps.rectangle.borderThickness) + offset;
        } else if (shProps.anchor == "Bottom") {
            out_p_min = ImVec2(grid_p_min.x, grid_p_max.y + spacingVertical + shProps.rectangle.borderThickness) + offset;
        } else if (shProps.anchor == "Right") {
            out_p_min = ImVec2(grid_p_max.x + spacingHorizontal + shProps.rectangle.borderThickness, grid_p_min.y) + offset;
        } else { // Left
            out_p_min = ImVec2(grid_p_min.x - spacingHorizontal - out_width - shProps.rectangle.borderThickness, grid_p_min.y) + offset;
        }
        out_p_max = ImVec2(out_p_min.x + out_width, out_p_min.y + out_height);
    }
    
    void CalcBracketGeometry(const SubgroupHeaderProperties_t& shProps, const ImVec2& grid_p_min, const ImVec2& grid_p_max, const ImVec2& text_size, const std::string& frameDirection, float spacingVertical, float spacingHorizontal, ImVec2& out_p_min, ImVec2& out_p_max, float& out_width, float& out_height)
    {
        ImVec2 offset = ImVec2((float)shProps.offset.x, (float)shProps.offset.y);
        float padX = ImGui::GetStyle().FramePadding.x * 4.0f;
        float padY = ImGui::GetStyle().FramePadding.y * 2.0f + 2.0f;
        bool isVerticalFrame = (frameDirection == "Top-to-bottom" || frameDirection == "Bottom-to-top");

        if (shProps.stretchToFitWidth) {
            out_width = isVerticalFrame ? (grid_p_max.x - grid_p_min.x) : (text_size.x + padX);
        } else {
            out_width = (float)text_size.x + padX;
        }
        
        if (shProps.stretchToFitHeight) {
            out_height = isVerticalFrame ? (text_size.y + padY) : (grid_p_max.y - grid_p_min.y);
        } else {
            out_height = (float)text_size.y + padY;
        }
        
        float thickness = (float)shProps.bracket.line.thickness;
        float margin = (float)shProps.bracket.margin;

        if (shProps.anchor == "Top") {
            out_p_min = ImVec2(grid_p_min.x + margin, grid_p_min.y - spacingVertical - out_height - thickness) + offset;
            out_width -= margin * 2.0f;
        } else if (shProps.anchor == "Bottom") {
            out_p_min = ImVec2(grid_p_min.x + margin, grid_p_max.y + spacingVertical + thickness) + offset;
            out_width -= margin * 2.0f;
        } else if (shProps.anchor == "Right") {
            out_p_min = ImVec2(grid_p_max.x + spacingHorizontal + thickness, grid_p_min.y + margin) + offset;
            out_height -= margin * 2.0f;
        } else { // Left
            out_p_min = ImVec2(grid_p_min.x - spacingHorizontal - out_width - thickness, grid_p_min.y + margin) + offset;
            out_height -= margin * 2.0f;
        }
        out_p_max = ImVec2(out_p_min.x + out_width, out_p_min.y + out_height);
    }
    
    void CalcDividerGeometry(const SubgroupHeaderProperties_t& shProps, const ImVec2& grid_p_min, const ImVec2& grid_p_max, const ImVec2& text_size, const std::string& squadDirection, const std::string& frameDirection, float spacingVertical, float spacingHorizontal, ImVec2& out_p_min, ImVec2& out_p_max, float& out_width, float& out_height)
    {
        ImVec2 offset = ImVec2((float)shProps.offset.x, (float)shProps.offset.y);
        std::string autoAnchor = "Top";
        if (squadDirection == "Top-to-bottom") autoAnchor = "Top";
        else if (squadDirection == "Bottom-to-top") autoAnchor = "Bottom";
        else if (squadDirection == "Left-to-right") autoAnchor = "Left";
        else if (squadDirection == "Right-to-left") autoAnchor = "Right";

        bool isRectOrTex = (shProps.type == "Rectangle");
        float padX = ImGui::GetStyle().FramePadding.x * 4.0f;
        float padY = ImGui::GetStyle().FramePadding.y * 2.0f + 2.0f;
        bool isVerticalFrame = (frameDirection == "Top-to-bottom" || frameDirection == "Bottom-to-top");

        if (autoAnchor == "Top" || autoAnchor == "Bottom")
        {
            out_height = isRectOrTex ? (shProps.stretchToFitHeight ? (isVerticalFrame ? (grid_p_max.y - grid_p_min.y) : (text_size.y + padY)) : (float)shProps.rectangle.dimensions.height) : ImMax((float)shProps.line.thickness, text_size.y);
            out_width = shProps.stretchToFitWidth ? (isVerticalFrame ? (text_size.x + padX) : (grid_p_max.x - grid_p_min.x)) : (isRectOrTex ? (float)shProps.rectangle.dimensions.width : (float)shProps.line.length);
            
            float alignX = 0.0f;
            if (shProps.alignment == "Centre") alignX = ((grid_p_max.x - grid_p_min.x) - out_width) * 0.5f;
            else if (shProps.alignment == "Right") alignX = (grid_p_max.x - grid_p_min.x) - out_width;
            
            float gapAlignY = ((float)shProps.spacing + 2.0f * spacingVertical) * 0.5f;
            out_p_min = (autoAnchor == "Top") ? ImVec2(grid_p_min.x + alignX, grid_p_min.y - out_height - gapAlignY) + offset : ImVec2(grid_p_min.x + alignX, grid_p_max.y + gapAlignY) + offset;
        }
        else // Left or Right
        {
            out_width = isRectOrTex ? (shProps.stretchToFitWidth ? (isVerticalFrame ? (text_size.x + padX) : (grid_p_max.x - grid_p_min.x)) : (float)shProps.rectangle.dimensions.width) : ImMax((float)shProps.line.thickness, text_size.x);
            out_height = shProps.stretchToFitHeight ? (isVerticalFrame ? (grid_p_max.y - grid_p_min.y) : (text_size.y + padY)) : (isRectOrTex ? (float)shProps.rectangle.dimensions.height : (float)shProps.line.length);
            
            float alignY = 0.0f;
            if (shProps.alignment == "Centre") alignY = ((grid_p_max.y - grid_p_min.y) - out_height) * 0.5f;
            else if (shProps.alignment == "Bottom") alignY = (grid_p_max.y - grid_p_min.y) - out_height;
            
            float gapAlignX = ((float)shProps.spacing + 2.0f * spacingHorizontal) * 0.5f;
            out_p_min = (autoAnchor == "Left") ? ImVec2(grid_p_min.x - out_width - gapAlignX, grid_p_min.y + alignY) + offset : ImVec2(grid_p_max.x + gapAlignX, grid_p_min.y + alignY) + offset;
        }
        out_p_max = ImVec2(out_p_min.x + out_width, out_p_min.y + out_height);
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

            std::string sortBy = context.layoutConfig->layout.sortBy;
            std::stable_sort(validUsers.begin(), validUsers.end(), [clientId, sortBy](const VitalSignsDataLink::UserData_t& a, const VitalSignsDataLink::UserData_t& b) {
                auto getDefaultPriority = [clientId](const VitalSignsDataLink::UserData_t& user) -> int {
                    if (user.SquadRole == VitalSignsDataLink::ESquadRole::Commander) return 0;
                    if (user.SquadRole == VitalSignsDataLink::ESquadRole::Lieutenant) return 1;
                    if (user.UserId == clientId) return 2;
                    return 3;
                };

                if (sortBy == "Roles")
                {
                    auto getRolePriority = [](const VitalSignsDataLink::UserData_t& user) -> int {
                        switch (user.PrimaryRole)
                        {
                            case VitalSignsDataLink::EPrimaryRole::HealerAlacrity: return 0;
                            case VitalSignsDataLink::EPrimaryRole::HealerQuickness: return 1;
                            case VitalSignsDataLink::EPrimaryRole::DamageAlacrity: return 2;
                            case VitalSignsDataLink::EPrimaryRole::DamageQuickness: return 3;
                            case VitalSignsDataLink::EPrimaryRole::Flexible: return 4;
                            case VitalSignsDataLink::EPrimaryRole::Damage: return 5;
                            case VitalSignsDataLink::EPrimaryRole::None:
                            default: return 6;
                        }
                    };
                    
                    int priorityA = getRolePriority(a);
                    int priorityB = getRolePriority(b);
                    
                    if (priorityA != priorityB) 
                        return priorityA < priorityB;
                        
                    return getDefaultPriority(a) < getDefaultPriority(b);
                }

                return getDefaultPriority(a) < getDefaultPriority(b);
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
        const int cellDirectionMax = (gridLayout.frameDirection == "Left-to-right" || gridLayout.frameDirection == "Right-to-left") ? gridLayout.maxColumns : gridLayout.maxRows;

        CalcGridDimensions(context.index, gridLayout, gridDrawProperties.rows, gridDrawProperties.columns);

        /* Frame properties */
        CellDrawProperties_t frameDrawProperties;
        frameDrawProperties.size = ImVec2(
            max(1.0f, static_cast<float>(context.layoutConfig->layout.grid.cellWidth - (2 * context.layoutConfig->layout.itemBorder))), 
            max(1.0f, static_cast<float>(context.layoutConfig->layout.grid.cellHeight - (2 * context.layoutConfig->layout.itemBorder))));
        frameDrawProperties.padding = _ImVec4(static_cast<float>(context.layoutConfig->layout.itemBorder));
        frameDrawProperties.spacing = ImVec2(static_cast<float>(context.layoutConfig->layout.grid.spacingHorizontal + context.layoutConfig->layout.itemBorder),
                                             static_cast<float>(context.layoutConfig->layout.grid.spacingVertical + context.layoutConfig->layout.itemBorder));
        frameDrawProperties.rounding = ImMax(context.layoutConfig->layout.grid.cellRounding, context.layoutConfig->layout.grid.cellRounding - context.layoutConfig->layout.itemBorder);

        /* Frame border properties */
        CellDrawProperties_t borderDrawProperties;
        borderDrawProperties.size = ImVec2(
            static_cast<float>(context.layoutConfig->layout.grid.cellWidth), 
            static_cast<float>(context.layoutConfig->layout.grid.cellHeight));
        borderDrawProperties.padding = _ImVec4(0.f);
        borderDrawProperties.spacing = ImVec2(static_cast<float>(context.layoutConfig->layout.grid.spacingHorizontal), static_cast<float>(context.layoutConfig->layout.grid.spacingVertical));
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
        int squadMax = (gridLayout.squadDirection == "Left-to-right" || gridLayout.squadDirection == "Right-to-left") ? gridLayout.maxColumns : gridLayout.maxRows;
        lastPopulatedGroupIndex = ImMin(lastPopulatedGroupIndex, squadMax - 1);
        
        int newGroupIndex = lastPopulatedGroupIndex + 1;
        
        if ((newGroupIndex < squadMax) && 
            ((newGroupIndex * cellDirectionMax) < MAX_GRID_CELLS))
        {
            lastPopulatedGroupIndex = newGroupIndex;
        }
        
        auto getSubgroupId = [&](int idx) -> VitalSignsDataLink::SubgroupId_t {
            return (idx == newGroupIndex) ? static_cast<VitalSignsDataLink::SubgroupId_t>(-1) : context.userData[idx * cellDirectionMax].SubgroupId;
        };

        /* Draw drop targets and highlights */
        SubgroupHeaderProperties_t& shProps = context.layoutConfig->layout.grid.subgroupHeader;

        /// TODO: Bit of a hack to sanitise the anchor position before rendering. Ideally this should not be required.
        if (gridLayout.squadDirection == "Left-to-right" || gridLayout.squadDirection == "Right-to-left")
        {
            if (shProps.anchor != "Top" && shProps.anchor != "Bottom") 
            {
                shProps.anchor = "Top";
            }
        }
        else
        {
            if (shProps.anchor != "Left" && shProps.anchor != "Right")
            {
                shProps.anchor = "Left";
            }
        }
        
        float effectiveFontSize = ImGui::GetIO().FontDefault->FontSize;
        if (shProps.textStyle.fontSizeSource == "Default font size")
        {
            effectiveFontSize = (float)ConfigText.fontSize;
        }
        else if (shProps.textStyle.fontSizeSource == "Custom font size")
        {
            effectiveFontSize = (float)shProps.textStyle.fontSize;
        }
        ImColor effectiveColor = (shProps.textStyle.colorSource == "Custom color") ? shProps.textStyle.color : ConfigText.color;
        bool effectiveShadow = (shProps.textStyle.decoratorSource == "Custom decorators") ? shProps.textStyle.shadow : ConfigText.shadow;
        ImColor effectiveShadowColor = (shProps.textStyle.decoratorSource == "Custom decorators") ? shProps.textStyle.shadowColor : ConfigText.shadowColor;
        bool effectiveOutline = (shProps.textStyle.decoratorSource == "Custom decorators") ? shProps.textStyle.outline : ConfigText.outline;
        ImColor effectiveOutlineColor = (shProps.textStyle.decoratorSource == "Custom decorators") ? shProps.textStyle.outlineColor : ConfigText.outlineColor;

        std::string effectiveFontSource = (shProps.textStyle.fontSource == "Default font") ? ConfigText.fontSource : shProps.textStyle.fontSource;
        std::string effectiveFontName = (shProps.textStyle.fontSource == "Default font") ? ConfigText.font : shProps.textStyle.font;

        ImFont* font = nullptr;
        if (effectiveFontSource != "Nexus font" && effectiveFontSource != "Default font")
        {
            font = utils::font::GetFont(effectiveFontName, effectiveFontSize);
        }

        ImFont* activeFont = font ? font : ImGui::GetFont();
        
        ImVec2 max_text_size = ImVec2(0.0f, 0.0f);
        for (int i = 1; i <= 15; i++)
        {
            ImVec2 s = activeFont->CalcTextSizeA(effectiveFontSize, FLT_MAX, 0.0f, std::to_string(i).c_str());
            max_text_size.x = ImMax(max_text_size.x, s.x);
            max_text_size.y = ImMax(max_text_size.y, s.y);
        }
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
            
            ImVec2 grid_p_min(ImMin(firstCellProps.position.x, lastCellProps.position.x), ImMin(firstCellProps.position.y, lastCellProps.position.y));
            ImVec2 grid_p_max(ImMax(firstCellProps.position.x + firstCellProps.width, lastCellProps.position.x + lastCellProps.width), ImMax(firstCellProps.position.y + firstCellProps.height, lastCellProps.position.y + lastCellProps.height));
            
            ImVec2 p_min = grid_p_min;
            ImVec2 p_max = grid_p_max;
            p_min.x -= (context.layoutConfig->layout.grid.spacingHorizontal / 2.f);
            p_min.y -= (context.layoutConfig->layout.grid.spacingVertical / 2.f);
            p_max.x += (context.layoutConfig->layout.grid.spacingHorizontal / 2.f);
            p_max.y += (context.layoutConfig->layout.grid.spacingVertical / 2.f);

            ImGui::SetCursorScreenPos(p_min);
            ImGui::PushID(groupIndex);
            ImGui::InvisibleButton("SubgroupTarget", ImVec2(p_max.x - p_min.x, p_max.y - p_min.y));
            ImGui::SetItemAllowOverlap();

            // Squad Manager: Subgroup header
            bool showHeader = Addon::isSquadManagerActive || context.layoutConfig->layout.grid.subgroupHeader.visibility == "Always show";
            if (context.layoutConfig->layout.grid.subgroupHeader.visibility == "Show on hover" && !showHeader)
            {
                showHeader = ImGui::IsMouseHoveringRect(p_min, p_max, false);
            }

            if (showHeader && droppedSubgroupId != static_cast<VitalSignsDataLink::SubgroupId_t>(-1))
            {
                std::string headerText = std::to_string(droppedSubgroupId + 1);
                ImVec2 text_size = activeFont->CalcTextSizeA(effectiveFontSize, FLT_MAX, 0.0f, headerText.c_str());

                ImVec2 header_p_min, header_p_max;
                float headerWidth = 0.0f, headerHeight = 0.0f;

                if (shProps.position == "External")
                {
                    if (shProps.type == "Bracket")
                    {
                        CalcBracketGeometry(shProps, grid_p_min, grid_p_max, max_text_size, gridLayout.frameDirection, (float)context.layoutConfig->layout.grid.spacingVertical, (float)context.layoutConfig->layout.grid.spacingHorizontal, header_p_min, header_p_max, headerWidth, headerHeight);
                        
                        ImU32 brktCol = shProps.bracket.line.color;
                        float thickness = (float)shProps.bracket.line.thickness;
                        float outerArmLength = (float)shProps.bracket.outerArmLength;
                        float innerArmLength = (float)shProps.bracket.innerArmLength;

                        std::vector<std::pair<ImVec2, ImVec2>> segments;

                        if (shProps.anchor == "Left") {
                            segments.push_back({ImVec2(header_p_max.x, header_p_min.y), ImVec2(header_p_max.x - outerArmLength, header_p_min.y)}); // Top arm length
                            segments.push_back({ImVec2(header_p_max.x - outerArmLength, header_p_min.y), ImVec2(header_p_max.x - outerArmLength, header_p_max.y)}); // Main line
                            segments.push_back({ImVec2(header_p_max.x - outerArmLength, header_p_max.y), ImVec2(header_p_max.x, header_p_max.y)}); // Bottom arm length
                            if (innerArmLength > 0) {
                                float midY = header_p_min.y + headerHeight * 0.5f;
                                segments.push_back({ImVec2(header_p_max.x - outerArmLength, midY), ImVec2(header_p_max.x - outerArmLength - innerArmLength, midY)}); // Inner arm length
                            }
                        } else if (shProps.anchor == "Right") {
                            segments.push_back({ImVec2(header_p_min.x, header_p_min.y), ImVec2(header_p_min.x + outerArmLength, header_p_min.y)}); // Top arm length
                            segments.push_back({ImVec2(header_p_min.x + outerArmLength, header_p_min.y), ImVec2(header_p_min.x + outerArmLength, header_p_max.y)}); // Main line
                            segments.push_back({ImVec2(header_p_min.x + outerArmLength, header_p_max.y), ImVec2(header_p_min.x, header_p_max.y)}); // Bottom arm length
                            if (innerArmLength > 0) {
                                float midY = header_p_min.y + headerHeight * 0.5f;
                                segments.push_back({ImVec2(header_p_min.x + outerArmLength, midY), ImVec2(header_p_min.x + outerArmLength + innerArmLength, midY)}); // Inner arm length
                            }
                        } else if (shProps.anchor == "Top") {
                            segments.push_back({ImVec2(header_p_min.x, header_p_max.y), ImVec2(header_p_min.x, header_p_max.y - outerArmLength)}); // Left arm length
                            segments.push_back({ImVec2(header_p_min.x, header_p_max.y - outerArmLength), ImVec2(header_p_max.x, header_p_max.y - outerArmLength)}); // Main line
                            segments.push_back({ImVec2(header_p_max.x, header_p_max.y - outerArmLength), ImVec2(header_p_max.x, header_p_max.y)}); // Right arm length
                            if (innerArmLength > 0) {
                                float midX = header_p_min.x + headerWidth * 0.5f;
                                segments.push_back({ImVec2(midX, header_p_max.y - outerArmLength), ImVec2(midX, header_p_max.y - outerArmLength - innerArmLength)}); // Inner arm length
                            }
                        } else if (shProps.anchor == "Bottom") {
                            segments.push_back({ImVec2(header_p_min.x, header_p_min.y), ImVec2(header_p_min.x, header_p_min.y + outerArmLength)}); // Left arm length
                            segments.push_back({ImVec2(header_p_min.x, header_p_min.y + outerArmLength), ImVec2(header_p_max.x, header_p_min.y + outerArmLength)}); // Main line
                            segments.push_back({ImVec2(header_p_max.x, header_p_min.y + outerArmLength), ImVec2(header_p_max.x, header_p_min.y)}); // Right arm length
                            if (innerArmLength > 0) {
                                float midX = header_p_min.x + headerWidth * 0.5f;
                                segments.push_back({ImVec2(midX, header_p_min.y + outerArmLength), ImVec2(midX, header_p_min.y + outerArmLength + innerArmLength)}); // Inner arm length
                            }
                        }

                        ImU32 shadowCol = shProps.bracket.shadowColor;
                        ImU32 outlineCol = shProps.bracket.outlineColor;
                        for (const auto& seg : segments) {
                            if (shProps.bracket.shadow) {
                                drawList->AddLine(ImVec2(seg.first.x + 1.0f, seg.first.y + 1.0f), ImVec2(seg.second.x + 1.0f, seg.second.y + 1.0f), shadowCol, thickness);
                            }
                            if (shProps.bracket.outline) {
                                drawList->AddLine(seg.first, seg.second, outlineCol, thickness + 2.0f);
                            }
                        }
                        for (const auto& seg : segments) {
                            drawList->AddLine(seg.first, seg.second, brktCol, thickness);
                        }
                    }
                    else if (shProps.type == "Rectangle")
                    {
                        CalcRectangleGeometry(shProps, grid_p_min, grid_p_max, max_text_size, gridLayout.frameDirection, (float)context.layoutConfig->layout.grid.spacingVertical, (float)context.layoutConfig->layout.grid.spacingHorizontal, header_p_min, header_p_max, headerWidth, headerHeight);

                        ImU32 bgCol = shProps.rectangle.color;
                        float rounding = (float)shProps.rectangle.rounding;
                        
                        drawList->AddRectFilled(header_p_min, header_p_max, bgCol, rounding);
                        if (shProps.rectangle.borderThickness > 0)
                        {
                            drawList->AddRect(header_p_min, header_p_max, shProps.rectangle.borderColor, rounding, ImDrawCornerFlags_All, (float)shProps.rectangle.borderThickness);
                        }
                    }
                }
                else if (shProps.position == "Internal")
                {
                    CalcDividerGeometry(shProps, grid_p_min, grid_p_max, max_text_size, gridLayout.squadDirection, gridLayout.frameDirection, (float)context.layoutConfig->layout.grid.spacingVertical, (float)context.layoutConfig->layout.grid.spacingHorizontal, header_p_min, header_p_max, headerWidth, headerHeight);
                    
                    if (shProps.type == "Line")
                    {
                        std::string autoAnchor = "Top";
                        if (gridLayout.squadDirection == "Top-to-bottom") autoAnchor = "Top";
                        else if (gridLayout.squadDirection == "Bottom-to-top") autoAnchor = "Bottom";
                        else if (gridLayout.squadDirection == "Left-to-right") autoAnchor = "Left";
                        else if (gridLayout.squadDirection == "Right-to-left") autoAnchor = "Right";
                        
                        bool isHorizontal = (autoAnchor == "Top" || autoAnchor == "Bottom");
                        ImU32 divCol = shProps.line.color;
                        float thickness = (float)shProps.line.thickness;

                        if (shProps.line.style == "Solid")
                        {
                            if (isHorizontal) {
                                float center_y = header_p_min.y + headerHeight * 0.5f;
                                drawList->AddRectFilled(ImVec2(header_p_min.x, center_y - thickness * 0.5f), ImVec2(header_p_max.x, center_y + thickness * 0.5f), divCol);
                            } else {
                                float center_x = header_p_min.x + headerWidth * 0.5f;
                                drawList->AddRectFilled(ImVec2(center_x - thickness * 0.5f, header_p_min.y), ImVec2(center_x + thickness * 0.5f, header_p_max.y), divCol);
                            }
                        }
                        else
                        {
                            ImVec2 p1 = isHorizontal ? ImVec2(header_p_min.x, header_p_min.y + headerHeight * 0.5f) : ImVec2(header_p_min.x + headerWidth * 0.5f, header_p_min.y);
                            ImVec2 p2 = isHorizontal ? ImVec2(header_p_max.x, header_p_min.y + headerHeight * 0.5f) : ImVec2(header_p_min.x + headerWidth * 0.5f, header_p_max.y);
                            float dashLen = (shProps.line.style == "Dotted") ? thickness : thickness * 3.f;
                            DrawDashedLine(drawList, p1, p2, divCol, thickness, dashLen, dashLen);
                        }
                    }
                    else if (shProps.type == "Rectangle")
                    {
                        ImU32 divCol = shProps.rectangle.color;
                        float rounding = (float)shProps.rectangle.rounding;
                        drawList->AddRectFilled(header_p_min, header_p_max, divCol, rounding);
                        if (shProps.rectangle.borderThickness > 0)
                        {
                            drawList->AddRect(header_p_min, header_p_max, shProps.rectangle.borderColor, rounding, ImDrawCornerFlags_All, (float)shProps.rectangle.borderThickness);
                        }
                    }
                }

                DrawProperties_t headerProps = {header_p_min, headerWidth, headerHeight, 0, ImDrawCornerFlags_All};
                ImVec2 startPosition = CalcItemPosition(headerProps, max_text_size, shProps.textPosition.anchor, shProps.textPosition.offset);
                
                // Enforce centered vertical/horizontal position next to the bracket
                if (shProps.position == "External" && shProps.type == "Bracket")
                {
                    float outerArmLength = (float)shProps.bracket.outerArmLength;
                    float innerArmLength = (float)shProps.bracket.innerArmLength;
                    ImVec2 offset = ImVec2((float)shProps.textPosition.offset.x, (float)shProps.textPosition.offset.y);
                    
                    if (shProps.anchor == "Left") {
                        float padding = (float)context.layoutConfig->layout.grid.spacingHorizontal;
                        startPosition = ImVec2(header_p_max.x - outerArmLength - innerArmLength - max_text_size.x - padding, header_p_min.y + (headerHeight - max_text_size.y) * 0.5f) + offset;
                    } else if (shProps.anchor == "Right") {
                        float padding = (float)context.layoutConfig->layout.grid.spacingHorizontal;
                        startPosition = ImVec2(header_p_min.x + outerArmLength + innerArmLength + padding, header_p_min.y + (headerHeight - max_text_size.y) * 0.5f) + offset;
                    } else if (shProps.anchor == "Top") {
                        float padding = (float)context.layoutConfig->layout.grid.spacingVertical;
                        startPosition = ImVec2(header_p_min.x + (headerWidth - max_text_size.x) * 0.5f, header_p_max.y - outerArmLength - innerArmLength - max_text_size.y - padding) + offset;
                    } else if (shProps.anchor == "Bottom") {
                        float padding = (float)context.layoutConfig->layout.grid.spacingVertical;
                        startPosition = ImVec2(header_p_min.x + (headerWidth - max_text_size.x) * 0.5f, header_p_min.y + outerArmLength + innerArmLength + padding) + offset;
                    }
                }
                
                ImVec2 textPosition = ImVec2(startPosition.x + (max_text_size.x - text_size.x) * 0.5f, startPosition.y + (max_text_size.y - text_size.y) * 0.5f);
                DrawTextWithDecorators(drawList, font, effectiveFontSize, textPosition, effectiveColor, effectiveShadow, effectiveShadowColor, effectiveOutline, effectiveOutlineColor, headerText);
            }
            else if (droppedSubgroupId == static_cast<VitalSignsDataLink::SubgroupId_t>(-1))
            {
                bool allowDragDrop = Addon::isSquadManagerActive || context.layoutConfig->layout.grid.squadManager.allowDragDropWithoutKeybind;
                const ImGuiPayload* payload = ImGui::GetDragDropPayload();
                bool isDraggingUser = (payload != nullptr && payload->IsDataType("VS_USER_ID"));
                bool showEmptySlot = Addon::isSquadManagerActive || (allowDragDrop && isDraggingUser);
                if (showEmptySlot)
                {
                    ImVec2 cell_p_min = firstCellProps.position;
                    ImVec2 cell_p_max = ImVec2(firstCellProps.position.x + firstCellProps.width, firstCellProps.position.y + firstCellProps.height);
                    ImU32 col = ImColor(255, 255, 255, 255);
                    float thickness = ImMax(1.0f, (float)context.layoutConfig->layout.itemBorder);
                    float dashLen = 4.0f;
                    float gapLen = 4.0f;
                    
                    // Ensure p1 < p2 for DrawDashedLine logic
                    DrawDashedLine(drawList, ImVec2(cell_p_min.x, cell_p_min.y), ImVec2(cell_p_max.x, cell_p_min.y), col, thickness, dashLen, gapLen); // Top
                    DrawDashedLine(drawList, ImVec2(cell_p_max.x, cell_p_min.y), ImVec2(cell_p_max.x, cell_p_max.y), col, thickness, dashLen, gapLen); // Right
                    DrawDashedLine(drawList, ImVec2(cell_p_min.x, cell_p_max.y), ImVec2(cell_p_max.x, cell_p_max.y), col, thickness, dashLen, gapLen); // Bottom
                    DrawDashedLine(drawList, ImVec2(cell_p_min.x, cell_p_min.y), ImVec2(cell_p_min.x, cell_p_max.y), col, thickness, dashLen, gapLen); // Left
                }
            }

            // Squad Manager: Drag-and-drop (drop)
            bool allowDragDrop = Addon::isSquadManagerActive || context.layoutConfig->layout.grid.squadManager.allowDragDropWithoutKeybind;
            if (allowDragDrop && ImGui::BeginDragDropTarget())
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

        ImDrawListSplitter splitter;
        splitter.Split(drawList, 2);

        /* Draw items */
        for (int i = 0; i < context.index; i++)
        {
            if (!context.isValid[i])
            {
                continue;
            }

            // Skip rendering items outside the bounds
            int cellDirectionMax = (context.layoutConfig->layout.grid.frameDirection == "Left-to-right" || context.layoutConfig->layout.grid.frameDirection == "Right-to-left") ? context.layoutConfig->layout.grid.maxColumns : context.layoutConfig->layout.grid.maxRows;
            int squadMax = (context.layoutConfig->layout.grid.squadDirection == "Left-to-right" || context.layoutConfig->layout.grid.squadDirection == "Right-to-left") ? context.layoutConfig->layout.grid.maxColumns : context.layoutConfig->layout.grid.maxRows;
            int current_s = i / cellDirectionMax;
            if (current_s >= squadMax)
            {
                continue; 
            }

            VitalSignsDataLink::UserData_t& userData = context.userData[i];

            ImGui::PushID(userData.AccountName.c_str());
            {
                drawables.clear();

                DrawProperties_t parentProperties = CalcDrawProperties(frameDrawProperties.size.x, frameDrawProperties.size.y, frameDrawProperties, ImDrawCornerFlags_All, gridDrawProperties, i);
    
                /* Invisible button (creates clickable region) */
                ImGui::SetCursorScreenPos(parentProperties.position);
                ImGui::InvisibleButton("", ImVec2(parentProperties.width, parentProperties.height));

                if (ImGui::IsItemHovered())
                {
                    ImGui::BeginTooltip();
                    {
                        ImVec2 cursorPos = ImGui::GetCursorScreenPos();
                        float radius = ImGui::GetTextLineHeight() * 0.35f;
                        ImVec2 center = ImVec2(cursorPos.x + radius, cursorPos.y + ImGui::GetTextLineHeight() * 0.5f);
                        
                        ImColor statusColor;
                        switch(userData.OnlineStatus)
                        {
                            case GW2RE::EOnlineStatus::Online: statusColor = ImColor(50, 205, 50, 255); break;
                            case GW2RE::EOnlineStatus::Away: statusColor = ImColor(255, 165, 0, 255); break;
                            case GW2RE::EOnlineStatus::LFG: statusColor = ImColor(34, 153, 238, 255); break;
                            case GW2RE::EOnlineStatus::Offline:
                            default: statusColor = ImColor(170, 170, 170, 255); break;
                        }
                        
                        ImGui::GetWindowDrawList()->AddCircleFilled(center, radius, statusColor);
                        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + radius * 2.0f + ImGui::GetStyle().ItemSpacing.x);
                        ImGui::Text(userData.GetAccountName().c_str());
                        ImGui::TextDisabled(userData.GetOnlineStatusName().c_str());
    
                        if (!userData.CharacterName.empty())
                        {
                            ImGui::Separator();
                            ImGui::Text(userData.GetDisplayName().c_str());
                            ImGui::Text("Level %u %s", userData.Level, VitalSignsDataLink::getProfessionShortString(userData.Profession, userData.Specialisation));
                            if (!userData.MapName.empty())
                            {
                                ImGui::Text(userData.MapName.c_str());
                            }
                        }
                    }
                    ImGui::EndTooltip();
                }

                /* Right-click context menu */
                if (ImGui::BeginPopupContextItem())
                {
                    const auto clientRole = VitalsData->getUserData(clientId).SquadRole;
                    const auto clientMap = VitalsData->getUserData(clientId).MapName;

                    /* User permissions */
                    const bool isNotClient = (clientId != userData.UserId);
                    const bool isInSquad = (VitalsData->getGroupType() == VitalSignsDataLink::E_GROUP_SQUAD_10) ||
                                           (VitalsData->getGroupType() == VitalSignsDataLink::E_GROUP_SQUAD_50);
                    const bool isInParty = (VitalsData->getGroupType() == VitalSignsDataLink::E_GROUP_PARTY);
                    const bool canSetRoles = isInSquad || isInParty;
                    const bool canSetSquadMarker = isInSquad && ((clientRole == VitalSignsDataLink::ESquadRole::Commander) || (clientRole == VitalSignsDataLink::ESquadRole::Lieutenant));
                    const bool canSetTarget = isNotClient && userData.IsInInstance;

                    /* Social features */
                    if (isNotClient)
                    {
                        if (ImGui::MenuItem("Whisper")) { VitalsData->whisperUser(userData.UserId); }
                        if (ImGui::MenuItem("Send Mail")) { VitalsData->sendMail(userData.AccountName); }
                        if (userData.IsInInstance)
                        {
                            if (ImGui::MenuItem("Inspect Cosmetics")) { VitalsData->inspectCosmetics(userData.UserId); }
                        }
                        if (clientMap == userData.MapName && !userData.IsInInstance)
                        {
                            if (ImGui::MenuItem("Join in Instance")) { VitalsData->joinUserInstance(userData.UserId); }
                        }
                        if (userData.IsHomesteadOwner && !userData.IsInInstance)
                        {
                            if (ImGui::MenuItem("Join in Homestead")) { VitalsData->joinUserHomestead(userData.UserId); }
                        }

                        ImGui::Separator();
                        if (userData.ContactType != VitalSignsDataLink::EContactType::Friend)
                        {
                            if (ImGui::MenuItem("Add Friend")) { VitalsData->addFriend(userData.UserId); }
                        }
                        else
                        {
                            if (ImGui::MenuItem("Remove Friend")) { VitalsData->removeFriend(userData.UserId); }
                        }
                        if (userData.ContactType != VitalSignsDataLink::EContactType::Blocked)
                        {
                            if (ImGui::MenuItem("Block")) { VitalsData->blockUser(userData.UserId); }
                        }
                        else
                        {
                            if (ImGui::MenuItem("Unblock")) { VitalsData->unblockUser(userData.UserId); }
                        }
                    }

                    /* Roles and markers */
                    if (canSetRoles || canSetSquadMarker || canSetTarget)
                    {
                        ImGui::Separator();
                        if (canSetRoles)
                        {
                            if (ImGui::BeginMenu("Set Primary Role"))
                            {
                                if (ImGui::MenuItem("Damage", nullptr, userData.PrimaryRole == VitalSignsDataLink::EPrimaryRole::Damage)) { VitalsData->setPrimaryRole(userData.UserId, VitalSignsDataLink::EPrimaryRole::Damage); }
                                if (ImGui::MenuItem("Damage (Alacrity)", nullptr, userData.PrimaryRole == VitalSignsDataLink::EPrimaryRole::DamageAlacrity)) { VitalsData->setPrimaryRole(userData.UserId, VitalSignsDataLink::EPrimaryRole::DamageAlacrity); }
                                if (ImGui::MenuItem("Damage (Quickness)", nullptr, userData.PrimaryRole == VitalSignsDataLink::EPrimaryRole::DamageQuickness)) { VitalsData->setPrimaryRole(userData.UserId, VitalSignsDataLink::EPrimaryRole::DamageQuickness); }
                                if (ImGui::MenuItem("Healer (Alacrity)", nullptr, userData.PrimaryRole == VitalSignsDataLink::EPrimaryRole::HealerAlacrity)) { VitalsData->setPrimaryRole(userData.UserId, VitalSignsDataLink::EPrimaryRole::HealerAlacrity); }
                                if (ImGui::MenuItem("Healer (Quickness)", nullptr, userData.PrimaryRole == VitalSignsDataLink::EPrimaryRole::HealerQuickness)) { VitalsData->setPrimaryRole(userData.UserId, VitalSignsDataLink::EPrimaryRole::HealerQuickness); }
                                if (ImGui::MenuItem("Flexible", nullptr, userData.PrimaryRole == VitalSignsDataLink::EPrimaryRole::Flexible)) { VitalsData->setPrimaryRole(userData.UserId, VitalSignsDataLink::EPrimaryRole::Flexible); }
                                if (ImGui::MenuItem("None", nullptr, userData.PrimaryRole == VitalSignsDataLink::EPrimaryRole::None)) { VitalsData->setPrimaryRole(userData.UserId, VitalSignsDataLink::EPrimaryRole::None); }
                                ImGui::EndMenu();
                            }
                            if (ImGui::BeginMenu("Set Secondary Role"))
                            {
                                if (ImGui::MenuItem("Tank", nullptr, userData.SecondaryRole == VitalSignsDataLink::ESecondaryRole::Tank)) { VitalsData->setSecondaryRole(userData.UserId, VitalSignsDataLink::ESecondaryRole::Tank); }
                                if (ImGui::MenuItem("Kiter", nullptr, userData.SecondaryRole == VitalSignsDataLink::ESecondaryRole::Kiter)) { VitalsData->setSecondaryRole(userData.UserId, VitalSignsDataLink::ESecondaryRole::Kiter); }
                                if (ImGui::MenuItem("Utility", nullptr, userData.SecondaryRole == VitalSignsDataLink::ESecondaryRole::Utility)) { VitalsData->setSecondaryRole(userData.UserId, VitalSignsDataLink::ESecondaryRole::Utility); }
                                if (ImGui::MenuItem("None", nullptr, userData.SecondaryRole == VitalSignsDataLink::ESecondaryRole::None)) { VitalsData->setSecondaryRole(userData.UserId, VitalSignsDataLink::ESecondaryRole::None); }
                                ImGui::EndMenu();
                            }
                        }
                        if (canSetSquadMarker)
                        {
                            if (ImGui::BeginMenu("Set Squad Marker"))
                            {
                                if (ImGui::MenuItem("Arrow", nullptr, userData.SquadMarker == VitalSignsDataLink::ESquadMarker::Arrow)) { VitalsData->setMarker(userData.Agent, VitalSignsDataLink::ESquadMarker::Arrow); }
                                if (ImGui::MenuItem("Circle", nullptr, userData.SquadMarker == VitalSignsDataLink::ESquadMarker::Circle)) { VitalsData->setMarker(userData.Agent, VitalSignsDataLink::ESquadMarker::Circle); }
                                if (ImGui::MenuItem("Heart", nullptr, userData.SquadMarker == VitalSignsDataLink::ESquadMarker::Heart)) { VitalsData->setMarker(userData.Agent, VitalSignsDataLink::ESquadMarker::Heart); }
                                if (ImGui::MenuItem("Square", nullptr, userData.SquadMarker == VitalSignsDataLink::ESquadMarker::Square)) { VitalsData->setMarker(userData.Agent, VitalSignsDataLink::ESquadMarker::Square); }
                                if (ImGui::MenuItem("Star", nullptr, userData.SquadMarker == VitalSignsDataLink::ESquadMarker::Star)) { VitalsData->setMarker(userData.Agent, VitalSignsDataLink::ESquadMarker::Star); }
                                if (ImGui::MenuItem("Spiral", nullptr, userData.SquadMarker == VitalSignsDataLink::ESquadMarker::Spiral)) { VitalsData->setMarker(userData.Agent, VitalSignsDataLink::ESquadMarker::Spiral); }
                                if (ImGui::MenuItem("Triangle", nullptr, userData.SquadMarker == VitalSignsDataLink::ESquadMarker::Triangle)) { VitalsData->setMarker(userData.Agent, VitalSignsDataLink::ESquadMarker::Triangle); }
                                if (ImGui::MenuItem("X", nullptr, userData.SquadMarker == VitalSignsDataLink::ESquadMarker::X)) { VitalsData->setMarker(userData.Agent, VitalSignsDataLink::ESquadMarker::X); }
                                ImGui::EndMenu();
                            }
                        }
                        if (canSetTarget)
                        {
                            if (ImGui::BeginMenu("Set Target"))
                            {
                                /* Selecting the current personal target clears it; the game cannot clear a party target */
                                if (ImGui::MenuItem("Personal Target", nullptr, userData.IsPersonalTarget)) { VitalsData->setPersonalTarget(userData.UserId, !userData.IsPersonalTarget); }
                                if (ImGui::MenuItem("Party Target", nullptr, userData.IsPartyTarget, !userData.IsPartyTarget)) { VitalsData->setPartyTarget(userData.UserId); }
                                ImGui::EndMenu();
                            }
                        }
                    }

                    /* Squad management */
                    if (isInSquad)
                    {
                        ImGui::Separator();
                        if (isNotClient)
                        {
                            if (clientRole == VitalSignsDataLink::ESquadRole::Commander)
                            {
                                if (userData.IsCommanderUnlocked)
                                {
                                    if (ImGui::MenuItem("Appoint Squad Leader")) { VitalsData->setCommander(userData.UserId); }
                                }
                                if (userData.SquadRole == VitalSignsDataLink::ESquadRole::None)
                                {
                                    if (ImGui::MenuItem("Appoint Lieutenant")) { VitalsData->setLieutenant(userData.UserId, true); }
                                }
                                if (userData.SquadRole == VitalSignsDataLink::ESquadRole::Lieutenant)
                                {
                                    if (ImGui::MenuItem("Demote Lieutenant")) { VitalsData->setLieutenant(userData.UserId, false); }
                                }
                            }
                            if (!VitalsData->hasSquadCommander())
                            {
                                /* Without a commander kicks are decided by vote */
                                if (VitalsData->canCancelKickVote(userData.UserId))
                                {
                                    if (ImGui::MenuItem("Cancel Vote to Kick from Squad")) { VitalsData->cancelKickVote(userData.UserId); }
                                }
                                else
                                {
                                    if (ImGui::MenuItem("Vote to Kick from Squad")) { VitalsData->kickUser(userData.UserId); }
                                }
                            }
                            else if ((clientRole == VitalSignsDataLink::ESquadRole::Commander) ||
                                     ((clientRole == VitalSignsDataLink::ESquadRole::Lieutenant) && (userData.SquadRole == VitalSignsDataLink::ESquadRole::None)))
                            {
                                /* Lieutenants cannot kick the commander or other lieutenants */
                                if (ImGui::MenuItem("Kick from Squad")) { VitalsData->kickUser(userData.UserId); }
                            }
                        }
                        if (ImGui::MenuItem("Leave Squad")) { VitalsData->leaveGroup(); }
                    }

                    /* Party management */
                    if (isInParty)
                    {
                        ImGui::Separator();
                        if (isNotClient)
                        {
                            if (VitalsData->canCancelKickVote(userData.UserId))
                            {
                                if (ImGui::MenuItem("Cancel Vote to Kick from Party")) { VitalsData->cancelKickVote(userData.UserId); }
                            }
                            else
                            {
                                if (ImGui::MenuItem("Vote to Kick from Party")) { VitalsData->kickUser(userData.UserId); }
                            }
                        }
                        if (ImGui::MenuItem("Leave Party")) { VitalsData->leaveGroup(); }
                    }

                    /* Report */
                    if (isNotClient)
                    {
                        ImGui::Separator();
                        if (ImGui::MenuItem("Report")) { VitalsData->reportUser(userData.UserId); }
                    }

                    ImGui::EndPopup();
                }

                const bool isHovered = IsItemHovered(parentProperties);
                const ImColor backgroundColour = GetBackgroundColour(context.colourPresets, context.layoutConfig->colors, userData.HealthType);
                
                const bool isShrouded = (userData.HealthType == VitalSignsDataLink::E_HEALTH_SHROUD_NECROMANCER || userData.HealthType == VitalSignsDataLink::E_HEALTH_SHROUD_SPECTER);
                const std::string& shroudMode = context.layoutConfig->layout.grid.playerFrames.shroudMode;

                VitalSignsDataLink::E_HEALTH_TYPE effectiveHealthType = userData.HealthType;
                float effectiveHealthRatio = userData.GetHealthRatio();
                if (isShrouded)
                {
                    if (shroudMode == "None" || shroudMode == "Split View")
                    {
                        effectiveHealthType = VitalSignsDataLink::E_HEALTH_ALIVE;
                    }
                    else // "Default"
                    {
                        effectiveHealthRatio = userData.GetShroudRatio();
                    }
                }
                ImColor healthColour = GetHealthColour(context.colourPresets, context.layoutConfig->colors, effectiveHealthType, effectiveHealthRatio, userData.Profession, userData.PrimaryRole);
                const ImColor barrierColour = GetBarrierColour(context.colourPresets, context.layoutConfig->colors);
                const Indicator_t* borderStyle = nullptr;
    
                /* Process indicators (returns a list of drawable items) */
                ProcessIndicatorsDFS(context.layoutConfig->indicators, userData, false, false, &healthColour, &borderStyle, drawables);
                if ((context.layoutConfig->previewNodeId != TreeNodeUID::NONE) && (context.layoutConfig->previewNodeId == context.layoutConfig->id))
                {
                    /* Preview mode */
                    ProcessIndicatorsDFS(context.layoutConfig->indicators, userData, false, true, &healthColour, &borderStyle, drawables);
                }

                // Evaluate Frame State Presets for current cell
                std::vector<FrameStatePreset_t*> activePresets;
                if (userData.SquadRole == VitalSignsDataLink::ESquadRole::Commander) activePresets.push_back(&FrameStatePresets.commander);
                if (VitalsData && userData.UserId == VitalsData->getClientId()) activePresets.push_back(&FrameStatePresets.self);
                if (isHovered) activePresets.push_back(&FrameStatePresets.hovered);
                if (userData.IsSelected) activePresets.push_back(&FrameStatePresets.selected);

                /* Border */
                DrawProperties_t borderProps = CalcDrawProperties(borderDrawProperties.size.x, borderDrawProperties.size.y, borderDrawProperties, ImDrawCornerFlags_All, gridDrawProperties, i);
                float borderThickness = 0.0f;
                
                const BorderIndicator_t* activeBorder = nullptr;
                for (auto preset : activePresets)
                {
                    if (preset->borderOverride) activeBorder = &preset->border;
                }

                if (borderStyle)
                {
                    activeBorder = &borderStyle->border;
                }

                // Channel 0: Outer Glows (rendered underneath the cell background)
                splitter.SetCurrentChannel(drawList, 0);
                for (auto preset : activePresets)
                {
                    if (preset->glowEnabled && preset->glow.position == "Outer")
                    {
                        DrawGlow(drawList, borderProps, preset->glow);
                    }
                }

                for (const auto& pair : drawables)
                {
                    if (pair.first->type == "Glow" && pair.first->glow.position == "Outer")
                    {
                        DrawGlow(drawList, borderProps, pair.first->glow);
                    }
                }

                // Channel 1: Cell Content
                splitter.SetCurrentChannel(drawList, 1);

                if (activeBorder)
                {
                    if (activeBorder->thickness >= 1)
                    {
                        borderThickness = (float)activeBorder->thickness;
                        DrawBorder(drawList, borderProps, activeBorder->color, borderThickness);
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
                contentProps.rounding = ImMax(0.0f, contentProps.rounding - borderThickness);
                
                /* Background */
                DrawCell(drawList, contentProps, backgroundColour);
                
                /* Health, Shroud & Barrier */
                if (isShrouded && shroudMode == "Split View")
                {
                    /* Split View: Top half is Health, Bottom half is Shroud with Barrier */
                    float topHeight = std::floor(contentProps.height * 0.5f);
                    float bottomHeight = contentProps.height - topHeight;

                    DrawProperties_t topProps = contentProps;
                    topProps.height = topHeight;

                    DrawProperties_t bottomProps = contentProps;
                    bottomProps.position.y += topHeight;
                    bottomProps.height = bottomHeight;

                    /* Top Half: Health */
                    float health = userData.GetHealthRatio();
                    if (health > 0.001f)
                    {
                        float healthWidth = topProps.width * health;
                        ImDrawCornerFlags roundingCorners = ((topProps.width - healthWidth) < (float)topProps.rounding) ? ImDrawCornerFlags_Top : ImDrawCornerFlags_TopLeft;
                        DrawProperties_t properties = topProps;
                        properties.width = healthWidth;
                        properties.roundingCorners = roundingCorners;

                        DrawCell(drawList, properties, healthColour);
                    }

                    /* Bottom Half: Shroud */
                    float shroud = userData.GetShroudRatio();
                    float barrier = userData.GetBarrierRatio();
                    ImColor shroudColour = GetHealthColour(context.colourPresets, context.layoutConfig->colors, userData.HealthType, shroud, userData.Profession, userData.PrimaryRole);

                    if (shroud > 0.001f)
                    {
                        float shroudWidth = bottomProps.width * shroud;
                        ImDrawCornerFlags roundingCorners = ((bottomProps.width - shroudWidth) < (float)bottomProps.rounding) ? ImDrawCornerFlags_Bot : ImDrawCornerFlags_BotLeft;
                        DrawProperties_t properties = bottomProps;
                        properties.width = shroudWidth;
                        properties.roundingCorners = roundingCorners;

                        DrawCell(drawList, properties, shroudColour);
                    }

                    /* Bottom Half: Barrier (accompanying Shroud) */
                    if (barrier > 0.001f)
                    {
                        float barrierWidth = bottomProps.width * barrier;
                        DrawProperties_t properties = bottomProps;
                        properties.width = barrierWidth;

                        if ((barrier + shroud) > 0.999f)
                        {
                            /* Overflow barrier */
                            properties.position.x += bottomProps.width - barrierWidth;
                            properties.roundingCorners = ImDrawCornerFlags_BotRight;
                            DrawCell(drawList, properties, barrierColour);
                        }
                        else
                        {
                            /* Padding barrier */
                            float shroudWidth = bottomProps.width * shroud;
                            ImDrawCornerFlags roundingCorners = ((shroudWidth + barrierWidth) < (bottomProps.width - (float)bottomProps.rounding)) ? ImDrawCornerFlags_None : ImDrawCornerFlags_BotRight;
                            properties.position.x += shroudWidth;
                            properties.roundingCorners = roundingCorners;
                            DrawCell(drawList, properties, barrierColour);
                        }
                    }
                }
                else
                {
                    /* Single Bar View: Default (switches to Shroud if shrouded) or None / Non-shrouded (Health) */
                    float displayedResource = (isShrouded && (shroudMode != "None")) ? userData.GetShroudRatio() : userData.GetHealthRatio();
                    float barrier = userData.GetBarrierRatio();

                    if (displayedResource > 0.001f)
                    {
                        float resourceWidth = contentProps.width * displayedResource;
                        ImDrawCornerFlags roundingCorners = ((contentProps.width - resourceWidth) < (float)contentProps.rounding) ? ImDrawCornerFlags_All : ImDrawCornerFlags_Left;
                        DrawProperties_t properties = contentProps;
                        properties.width = resourceWidth;
                        properties.roundingCorners = roundingCorners;

                        DrawCell(drawList, properties, healthColour);
                    }

                    /* Barrier */
                    if (barrier > 0.001f)
                    {
                        float barrierWidth = contentProps.width * barrier;
                        DrawProperties_t properties = contentProps;
                        properties.width = barrierWidth;

                        if ((barrier + displayedResource) > 0.999f)
                        {
                            /* Overflow barrier */
                            properties.position.x += contentProps.width - barrierWidth;
                            properties.roundingCorners = ImDrawCornerFlags_Right;
                            DrawCell(drawList, properties, barrierColour);
                        }
                        else
                        {
                            /* Padding barrier */
                            float resourceWidth = contentProps.width * displayedResource;
                            ImDrawCornerFlags roundingCorners = ((resourceWidth + barrierWidth) < (contentProps.width - (float)contentProps.rounding)) ? ImDrawCornerFlags_None : ImDrawCornerFlags_Right;
                            properties.position.x += resourceWidth;
                            properties.roundingCorners = roundingCorners;
                            DrawCell(drawList, properties, barrierColour);
                        }
                    }
                }
    
                /* Threshold */
                for (const auto& pair : drawables)
                {
                    if (pair.first->type == "Threshold")
                    {
                        DrawThreshold(drawList, contentProps, pair.first->threshold);
                    }
                }

                // Inner glows
                for (auto preset : activePresets)
                {
                    if (preset->glowEnabled && preset->glow.position == "Inner")
                    {
                        DrawGlow(drawList, contentProps, preset->glow);
                    }
                }

                for (const auto& pair : drawables)
                {
                    if (pair.first->type == "Glow" && pair.first->glow.position == "Inner")
                    {
                        DrawGlow(drawList, contentProps, pair.first->glow);
                    }
                }

                /* Indicators */
                for (const auto& pair : drawables)
                {
                    DrawIndicator(drawList, parentProperties, contentProps, pair.first, userData, pair.second);
                }

                // Overlays
                for (auto preset : activePresets)
                {
                    if (preset->overlayEnabled)
                    {
                        DrawCell(drawList, contentProps, preset->overlayColor);
                    }
                }

                /* Squad Manager: Drag-and-drop (drag) */
                bool allowDragDrop = Addon::isSquadManagerActive || context.layoutConfig->layout.grid.squadManager.allowDragDropWithoutKeybind;
                if (allowDragDrop && ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
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
        }

        splitter.Merge(drawList);
        drawList->PopClipRect();

        // Squad Manager: Move and re-size
        bool allowMove = Addon::isSquadManagerActive || context.layoutConfig->layout.grid.squadManager.allowMoveWithoutKeybind;
        bool allowResize = Addon::isSquadManagerActive || context.layoutConfig->layout.grid.squadManager.allowResizeWithoutKeybind;

        if (allowMove || allowResize)
        {
            // Compute first cell for move grip
            DrawProperties_t firstCell = CalcDrawProperties(frameDrawProperties.size.x, frameDrawProperties.size.y, frameDrawProperties, ImDrawCornerFlags_All, gridDrawProperties, 0);

            // Compute last possible cell for resize grip based on current squadMax and cellDirectionMax
            int cellDirectionMax = (context.layoutConfig->layout.grid.frameDirection == "Left-to-right" || context.layoutConfig->layout.grid.frameDirection == "Right-to-left") ? context.layoutConfig->layout.grid.maxColumns : context.layoutConfig->layout.grid.maxRows;
            int squadMax = (context.layoutConfig->layout.grid.squadDirection == "Left-to-right" || context.layoutConfig->layout.grid.squadDirection == "Right-to-left") ? context.layoutConfig->layout.grid.maxColumns : context.layoutConfig->layout.grid.maxRows;
            int max_index = (squadMax * cellDirectionMax) - 1;
            DrawProperties_t lastCell = CalcDrawProperties(frameDrawProperties.size.x, frameDrawProperties.size.y, frameDrawProperties, ImDrawCornerFlags_All, gridDrawProperties, max_index);

            ImDrawList* fgDrawList = ImGui::GetForegroundDrawList();

            ImVec2 border_min = ImVec2(ImMin(firstCell.position.x, lastCell.position.x) - 2.0f, ImMin(firstCell.position.y, lastCell.position.y) - 2.0f);
            ImVec2 border_max = ImVec2(ImMax(firstCell.position.x + firstCell.width, lastCell.position.x + lastCell.width) + 2.0f, ImMax(firstCell.position.y + firstCell.height, lastCell.position.y + lastCell.height) + 2.0f);

            // Move Grip
            if (allowMove)
            {
                float moveGripWidth = 22.0f;
                float moveGripHeight = 24.0f;
                ImVec2 moveGripMin;
                std::string cellDir = context.layoutConfig->layout.grid.frameDirection;
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
            }

            // Resize Grip
            if (allowResize)
            {
                static ImVec2 resize_drag_accum(0.0f, 0.0f);

                auto drawDashedLine = [&](ImVec2 p1, ImVec2 p2, ImU32 color, float thickness, float dashLength, float gapLength) {
                    float dx = p2.x - p1.x;
                    float dy = p2.y - p1.y;
                    float length = std::sqrt(dx*dx + dy*dy);
                    if (length == 0.0f) return;
                    float nx = dx / length;
                    float ny = dy / length;
                    for (float t = 0.0f; t < length; t += dashLength + gapLength) {
                        float t_end = ImMin(t + dashLength, length);
                        fgDrawList->AddLine(ImVec2(p1.x + nx * t, p1.y + ny * t), ImVec2(p1.x + nx * t_end, p1.y + ny * t_end), color, thickness);
                    }
                };

                // Draw resize border
                ImU32 dashColor = ImColor(200, 200, 200, 150);
                drawDashedLine(border_min, ImVec2(border_max.x, border_min.y), dashColor, 1.0f, 4.0f, 4.0f);
                drawDashedLine(ImVec2(border_max.x, border_min.y), border_max, dashColor, 1.0f, 4.0f, 4.0f);
                drawDashedLine(border_max, ImVec2(border_min.x, border_max.y), dashColor, 1.0f, 4.0f, 4.0f);
                drawDashedLine(ImVec2(border_min.x, border_max.y), border_min, dashColor, 1.0f, 4.0f, 4.0f);
                
                float resizeGripSize = 24.0f;
                ImVec2 resizeGripMax(border_max.x + 6.0f, border_max.y + 6.0f);
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

                    float cellTotalWidth = frameDrawProperties.size.x + frameDrawProperties.spacing.x + frameDrawProperties.padding.w;
                    float cellTotalHeight = frameDrawProperties.size.y + frameDrawProperties.spacing.y + frameDrawProperties.padding.x;
                    if (std::abs(resize_drag_accum.x) > cellTotalWidth)
                    {
                        int drag_units = (int)(resize_drag_accum.x / cellTotalWidth);
                        context.layoutConfig->layout.grid.maxColumns = ImMax(1, ImMin((int)UI::SQUAD_MEMBER_LIMIT, context.layoutConfig->layout.grid.maxColumns + drag_units));
                        resize_drag_accum.x -= drag_units * cellTotalWidth;
                    }
                    
                    if (std::abs(resize_drag_accum.y) > cellTotalHeight)
                    {
                        int drag_units = (int)(resize_drag_accum.y / cellTotalHeight);
                        context.layoutConfig->layout.grid.maxRows = ImMax(1, ImMin((int)UI::SQUAD_MEMBER_LIMIT, context.layoutConfig->layout.grid.maxRows + drag_units));
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

        int cellDirectionMax = (context.layoutConfig->layout.grid.frameDirection == "Left-to-right" || context.layoutConfig->layout.grid.frameDirection == "Right-to-left") ? context.layoutConfig->layout.grid.maxColumns : context.layoutConfig->layout.grid.maxRows;
        bool isPreview = (context.layoutConfig->previewNodeId != TreeNodeUID::NONE) && (context.layoutConfig->previewNodeId == context.layoutConfig->id);

        /* Skip trailing empty cells */
        if (context.isItemPending && !isPreview && Addon::isSquadManagerActive)
        {
            auto users = VitalsData->getUsers();
            if (users.find(context.userData[context.index].SubgroupId) == users.end())
            {
                if (context.index % cellDirectionMax != 0)
                {
                    context.index = ((context.index / cellDirectionMax) + 1) * cellDirectionMax;
                }
            }
        }
        
        if ((context.index > 0) && (context.userData[context.index - 1].SubgroupId != userData.SubgroupId))
        {
            if (context.index % cellDirectionMax != 0)
            {
                context.index = ((context.index / cellDirectionMax) + 1) * cellDirectionMax;
            }
        }

        int current_s = context.index / cellDirectionMax;

        if (context.index < MAX_GRID_CELLS)
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

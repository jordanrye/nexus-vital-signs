#include "ui_radial.h"

#include "imgui/imgui_internal.h"
#include "imgui_extensions.h"

#include "shared.h"
#include "ui_common.h"

namespace UI::Radial {
    
    static struct RadialContext_t
    {
        // Configuration
        Position_t positionConfig;
        Layout_t layoutConfig;
        std::string palette;
        ColourPresets_t colourPresets;

        // Menu state
        int lastFrameCount;
        bool isActive;
        bool isItemPending = false;
        bool isClose;
        ImVec2 menuPosition;
        int index;
        int indexHovered;

        // Item state
        std::string charName[SQUAD_MEMBER_LIMIT];
        VitalSignsData::EProfession profession[SQUAD_MEMBER_LIMIT];
        float health[SQUAD_MEMBER_LIMIT];
        VitalSignsData::E_HEALTH_TYPE healthType[SQUAD_MEMBER_LIMIT];
        float barrier[SQUAD_MEMBER_LIMIT];
        VitalSignsData::Effects_t effects[SQUAD_MEMBER_LIMIT];
    } context;

    struct SectorDrawProperties_t 
    {
        float centralAngle;
        float radiusMin;
        float radiusMax;
        int arcSegments;
        float thetaMinInner;
        float thetaMaxInner;
        float thetaMinOuter;
        float thetaMaxOuter;
    };

    bool IsItemHovered(SectorDrawProperties_t& properties)
    {
        bool isHovered = false;

        if (context.isActive)
        {
            /* Calculate drag distance */
            const ImVec2 mousePos = ImGui::GetIO().MousePos;
            const ImVec2 dragDist = ImVec2((mousePos.x - context.menuPosition.x), (mousePos.y - context.menuPosition.y));
            const float dragDistSqr = (dragDist.x*dragDist.x) + (dragDist.y*dragDist.y);
            const float minDragDistSqr = (context.layoutConfig.radial.sectorRadiusInner * context.layoutConfig.radial.sectorRadiusInner);

            if (dragDistSqr >= minDragDistSqr)
            {
                /* Calculate drag angle */
                float dragAngle = atan2f(dragDist.y, dragDist.x);
                if (dragAngle < (-0.5f * properties.centralAngle))
                {
                    dragAngle += (2.0f * IM_PI);
                }

                /* Determine hovered status */
                if ((dragAngle >= properties.thetaMinInner) && (dragAngle < properties.thetaMaxInner))
                {
                    isHovered = true;
                }
            }
        }

        return isHovered;
    }

    void DrawSector(ImDrawList * const drawList, SectorDrawProperties_t properties, ImVec2 position, ImColor colour)
    {
        const ImVec2& texUvWhitePixel = ImGui::GetDrawListSharedData()->TexUvWhitePixel;
        const float thetaStepInner = (properties.thetaMaxInner - properties.thetaMinInner) / properties.arcSegments;
        const float thetaStepOuter = (properties.thetaMaxOuter - properties.thetaMinOuter) / properties.arcSegments;

        drawList->PrimReserve(properties.arcSegments * 6, (properties.arcSegments + 1) * 2);

        for (int i = 0; i <= properties.arcSegments; i++)
        {
            float cosInner = cosf(properties.thetaMinInner + thetaStepInner * i);
            float sinInner = sinf(properties.thetaMinInner + thetaStepInner * i);
            float cosOuter = cosf(properties.thetaMinOuter + thetaStepOuter * i);
            float sinOuter = sinf(properties.thetaMinOuter + thetaStepOuter * i);
            
            if (i < properties.arcSegments)
            {
                drawList->PrimWriteIdx(drawList->_VtxCurrentIdx + 0);
                drawList->PrimWriteIdx(drawList->_VtxCurrentIdx + 2);
                drawList->PrimWriteIdx(drawList->_VtxCurrentIdx + 1);
                drawList->PrimWriteIdx(drawList->_VtxCurrentIdx + 3);
                drawList->PrimWriteIdx(drawList->_VtxCurrentIdx + 2);
                drawList->PrimWriteIdx(drawList->_VtxCurrentIdx + 1);
            }
            
            drawList->PrimWriteVtx(ImVec2(position.x + cosInner * properties.radiusMin, position.y + sinInner * properties.radiusMin), texUvWhitePixel, colour);
            drawList->PrimWriteVtx(ImVec2(position.x + cosOuter * properties.radiusMax, position.y + sinOuter * properties.radiusMax), texUvWhitePixel, colour);
        }
    }

    void DrawIcon(ImDrawList * const drawList, SectorDrawProperties_t properties, ImVec2 position, Texture* texture)
    {
        if (nullptr != texture)
        {
            /* Calculate texture size. Divide the annulus width by sqrt(2) to get the
             * leg, which we've used for the size of the icon to ensure that it never 
             * exceeds the width/height of the parent sector. */
            static const float sqrt2 = std::sqrt(2);
            float leg = (properties.radiusMax - properties.radiusMin) / sqrt2;
            ImVec2 textureSize(leg, leg);
            
            /* Calculate texture position */
            float cosInner = cosf((properties.thetaMinInner + properties.thetaMaxInner) * 0.5f);
            float sinInner = sinf((properties.thetaMinInner + properties.thetaMaxInner) * 0.5f);
            float radiusMidPoint = (properties.radiusMin + properties.radiusMax) * 0.5f;
            ImVec2 sectorOffset((cosInner * radiusMidPoint), (sinInner * radiusMidPoint));
            ImVec2 texturePosition(
                (position.x + sectorOffset.x - (textureSize.x * 0.5f)), 
                (position.y + sectorOffset.y - (textureSize.y * 0.5f)));
            
            drawList->AddImage(texture->Resource, texturePosition, ImVec2(texturePosition.x + textureSize.x, texturePosition.y + textureSize.y));
        }
    }

    void DrawText(ImDrawList * const drawList, SectorDrawProperties_t properties, ImVec2 position, std::string text, ImColor colour, ImVec2 offset = { 0, 0 })
    {
        /* Calculate text size */
        ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
        
        /* Calculate text position */
        float cosInner = cosf((properties.thetaMinInner + properties.thetaMaxInner) * 0.5f);
        float sinInner = sinf((properties.thetaMinInner + properties.thetaMaxInner) * 0.5f);
        float radiusMidPoint = (properties.radiusMin + properties.radiusMax) * 0.5f;
        ImVec2 sectorOffset((cosInner * radiusMidPoint), (sinInner * radiusMidPoint));
        ImVec2 textPosition(
            (position.x + sectorOffset.x - (textSize.x * 0.5f) + offset.x), 
            (position.y + sectorOffset.y - (textSize.y * 0.5f) + offset.y));

        drawList->AddText(textPosition, colour, text.c_str());
    }
            
    SectorDrawProperties_t CalcSectorProperties(float centralAngle, int arcSegments, float radiusMin, float radiusMax, int itemSpacing, int index, int childIndex = 0U, int childCount = 0U)
    {
        /* Normalised angular spacing. This is not a direct angle in radians, but rather a 
         * factor that when multiplied by `centralAngle` provides an angular offset. */
        const float innerSpacing = itemSpacing / radiusMin / 2;
        const float outerSpacing = itemSpacing / radiusMax / 2;

        const float thetaMinInner = centralAngle * (index - 0.5f + innerSpacing);
        const float thetaMaxInner = centralAngle * (index + 0.5f - innerSpacing);
        const float thetaMinOuter = centralAngle * (index - 0.5f + outerSpacing);
        const float thetaMaxOuter = centralAngle * (index + 0.5f - outerSpacing);
        
        const float thetaStepInner = (thetaMaxInner - thetaMinInner) / ImMax(1, childCount);
        const float thetaStepOuter = (thetaMaxOuter - thetaMinOuter) / ImMax(1, childCount);

        SectorDrawProperties_t properties{};
        properties.centralAngle = centralAngle / ImMax(1, childCount);
        properties.radiusMin = radiusMin;
        properties.radiusMax = radiusMax;
        properties.arcSegments = (arcSegments / ImMax(1, childCount)) + 2;
        properties.thetaMinInner = thetaMinInner + ((childIndex + 0) * thetaStepInner);
        properties.thetaMaxInner = thetaMinInner + ((childIndex + 1) * thetaStepInner);
        properties.thetaMinOuter = thetaMinOuter + ((childIndex + 0) * thetaStepOuter);
        properties.thetaMaxOuter = thetaMinOuter + ((childIndex + 1) * thetaStepOuter);

        return properties;
    }

    bool BeginRadialMenu(const char* name, const Position_t& position, const Layout_t& layout, const std::string& palette, const ColourPresets_t& colours, bool isActive)
    {
        bool isOpen = false;
        bool isAlwaysDisplayed = ((layout.visibility == 0) || (layout.visibility == 1));

        if (isActive || isAlwaysDisplayed || context.isItemPending)
        {
            context.positionConfig = position;
            context.layoutConfig = layout;
            context.palette = palette;
            context.colourPresets = colours;

            context.isActive = isActive;
            context.isClose = false;

            if (ImGui::Begin("##HealingRadial", nullptr, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoBackground|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoCollapse))
            {
                if (!context.isActive)
                {
                    // Disable interaction for everything within this block
                    ImGui::BeginDisabled();

                    // Override opacity
                    context.colourPresets.COLOUR_BACKGROUND.Value.w *= context.layoutConfig.inactiveOpacity;
                    context.colourPresets.COLOUR_HEALTH.Value.w *= context.layoutConfig.inactiveOpacity;
                    context.colourPresets.COLOUR_HEALTH_DOWNED.Value.w *= context.layoutConfig.inactiveOpacity;
                    context.colourPresets.COLOUR_HEALTH_DEFEATED.Value.w *= context.layoutConfig.inactiveOpacity;
                    context.colourPresets.COLOUR_SHROUD_NECROMANCER.Value.w *= context.layoutConfig.inactiveOpacity;
                    context.colourPresets.COLOUR_SHROUD_SPECTER.Value.w *= context.layoutConfig.inactiveOpacity;
                    context.colourPresets.COLOUR_BARRIER.Value.w *= context.layoutConfig.inactiveOpacity;
                    context.colourPresets.COLOUR_HOVERED.Value.w *= context.layoutConfig.inactiveOpacity;
                    
                    context.colourPresets.COLOUR_BOONS_1.Value.w *= context.layoutConfig.inactiveOpacity;
                    context.colourPresets.COLOUR_BOONS_2.Value.w *= context.layoutConfig.inactiveOpacity;
                    context.colourPresets.COLOUR_CONDITIONS_1.Value.w *= context.layoutConfig.inactiveOpacity;
                    context.colourPresets.COLOUR_CONDITIONS_2.Value.w *= context.layoutConfig.inactiveOpacity;
                    
                    context.isItemPending = false;
                }

                int frameCount = ImGui::GetFrameCount();
                if ((context.lastFrameCount < (frameCount - 1)) || isAlwaysDisplayed)
                {
                    /* Fixed position */
                    context.menuPosition = ImVec2(
                        (ImGui::GetIO().DisplaySize.x / 2.f + context.positionConfig.offset.x), 
                        (ImGui::GetIO().DisplaySize.y / 2.f + context.positionConfig.offset.y));

                    /* Cursor position */
                    // context.menuPosition = ImGui::GetIO().MousePos;
                }
                context.lastFrameCount = frameCount;
                
                context.index = 0;
                if (context.isActive)
                {
                    context.indexHovered = -1;
                }

                isOpen = true;
            }
            else
            {
                if (!context.isActive)
                {
                    ImGui::EndDisabled();
                }
                ImGui::End();
            }
        }

        return isOpen;
    }

    void EndRadialMenu()
    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->PushClipRectFullScreen();
        
        /* Radial properties */
        const float centralAngle = (2 * IM_PI) / ImMax(context.layoutConfig.radial.sectorCountMin, context.index);
        const int arcSegments = (int)(64 * centralAngle / (2 * IM_PI)) + 1;
        const int itemSpacing = context.layoutConfig.itemSpacing;
        const float annulusRadiusMin = context.layoutConfig.radial.sectorRadiusInner + itemSpacing;
        const float annulusRadiusMax = context.layoutConfig.radial.sectorRadiusOuter - itemSpacing;
        const float annulusWidth = annulusRadiusMax - annulusRadiusMin;

        /* Draw items */
        for (int i = 0; i < context.index; i++)
        {
            SectorDrawProperties_t properties = CalcSectorProperties(centralAngle, arcSegments, annulusRadiusMin, annulusRadiusMax, itemSpacing, i);
            bool isHovered = IsItemHovered(properties);
            
            /* Background */
            DrawSector(drawList, properties, context.menuPosition, context.colourPresets.COLOUR_BACKGROUND);
            
            /* Health */
            if (context.health[i] > 0.001f)
            {
                float healthRadiusMin = annulusRadiusMin;
                float healthRadiusMax = annulusRadiusMin + (annulusWidth * context.health[i]);
                SectorDrawProperties_t healthProperties = CalcSectorProperties(centralAngle, arcSegments, healthRadiusMin, healthRadiusMax, itemSpacing, i);
                DrawSector(drawList, healthProperties, context.menuPosition, GetHealthColour(context.colourPresets, context.palette, context.healthType[i], context.profession[i]));

                if (isHovered)
                {
                    DrawSector(drawList, healthProperties, context.menuPosition, context.colourPresets.COLOUR_HOVERED);
                }
            }
            
            /* Barrier */
            if (context.barrier[i] > 0.001f)
            {
                if ((context.barrier[i] + context.health[i]) > 0.999f)
                {
                    /* Overflow barrier */
                    float barrierRadiusMin = annulusRadiusMax - (annulusWidth * context.barrier[i]);
                    float barrierRadiusMax = annulusRadiusMax;
                    SectorDrawProperties_t barrierProperties = CalcSectorProperties(centralAngle, arcSegments, barrierRadiusMin, barrierRadiusMax, itemSpacing, i);
                    DrawSector(drawList, barrierProperties, context.menuPosition, context.colourPresets.COLOUR_BARRIER);
                }
                else
                {
                    /* Padding barrier */
                    float barrierRadiusMin = annulusRadiusMin + (annulusWidth * context.health[i]); /* healthRadiusMax */
                    float barrierRadiusMax = barrierRadiusMin + (annulusWidth * context.barrier[i]);
                    SectorDrawProperties_t barrierProperties = CalcSectorProperties(centralAngle, arcSegments, barrierRadiusMin, barrierRadiusMax, itemSpacing, i);
                    DrawSector(drawList, barrierProperties, context.menuPosition, context.colourPresets.COLOUR_BARRIER);
                }
            }

            /* Character name */
            DrawText(drawList, properties, context.menuPosition, context.charName[i], ConfigText.shadowColor, ImVec2(1, 1));
            DrawText(drawList, properties, context.menuPosition, context.charName[i], ConfigText.color);

            /** FIXME: Re-implement Boons and Conditions to use an IconList for RadialLayout. */
            // /* Boons */
            // {
            //     int j = 0;
            //     bool alternateColour = true;
                
            //     for (const auto& boon : context.effects[i].Boons)
            //     {
            //         alternateColour = !alternateColour;
                    
            //         ImColor boonColour = alternateColour ? context.colourPresets.COLOUR_BOONS_1 : context.colourPresets.COLOUR_BOONS_2;
            //         float boonRadiusMin = annulusRadiusMax;
            //         float boonRadiusMax = annulusRadiusMax + (annulusRadiusMax * (centralAngle / ImMax(1, BOON_LIMIT)));
            //         SectorDrawProperties_t boonProperties = CalcSectorProperties(centralAngle, arcSegments, boonRadiusMin, boonRadiusMax, itemSpacing, i, j, BOON_LIMIT);
            //         DrawSector(drawList, boonProperties, context.menuPosition, boonColour);
            //         DrawIcon(drawList, boonProperties, context.menuPosition, Textures[boon.second.name]);
    
            //         j++;
            //     }
            // }

            // /* Conditions */
            // {
            //     int j = 0;
            //     bool alternateColour = true;

            //     for (const auto& condition : context.effects[i].Conditions)
            //     {
            //         alternateColour = !alternateColour;
    
            //         ImColor conditionColour = alternateColour ? context.colourPresets.COLOUR_CONDITIONS_1 : context.colourPresets.COLOUR_CONDITIONS_2;
            //         float conditionRadiusMin = annulusRadiusMax + (annulusRadiusMax * (centralAngle / ImMax(1, BOON_LIMIT))); /* boonRadiusMax */
            //         float conditionRadiusMax = conditionRadiusMin + (conditionRadiusMin * (centralAngle / ImMax(1, CONDITION_LIMIT)));
            //         SectorDrawProperties_t conditionProperties = CalcSectorProperties(centralAngle, arcSegments, conditionRadiusMin, conditionRadiusMax, itemSpacing, i, j, CONDITION_LIMIT);
            //         DrawSector(drawList, conditionProperties, context.menuPosition, conditionColour);
            //         DrawIcon(drawList, conditionProperties, context.menuPosition, Textures[condition.second.name]);
    
            //         j++;
            //     }
            // }

            if (isHovered)
            {
                context.indexHovered = i;

                context.isItemPending = true;
            }
        }

        drawList->PopClipRect();

        if (context.isClose || !context.isActive)
        {
            context.indexHovered = -1;
            
            if (!context.isActive)
            {
                ImGui::EndDisabled();
            }
        }

        ImGui::End();
    }

    bool RadialMenuItem(const char* name, VitalSignsData::EProfession profession, float health, VitalSignsData::E_HEALTH_TYPE healthType, float barrier, VitalSignsData::Effects_t& effects)
    {
        bool isSelected = false;

        if (context.index < context.layoutConfig.radial.sectorCountMax)
        {
            context.charName[context.index] = std::string(name);
            context.profession[context.index] = profession;
            context.health[context.index] = health;
            context.healthType[context.index] = healthType;
            context.barrier[context.index] = barrier;
            context.effects[context.index] = effects;
    
            if (context.index == context.indexHovered)
            {
                isSelected = true;
                context.isClose = true;
            }
    
            context.index++;
        }

        return isSelected;
    }

} // namespace UI::Radial

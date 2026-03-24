#include "json_addon_serialisers.h"

#include "json_serialisers.h"
#include "json_imgui_serialisers.h"
#include "shared.h"
#include "utilities.h"

void dser_Size_t(json& object, Size_t& size)
{
    if (!object.is_null())
    {
        dser_BasicType(object["width"], size.width);
        dser_BasicType(object["height"], size.height);
    }
}

void dser_Coordinate_t(json& object, Coordinate_t& coordinate)
{
    if (!object.is_null())
    {
        dser_BasicType(object["x"], coordinate.x);
        dser_BasicType(object["y"], coordinate.y);
    }
}

void dser_Position_t(json& object, Position_t& position)
{
    if (!object.is_null())
    {
        dser_BasicType(object["anchor"], position.anchor);
        dser_Coordinate_t(object["offset"], position.offset);
    }
}

void dser_GridProperties_t(json& object, GridProperties_t& properties)
{
    if (!object.is_null())
    {
        dser_BasicType(object["cell-direction"], properties.cellDirection);
        dser_BasicType(object["cell-direction-max"], properties.cellDirectionMax);
        dser_BasicType(object["cell-max"], properties.cellMax);
        dser_BasicType(object["cell-width"], properties.cellWidth);
        dser_BasicType(object["cell-height"], properties.cellHeight);
        dser_BasicType(object["cell-rounding"], properties.cellRounding);
    }
}

void dser_RadialProperties_t(json& object, RadialProperties_t& properties)
{
    if (!object.is_null())
    {
        dser_BasicType(object["radius-inner"], properties.sectorRadiusInner);
        dser_BasicType(object["radius-outer"], properties.sectorRadiusOuter);
        dser_BasicType(object["sector-min"], properties.sectorCountMin);
        dser_BasicType(object["sector-max"], properties.sectorCountMax);
    }
}

void dser_Layout_t(json& object, Layout_t& layout)
{
    if (!object.is_null())
    {
        dser_BasicType(object["type"], layout.type);
        dser_GridProperties_t(object["grid"], layout.grid);
        dser_RadialProperties_t(object["radial"], layout.radial);
        dser_BasicType(object["border"], layout.itemBorder);
        dser_BasicType(object["spacing"], layout.itemSpacing);
        dser_BasicType(object["visibility"], layout.visibility);
        dser_BasicType(object["inactive-opacity"], layout.inactiveOpacity);
    }
}

void dser_GeneralConfig_t(json& object, GeneralConfig_t& config)
{
    if (!object.is_null())
    {
        dser_BasicType(object["party-layout"], config.partyLayout);
        dser_BasicType(object["party-hide-native"], config.isHiddenNativeParty);
        
        dser_BasicType(object["raid-layout"], config.raidLayout);
        dser_BasicType(object["raid-hide-native"], config.isHiddenNativeRaid);
        
        dser_BasicType(object["squad-layout"], config.squadLayout);
        dser_BasicType(object["squad-hide-native"], config.isHiddenNativeSquad);

        dser_BasicType(object["solo-layout"], config.soloLayout);
    }
}

void dser_Trigger_t(json& object, Trigger_t& trigger)
{
    if (!object.is_null())
    {
        dser_BasicType(object["category"], trigger.category);
        dser_BasicType(object["effect"], trigger.effect);
        dser_BasicType(object["condition"], trigger.condition);
        dser_BasicType(object["threshold"], trigger.threshold);
        dser_BasicType(object["threshold-max"], trigger.thresholdMax);
    }
}

void dser_Group_t(json& object, Group_t& group)
{
    if (!object.is_null())
    {
        dser_Trigger_t(object["trigger"], group.trigger);
        dser_BasicType(object["priority-group"], group.priorityGroup);

        if (!object["indicators"].is_null() && object["indicators"].is_array())
        {
            for (auto& indicator_json : object["indicators"])
            {
                if (!indicator_json.is_null())
                {
                    Indicator_t indicator{};
                    dser_Indicator_t(indicator_json, indicator);
                    indicator.id = g_LayoutEditor.GenerateUID();
                    group.indicators.push_back(indicator);
                }
            }
        }
    }
}

void dser_Icon_t(json& object, Icon_t& icon)
{
    if (!object.is_null())
    {
        dser_BasicType(object["source"], icon.source);
        dser_BasicType(object["path"], icon.path);
        dser_Trigger_t(object["trigger"], icon.trigger);

        /** TODO: Is this needed, or can the UID be moved to IconSingle/IconList? */
        icon.id = g_LayoutEditor.GenerateUID();
    }
}

void dser_TextStyle_t(json& object, TextStyle_t& textStyle)
{
    if (!object.is_null())
    {
        dser_BasicType(object["font-type"], textStyle.fontSource);
        dser_BasicType(object["font"], textStyle.font);
        dser_BasicType(object["font-size-type"], textStyle.fontSizeSource);
        dser_BasicType(object["font-size"], textStyle.fontSize);
        dser_BasicType(object["color-type"], textStyle.colorSource);
        dser_ImColor(object["color"], textStyle.color);
        dser_BasicType(object["decorators-type"], textStyle.decoratorSource);
        dser_BasicType(object["shadow"], textStyle.shadow);
        dser_ImColor(object["shadow-color"], textStyle.shadowColor);
        dser_BasicType(object["outline"], textStyle.outline);
        dser_ImColor(object["outline-color"], textStyle.outlineColor);
    }
}

void dser_IconTextTrigger_t(json& object, IconTextTrigger_t& trigger)
{
    if (!object.is_null())
    {
        dser_BasicType(object["source"], trigger.source);
        dser_BasicType(object["condition"], trigger.condition);
        dser_BasicType(object["threshold"], trigger.threshold);
        dser_BasicType(object["threshold-max"], trigger.thresholdMax);
    }
}

void dser_IconText_t(json& object, IconText_t& iconText)
{
    if (!object.is_null())
    {
        dser_BasicType(object["position-source"], iconText.positionSource);
        dser_Position_t(object["position"], iconText.position);
        dser_TextStyle_t(object["text-style"], iconText.textStyle);
        dser_BasicType(object["text-format-source"], iconText.textFormatSource);
        dser_BasicType(object["text-format-precision"], iconText.textFormatPrecision);
        dser_IconTextTrigger_t(object["trigger"], iconText.trigger);
    }
}

void dser_IconSingle_t(json& object, IconSingle_t& iconSingle)
{
    if (!object.is_null())
    {
        dser_Size_t(object["size"], iconSingle.size);
        dser_Position_t(object["position"], iconSingle.position);
        dser_Icon_t(object["icon"], iconSingle.icon);
        dser_BasicType(object["show-duration"], iconSingle.showDuration);
        dser_IconText_t(object["duration-text"], iconSingle.durationText);
        dser_BasicType(object["show-stacks"], iconSingle.showStacks);
        dser_IconText_t(object["stacks-text"], iconSingle.stacksText);
    }
}

void dser_IconList_t(json& object, IconList_t& iconList)
{
    if (!object.is_null())
    {
        dser_Size_t(object["size"], iconList.size);
        dser_Position_t(object["position"], iconList.position);
        dser_BasicType(object["listLength"], iconList.listLength);
        dser_BasicType(object["listDirection"], iconList.listDirection);
        dser_BasicType(object["listSpacing"], iconList.listSpacing);
        dser_BasicType(object["show-duration"], iconList.showDuration);
        dser_IconText_t(object["duration-text"], iconList.durationText);
        dser_BasicType(object["show-stacks"], iconList.showStacks);
        dser_IconText_t(object["stacks-text"], iconList.stacksText);

        if (!object["list"].is_null() && object["list"].is_array())
        {
            for (auto& icon : object["list"])
            {
                if (!icon.is_null())
                {
                    Icon_t temp{};
                    dser_Icon_t(icon, temp);
                    iconList.list.push_back(temp);
                }
            }
        }
    }
}

void dser_BorderIndicator_t(json& object, BorderIndicator_t& border)
{
    if (!object.is_null())
    {
        dser_ImColor(object["color"], border.color);
        dser_BasicType(object["thickness"], border.thickness);
        dser_Trigger_t(object["trigger"], border.trigger);
    }
}

void dser_ColourIndicator_t(json& object, ColourIndicator_t& colour)
{
    if (!object.is_null())
    {
        dser_ImColor(object["color"], colour.color);
        dser_Trigger_t(object["trigger"], colour.trigger);
    }
}

void dser_HighlightIndicator_t(json& object, HighlightIndicator_t& highlight)
{
    if (!object.is_null())
    {
        dser_ImColor(object["color"], highlight.color);
        dser_BasicType(object["position"], highlight.position);
        dser_BasicType(object["size"], highlight.size);
        dser_Trigger_t(object["trigger"], highlight.trigger);
    }
}

void dser_TextIndicator_t(json& object, TextIndicator_t& text)
{
    if (!object.is_null())
    {
        dser_Position_t(object["position"], text.position);
        dser_BasicType(object["text-content"], text.textContent);
        dser_BasicType(object["text-custom"], text.textCustom);
        dser_BasicType(object["font-type"], text.fontType);
        dser_BasicType(object["font"], text.font);
        dser_BasicType(object["font-size-type"], text.fontSizeType);
        dser_BasicType(object["font-size"], text.fontSize);
        dser_BasicType(object["color-type"], text.colorType);
        dser_ImColor(object["color"], text.color);
        dser_BasicType(object["decorators-type"], text.decoratorsType);
        dser_BasicType(object["shadow"], text.shadow);
        dser_ImColor(object["shadow-color"], text.shadowColor);
        dser_BasicType(object["outline"], text.outline);
        dser_ImColor(object["outline-color"], text.outlineColor);
        dser_BasicType(object["width-type"], text.widthType);
        dser_BasicType(object["width-value"], text.widthValue);
        dser_Trigger_t(object["trigger"], text.trigger);
    }
}

void dser_Indicator_t(json& object, Indicator_t& indicator)
{
    if (!object.is_null())
    {
        dser_BasicType(object["enabled"], indicator.enabled);
        dser_BasicType(object["name"], indicator.name);
        dser_BasicType(object["type"], indicator.type);

        if ("Group" == indicator.type) {
            dser_Group_t(object["group"], indicator.group);
        }
        else if ("Icon" == indicator.type) {
            dser_IconSingle_t(object["icon-single"], indicator.iconSingle);
        }
        else if ("Icon List" == indicator.type) {
            dser_IconList_t(object["icon-list"], indicator.iconList);
        }
        else if ("Border" == indicator.type) {
            dser_BorderIndicator_t(object["border"], indicator.border);
        }
        else if ("Colour" == indicator.type) {
            dser_ColourIndicator_t(object["colour"], indicator.colour);
        }
        else if ("Highlight" == indicator.type) {
            dser_HighlightIndicator_t(object["highlight"], indicator.highlight);
        }
        else if ("Text" == indicator.type) {
            dser_TextIndicator_t(object["text"], indicator.text);
        }
    }
}

json ser_Size_t(const Size_t& size)
{
    json object = json::object();
    object["width"] = size.width;
    object["height"] = size.height;
    return object;
}

json ser_Coordinate_t(const Coordinate_t& coordinate)
{
    json object = json::object();
    object["x"] = coordinate.x;
    object["y"] = coordinate.y;
    return object;
}

json ser_Position_t(const Position_t& position)
{
    json object = json::object();
    object["anchor"] = position.anchor;
    object["offset"] = ser_Coordinate_t(position.offset);
    return object;
}

json ser_GridProperties_t(const GridProperties_t& properties)
{
    json object = json::object();
    object["cell-direction"] = properties.cellDirection;
    object["cell-direction-max"] = properties.cellDirectionMax;
    object["cell-max"] = properties.cellMax;
    object["cell-width"] = properties.cellWidth;
    object["cell-height"] = properties.cellHeight;
    object["cell-rounding"] = properties.cellRounding;
    return object;
}

json ser_RadialProperties_t(const RadialProperties_t& properties)
{
    json object = json::object();
    object["radius-inner"] = properties.sectorRadiusInner;
    object["radius-outer"] = properties.sectorRadiusOuter;
    object["sector-min"] = properties.sectorCountMin;
    object["sector-max"] = properties.sectorCountMax;
    return object;
}

json ser_Layout_t(const Layout_t& layout)
{
    json object = json::object();
    object["type"] = layout.type;
    object["grid"] = ser_GridProperties_t(layout.grid);
    object["radial"] = ser_RadialProperties_t(layout.radial);
    return object;
}

json ser_GeneralConfig_t(const GeneralConfig_t& config)
{
    json object = json::object();
    object["party-layout"] = config.partyLayout;
    object["party-hide-native"] = config.isHiddenNativeParty;
    object["raid-layout"] = config.raidLayout;
    object["raid-hide-native"] = config.isHiddenNativeRaid;
    object["squad-layout"] = config.squadLayout;
    object["squad-hide-native"] = config.isHiddenNativeSquad;
    object["solo-layout"] = config.soloLayout;
    return object;
}

json ser_Trigger_t(const Trigger_t& trigger)
{
    json object = json::object();
    object["category"] = trigger.category;
    object["effect"] = trigger.effect;
    object["condition"] = trigger.condition;
    object["threshold"] = trigger.threshold;
    object["threshold-max"] = trigger.thresholdMax;
    return object;
}

json ser_Group_t(const Group_t& group)
{
    json object = json::object();
    object["trigger"] = ser_Trigger_t(group.trigger);
    object["priority-group"] = group.priorityGroup;
    object["indicators"] = json::array();
    for (const auto& indicator : group.indicators)
    {
        object["indicators"].push_back(ser_Indicator_t(indicator));
    }
    return object;
}

json ser_Icon_t(const Icon_t& icon)
{
    json object = json::object();
    object["source"] = icon.source;
    object["path"] = icon.path;
    object["trigger"] = ser_Trigger_t(icon.trigger);
    return object;
}

json ser_TextStyle_t(const TextStyle_t& textStyle)
{
    json object = json::object();
    object["font-type"] = textStyle.fontSource;
    object["font"] = textStyle.font;
    object["font-size-type"] = textStyle.fontSizeSource;
    object["font-size"] = textStyle.fontSize;
    object["color-type"] = textStyle.colorSource;
    object["color"] = ser_ImColor(textStyle.color);
    object["decorators-type"] = textStyle.decoratorSource;
    object["shadow"] = textStyle.shadow;
    object["shadow-color"] = ser_ImColor(textStyle.shadowColor);
    object["outline"] = textStyle.outline;
    object["outline-color"] = ser_ImColor(textStyle.outlineColor);
    return object;
}

json ser_IconTextTrigger_t(const IconTextTrigger_t& trigger)
{
    json object = json::object();
    object["source"] = trigger.source;
    object["condition"] = trigger.condition;
    object["threshold"] = trigger.threshold;
    object["threshold-max"] = trigger.thresholdMax;
    return object;
}

json ser_IconText_t(const IconText_t& iconText)
{
    json object = json::object();
    object["position-source"] = iconText.positionSource;
    object["position"] = ser_Position_t(iconText.position);
    object["text-style"] = ser_TextStyle_t(iconText.textStyle);
    object["text-format-source"] = iconText.textFormatSource;
    object["text-format-precision"] = iconText.textFormatPrecision;
    object["trigger"] = ser_IconTextTrigger_t(iconText.trigger);
    return object;
}

json ser_IconSingle_t(const IconSingle_t& iconSingle)
{
    json object = json::object();
    object["size"] = ser_Size_t(iconSingle.size);
    object["position"] = ser_Position_t(iconSingle.position);
    object["icon"] = ser_Icon_t(iconSingle.icon);
    object["show-duration"] = iconSingle.showDuration;
    object["duration-text"] = ser_IconText_t(iconSingle.durationText);
    object["show-stacks"] = iconSingle.showStacks;
    object["stacks-text"] = ser_IconText_t(iconSingle.stacksText);
    return object;
}

json ser_IconList_t(const IconList_t& iconList)
{
    json object = json::object();
    object["size"] = ser_Size_t(iconList.size);
    object["position"] = ser_Position_t(iconList.position);
    object["listLength"] = iconList.listLength;
    object["listDirection"] = iconList.listDirection;
    object["listSpacing"] = iconList.listSpacing;
    object["show-duration"] = iconList.showDuration;
    object["duration-text"] = ser_IconText_t(iconList.durationText);
    object["show-stacks"] = iconList.showStacks;
    object["stacks-text"] = ser_IconText_t(iconList.stacksText);

    object["list"] = json::array();
    for (auto& icon : iconList.list)
    {
        object["list"].push_back(ser_Icon_t(icon));
    }

    return object;
}

json ser_BorderIndicator_t(const BorderIndicator_t& border)
{
    json object = json::object();
    object["color"] = ser_ImColor(border.color);
    object["thickness"] = border.thickness;
    object["trigger"] = ser_Trigger_t(border.trigger);
    return object;
}

json ser_ColourIndicator_t(const ColourIndicator_t& colour)
{
    json object = json::object();
    object["color"] = ser_ImColor(colour.color);
    object["trigger"] = ser_Trigger_t(colour.trigger);
    return object;
}

json ser_HighlightIndicator_t(const HighlightIndicator_t& highlight)
{
    json object = json::object();
    object["color"] = ser_ImColor(highlight.color);
    object["position"] = highlight.position;
    object["size"] = highlight.size;
    object["trigger"] = ser_Trigger_t(highlight.trigger);
    return object;
}

json ser_TextIndicator_t(const TextIndicator_t& text)
{
    json object = json::object();
    object["position"] = ser_Position_t(text.position);
    object["text-content"] = text.textContent;
    object["text-custom"] = text.textCustom;
    object["font-type"] = text.fontType;
    object["font"] = text.font;
    object["font-size-type"] = text.fontSizeType;
    object["font-size"] = text.fontSize;
    object["color-type"] = text.colorType;
    object["color"] = ser_ImColor(text.color);
    object["decorators-type"] = text.decoratorsType;
    object["shadow"] = text.shadow;
    object["shadow-color"] = ser_ImColor(text.shadowColor);
    object["outline"] = text.outline;
    object["outline-color"] = ser_ImColor(text.outlineColor);
    object["width-type"] = text.widthType;
    object["width-value"] = text.widthValue;
    object["trigger"] = ser_Trigger_t(text.trigger);
    return object;
}

json ser_Indicator_t(const Indicator_t& indicator)
{
    json object = json::object();
    object["enabled"] = indicator.enabled;
    object["name"] = indicator.name;
    object["type"] = indicator.type;

    if ("Group" == indicator.type) {
        object["group"] = ser_Group_t(indicator.group);
    }
    else if ("Icon" == indicator.type) {
        object["icon-single"] = ser_IconSingle_t(indicator.iconSingle);
    }
    else if ("Icon List" == indicator.type) {
        object["icon-list"] = ser_IconList_t(indicator.iconList);
    }
    else if ("Border" == indicator.type) {
        object["border"] = ser_BorderIndicator_t(indicator.border);
    }
    else if ("Colour" == indicator.type) {
        object["colour"] = ser_ColourIndicator_t(indicator.colour);
    }
    else if ("Highlight" == indicator.type) {
        object["highlight"] = ser_HighlightIndicator_t(indicator.highlight);
    }
    else if ("Text" == indicator.type) {
        object["text"] = ser_TextIndicator_t(indicator.text);
    }

    return object;
}

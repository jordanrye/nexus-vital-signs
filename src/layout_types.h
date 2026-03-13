#pragma once

#include "imgui_tree_view.h"

struct Size_t
{
    int width = 32;
    int height = 32;
};

struct Coordinate_t
{
    int x;
    int y;

    Coordinate_t() = default;
    Coordinate_t(int x, int y): x(x), y(y) {};
};

struct Position_t
{
    std::string anchor = "Centre";
    Coordinate_t offset;
};

struct GridProperties_t
{
    std::string cellDirection = "Left-to-right";
    int cellDirectionMax = 5;
    int cellMax = 10;
    int cellWidth = 200;
    int cellHeight = 60;
    int cellRounding = 4;
};

struct RadialProperties_t
{
    float sectorRadiusInner = 60;
    float sectorRadiusOuter = 180;
    int sectorCountMin = 4;
    int sectorCountMax = 10;
};

struct Layout_t
{
    std::string type = "Grid";
    GridProperties_t grid;
    RadialProperties_t radial;
    int itemBorder = 1;
    int itemSpacing = 4;
    int visibility = 0;
    float inactiveOpacity = 0.33;
};

struct Trigger_t
{
    std::string category;
    std::string effect;
    std::string condition;
    float threshold = 0.0f;
    float thresholdMax = 0.0f;
};

struct Icon_t
{
    std::string source = "File"; /** TODO: Create File_t for (source, path) pairs. */
    std::string path;
    Trigger_t trigger;

    /* Dynamic identifier. */
    TreeNodeUID id;
};

struct IconSingle_t
{
    Size_t size;
    Position_t position;
    Icon_t icon;

    bool showDuration = false;
    bool showStacks = false;
};

struct IconList_t
{
    Size_t size;
    Position_t position;
    std::string listLength;
    std::string listDirection;
    int listSpacing;
    std::vector<Icon_t> list;
    
    bool showDuration = false;
    bool showStacks = false;
};

struct BorderIndicator_t
{
    ImColor color = ImColor(255, 255, 255, 255);
    int thickness = 1;
    Trigger_t trigger;
};

struct ColourIndicator_t
{
    ImColor color = ImColor(255, 255, 255, 255);
    Trigger_t trigger;
};

struct HighlightIndicator_t
{
    ImColor color = ImColor(255, 255, 255, 255);
    std::string position = "Bottom";
    float size = 50.f;
    Trigger_t trigger;
};

struct TextIndicator_t
{
    Position_t position;
    std::string textContent = "Character name";
    std::string textCustom;
    
    std::string fontType = "Default font";
    std::string fontPath;
    
    std::string fontSizeType = "Default font size";
    float fontSize = 10.0f;
    
    std::string colorType = "Default color";
    ImColor color = ImColor(255, 255, 255, 255);
    
    std::string decoratorsType = "Default decorators";
    bool shadow = false;
    ImColor shadowColor = ImColor(0, 0, 0, 255);
    bool outline = false;
    ImColor outlineColor = ImColor(0, 0, 0, 255);
    
    std::string widthType = "None";
    float widthValue = 0.0f;
    Trigger_t trigger;
};

// Forward declare so that Group_t can both be and contain an Indicator_t.
struct Indicator_t;

struct Group_t
{
    Trigger_t trigger;
    bool priorityGroup = false;
    std::vector<Indicator_t> indicators;
};

struct Indicator_t
{
    bool enabled;
    std::string name;
    std::string type;
    
    /* Content types. */
    IconSingle_t iconSingle;
    IconList_t iconList;
    BorderIndicator_t border;
    Group_t group;
    ColourIndicator_t colour;
    HighlightIndicator_t highlight;
    TextIndicator_t text;

    /* Dynamic identifier. */
    TreeNodeUID id;
};

struct TextConfig_t
{
    std::string fontType = "Nexus font";
    std::string fontPath;
    std::string fontSizeType = "Nexus font size";
    float fontSize = 13.0f;
    ImColor color = ImColor(255, 255, 255, 255);
    bool shadow = false;
    ImColor shadowColor = ImColor(0, 0, 0, 255);
    bool outline = false;
    ImColor outlineColor = ImColor(0, 0, 0, 255);
};

struct IconTextConfig_t : public TextConfig_t
{
    Position_t position;
    IconTextConfig_t(std::string anchor = "Centre") { position.anchor = anchor; }
};

struct LayoutConfig_t
{
    std::string name;
    std::string colors = "Generic";
    Position_t position;
    Layout_t layout;
    std::vector<Indicator_t> indicators;

    /* Dynamic identifier. */
    TreeNodeUID id;

    /* Preview state. */
    TreeNodeUID previewNodeId = TreeNodeUID::NONE;
};

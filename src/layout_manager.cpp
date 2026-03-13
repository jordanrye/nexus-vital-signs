#include "layout_manager.h"

#include <string>

#include "shared.h"

static LayoutConfig_t GetDefaultGridLayout();
static LayoutConfig_t GetDefaultRadialLayout();

static void InsertRecursive(const std::vector<Indicator_t>& indicators, TreeNodeUID layoutId, std::map<TreeNodeUID, TreeNodeUID>& map);

void LayoutManager::Create(const std::string& name, const std::string& type, bool createFromTemplate, const std::filesystem::path& fileDir)
{
    // Get a sanitised filename
    std::string fileName;
    fileName.reserve(name.length()); // Avoid reallocations
    for (char c : name)
    {
        c = std::tolower(c); // Convert character to lowercase

        if (c == ' ') {
            fileName += '-'; // Replace spaces with hyphens
        }
        else if (std::isalnum(c) || c == '-' || c == '_') {
            fileName += c; // Keep alphanumeric, hyphens, and underscores
        }
        else {
            // All other characters are ignored
        }
    }

    // Get the full path
    std::filesystem::path filePath = fileDir / (fileName + ".json");

    // Ensure the filename is unique to avoid overwriting
    int counter = 1;
    while (std::filesystem::exists(filePath)) {
        filePath = fileDir / (fileName + "-" + std::to_string(counter++) + ".json");
    }

    // Construct layout
    LayoutConfig_t layout{};
    if (createFromTemplate)
    {
        if (type == "Grid")
        {
            layout = GetDefaultGridLayout();
        }
        else if (type == "Radial")
        {
            layout = GetDefaultRadialLayout();
        }
    }
    layout.name = name;
    layout.layout.type = type;
    layout.id = g_LayoutEditor.GenerateUID();

    m_layouts.insert_or_assign(filePath, layout);
    m_idToPath.insert_or_assign(layout.id, filePath);
    InsertRecursive(layout.indicators, layout.id, m_indicatorToLayout);

    if (m_onLayoutCreatedCallback) {
        m_onLayoutCreatedCallback(filePath, layout.id, layout.name, layout.layout.type);
    }

    if (createFromTemplate)
    {
        RegisterIndicatorsRecursive(layout.id, layout.indicators);
    }
}

void LayoutManager::Insert(const std::filesystem::path& path, const LayoutConfig_t& layout)
{
    m_layouts.insert_or_assign(path, layout);
    m_idToPath.insert_or_assign(layout.id, path);
    InsertRecursive(layout.indicators, layout.id, m_indicatorToLayout);
}

bool LayoutManager::Delete(TreeNodeUID id)
{
    bool success = false;
    auto it = m_idToPath.find(id);

    if (it != m_idToPath.end())
    {
        // Erase layout lookup
        const std::filesystem::path path = it->second;
        m_idToPath.erase(it);

        // Erase indicator lookups for this layout
        auto layout_it = m_layouts.find(path);
        if (layout_it != m_layouts.end())
        {
            for (const auto& indicator : layout_it->second.indicators)
            {
                m_indicatorToLayout.erase(indicator.id);
            }
        }

        // Erase layout
        m_layouts.erase(path);
        std::filesystem::remove(path);
        
        if (m_onLayoutDeletedCallback)
        {
            m_onLayoutDeletedCallback(id);
        }

        success = true;
    }
    
    return success;
}

static Indicator_t* FindIndicatorRecursive(std::vector<Indicator_t>& indicators, TreeNodeUID id)
{
    for (auto& indicator : indicators)
    {
        if (indicator.id == id)
            return &indicator;

        if (indicator.type == "Group")
        {
            if (Indicator_t* found = FindIndicatorRecursive(indicator.group.indicators, id))
                return found;
        }
    }
    return nullptr;
}

TreeNodeUID LayoutManager::AddIndicator(TreeNodeUID parentId, const std::string& name, const std::string& type)
{
    auto it = m_idToPath.find(parentId);

    if (it != m_idToPath.end())
    {
        auto& layout = m_layouts.at(it->second);

        Indicator_t indicator{};
        indicator.id = g_LayoutEditor.GenerateUID();
        indicator.enabled = true;
        indicator.name = name;
        indicator.type = type;

        layout.indicators.push_back(indicator);
        m_indicatorToLayout[indicator.id] = parentId;

        if (m_onIndicatorCreatedCallback)
        {
            m_onIndicatorCreatedCallback(parentId, indicator.id, indicator.name, indicator.type);
        }

        return indicator.id;
    }

    auto it_ind = m_indicatorToLayout.find(parentId);
    if (it_ind != m_indicatorToLayout.end())
    {
        TreeNodeUID layoutId = it_ind->second;
        auto it_path = m_idToPath.find(layoutId);

        if (it_path != m_idToPath.end())
        {
            auto& layout = m_layouts.at(it_path->second);

            if (Indicator_t* parentIndicator = FindIndicatorRecursive(layout.indicators, parentId))
            {
                if (parentIndicator->type == "Group")
                {
                    Indicator_t indicator{};
                    indicator.id = g_LayoutEditor.GenerateUID();
                    indicator.enabled = true;
                    indicator.name = name;
                    indicator.type = type;

                    parentIndicator->group.indicators.push_back(indicator);
                    m_indicatorToLayout[indicator.id] = layoutId;

                    if (m_onIndicatorCreatedCallback)
                    {
                        m_onIndicatorCreatedCallback(parentId, indicator.id, indicator.name, indicator.type);
                    }

                    return indicator.id;
                }
            }
        }
    }
    
    return TreeNodeUID::NONE;
}

static bool DeleteIndicatorRecursive(std::vector<Indicator_t>& indicators, TreeNodeUID id, std::map<TreeNodeUID, TreeNodeUID>& map)
{
    for (auto it = indicators.begin(); it != indicators.end(); ++it)
    {
        if (it->id == id)
        {
            map.erase(id);
            indicators.erase(it);
            return true;
        }
        if (it->type == "Group")
            if (DeleteIndicatorRecursive(it->group.indicators, id, map))
                return true;
    }
    return false;
}

bool LayoutManager::DeleteIndicator(TreeNodeUID id)
{
    auto it_layout = m_indicatorToLayout.find(id);
    
    if (it_layout != m_indicatorToLayout.end())
    {
        TreeNodeUID layoutId = it_layout->second;
        auto it_path = m_idToPath.find(layoutId);

        if (it_path != m_idToPath.end())
        {
            auto& indicators = m_layouts.at(it_path->second).indicators;
            
            if (DeleteIndicatorRecursive(indicators, id, m_indicatorToLayout))
            {

                if (m_onIndicatorDeletedCallback)
                {
                    m_onIndicatorDeletedCallback(id);
                }

                return true;
            }
        }
    }
    return false;
}

void LayoutManager::ReorderIndicators(TreeNodeUID layoutId, size_t oldIndex, size_t newIndex)
{
    std::vector<Indicator_t>* indicators = nullptr;

    /* Check if parent is a Layout */
    auto it_path = m_idToPath.find(layoutId);
    if (it_path != m_idToPath.end())
    {
        indicators = &m_layouts.at(it_path->second).indicators;
    }
    else
    {
        /* Check if parent is a Group Indicator */
        auto it_uid = m_indicatorToLayout.find(layoutId);
        if (it_uid != m_indicatorToLayout.end())
        {
            TreeNodeUID rootId = it_uid->second;
            auto& layout = m_layouts.at(m_idToPath.at(rootId));
            Indicator_t* parent = FindIndicatorRecursive(layout.indicators, layoutId);
            if (parent && parent->type == "Group")
            {
                indicators = &parent->group.indicators;
            }
        }
    }

    if (indicators && oldIndex < indicators->size() && newIndex < indicators->size() && oldIndex != newIndex)
    {
        std::swap((*indicators)[oldIndex], (*indicators)[newIndex]);

        if (m_onIndicatorSwapCallback)
        {
            m_onIndicatorSwapCallback(layoutId, oldIndex, newIndex);
        }
    }
}

std::map<std::filesystem::path, LayoutConfig_t>& LayoutManager::GetAllLayouts()
{
    return m_layouts;
}

LayoutConfig_t& LayoutManager::GetLayoutFromFilePath(const std::filesystem::path& path)
{
    return m_layouts.at(path);
}

LayoutConfig_t* LayoutManager::GetLayoutFromNodeID(TreeNodeUID id)
{
    auto it_path = m_idToPath.find(id);
    if (it_path != m_idToPath.end())
    {
        return &m_layouts.at(it_path->second);
    }

    auto it_ind = m_indicatorToLayout.find(id);
    if (it_ind != m_indicatorToLayout.end())
    {
        auto it_path_layout = m_idToPath.find(it_ind->second);
        if (it_path_layout != m_idToPath.end())
        {
            return &m_layouts.at(it_path_layout->second);
        }
    }

    return nullptr;
}

Indicator_t* LayoutManager::GetIndicator(TreeNodeUID id)
{
    auto it_prof = m_indicatorToLayout.find(id);
    if (it_prof != m_indicatorToLayout.end())
    {
        auto it_path = m_idToPath.find(it_prof->second);
        if (it_path != m_idToPath.end())
        {
            return FindIndicatorRecursive(m_layouts.at(it_path->second).indicators, id);
        }
    }
    return nullptr;
}

static void InsertRecursive(const std::vector<Indicator_t>& indicators, TreeNodeUID layoutId, std::map<TreeNodeUID, TreeNodeUID>& map)
{
    for (const auto& indicator : indicators)
    {
        map.insert_or_assign(indicator.id, layoutId);
        
        if (indicator.type == "Group")
        {
            InsertRecursive(indicator.group.indicators, layoutId, map);
        }
    }
}

void LayoutManager::RegisterIndicatorsRecursive(TreeNodeUID parentId, const std::vector<Indicator_t>& indicators)
{
    for (const auto& indicator : indicators)
    {
        if (m_onIndicatorCreatedCallback)
        {
            m_onIndicatorCreatedCallback(parentId, indicator.id, indicator.name, indicator.type);
        }

        if (indicator.type == "Group")
        {
            RegisterIndicatorsRecursive(indicator.id, indicator.group.indicators);
        }
    }
}

static LayoutConfig_t GetDefaultGridLayout()
{
    LayoutConfig_t layout{};
    layout.enabled = true;
    layout.colors = "Generic";
    layout.position.anchor = "Centre";
    layout.position.offset = { 0, 0 };
    layout.layout.type = "Grid";
    // Grid specific properties
    layout.layout.grid.cellDirection = "Top-to-bottom";
    layout.layout.grid.cellDirectionMax = 5;
    layout.layout.grid.cellWidth = 180;
    layout.layout.grid.cellHeight = 100;
    layout.layout.grid.cellRounding = 4;
    layout.layout.itemBorder = 1;
    layout.layout.itemSpacing = 4;
    layout.layout.visibility = 0; // Always visible
    layout.layout.inactiveOpacity = 0.33f;

    // Add a default indicator for character name.
    Indicator_t nameIndicator{};
    nameIndicator.id = g_LayoutEditor.GenerateUID();
    nameIndicator.enabled = true;
    nameIndicator.name = "Character Name";
    nameIndicator.type = "Text";
    nameIndicator.text.position.anchor = "Centre";
    nameIndicator.text.position.offset = { 0, 0 };
    nameIndicator.text.textContent = "Character name";
    layout.indicators.push_back(nameIndicator);

    // Add a default set of indicators for key boons.
    Indicator_t boonIndicator{};
    boonIndicator.id = g_LayoutEditor.GenerateUID();
    boonIndicator.enabled = true;
    boonIndicator.name = "Boons";
    boonIndicator.type = "Icon List";
    boonIndicator.iconList.position.anchor = "Top-right";
    boonIndicator.iconList.position.offset = { -2, 2 };
    boonIndicator.iconList.listDirection = "Right-to-left";
    boonIndicator.iconList.listLength = "Dynamic";
    boonIndicator.iconList.size = { 24, 24 };
    boonIndicator.iconList.showStacks = true;
    Icon_t boonIndicatorAlacrity{};
    boonIndicatorAlacrity.id = g_LayoutEditor.GenerateUID();
    boonIndicatorAlacrity.source = "URL";
    boonIndicatorAlacrity.path = "https://wiki.guildwars2.com/images/4/4c/Alacrity.png";
    boonIndicatorAlacrity.trigger.category = "Boons";
    boonIndicatorAlacrity.trigger.effect = "Alacrity";
    boonIndicatorAlacrity.trigger.condition = "Status: Active";
    boonIndicator.iconList.list.push_back(boonIndicatorAlacrity);
    Icon_t boonIndicatorQuickness{};
    boonIndicatorQuickness.id = g_LayoutEditor.GenerateUID();
    boonIndicatorQuickness.source = "URL";
    boonIndicatorQuickness.path = "https://wiki.guildwars2.com/images/b/b4/Quickness.png";
    boonIndicatorQuickness.trigger.category = "Boons";
    boonIndicatorQuickness.trigger.effect = "Quickness";
    boonIndicatorQuickness.trigger.condition = "Status: Active";
    boonIndicator.iconList.list.push_back(boonIndicatorQuickness);
    layout.indicators.push_back(boonIndicator);
    Icon_t boonIndicatorMight{};
    boonIndicatorMight.id = g_LayoutEditor.GenerateUID();
    boonIndicatorMight.source = "URL";
    boonIndicatorMight.path = "https://wiki.guildwars2.com/images/7/7c/Might.png";
    boonIndicatorMight.trigger.category = "Boons";
    boonIndicatorMight.trigger.effect = "Might";
    boonIndicatorMight.trigger.condition = "Status: Active";
    boonIndicator.iconList.list.push_back(boonIndicatorMight);

    // Add a default set of indicators for all conditions.
    Indicator_t conditionIndicator{};
    conditionIndicator.id = g_LayoutEditor.GenerateUID();
    conditionIndicator.enabled = true;
    conditionIndicator.name = "Conditions";
    conditionIndicator.type = "Icon List";
    conditionIndicator.iconList.position.anchor = "Bottom-left";
    conditionIndicator.iconList.position.offset = { 2, -2 };
    conditionIndicator.iconList.listDirection = "Left-to-right";
    conditionIndicator.iconList.listLength = "Dynamic";
    conditionIndicator.iconList.size = { 16, 16 };
    conditionIndicator.iconList.showStacks = true;
    Icon_t conditionIndicatorBleeding{};
    conditionIndicatorBleeding.id = g_LayoutEditor.GenerateUID();
    conditionIndicatorBleeding.source = "URL";
    conditionIndicatorBleeding.path = "https://wiki.guildwars2.com/images/3/33/Bleeding.png";
    conditionIndicatorBleeding.trigger.category = "Conditions";
    conditionIndicatorBleeding.trigger.effect = "Bleeding";
    conditionIndicatorBleeding.trigger.condition = "Status: Active";
    conditionIndicator.iconList.list.push_back(conditionIndicatorBleeding);
    Icon_t conditionIndicatorBurning{};
    conditionIndicatorBurning.id = g_LayoutEditor.GenerateUID();
    conditionIndicatorBurning.source = "URL";
    conditionIndicatorBurning.path = "https://wiki.guildwars2.com/images/4/45/Burning.png";
    conditionIndicatorBurning.trigger.category = "Conditions";
    conditionIndicatorBurning.trigger.effect = "Burning";
    conditionIndicatorBurning.trigger.condition = "Status: Active";
    conditionIndicator.iconList.list.push_back(conditionIndicatorBurning);
    Icon_t conditionIndicatorConfusion{};
    conditionIndicatorConfusion.id = g_LayoutEditor.GenerateUID();
    conditionIndicatorConfusion.source = "URL";
    conditionIndicatorConfusion.path = "https://wiki.guildwars2.com/images/e/e6/Confusion.png";
    conditionIndicatorConfusion.trigger.category = "Conditions";
    conditionIndicatorConfusion.trigger.effect = "Confusion";
    conditionIndicatorConfusion.trigger.condition = "Status: Active";
    conditionIndicator.iconList.list.push_back(conditionIndicatorConfusion);
    Icon_t conditionIndicatorPoisoned{};
    conditionIndicatorPoisoned.id = g_LayoutEditor.GenerateUID();
    conditionIndicatorPoisoned.source = "URL";
    conditionIndicatorPoisoned.path = "https://wiki.guildwars2.com/images/1/11/Poisoned.png";
    conditionIndicatorPoisoned.trigger.category = "Conditions";
    conditionIndicatorPoisoned.trigger.effect = "Poisoned";
    conditionIndicatorPoisoned.trigger.condition = "Status: Active";
    conditionIndicator.iconList.list.push_back(conditionIndicatorPoisoned);
    Icon_t conditionIndicatorTorment{};
    conditionIndicatorTorment.id = g_LayoutEditor.GenerateUID();
    conditionIndicatorTorment.source = "URL";
    conditionIndicatorTorment.path = "https://wiki.guildwars2.com/images/0/08/Torment.png";
    conditionIndicatorTorment.trigger.category = "Conditions";
    conditionIndicatorTorment.trigger.effect = "Torment";
    conditionIndicatorTorment.trigger.condition = "Status: Active";
    conditionIndicator.iconList.list.push_back(conditionIndicatorTorment);
    Icon_t conditionIndicatorBlinded{};
    conditionIndicatorBlinded.id = g_LayoutEditor.GenerateUID();
    conditionIndicatorBlinded.source = "URL";
    conditionIndicatorBlinded.path = "https://wiki.guildwars2.com/images/3/33/Blinded.png";
    conditionIndicatorBlinded.trigger.category = "Conditions";
    conditionIndicatorBlinded.trigger.effect = "Blinded";
    conditionIndicatorBlinded.trigger.condition = "Status: Active";
    conditionIndicator.iconList.list.push_back(conditionIndicatorBlinded);
    Icon_t conditionIndicatorChilled{};
    conditionIndicatorChilled.id = g_LayoutEditor.GenerateUID();
    conditionIndicatorChilled.source = "URL";
    conditionIndicatorChilled.path = "https://wiki.guildwars2.com/images/a/a6/Chilled.png";
    conditionIndicatorChilled.trigger.category = "Conditions";
    conditionIndicatorChilled.trigger.effect = "Chilled";
    conditionIndicatorChilled.trigger.condition = "Status: Active";
    conditionIndicator.iconList.list.push_back(conditionIndicatorChilled);
    Icon_t conditionIndicatorCrippled{};
    conditionIndicatorCrippled.id = g_LayoutEditor.GenerateUID();
    conditionIndicatorCrippled.source = "URL";
    conditionIndicatorCrippled.path = "https://wiki.guildwars2.com/images/f/fb/Crippled.png";
    conditionIndicatorCrippled.trigger.category = "Conditions";
    conditionIndicatorCrippled.trigger.effect = "Crippled";
    conditionIndicatorCrippled.trigger.condition = "Status: Active";
    conditionIndicator.iconList.list.push_back(conditionIndicatorCrippled);
    Icon_t conditionIndicatorFear{};
    conditionIndicatorFear.id = g_LayoutEditor.GenerateUID();
    conditionIndicatorFear.source = "URL";
    conditionIndicatorFear.path = "https://wiki.guildwars2.com/images/e/e6/Fear.png";
    conditionIndicatorFear.trigger.category = "Conditions";
    conditionIndicatorFear.trigger.effect = "Fear";
    conditionIndicatorFear.trigger.condition = "Status: Active";
    conditionIndicator.iconList.list.push_back(conditionIndicatorFear);
    Icon_t conditionIndicatorImmobilized{};
    conditionIndicatorImmobilized.id = g_LayoutEditor.GenerateUID();
    conditionIndicatorImmobilized.source = "URL";
    conditionIndicatorImmobilized.path = "https://wiki.guildwars2.com/images/3/32/Immobile.png";
    conditionIndicatorImmobilized.trigger.category = "Conditions";
    conditionIndicatorImmobilized.trigger.effect = "Immobilized";
    conditionIndicatorImmobilized.trigger.condition = "Status: Active";
    conditionIndicator.iconList.list.push_back(conditionIndicatorImmobilized);
    Icon_t conditionIndicatorSlow{};
    conditionIndicatorSlow.id = g_LayoutEditor.GenerateUID();
    conditionIndicatorSlow.source = "URL";
    conditionIndicatorSlow.path = "https://wiki.guildwars2.com/images/f/f5/Slow.png";
    conditionIndicatorSlow.trigger.category = "Conditions";
    conditionIndicatorSlow.trigger.effect = "Slow";
    conditionIndicatorSlow.trigger.condition = "Status: Active";
    conditionIndicator.iconList.list.push_back(conditionIndicatorSlow);
    Icon_t conditionIndicatorTaunt{};
    conditionIndicatorTaunt.id = g_LayoutEditor.GenerateUID();
    conditionIndicatorTaunt.source = "URL";
    conditionIndicatorTaunt.path = "https://wiki.guildwars2.com/images/c/cc/Taunt.png";
    conditionIndicatorTaunt.trigger.category = "Conditions";
    conditionIndicatorTaunt.trigger.effect = "Taunt";
    conditionIndicatorTaunt.trigger.condition = "Status: Active";
    conditionIndicator.iconList.list.push_back(conditionIndicatorTaunt);
    Icon_t conditionIndicatorWeakness{};
    conditionIndicatorWeakness.id = g_LayoutEditor.GenerateUID();
    conditionIndicatorWeakness.source = "URL";
    conditionIndicatorWeakness.path = "https://wiki.guildwars2.com/images/f/f9/Weakness.png";
    conditionIndicatorWeakness.trigger.category = "Conditions";
    conditionIndicatorWeakness.trigger.effect = "Weakness";
    conditionIndicatorWeakness.trigger.condition = "Status: Active";
    conditionIndicator.iconList.list.push_back(conditionIndicatorWeakness);
    Icon_t conditionIndicatorVulnerability{};
    conditionIndicatorVulnerability.id = g_LayoutEditor.GenerateUID();
    conditionIndicatorVulnerability.source = "URL";
    conditionIndicatorVulnerability.path = "https://wiki.guildwars2.com/images/a/af/Vulnerability.png";
    conditionIndicatorVulnerability.trigger.category = "Conditions";
    conditionIndicatorVulnerability.trigger.effect = "Vulnerability";
    conditionIndicatorVulnerability.trigger.condition = "Status: Active";
    conditionIndicator.iconList.list.push_back(conditionIndicatorVulnerability);
    layout.indicators.push_back(conditionIndicator);

    return layout;
}

static LayoutConfig_t GetDefaultRadialLayout()
{
    LayoutConfig_t layout{};
    layout.enabled = true;
    layout.colors = "Generic";
    layout.position.anchor = "Centre";
    layout.position.offset = { 0, 0 };
    layout.layout.type = "Radial";
    // Radial specific properties
    layout.layout.radial.sectorRadiusInner = 60;
    layout.layout.radial.sectorRadiusOuter = 180;
    layout.layout.radial.sectorCountMin = 4;
    layout.layout.radial.sectorCountMax = 10;
    layout.layout.itemSpacing = 4;
    layout.layout.visibility = 2; // Visible while activated
    layout.layout.inactiveOpacity = 0.33f;

    return layout;
}

#include "layout_manager.h"

#include <string>

#include "shared.h"

static LayoutConfig_t GetDefaultGridLayout();
static LayoutConfig_t GetDefaultRadialLayout();

static void InsertRecursive(const std::vector<Indicator_t>& indicators, TreeNodeUID layoutId, std::map<TreeNodeUID, TreeNodeUID>& map);

void LayoutManager::Clear()
{
    m_layouts.clear();
    m_idToPath.clear();
    m_indicatorToLayout.clear();
}

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
    layout.layout.type = "Grid";
    // Position
    layout.position.anchor = "Centre";
    layout.position.offset = { 0, 0 };
    // Colors
    layout.colors = "Role";
    // Sort
    layout.layout.sortBy = "Roles";
    // Grid Properties
    layout.layout.grid.frameDirection = "Left-to-right";
    layout.layout.grid.squadDirection = "Top-to-bottom";
    layout.layout.grid.maxRows = 5;
    layout.layout.grid.maxColumns = 10;
    // Frame Properties
    layout.layout.grid.cellWidth = 180;
    layout.layout.grid.cellHeight = 100;
    layout.layout.grid.cellRounding = 4;
    layout.layout.itemBorder = 1;
    layout.layout.grid.spacingHorizontal = 4;
    layout.layout.grid.spacingVertical = 4;

    // Add a default indicator for character name.
    Indicator_t nameIndicator{};
    nameIndicator.id = g_LayoutEditor.GenerateUID();
    nameIndicator.enabled = true;
    nameIndicator.name = "Character Name";
    nameIndicator.type = "Text";
    nameIndicator.text.position.anchor = "Centre";
    nameIndicator.text.position.offset = { 0, 0 };
    nameIndicator.text.textContent = "Character name";
    nameIndicator.text.trigger.category = "<Inherit From Parent>";
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
    Icon_t boonIndicatorMight{};
    boonIndicatorMight.id = g_LayoutEditor.GenerateUID();
    boonIndicatorMight.source = "URL";
    boonIndicatorMight.path = "https://wiki.guildwars2.com/images/7/7c/Might.png";
    boonIndicatorMight.trigger.category = "Boons";
    boonIndicatorMight.trigger.effect = "Might";
    boonIndicatorMight.trigger.condition = "Status: Active";
    boonIndicator.iconList.list.push_back(boonIndicatorMight);
    layout.indicators.push_back(boonIndicator);

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

    // Add a default set of indicators for all squad roles.
    Indicator_t squadRolesIndicator{};
    squadRolesIndicator.id = g_LayoutEditor.GenerateUID();
    squadRolesIndicator.enabled = true;
    squadRolesIndicator.name = "Squad Roles";
    squadRolesIndicator.type = "Icon List";
    squadRolesIndicator.iconList.position.anchor = "Top-left";
    squadRolesIndicator.iconList.position.offset = { 26, 2 };
    squadRolesIndicator.iconList.listDirection = "Left-to-right";
    squadRolesIndicator.iconList.listLength = "Dynamic";
    squadRolesIndicator.iconList.listSpacing = -24;
    squadRolesIndicator.iconList.size = { 24, 24 };
    squadRolesIndicator.iconList.showStacks = true;
    Icon_t squadRolesIndicatorCommander{};
    squadRolesIndicatorCommander.id = g_LayoutEditor.GenerateUID();
    squadRolesIndicatorCommander.source = "URL";
    squadRolesIndicatorCommander.path = "https://wiki.guildwars2.com/images/b/b2/Squad_UI_commander.png";
    squadRolesIndicatorCommander.trigger.category = "Roles";
    squadRolesIndicatorCommander.trigger.effect = "Commander";
    squadRolesIndicator.iconList.list.push_back(squadRolesIndicatorCommander);
    Icon_t squadRolesIndicatorLieutenant{};
    squadRolesIndicatorLieutenant.id = g_LayoutEditor.GenerateUID();
    squadRolesIndicatorLieutenant.source = "URL";
    squadRolesIndicatorLieutenant.path = "https://wiki.guildwars2.com/images/4/4c/Squad_UI_lieutenant.png";
    squadRolesIndicatorLieutenant.trigger.category = "Roles";
    squadRolesIndicatorLieutenant.trigger.effect = "Lieutenant";
    squadRolesIndicator.iconList.list.push_back(squadRolesIndicatorLieutenant);
    layout.indicators.push_back(squadRolesIndicator);

    // Add a default set of indicators for all professions.
    Indicator_t professionsIndicator{};
    professionsIndicator.id = g_LayoutEditor.GenerateUID();
    professionsIndicator.enabled = true;
    professionsIndicator.name = "Professions";
    professionsIndicator.type = "Icon List";
    professionsIndicator.iconList.position.anchor = "Top-left";
    professionsIndicator.iconList.position.offset = { 2, 2 };
    professionsIndicator.iconList.listDirection = "Left-to-right";
    professionsIndicator.iconList.listLength = "Dynamic";
    professionsIndicator.iconList.listSpacing = -24;
    professionsIndicator.iconList.size = { 24, 24 };
    Icon_t professionsIndicatorElementalist{};
    professionsIndicatorElementalist.id = g_LayoutEditor.GenerateUID();
    professionsIndicatorElementalist.source = "URL";
    professionsIndicatorElementalist.path = "https://wiki.guildwars2.com/images/b/b4/Elementalist_icon_white.png";
    professionsIndicatorElementalist.trigger.category = "Professions";
    professionsIndicatorElementalist.trigger.effect = "Elementalist";
    professionsIndicatorElementalist.trigger.condition = "<Inherit From Parent>";
    professionsIndicator.iconList.list.push_back(professionsIndicatorElementalist);
    Icon_t professionsIndicatorElementalistTempest{};
    professionsIndicatorElementalistTempest.id = g_LayoutEditor.GenerateUID();
    professionsIndicatorElementalistTempest.source = "URL";
    professionsIndicatorElementalistTempest.path = "https://wiki.guildwars2.com/images/7/70/Tempest_icon_white.png";
    professionsIndicatorElementalistTempest.trigger.category = "Professions";
    professionsIndicatorElementalistTempest.trigger.effect = "Elementalist: Tempest";
    professionsIndicatorElementalistTempest.trigger.condition = "<Inherit From Parent>";
    professionsIndicator.iconList.list.push_back(professionsIndicatorElementalistTempest);
    Icon_t professionsIndicatorElementalistWeaver{};
    professionsIndicatorElementalistWeaver.id = g_LayoutEditor.GenerateUID();
    professionsIndicatorElementalistWeaver.source = "URL";
    professionsIndicatorElementalistWeaver.path = "https://wiki.guildwars2.com/images/c/cb/Weaver_icon_white.png";
    professionsIndicatorElementalistWeaver.trigger.category = "Professions";
    professionsIndicatorElementalistWeaver.trigger.effect = "Elementalist: Weaver";
    professionsIndicatorElementalistWeaver.trigger.condition = "<Inherit From Parent>";
    professionsIndicator.iconList.list.push_back(professionsIndicatorElementalistWeaver);
    Icon_t professionsIndicatorElementalistCatalyst{};
    professionsIndicatorElementalistCatalyst.id = g_LayoutEditor.GenerateUID();
    professionsIndicatorElementalistCatalyst.source = "URL";
    professionsIndicatorElementalistCatalyst.path = "https://wiki.guildwars2.com/images/9/93/Catalyst_icon_white.png";
    professionsIndicatorElementalistCatalyst.trigger.category = "Professions";
    professionsIndicatorElementalistCatalyst.trigger.effect = "Elementalist: Catalyst";
    professionsIndicatorElementalistCatalyst.trigger.condition = "<Inherit From Parent>";
    professionsIndicator.iconList.list.push_back(professionsIndicatorElementalistCatalyst);
    Icon_t professionsIndicatorElementalistEvoker{};
    professionsIndicatorElementalistEvoker.id = g_LayoutEditor.GenerateUID();
    professionsIndicatorElementalistEvoker.source = "URL";
    professionsIndicatorElementalistEvoker.path = "https://wiki.guildwars2.com/images/a/a4/Evoker_icon_white.png";
    professionsIndicatorElementalistEvoker.trigger.category = "Professions";
    professionsIndicatorElementalistEvoker.trigger.effect = "Elementalist: Evoker";
    professionsIndicatorElementalistEvoker.trigger.condition = "<Inherit From Parent>";
    professionsIndicator.iconList.list.push_back(professionsIndicatorElementalistEvoker);
    Icon_t professionsIndicatorEngineer{};
    professionsIndicatorEngineer.id = g_LayoutEditor.GenerateUID();
    professionsIndicatorEngineer.source = "URL";
    professionsIndicatorEngineer.path = "https://wiki.guildwars2.com/images/b/bd/Engineer_icon_white.png";
    professionsIndicatorEngineer.trigger.category = "Professions";
    professionsIndicatorEngineer.trigger.effect = "Engineer";
    professionsIndicatorEngineer.trigger.condition = "<Inherit From Parent>";
    professionsIndicator.iconList.list.push_back(professionsIndicatorEngineer);
    Icon_t professionsIndicatorEngineerScrapper{};
    professionsIndicatorEngineerScrapper.id = g_LayoutEditor.GenerateUID();
    professionsIndicatorEngineerScrapper.source = "URL";
    professionsIndicatorEngineerScrapper.path = "https://wiki.guildwars2.com/images/6/68/Scrapper_icon_white.png";
    professionsIndicatorEngineerScrapper.trigger.category = "Professions";
    professionsIndicatorEngineerScrapper.trigger.effect = "Engineer: Scrapper";
    professionsIndicatorEngineerScrapper.trigger.condition = "<Inherit From Parent>";
    professionsIndicator.iconList.list.push_back(professionsIndicatorEngineerScrapper);
    Icon_t professionsIndicatorEngineerHolosmith{};
    professionsIndicatorEngineerHolosmith.id = g_LayoutEditor.GenerateUID();
    professionsIndicatorEngineerHolosmith.source = "URL";
    professionsIndicatorEngineerHolosmith.path = "https://wiki.guildwars2.com/images/3/3a/Holosmith_icon_white.png";
    professionsIndicatorEngineerHolosmith.trigger.category = "Professions";
    professionsIndicatorEngineerHolosmith.trigger.effect = "Engineer: Holosmith";
    professionsIndicatorEngineerHolosmith.trigger.condition = "<Inherit From Parent>";
    professionsIndicator.iconList.list.push_back(professionsIndicatorEngineerHolosmith);
    Icon_t professionsIndicatorEngineerMechanist{};
    professionsIndicatorEngineerMechanist.id = g_LayoutEditor.GenerateUID();
    professionsIndicatorEngineerMechanist.source = "URL";
    professionsIndicatorEngineerMechanist.path = "https://wiki.guildwars2.com/images/6/6c/Mechanist_icon_white.png";
    professionsIndicatorEngineerMechanist.trigger.category = "Professions";
    professionsIndicatorEngineerMechanist.trigger.effect = "Engineer: Mechanist";
    professionsIndicatorEngineerMechanist.trigger.condition = "<Inherit From Parent>";
    professionsIndicator.iconList.list.push_back(professionsIndicatorEngineerMechanist);
    Icon_t professionsIndicatorEngineerAmalgam{};
    professionsIndicatorEngineerAmalgam.id = g_LayoutEditor.GenerateUID();
    professionsIndicatorEngineerAmalgam.source = "URL";
    professionsIndicatorEngineerAmalgam.path = "https://wiki.guildwars2.com/images/9/9e/Amalgam_icon_white.png";
    professionsIndicatorEngineerAmalgam.trigger.category = "Professions";
    professionsIndicatorEngineerAmalgam.trigger.effect = "Engineer: Amalgam";
    professionsIndicatorEngineerAmalgam.trigger.condition = "<Inherit From Parent>";
    professionsIndicator.iconList.list.push_back(professionsIndicatorEngineerAmalgam);
    Icon_t professionsIndicatorGuardian{};
    professionsIndicatorGuardian.id = g_LayoutEditor.GenerateUID();
    professionsIndicatorGuardian.source = "URL";
    professionsIndicatorGuardian.path = "https://wiki.guildwars2.com/images/1/14/Guardian_icon_white.png";
    professionsIndicatorGuardian.trigger.category = "Professions";
    professionsIndicatorGuardian.trigger.effect = "Guardian";
    professionsIndicatorGuardian.trigger.condition = "<Inherit From Parent>";
    professionsIndicator.iconList.list.push_back(professionsIndicatorGuardian);
    Icon_t professionsIndicatorGuardianDragonhunter{};
    professionsIndicatorGuardianDragonhunter.id = g_LayoutEditor.GenerateUID();
    professionsIndicatorGuardianDragonhunter.source = "URL";
    professionsIndicatorGuardianDragonhunter.path = "https://wiki.guildwars2.com/images/a/a4/Dragonhunter_icon_white.png";
    professionsIndicatorGuardianDragonhunter.trigger.category = "Professions";
    professionsIndicatorGuardianDragonhunter.trigger.effect = "Guardian: Dragonhunter";
    professionsIndicatorGuardianDragonhunter.trigger.condition = "<Inherit From Parent>";
    professionsIndicator.iconList.list.push_back(professionsIndicatorGuardianDragonhunter);
    Icon_t professionsIndicatorGuardianFirebrand{};
    professionsIndicatorGuardianFirebrand.id = g_LayoutEditor.GenerateUID();
    professionsIndicatorGuardianFirebrand.source = "URL";
    professionsIndicatorGuardianFirebrand.path = "https://wiki.guildwars2.com/images/d/d5/Firebrand_icon_white.png";
    professionsIndicatorGuardianFirebrand.trigger.category = "Professions";
    professionsIndicatorGuardianFirebrand.trigger.effect = "Guardian: Firebrand";
    professionsIndicatorGuardianFirebrand.trigger.condition = "<Inherit From Parent>";
    professionsIndicator.iconList.list.push_back(professionsIndicatorGuardianFirebrand);
    Icon_t professionsIndicatorGuardianWillbender{};
    professionsIndicatorGuardianWillbender.id = g_LayoutEditor.GenerateUID();
    professionsIndicatorGuardianWillbender.source = "URL";
    professionsIndicatorGuardianWillbender.path = "https://wiki.guildwars2.com/images/d/d8/Willbender_icon_white.png";
    professionsIndicatorGuardianWillbender.trigger.category = "Professions";
    professionsIndicatorGuardianWillbender.trigger.effect = "Guardian: Willbender";
    professionsIndicatorGuardianWillbender.trigger.condition = "<Inherit From Parent>";
    professionsIndicator.iconList.list.push_back(professionsIndicatorGuardianWillbender);
    Icon_t professionsIndicatorGuardianLuminary{};
    professionsIndicatorGuardianLuminary.id = g_LayoutEditor.GenerateUID();
    professionsIndicatorGuardianLuminary.source = "URL";
    professionsIndicatorGuardianLuminary.path = "https://wiki.guildwars2.com/images/0/02/Luminary_icon_white.png";
    professionsIndicatorGuardianLuminary.trigger.category = "Professions";
    professionsIndicatorGuardianLuminary.trigger.effect = "Guardian: Luminary";
    professionsIndicatorGuardianLuminary.trigger.condition = "<Inherit From Parent>";
    professionsIndicator.iconList.list.push_back(professionsIndicatorGuardianLuminary);
    Icon_t professionsIndicatorMesmer{};
    professionsIndicatorMesmer.id = g_LayoutEditor.GenerateUID();
    professionsIndicatorMesmer.source = "URL";
    professionsIndicatorMesmer.path = "https://wiki.guildwars2.com/images/7/74/Mesmer_icon_white.png";
    professionsIndicatorMesmer.trigger.category = "Professions";
    professionsIndicatorMesmer.trigger.effect = "Mesmer";
    professionsIndicatorMesmer.trigger.condition = "<Inherit From Parent>";
    professionsIndicator.iconList.list.push_back(professionsIndicatorMesmer);
    Icon_t professionsIndicatorMesmerChronomancer{};
    professionsIndicatorMesmerChronomancer.id = g_LayoutEditor.GenerateUID();
    professionsIndicatorMesmerChronomancer.source = "URL";
    professionsIndicatorMesmerChronomancer.path = "https://wiki.guildwars2.com/images/a/a9/Chronomancer_icon_white.png";
    professionsIndicatorMesmerChronomancer.trigger.category = "Professions";
    professionsIndicatorMesmerChronomancer.trigger.effect = "Mesmer: Chronomancer";
    professionsIndicatorMesmerChronomancer.trigger.condition = "<Inherit From Parent>";
    professionsIndicator.iconList.list.push_back(professionsIndicatorMesmerChronomancer);
    Icon_t professionsIndicatorMesmerMirage{};
    professionsIndicatorMesmerMirage.id = g_LayoutEditor.GenerateUID();
    professionsIndicatorMesmerMirage.source = "URL";
    professionsIndicatorMesmerMirage.path = "https://wiki.guildwars2.com/images/8/80/Mirage_icon_white.png";
    professionsIndicatorMesmerMirage.trigger.category = "Professions";
    professionsIndicatorMesmerMirage.trigger.effect = "Mesmer: Mirage";
    professionsIndicatorMesmerMirage.trigger.condition = "<Inherit From Parent>";
    professionsIndicator.iconList.list.push_back(professionsIndicatorMesmerMirage);
    Icon_t professionsIndicatorMesmerVirtuoso{};
    professionsIndicatorMesmerVirtuoso.id = g_LayoutEditor.GenerateUID();
    professionsIndicatorMesmerVirtuoso.source = "URL";
    professionsIndicatorMesmerVirtuoso.path = "https://wiki.guildwars2.com/images/b/bc/Virtuoso_icon_white.png";
    professionsIndicatorMesmerVirtuoso.trigger.category = "Professions";
    professionsIndicatorMesmerVirtuoso.trigger.effect = "Mesmer: Virtuoso";
    professionsIndicatorMesmerVirtuoso.trigger.condition = "<Inherit From Parent>";
    professionsIndicator.iconList.list.push_back(professionsIndicatorMesmerVirtuoso);
    Icon_t professionsIndicatorMesmerTroubadour{};
    professionsIndicatorMesmerTroubadour.id = g_LayoutEditor.GenerateUID();
    professionsIndicatorMesmerTroubadour.source = "URL";
    professionsIndicatorMesmerTroubadour.path = "https://wiki.guildwars2.com/images/b/b2/Troubadour_icon_white.png";
    professionsIndicatorMesmerTroubadour.trigger.category = "Professions";
    professionsIndicatorMesmerTroubadour.trigger.effect = "Mesmer: Troubadour";
    professionsIndicatorMesmerTroubadour.trigger.condition = "<Inherit From Parent>";
    professionsIndicator.iconList.list.push_back(professionsIndicatorMesmerTroubadour);
    Icon_t professionsIndicatorNecromancer{};
    professionsIndicatorNecromancer.id = g_LayoutEditor.GenerateUID();
    professionsIndicatorNecromancer.source = "URL";
    professionsIndicatorNecromancer.path = "https://wiki.guildwars2.com/images/a/ab/Necromancer_icon_white.png";
    professionsIndicatorNecromancer.trigger.category = "Professions";
    professionsIndicatorNecromancer.trigger.effect = "Necromancer";
    professionsIndicatorNecromancer.trigger.condition = "<Inherit From Parent>";
    professionsIndicator.iconList.list.push_back(professionsIndicatorNecromancer);
    Icon_t professionsIndicatorNecromancerReaper{};
    professionsIndicatorNecromancerReaper.id = g_LayoutEditor.GenerateUID();
    professionsIndicatorNecromancerReaper.source = "URL";
    professionsIndicatorNecromancerReaper.path = "https://wiki.guildwars2.com/images/b/be/Reaper_icon_white.png";
    professionsIndicatorNecromancerReaper.trigger.category = "Professions";
    professionsIndicatorNecromancerReaper.trigger.effect = "Necromancer: Reaper";
    professionsIndicatorNecromancerReaper.trigger.condition = "<Inherit From Parent>";
    professionsIndicator.iconList.list.push_back(professionsIndicatorNecromancerReaper);
    Icon_t professionsIndicatorNecromancerScourge{};
    professionsIndicatorNecromancerScourge.id = g_LayoutEditor.GenerateUID();
    professionsIndicatorNecromancerScourge.source = "URL";
    professionsIndicatorNecromancerScourge.path = "https://wiki.guildwars2.com/images/8/80/Scourge_icon_white.png";
    professionsIndicatorNecromancerScourge.trigger.category = "Professions";
    professionsIndicatorNecromancerScourge.trigger.effect = "Necromancer: Scourge";
    professionsIndicatorNecromancerScourge.trigger.condition = "<Inherit From Parent>";
    professionsIndicator.iconList.list.push_back(professionsIndicatorNecromancerScourge);
    Icon_t professionsIndicatorNecromancerHarbinger{};
    professionsIndicatorNecromancerHarbinger.id = g_LayoutEditor.GenerateUID();
    professionsIndicatorNecromancerHarbinger.source = "URL";
    professionsIndicatorNecromancerHarbinger.path = "https://wiki.guildwars2.com/images/a/a7/Harbinger_icon_white.png";
    professionsIndicatorNecromancerHarbinger.trigger.category = "Professions";
    professionsIndicatorNecromancerHarbinger.trigger.effect = "Necromancer: Harbinger";
    professionsIndicatorNecromancerHarbinger.trigger.condition = "<Inherit From Parent>";
    professionsIndicator.iconList.list.push_back(professionsIndicatorNecromancerHarbinger);
    Icon_t professionsIndicatorNecromancerRitualist{};
    professionsIndicatorNecromancerRitualist.id = g_LayoutEditor.GenerateUID();
    professionsIndicatorNecromancerRitualist.source = "URL";
    professionsIndicatorNecromancerRitualist.path = "https://wiki.guildwars2.com/images/0/03/Ritualist_icon_white.png";
    professionsIndicatorNecromancerRitualist.trigger.category = "Professions";
    professionsIndicatorNecromancerRitualist.trigger.effect = "Necromancer: Ritualist";
    professionsIndicatorNecromancerRitualist.trigger.condition = "<Inherit From Parent>";
    professionsIndicator.iconList.list.push_back(professionsIndicatorNecromancerRitualist);
    Icon_t professionsIndicatorRanger{};
    professionsIndicatorRanger.id = g_LayoutEditor.GenerateUID();
    professionsIndicatorRanger.source = "URL";
    professionsIndicatorRanger.path = "https://wiki.guildwars2.com/images/0/06/Ranger_icon_white.png";
    professionsIndicatorRanger.trigger.category = "Professions";
    professionsIndicatorRanger.trigger.effect = "Ranger";
    professionsIndicatorRanger.trigger.condition = "<Inherit From Parent>";
    professionsIndicator.iconList.list.push_back(professionsIndicatorRanger);
    Icon_t professionsIndicatorRangerDruid{};
    professionsIndicatorRangerDruid.id = g_LayoutEditor.GenerateUID();
    professionsIndicatorRangerDruid.source = "URL";
    professionsIndicatorRangerDruid.path = "https://wiki.guildwars2.com/images/0/01/Druid_icon_white.png";
    professionsIndicatorRangerDruid.trigger.category = "Professions";
    professionsIndicatorRangerDruid.trigger.effect = "Ranger: Druid";
    professionsIndicatorRangerDruid.trigger.condition = "<Inherit From Parent>";
    professionsIndicator.iconList.list.push_back(professionsIndicatorRangerDruid);
    Icon_t professionsIndicatorRangerSoulbeast{};
    professionsIndicatorRangerSoulbeast.id = g_LayoutEditor.GenerateUID();
    professionsIndicatorRangerSoulbeast.source = "URL";
    professionsIndicatorRangerSoulbeast.path = "https://wiki.guildwars2.com/images/1/14/Soulbeast_icon_white.png";
    professionsIndicatorRangerSoulbeast.trigger.category = "Professions";
    professionsIndicatorRangerSoulbeast.trigger.effect = "Ranger: Soulbeast";
    professionsIndicatorRangerSoulbeast.trigger.condition = "<Inherit From Parent>";
    professionsIndicator.iconList.list.push_back(professionsIndicatorRangerSoulbeast);
    Icon_t professionsIndicatorRangerUntamed{};
    professionsIndicatorRangerUntamed.id = g_LayoutEditor.GenerateUID();
    professionsIndicatorRangerUntamed.source = "URL";
    professionsIndicatorRangerUntamed.path = "https://wiki.guildwars2.com/images/1/13/Untamed_icon_white.png";
    professionsIndicatorRangerUntamed.trigger.category = "Professions";
    professionsIndicatorRangerUntamed.trigger.effect = "Ranger: Untamed";
    professionsIndicatorRangerUntamed.trigger.condition = "<Inherit From Parent>";
    professionsIndicator.iconList.list.push_back(professionsIndicatorRangerUntamed);
    Icon_t professionsIndicatorRangerGaleshot{};
    professionsIndicatorRangerGaleshot.id = g_LayoutEditor.GenerateUID();
    professionsIndicatorRangerGaleshot.source = "URL";
    professionsIndicatorRangerGaleshot.path = "https://wiki.guildwars2.com/images/b/be/Galeshot_icon_white.png";
    professionsIndicatorRangerGaleshot.trigger.category = "Professions";
    professionsIndicatorRangerGaleshot.trigger.effect = "Ranger: Galeshot";
    professionsIndicatorRangerGaleshot.trigger.condition = "<Inherit From Parent>";
    professionsIndicator.iconList.list.push_back(professionsIndicatorRangerGaleshot);
    Icon_t professionsIndicatorRevenant{};
    professionsIndicatorRevenant.id = g_LayoutEditor.GenerateUID();
    professionsIndicatorRevenant.source = "URL";
    professionsIndicatorRevenant.path = "https://wiki.guildwars2.com/images/0/08/Revenant_icon_white.png";
    professionsIndicatorRevenant.trigger.category = "Professions";
    professionsIndicatorRevenant.trigger.effect = "Revenant";
    professionsIndicatorRevenant.trigger.condition = "<Inherit From Parent>";
    professionsIndicator.iconList.list.push_back(professionsIndicatorRevenant);
    Icon_t professionsIndicatorRevenantHerald{};
    professionsIndicatorRevenantHerald.id = g_LayoutEditor.GenerateUID();
    professionsIndicatorRevenantHerald.source = "URL";
    professionsIndicatorRevenantHerald.path = "https://wiki.guildwars2.com/images/9/9f/Herald_icon_white.png";
    professionsIndicatorRevenantHerald.trigger.category = "Professions";
    professionsIndicatorRevenantHerald.trigger.effect = "Revenant: Herald";
    professionsIndicatorRevenantHerald.trigger.condition = "<Inherit From Parent>";
    professionsIndicator.iconList.list.push_back(professionsIndicatorRevenantHerald);
    Icon_t professionsIndicatorRevenantRenegade{};
    professionsIndicatorRevenantRenegade.id = g_LayoutEditor.GenerateUID();
    professionsIndicatorRevenantRenegade.source = "URL";
    professionsIndicatorRevenantRenegade.path = "https://wiki.guildwars2.com/images/d/da/Renegade_icon_white.png";
    professionsIndicatorRevenantRenegade.trigger.category = "Professions";
    professionsIndicatorRevenantRenegade.trigger.effect = "Revenant: Renegade";
    professionsIndicatorRevenantRenegade.trigger.condition = "<Inherit From Parent>";
    professionsIndicator.iconList.list.push_back(professionsIndicatorRevenantRenegade);
    Icon_t professionsIndicatorRevenantVindicator{};
    professionsIndicatorRevenantVindicator.id = g_LayoutEditor.GenerateUID();
    professionsIndicatorRevenantVindicator.source = "URL";
    professionsIndicatorRevenantVindicator.path = "https://wiki.guildwars2.com/images/2/20/Vindicator_icon_white.png";
    professionsIndicatorRevenantVindicator.trigger.category = "Professions";
    professionsIndicatorRevenantVindicator.trigger.effect = "Revenant: Vindicator";
    professionsIndicatorRevenantVindicator.trigger.condition = "<Inherit From Parent>";
    professionsIndicator.iconList.list.push_back(professionsIndicatorRevenantVindicator);
    Icon_t professionsIndicatorRevenantConduit{};
    professionsIndicatorRevenantConduit.id = g_LayoutEditor.GenerateUID();
    professionsIndicatorRevenantConduit.source = "URL";
    professionsIndicatorRevenantConduit.path = "https://wiki.guildwars2.com/images/5/56/Conduit_icon_white.png";
    professionsIndicatorRevenantConduit.trigger.category = "Professions";
    professionsIndicatorRevenantConduit.trigger.effect = "Revenant: Conduit";
    professionsIndicatorRevenantConduit.trigger.condition = "<Inherit From Parent>";
    professionsIndicator.iconList.list.push_back(professionsIndicatorRevenantConduit);
    Icon_t professionsIndicatorThief{};
    professionsIndicatorThief.id = g_LayoutEditor.GenerateUID();
    professionsIndicatorThief.source = "URL";
    professionsIndicatorThief.path = "https://wiki.guildwars2.com/images/6/62/Thief_icon_white.png";
    professionsIndicatorThief.trigger.category = "Professions";
    professionsIndicatorThief.trigger.effect = "Thief";
    professionsIndicatorThief.trigger.condition = "<Inherit From Parent>";
    professionsIndicator.iconList.list.push_back(professionsIndicatorThief);
    Icon_t professionsIndicatorThiefDaredevil{};
    professionsIndicatorThiefDaredevil.id = g_LayoutEditor.GenerateUID();
    professionsIndicatorThiefDaredevil.source = "URL";
    professionsIndicatorThiefDaredevil.path = "https://wiki.guildwars2.com/images/6/64/Daredevil_icon_white.png";
    professionsIndicatorThiefDaredevil.trigger.category = "Professions";
    professionsIndicatorThiefDaredevil.trigger.effect = "Thief: Daredevil";
    professionsIndicatorThiefDaredevil.trigger.condition = "<Inherit From Parent>";
    professionsIndicator.iconList.list.push_back(professionsIndicatorThiefDaredevil);
    Icon_t professionsIndicatorThiefDeadeye{};
    professionsIndicatorThiefDeadeye.id = g_LayoutEditor.GenerateUID();
    professionsIndicatorThiefDeadeye.source = "URL";
    professionsIndicatorThiefDeadeye.path = "https://wiki.guildwars2.com/images/3/3d/Deadeye_icon_white.png";
    professionsIndicatorThiefDeadeye.trigger.category = "Professions";
    professionsIndicatorThiefDeadeye.trigger.effect = "Thief: Deadeye";
    professionsIndicatorThiefDeadeye.trigger.condition = "<Inherit From Parent>";
    professionsIndicator.iconList.list.push_back(professionsIndicatorThiefDeadeye);
    Icon_t professionsIndicatorThiefSpecter{};
    professionsIndicatorThiefSpecter.id = g_LayoutEditor.GenerateUID();
    professionsIndicatorThiefSpecter.source = "URL";
    professionsIndicatorThiefSpecter.path = "https://wiki.guildwars2.com/images/3/38/Specter_icon_white.png";
    professionsIndicatorThiefSpecter.trigger.category = "Professions";
    professionsIndicatorThiefSpecter.trigger.effect = "Thief: Specter";
    professionsIndicatorThiefSpecter.trigger.condition = "<Inherit From Parent>";
    professionsIndicator.iconList.list.push_back(professionsIndicatorThiefSpecter);
    Icon_t professionsIndicatorThiefAntiquary{};
    professionsIndicatorThiefAntiquary.id = g_LayoutEditor.GenerateUID();
    professionsIndicatorThiefAntiquary.source = "URL";
    professionsIndicatorThiefAntiquary.path = "https://wiki.guildwars2.com/images/7/7d/Antiquary_icon_white.png";
    professionsIndicatorThiefAntiquary.trigger.category = "Professions";
    professionsIndicatorThiefAntiquary.trigger.effect = "Thief: Antiquary";
    professionsIndicatorThiefAntiquary.trigger.condition = "<Inherit From Parent>";
    professionsIndicator.iconList.list.push_back(professionsIndicatorThiefAntiquary);
    Icon_t professionsIndicatorWarrior{};
    professionsIndicatorWarrior.id = g_LayoutEditor.GenerateUID();
    professionsIndicatorWarrior.source = "URL";
    professionsIndicatorWarrior.path = "https://wiki.guildwars2.com/images/7/7c/Warrior_icon_white.png";
    professionsIndicatorWarrior.trigger.category = "Professions";
    professionsIndicatorWarrior.trigger.effect = "Warrior";
    professionsIndicatorWarrior.trigger.condition = "<Inherit From Parent>";
    professionsIndicator.iconList.list.push_back(professionsIndicatorWarrior);
    Icon_t professionsIndicatorWarriorBerserker{};
    professionsIndicatorWarriorBerserker.id = g_LayoutEditor.GenerateUID();
    professionsIndicatorWarriorBerserker.source = "URL";
    professionsIndicatorWarriorBerserker.path = "https://wiki.guildwars2.com/images/a/a3/Berserker_icon_white.png";
    professionsIndicatorWarriorBerserker.trigger.category = "Professions";
    professionsIndicatorWarriorBerserker.trigger.effect = "Warrior: Berserker";
    professionsIndicatorWarriorBerserker.trigger.condition = "<Inherit From Parent>";
    professionsIndicator.iconList.list.push_back(professionsIndicatorWarriorBerserker);
    Icon_t professionsIndicatorWarriorSpellbreaker{};
    professionsIndicatorWarriorSpellbreaker.id = g_LayoutEditor.GenerateUID();
    professionsIndicatorWarriorSpellbreaker.source = "URL";
    professionsIndicatorWarriorSpellbreaker.path = "https://wiki.guildwars2.com/images/1/17/Spellbreaker_icon_white.png";
    professionsIndicatorWarriorSpellbreaker.trigger.category = "Professions";
    professionsIndicatorWarriorSpellbreaker.trigger.effect = "Warrior: Spellbreaker";
    professionsIndicatorWarriorSpellbreaker.trigger.condition = "<Inherit From Parent>";
    professionsIndicator.iconList.list.push_back(professionsIndicatorWarriorSpellbreaker);
    Icon_t professionsIndicatorWarriorBladesworn{};
    professionsIndicatorWarriorBladesworn.id = g_LayoutEditor.GenerateUID();
    professionsIndicatorWarriorBladesworn.source = "URL";
    professionsIndicatorWarriorBladesworn.path = "https://wiki.guildwars2.com/images/a/af/Bladesworn_icon_white.png";
    professionsIndicatorWarriorBladesworn.trigger.category = "Professions";
    professionsIndicatorWarriorBladesworn.trigger.effect = "Warrior: Bladesworn";
    professionsIndicatorWarriorBladesworn.trigger.condition = "<Inherit From Parent>";
    professionsIndicator.iconList.list.push_back(professionsIndicatorWarriorBladesworn);
    Icon_t professionsIndicatorWarriorParagon{};
    professionsIndicatorWarriorParagon.id = g_LayoutEditor.GenerateUID();
    professionsIndicatorWarriorParagon.source = "URL";
    professionsIndicatorWarriorParagon.path = "https://wiki.guildwars2.com/images/1/17/Paragon_icon_white.png";
    professionsIndicatorWarriorParagon.trigger.category = "Professions";
    professionsIndicatorWarriorParagon.trigger.effect = "Warrior: Paragon";
    professionsIndicatorWarriorParagon.trigger.condition = "<Inherit From Parent>";
    professionsIndicator.iconList.list.push_back(professionsIndicatorWarriorParagon);
    layout.indicators.push_back(professionsIndicator);

    return layout;
}

static LayoutConfig_t GetDefaultRadialLayout()
{
    LayoutConfig_t layout{};
    layout.colors = "Default";
    layout.position.anchor = "Centre";
    layout.position.offset = { 0, 0 };
    layout.layout.type = "Radial";
    // Radial specific properties
    layout.layout.radial.sectorRadiusInner = 60;
    layout.layout.radial.sectorRadiusOuter = 180;
    layout.layout.radial.sectorCountMin = 4;
    layout.layout.radial.sectorCountMax = 10;
    layout.layout.radial.itemSpacing = 4;

    return layout;
}

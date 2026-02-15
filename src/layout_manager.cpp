#include "layout_manager.h"

#include <string>

#include "shared.h"

void LayoutManager::Create(const std::string& name, const std::string& type, const std::filesystem::path& fileDir)
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

    // Construct an empty layout
    LayoutConfig_t layout{};
    layout.name = name;
    layout.layout.type = type;
    layout.id = g_LayoutEditor.GenerateUID();

    m_layouts.insert_or_assign(filePath, layout);
    m_idToPath.insert_or_assign(layout.id, filePath);

    if (m_onLayoutCreatedCallback) {
        m_onLayoutCreatedCallback(filePath, layout.id, layout.name, layout.layout.type);
    }
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

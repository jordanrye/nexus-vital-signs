#pragma once

#include <filesystem>
#include <functional>
#include <map>
#include <string>
#include <vector>

#include "imgui_tree_view.h"
#include "layout_types.h"

class LayoutManager
{
public:
    // Callback types for UI decoupling
    using OnLayoutCreatedCallback = std::function<void(const std::filesystem::path& path, TreeNodeUID uid, const std::string& label, const std::string& type)>;
    using OnLayoutDeletedCallback = std::function<void(TreeNodeUID uid)>;
    using OnIndicatorCreatedCallback = std::function<void(TreeNodeUID parentId, TreeNodeUID indicatorId, const std::string& label, const std::string& type)>;
    using OnIndicatorDeletedCallback = std::function<void(TreeNodeUID indicatorId)>;
    using OnIndicatorSwapCallback = std::function<void(TreeNodeUID layoutId, size_t oldIndex, size_t newIndex)>;

    // Data management
    void Create(const std::string& name, const std::string& type, bool createFromTemplate, const std::filesystem::path& path);
    void Insert(const std::filesystem::path& path, const LayoutConfig_t& layout);
    bool Delete(TreeNodeUID id);
    TreeNodeUID AddIndicator(TreeNodeUID parentId, const std::string& name, const std::string& type);
    bool DeleteIndicator(TreeNodeUID indicatorId);
    void ReorderIndicators(TreeNodeUID layoutId, size_t oldIndex, size_t newIndex);

    // Helper functions
    std::map<std::filesystem::path, LayoutConfig_t>& GetAllLayouts();
    LayoutConfig_t& GetLayoutFromFilePath(const std::filesystem::path& path);
    LayoutConfig_t* GetLayoutFromNodeID(TreeNodeUID id);
    Indicator_t* GetIndicator(TreeNodeUID id);

    // Register observers
    void SetOnLayoutCreatedCallback(OnLayoutCreatedCallback cb) { m_onLayoutCreatedCallback = cb; }
    void SetOnLayoutDeletedCallback(OnLayoutDeletedCallback cb) { m_onLayoutDeletedCallback = cb; }
    void SetOnIndicatorCreatedCallback(OnIndicatorCreatedCallback cb) { m_onIndicatorCreatedCallback = cb; }
    void SetOnIndicatorDeletedCallback(OnIndicatorDeletedCallback cb) { m_onIndicatorDeletedCallback = cb; }
    void SetOnIndicatorSwapCallback(OnIndicatorSwapCallback cb) { m_onIndicatorSwapCallback = cb; }

private:
    std::map<std::filesystem::path, LayoutConfig_t> m_layouts;
    std::map<TreeNodeUID, std::filesystem::path> m_idToPath;
    std::map<TreeNodeUID, TreeNodeUID> m_indicatorToLayout;

    // Observers
    OnLayoutCreatedCallback m_onLayoutCreatedCallback;
    OnLayoutDeletedCallback m_onLayoutDeletedCallback;
    OnIndicatorCreatedCallback m_onIndicatorCreatedCallback;
    OnIndicatorDeletedCallback m_onIndicatorDeletedCallback;
    OnIndicatorSwapCallback m_onIndicatorSwapCallback;

    void RegisterIndicatorsRecursive(TreeNodeUID parentId, const std::vector<Indicator_t>& indicators);
};

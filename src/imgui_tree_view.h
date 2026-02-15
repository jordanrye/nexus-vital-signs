#pragma once

#include <algorithm>
#include <functional>
#include <map>
#include <string>
#include <vector>

#include "imgui/imgui.h"

/* Typedef for content rendering functions */
using ContentRenderer = std::function<void()>;

enum class TreeNodeUID {
    NONE = 0,
    DYNAMIC_START = 1
};

enum class TreeNodeType {
    ROOT = 0,
    BRANCH = 1,
    LEAF = 2
};

/**
 * @brief Prefix increment operator for TreeNodeUID.
 */
inline TreeNodeUID& operator++(TreeNodeUID& UID) {
    UID = static_cast<TreeNodeUID>(static_cast<int>(UID) + 1);
    return UID;
}

/**
 * @brief Postfix increment operator for TreeNodeUID.
 */
inline TreeNodeUID operator++(TreeNodeUID& UID, int) {
    TreeNodeUID tmp = UID;
    ++UID; 
    return tmp;
}

/**
 * @brief Represents a single node in the menu tree.
 */
struct TreeNode
{
    TreeNodeUID id;
    std::string labelName;
    std::string labelType;
    std::vector<TreeNode> children;
    TreeNodeType type = TreeNodeType::LEAF;

    /* Helpers to determine the type of node. */
    bool is_root() const { return (type == TreeNodeType::ROOT); }
    bool is_branch() const { return (type == TreeNodeType::BRANCH); }
    bool is_leaf() const { return (type == TreeNodeType::LEAF) || children.empty(); }
};

class TreeView
{
public:
    // Callback functions
    using AddIndicatorCallback = std::function<void(TreeNodeUID, const std::string&, const std::string&)>;
    using DeleteNodeCallback = std::function<bool(TreeNodeUID)>;
    using ReorderNodeCallback = std::function<void(TreeNodeUID, size_t, size_t)>;

    // Constructors
    TreeView() = default;

    // Model management
    void AppendNode(TreeNodeUID parentId, TreeNodeUID id, const std::string& labelName, const std::string& labelType, TreeNodeType type, std::vector<TreeNode>&& children = {});
    void RemoveNode(TreeNodeUID id);
    void SwapNode(TreeNodeUID parentId, size_t idx_1, size_t idx_2);
    
    // View management
    void RegisterContentView(TreeNodeUID id, ContentRenderer&& renderer);
    void RenderTreeView(DeleteNodeCallback deleteCb, AddIndicatorCallback addCb, ReorderNodeCallback reorderCb);
    void RenderContentView();
    void SetActiveNode(TreeNodeUID id) { m_selectedId = id; }
    TreeNodeUID GetActiveNode() const { return m_selectedId; }
    void UpdateNodeLabel(TreeNodeUID id, const std::string& new_label);
    void UpdateTreeViewHeader(const std::string& title) { m_title = title; }

    // Helper functions
    TreeNodeUID GenerateUID() { return m_nextId++; }
    void Clear() { Nodes.clear(); Forms.clear(); }

private:
    std::vector<TreeNode> Nodes;
    std::map<TreeNodeUID, ContentRenderer> Forms;

    TreeNodeUID m_selectedId = TreeNodeUID::NONE;
    TreeNodeUID m_nextId = TreeNodeUID::DYNAMIC_START;
    std::string m_title = "Select Node...";

    // Internal helpers
    TreeNode* FindNode(std::vector<TreeNode>& nodes, TreeNodeUID id);
    void RenderNodes(std::vector<TreeNode>& nodes, TreeNodeUID parent_id, DeleteNodeCallback deleteCb, AddIndicatorCallback addCb, ReorderNodeCallback reorderCb);
    bool RenderNode(const char* labelName, const char* labelType, TreeNodeUID id, ImGuiTreeNodeFlags flags, bool& is_clicked);
};

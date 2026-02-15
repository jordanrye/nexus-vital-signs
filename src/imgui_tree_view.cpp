#include "imgui_tree_view.h"

#include <Windows.h>
#include <algorithm>
#include <queue>

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui_extensions.h"

#include "forms/forms.h"
#include "utils_deletion.h"

void TreeView::AppendNode(TreeNodeUID parentId, TreeNodeUID id, const std::string& labelName, const std::string& labelType, TreeNodeType type, std::vector<TreeNode>&& children)
{
    if (parentId == TreeNodeUID::NONE)
    {
        Nodes.push_back({id, labelName, labelType, std::move(children), type});
    }
    else if (TreeNode* parent = FindNode(Nodes, parentId))
    {
        parent->children.push_back({id, labelName, labelType, std::move(children), type});
    }
}

static bool RemoveNodeRecursive(std::vector<TreeNode>& nodes, TreeNodeUID id)
{
    for (auto it = nodes.begin(); it != nodes.end(); ++it)
    {
        if (it->id == id)
        {
            nodes.erase(it);
            return true;
        }
        if (RemoveNodeRecursive(it->children, id))
        {
            return true;
        }
    }
    return false;
}

void TreeView::RemoveNode(TreeNodeUID id)
{
    if (RemoveNodeRecursive(Nodes, id))
    {
        if (m_selectedId != TreeNodeUID::NONE && FindNode(Nodes, m_selectedId) == nullptr)
        {
            m_selectedId = TreeNodeUID::NONE;
        }
    }
}

void TreeView::SwapNode(TreeNodeUID parentId, size_t idx_1, size_t idx_2)
{
    TreeNode* parent = FindNode(Nodes, parentId);
    if (parent && (idx_1 < parent->children.size()) && (idx_2 < parent->children.size()))
    {
        std::swap(parent->children[idx_1], parent->children[idx_2]);
    }
}

void TreeView::UpdateNodeLabel(TreeNodeUID id, const std::string& new_label)
{
    TreeNode* node = FindNode(Nodes, id);
    if (node)
    {
        node->labelName = new_label;
    }
}

void TreeView::RegisterContentView(TreeNodeUID id, ContentRenderer&& renderer)
{
    Forms.insert_or_assign(id, std::move(renderer));
}

void TreeView::RenderTreeView(DeleteNodeCallback deleteCb, AddIndicatorCallback addCb, ReorderNodeCallback reorderCb)
{
    static const float menuWidth = 200.0f;

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.f, 2.f));
    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 10.f);

    if (ImGui::BeginChild("TreeViewView", ImVec2(menuWidth, 0), true))
    {
        ImGui::TextDisabled(m_title.c_str());
        ImGui::Separator();

        RenderNodes(Nodes, TreeNodeUID::NONE, deleteCb, addCb, reorderCb);

        ImGui::EndChild();
    }

    ImGui::PopStyleVar(2);

    /* Start the ContentView on the same line. */
    ImGui::SameLine();
}

void TreeView::RenderContentView()
{
    if (ImGui::BeginChild("ContentView", ImVec2(0, 0), true))
    {
        auto it = Forms.find(m_selectedId);

        if (it != Forms.end())
        {
            /* Call the registered rendering function */
            it->second();
        }

        ImGui::EndChild();
    }
}

TreeNode* TreeView::FindNode(std::vector<TreeNode>& nodes, TreeNodeUID id)
{
    for (auto& node : nodes)
    {
        if (node.id == id) return &node;
        
        TreeNode* found = FindNode(node.children, id);
        if (found) return found;
    }
    return nullptr;
}

void TreeView::RenderNodes(std::vector<TreeNode>& nodes, TreeNodeUID parent_id, DeleteNodeCallback deleteCb, AddIndicatorCallback addCb, ReorderNodeCallback reorderCb)
{
    DeletionQueue queueDelete;

    for (int i = 0; i < nodes.size(); i++)
    {
        auto& node = nodes[i];
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
        bool is_open = false;
        bool is_tree_node_clicked = false;
        bool is_button_clicked = false;
        int idx_button = 0;

        ImGui::PushID(static_cast<int>(node.id));
        {
            if (node.is_leaf())
            {
                flags |= (ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            }
    
            is_open = RenderNode(node.labelName.c_str(), node.labelType.c_str(), node.id, flags, is_tree_node_clicked);
    
            /* Action buttons */
            {
                float buttonSpacing = ImGui::GetItemRectSize().y + ImGui::GetStyle().ItemSpacing.x; /* button width + item spacing */
    
                if (deleteCb)
                {
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 0.f));
                    ImGui::SameLine(ImGui::GetContentRegionMax().x - (++idx_button * buttonSpacing));
                    if (ImGui::ButtonCross("x"))
                    {
                        is_button_clicked = true;
                        ImGui::OpenPopup("Delete");
                    }
                    ImGui::PopStyleVar(1);
    
                    if (ImGui::BeginPopupModal("Delete", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
                    {
                        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + 200.0f);
                        ImGui::Text(std::string("Are you sure you want to delete '" + node.labelName + "'? This action is permanent.").c_str());
                        ImGui::PopTextWrapPos();
    
                        ImGui::Spacing();
                        ImGui::Separator();
                        ImGui::Spacing();
    
                        if (ImGui::Button("Cancel", ImVec2(100, 0)))
                        {
                            ImGui::CloseCurrentPopup();
                        }
    
                        ImGui::SameLine();
    
                        ImGui::PushStyleColor(ImGuiCol_Button, (ImU32)ImColor(170, 30, 30));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImU32)ImColor(210, 40, 40));
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImU32)ImColor(130, 20, 20));
                        if (ImGui::Button("Delete", ImVec2(100, 0)))
                        {
                            queueDelete.Queue(i);
                            ImGui::CloseCurrentPopup();
                        }
                        ImGui::PopStyleColor(3);
    
                        ImGui::EndPopup();
                    }
                }
    
                if (!node.is_root() && reorderCb)
                {
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 0.f));
                    ImGui::SameLine(ImGui::GetContentRegionMax().x - (++idx_button * buttonSpacing));
                    if (ImGui::ButtonArrow("MoveDown", ImGuiDir_Down, i == nodes.size()-1))
                    {
                        is_button_clicked = true;
                        if (parent_id != TreeNodeUID::NONE && reorderCb)
                        {
                            reorderCb(parent_id, i, i + 1);
                        }
                    }
                    ImGui::PopStyleVar(1);
            
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 0.f));
                    ImGui::SameLine(ImGui::GetContentRegionMax().x - (++idx_button * buttonSpacing));
                    if (ImGui::ButtonArrow("MoveUp", ImGuiDir_Up, i == 0))
                    {
                        is_button_clicked = true;
                        if (parent_id != TreeNodeUID::NONE && reorderCb)
                        {
                            reorderCb(parent_id, i, i - 1);
                        }
                    }
                    ImGui::PopStyleVar(1);
                }
    
                if ((node.is_root() || node.is_branch()) && addCb)
                {
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 0.f));
                    ImGui::SameLine(ImGui::GetContentRegionMax().x - (++idx_button * buttonSpacing));
                    if (ImGui::ButtonPlus("+"))
                    {
                        is_button_clicked = true;
                        ImGui::OpenPopup("Add");
                    }
                    ImGui::PopStyleVar(1);
    
                    static char inputBuff_Name[MAX_PATH] = "";
                    static std::string indicatorType = "";
    
                    if (ImGui::BeginPopupModal("Add", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
                    {
                        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + 200.0f);
                        ImGui::Text(std::string("Add a new indicator to '" + node.labelName + "'.").c_str());
                        ImGui::PopTextWrapPos();
    
                        ImGui::InputText("Name", inputBuff_Name, IM_ARRAYSIZE(inputBuff_Name));
                        form_SelectIndicatorType(indicatorType);
    
                        ImGui::Spacing();
                        ImGui::Separator();
                        ImGui::Spacing();
    
                        if (ImGui::Button("Cancel", ImVec2(100, 0)))
                        {
                            memset(inputBuff_Name, 0, sizeof(inputBuff_Name));
                            ImGui::CloseCurrentPopup();
                        }
    
                        ImGui::SameLine();
    
                        ImGui::PushStyleColor(ImGuiCol_Button, (ImU32)ImColor(38, 128, 20));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImU32)ImColor(48, 160, 25));
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImU32)ImColor(30, 100, 15));
    
                        if (ImGui::Button("Add", ImVec2(100, 0)))
                        {
                            if (addCb)
                            {
                                addCb(node.id, std::string(inputBuff_Name), indicatorType);
                            }
    
                            memset(inputBuff_Name, 0, sizeof(inputBuff_Name));
                            ImGui::CloseCurrentPopup();
                        }
                        ImGui::PopStyleColor(3);
                        
                        ImGui::EndPopup();
                    }
                }
            }
    
            if (is_tree_node_clicked && !is_button_clicked)
            {
                m_selectedId = (m_selectedId == node.id) ? TreeNodeUID::NONE : node.id;
            }
    
            if (is_open)
            {
                if (!node.is_leaf())
                {
                    RenderNodes(node.children, node.id, deleteCb, addCb, reorderCb);
                }
                ImGui::TreePop();
            }
        }
        ImGui::PopID();
    }

    if (queueDelete.HasItems())
    {
        std::vector<int> indices;
        queueDelete.Apply([&](int idx) {
            indices.push_back(idx);
        });
        std::sort(indices.begin(), indices.end(), std::greater<int>());

        for (int idx : indices) {
            if (deleteCb) {
                deleteCb(nodes[idx].id);
            }
        }
    }
}

bool TreeView::RenderNode(const char* labelName, const char* labelType, TreeNodeUID id, ImGuiTreeNodeFlags flags, bool& is_clicked)
{
    bool is_open = false;
    bool is_selected = (m_selectedId == id);
    bool is_leaf = (flags & ImGuiTreeNodeFlags_Leaf);

    if (is_selected)
    {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    if (is_leaf)
    {
        flags |= ImGuiTreeNodeFlags_NoTreePushOnOpen;
    }

    is_open = ImGui::TreeNodeEx((void*)(intptr_t)id, flags, "");
    is_clicked = ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Left) && !ImGui::IsItemToggledOpen();

    ImGui::SetItemAllowOverlap();

    auto& item_spacing = ImGui::GetStyle().ItemSpacing.x;
    if (labelType && *labelType)
    {
        ImGui::SameLine(0, item_spacing); ImGui::TextDisabled(labelType);
    }
    if (labelName && *labelName)
    {
        ImGui::SameLine(0, item_spacing); ImGui::Text(labelName);
    }


    return ((flags & ImGuiTreeNodeFlags_Leaf) ? false : is_open);
}

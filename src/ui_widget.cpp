#include "ui_widget.h"

#include <algorithm>
#include <string>
#include <vector>

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

#include "resource.h"
#include "shared.h"
#include "ui_common.h"

namespace UI::Widget {

    enum class EGroupIcon
    {
        Solo,
        Party,
        Squad,
        Raid,
        Mentor,
        Leave,
        ReadyCheck,
        Ready,
        NotReady,
        SquadMessage
    };

    static Texture* GetGroupIconTexture(const EGroupIcon icon, const bool isHovered)
    {
        switch (icon)
        {
            case EGroupIcon::Party:
                return isHovered
                    ? UI::GetOrCreateTextureFromResource("TEX_VS_GROUP_ICON_PARTY_HOVERED", IDB_GROUP_ICON_PARTY_HOVERED)
                    : UI::GetOrCreateTextureFromResource("TEX_VS_GROUP_ICON_PARTY_NORMAL", IDB_GROUP_ICON_PARTY_NORMAL);
            case EGroupIcon::Squad:
                return isHovered
                    ? UI::GetOrCreateTextureFromResource("TEX_VS_GROUP_ICON_SQUAD_HOVERED", IDB_GROUP_ICON_SQUAD_HOVERED)
                    : UI::GetOrCreateTextureFromResource("TEX_VS_GROUP_ICON_SQUAD_NORMAL", IDB_GROUP_ICON_SQUAD_NORMAL);
            case EGroupIcon::Raid:
                return isHovered
                    ? UI::GetOrCreateTextureFromResource("TEX_VS_GROUP_ICON_RAID_HOVERED", IDB_GROUP_ICON_RAID_HOVERED)
                    : UI::GetOrCreateTextureFromResource("TEX_VS_GROUP_ICON_RAID_NORMAL", IDB_GROUP_ICON_RAID_NORMAL);
            case EGroupIcon::Mentor:
                return isHovered
                    ? UI::GetOrCreateTextureFromResource("TEX_VS_GROUP_ICON_MENTOR_HOVERED", IDB_GROUP_ICON_MENTOR_HOVERED)
                    : UI::GetOrCreateTextureFromResource("TEX_VS_GROUP_ICON_MENTOR_NORMAL", IDB_GROUP_ICON_MENTOR_NORMAL);
            case EGroupIcon::Leave:
                return isHovered
                    ? UI::GetOrCreateTextureFromResource("TEX_VS_GROUP_ICON_LEAVE_HOVERED", IDB_GROUP_ICON_LEAVE_HOVERED)
                    : UI::GetOrCreateTextureFromResource("TEX_VS_GROUP_ICON_LEAVE_NORMAL", IDB_GROUP_ICON_LEAVE_NORMAL);
            case EGroupIcon::ReadyCheck:
                return isHovered
                    ? UI::GetOrCreateTextureFromResource("TEX_VS_GROUP_ICON_READY_CHECK_HOVERED", IDB_GROUP_ICON_READY_CHECK_HOVERED)
                    : UI::GetOrCreateTextureFromResource("TEX_VS_GROUP_ICON_READY_CHECK_NORMAL", IDB_GROUP_ICON_READY_CHECK_NORMAL);
            case EGroupIcon::Ready:
                return isHovered
                    ? UI::GetOrCreateTextureFromResource("TEX_VS_GROUP_ICON_READY_HOVERED", IDB_GROUP_ICON_READY_HOVERED)
                    : UI::GetOrCreateTextureFromResource("TEX_VS_GROUP_ICON_READY_NORMAL", IDB_GROUP_ICON_READY_NORMAL);
            case EGroupIcon::NotReady:
                return isHovered
                    ? UI::GetOrCreateTextureFromResource("TEX_VS_GROUP_ICON_NOT_READY_HOVERED", IDB_GROUP_ICON_NOT_READY_HOVERED)
                    : UI::GetOrCreateTextureFromResource("TEX_VS_GROUP_ICON_NOT_READY_NORMAL", IDB_GROUP_ICON_NOT_READY_NORMAL);
            case EGroupIcon::SquadMessage:
                return isHovered
                    ? UI::GetOrCreateTextureFromResource("TEX_VS_GROUP_ICON_SQUAD_MESSAGE_HOVERED", IDB_GROUP_ICON_SQUAD_MESSAGE_HOVERED)
                    : UI::GetOrCreateTextureFromResource("TEX_VS_GROUP_ICON_SQUAD_MESSAGE_NORMAL", IDB_GROUP_ICON_SQUAD_MESSAGE_NORMAL);
            default:
                return isHovered
                    ? UI::GetOrCreateTextureFromResource("TEX_VS_GROUP_ICON_SOLO_HOVERED", IDB_GROUP_ICON_SOLO_HOVERED)
                    : UI::GetOrCreateTextureFromResource("TEX_VS_GROUP_ICON_SOLO_NORMAL", IDB_GROUP_ICON_SOLO_NORMAL);
        }
    }

    /* Group icons are as tall as a button or text box, so they line up with
     * them in the current style. The width keeps the texture's own aspect
     * ratio, rounded down to a whole pixel to stay sharp. Square until the
     * texture has loaded. */
    static ImVec2 GetGroupIconSize(const Texture* texture)
    {
        const float height = ImGui::GetFrameHeight();

        if (!texture || (texture->Height == 0U))
        {
            return ImVec2(height, height);
        }

        const float aspectRatio = static_cast<float>(texture->Width) / static_cast<float>(texture->Height);

        return ImVec2(IM_FLOOR(height * aspectRatio), height);
    }

    /* Button drawn as a group icon at the cursor, swapping to the hovered
     * texture under the mouse. The tooltip is optional. */
    static bool GroupIconButton(const char* id, const EGroupIcon icon, const char* tooltip)
    {
        /* Sized from the normal texture, so hovering never changes the button's width */
        const bool isPressed = ImGui::InvisibleButton(id, GetGroupIconSize(GetGroupIconTexture(icon, false)));
        const bool isHovered = ImGui::IsItemHovered();

        /* Drawn once loaded, the button still works without it */
        Texture* texture = GetGroupIconTexture(icon, isHovered);

        if (texture)
        {
            ImGui::GetWindowDrawList()->AddImage(texture->Resource, ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
        }

        if (isHovered && tooltip)
        {
            ImGui::SetTooltip("%s", tooltip);
        }

        return isPressed;
    }

    /* Group widget, laid out after the native party/squad bar. Quick actions sit
     * next to the group icon, and the icon opens the settings popup. Actions
     * that are not valid for the group state or the client's permissions are
     * hidden, never disabled. */
    void RenderGroup(const VitalSignsDataLink::EGroupType groupType)
    {
        const char* POPUP_GROUP_SETTINGS = "GroupSettings##VitalSignsGroupWidget";
        const char* JOIN_BEHAVIOR_LABELS[] = { "Yes", "Yes, after approval", "No" };
        const uint32_t RAID_SQUAD_CAPACITY = 10U;

        /* Not persisted between sessions */
        static char inviteName[64] = "";
        static char broadcastText[256] = "";

        const bool isRaidSquad = (VitalSignsDataLink::EGroupType::Squad10 == groupType);
        const bool isSquad = (isRaidSquad || (VitalSignsDataLink::EGroupType::Squad50 == groupType));
        const bool isParty = (VitalSignsDataLink::EGroupType::Party == groupType);

        /* Lieutenants get the member view, except they can always invite as in native */
        const VitalSignsDataLink::ESquadRole squadRole = VitalsData->getClientSquadRole();
        const bool isCommander = (isSquad && (VitalSignsDataLink::ESquadRole::Commander == squadRole));
        const bool isLieutenant = (isSquad && (VitalSignsDataLink::ESquadRole::Lieutenant == squadRole));
        const bool isCommanderUnlocked = VitalsData->isClientCommanderUnlocked();
        const bool isOpenWorld = VitalsData->isOpenWorldMap();
        const bool canMentor = VitalsData->canClientMentor();
        const uint32_t squadMemberCount = VitalsData->getSquadMemberCount();
        const VitalSignsDataLink::SquadSettings_t squadSettings = VitalsData->getSquadSettings();

        /* Invite text box with a `+` button, ENTER or `+` sends the invite */
        auto inviteBox = []()
        {
            ImGui::SetNextItemWidth(160.0f);
            bool isSubmitted = ImGui::InputTextWithHint("##InviteName", "Player name", inviteName, sizeof(inviteName), ImGuiInputTextFlags_EnterReturnsTrue);
            ImGui::SameLine(0.0f, 2.0f);
            isSubmitted |= ImGui::Button("+##Invite");

            if (isSubmitted && (inviteName[0] != '\0'))
            {
                VitalsData->inviteByName(inviteName);
                inviteName[0] = '\0';
            }
        };

        /* Popup sections are separated by a rule, skipped before the first section with content */
        bool hasPopupSection = false;
        auto beginPopupSection = [&hasPopupSection]()
        {
            if (hasPopupSection)
            {
                ImGui::Separator();
            }

            hasPopupSection = true;
        };

        if (ImGui::Begin("Group##VitalSignsGroupWidget", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse))
        {
            /* Group icon */
            const EGroupIcon groupIcon = isRaidSquad ? EGroupIcon::Raid : (isSquad ? EGroupIcon::Squad : (isParty ? EGroupIcon::Party : EGroupIcon::Solo));
            if (GroupIconButton("##GroupIcon", groupIcon, nullptr))
            {
                ImGui::OpenPopup(POPUP_GROUP_SETTINGS);
            }

            /* Quick actions, right of the icon. The icons are as tall as a button, so they line up. */
            if (isSquad)
            {
                /* Divider between the group icon and the quick actions, as tall as the icon */
                ImGui::SameLine();
                ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
                ImGui::SameLine();

                ImGui::AlignTextToFramePadding();
                ImGui::Text("%u/%u", squadMemberCount, VitalsData->getSquadCapacity());

                ImGui::SameLine();

                if (GroupIconButton("##SquadMessage", EGroupIcon::SquadMessage, isCommander ? "Edit Squad Message" : "View Squad Message"))
                {
                    VitalsData->openSquadMessageDialog();
                }

                /* Ready check, cancelling reuses the Not Ready icon */
                if (VitalsData->canCancelReadyCheck())
                {
                    ImGui::SameLine();

                    if (GroupIconButton("##CancelReadyCheck", EGroupIcon::NotReady, "Cancel Ready Check"))
                    {
                        VitalsData->cancelReadyCheck();
                    }
                }
                else if (VitalsData->canStartReadyCheck())
                {
                    ImGui::SameLine();

                    if (GroupIconButton("##InitiateReadyCheck", EGroupIcon::ReadyCheck, "Initiate Ready Check"))
                    {
                        VitalsData->startReadyCheck();
                    }
                }
                else if (VitalsData->canRespondReadyCheck())
                {
                    /* Shows the client's current response, clicking toggles it */
                    const bool isClientReady = VitalsData->getUserData(VitalsData->getClientId()).IsReady;

                    ImGui::SameLine();

                    if (GroupIconButton("##ReadyCheckResponse", isClientReady ? EGroupIcon::Ready : EGroupIcon::NotReady, isClientReady ? "Ready" : "Not Ready"))
                    {
                        VitalsData->respondReadyCheck(!isClientReady);
                    }
                }

                if (isCommander)
                {
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(220.0f);

                    if (ImGui::InputTextWithHint("##Broadcast", "Broadcast to Squad", broadcastText, sizeof(broadcastText), ImGuiInputTextFlags_EnterReturnsTrue) &&
                        (broadcastText[0] != '\0'))
                    {
                        VitalsData->broadcastToSquad(broadcastText);
                        broadcastText[0] = '\0';
                    }
                }
            }

            /* Settings popup, closes when clicking elsewhere */
            if (ImGui::BeginPopup(POPUP_GROUP_SETTINGS))
            {
                /* (a) Group type, always has Leave in a group, can be empty when solo */
                if (isParty || isSquad || isOpenWorld || canMentor)
                {
                    beginPopupSection();
                }

                /* The section's buttons share one row */
                bool hasGroupTypeButton = false;
                auto beginGroupTypeButton = [&hasGroupTypeButton]()
                {
                    if (hasGroupTypeButton)
                    {
                        ImGui::SameLine();
                    }

                    hasGroupTypeButton = true;
                };

                if (!isSquad)
                {
                    /* Without a commander tag the server creates a commanderless raid
                     * squad, so only the raid squad is offered. A party is converted
                     * into the squad. */
                    if (isOpenWorld)
                    {
                        if (isCommanderUnlocked)
                        {
                            beginGroupTypeButton();

                            if (GroupIconButton("##CreateSquad", EGroupIcon::Squad, isParty ? "Convert to Squad" : "Create Squad"))
                            {
                                VitalsData->createSquad(0U, false);
                                ImGui::CloseCurrentPopup();
                            }
                        }

                        beginGroupTypeButton();

                        if (GroupIconButton("##CreateRaidSquad", EGroupIcon::Raid, isParty ? "Convert to Raid Squad" : "Create Raid Squad"))
                        {
                            VitalsData->createSquad(0U, true);
                            ImGui::CloseCurrentPopup();
                        }
                    }

                    if (canMentor)
                    {
                        beginGroupTypeButton();

                        if (GroupIconButton("##Mentor", EGroupIcon::Mentor, "Mentor"))
                        {
                            VitalsData->toggleMentor();
                            ImGui::CloseCurrentPopup();
                        }
                    }
                }
                else if (isCommander)
                {
                    if (isOpenWorld && isRaidSquad)
                    {
                        beginGroupTypeButton();

                        if (GroupIconButton("##ConvertToSquad", EGroupIcon::Squad, "Convert to Squad"))
                        {
                            VitalsData->setSquadRaidMode(false);
                            ImGui::CloseCurrentPopup();
                        }
                    }
                    else if (isOpenWorld && (squadMemberCount <= RAID_SQUAD_CAPACITY))
                    {
                        beginGroupTypeButton();

                        if (GroupIconButton("##ConvertToRaidSquad", EGroupIcon::Raid, "Convert to Raid Squad"))
                        {
                            VitalsData->setSquadRaidMode(true);
                            ImGui::CloseCurrentPopup();
                        }
                    }
                }
                else if (!VitalsData->hasSquadCommander() && isCommanderUnlocked)
                {
                    /* Native takes command without changing the squad size, so the icon matches the current squad */
                    beginGroupTypeButton();

                    if (GroupIconButton("##TakeCommand", groupIcon, isRaidSquad ? "Take Command of Raid Squad" : "Take Command of Squad"))
                    {
                        VitalsData->takeCommand();
                        ImGui::CloseCurrentPopup();
                    }
                }

                if (isParty || isSquad)
                {
                    beginGroupTypeButton();

                    if (GroupIconButton("##LeaveGroup", EGroupIcon::Leave, isSquad ? "Leave Squad" : "Leave Party"))
                    {
                        VitalsData->leaveGroup();
                        ImGui::CloseCurrentPopup();
                    }
                }

                /* (b) Group invite */
                if (!isSquad || isCommander || isLieutenant || squadSettings.IsAllowInvites)
                {
                    beginPopupSection();
                    inviteBox();
                }

                /* (c) Group settings */
                if (isCommander)
                {
                    beginPopupSection();

                    bool isSubgroupsLocked = squadSettings.IsSubgroupsLocked;
                    if (ImGui::Checkbox("Lock All Subgroups", &isSubgroupsLocked))
                    {
                        VitalsData->setSquadSubgroupsLocked(isSubgroupsLocked);
                    }

                    bool isAllowInvites = squadSettings.IsAllowInvites;
                    if (ImGui::Checkbox("Allow Members to Send Invites", &isAllowInvites))
                    {
                        VitalsData->setSquadAllowInvites(isAllowInvites);
                    }

                    bool isPrivate = squadSettings.IsPrivate;
                    if (ImGui::Checkbox("Private Squad", &isPrivate))
                    {
                        VitalsData->setSquadPrivate(isPrivate);
                    }

                    ImGui::Text("Allow uninvited players to join?");
                    int joinBehavior = static_cast<int>(squadSettings.JoinBehavior);
                    ImGui::SetNextItemWidth(200.0f);

                    if (ImGui::Combo("##JoinBehavior", &joinBehavior, JOIN_BEHAVIOR_LABELS, IM_ARRAYSIZE(JOIN_BEHAVIOR_LABELS)))
                    {
                        VitalsData->setSquadJoinBehavior(static_cast<VitalSignsDataLink::ESquadJoinBehavior>(joinBehavior));
                    }

                    /* Commander tag, shown by id until the tag textures are available */
                    std::vector<VitalSignsDataLink::CommanderTag_t> commanderTags = VitalsData->getCommanderTags();
                    commanderTags.erase(std::remove_if(commanderTags.begin(), commanderTags.end(),
                        [](const VitalSignsDataLink::CommanderTag_t& tag) { return !tag.IsUnlocked; }), commanderTags.end());

                    if (isCommanderUnlocked && !commanderTags.empty())
                    {
                        const uint32_t currentTag = VitalsData->getCommanderTag();
                        const std::string currentLabel = "Tag " + std::to_string(currentTag);

                        ImGui::Text("Commander tag");
                        ImGui::SetNextItemWidth(200.0f);

                        if (ImGui::BeginCombo("##CommanderTag", currentLabel.c_str()))
                        {
                            for (const VitalSignsDataLink::CommanderTag_t& tag : commanderTags)
                            {
                                const std::string label = "Tag " + std::to_string(tag.Id);

                                if (ImGui::Selectable(label.c_str(), tag.Id == currentTag))
                                {
                                    VitalsData->setCommanderTag(tag.Id);
                                }
                            }

                            ImGui::EndCombo();
                        }
                    }
                }

                ImGui::EndPopup();
            }
        }
        ImGui::End();
    }

} // namespace UI::Widget

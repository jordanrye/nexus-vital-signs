#include "ui_widget.h"

#include <algorithm>
#include <iterator>
#include <string>
#include <vector>

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

#include "resource.h"
#include "shared.h"
#include "ui_common.h"

namespace UI::Widget {

    static const int BROADCAST_TEXT_BOX_LENGTH = 32;
    static const int PANEL_TEXT_BOX_LENGTH = 24;

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
        SquadMessage,
        RightArrow
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
            case EGroupIcon::RightArrow:
                return isHovered
                    ? UI::GetOrCreateTextureFromResource("TEX_VS_GROUP_ICON_RIGHT_ARROW_HOVERED", IDB_GROUP_ICON_RIGHT_ARROW_HOVERED)
                    : UI::GetOrCreateTextureFromResource("TEX_VS_GROUP_ICON_RIGHT_ARROW_NORMAL", IDB_GROUP_ICON_RIGHT_ARROW_NORMAL);
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

    /* Draws the texture centred in a square, scaled to fit while keeping its
     * aspect ratio. Only the part between the UVs is drawn, and sets the aspect
     * ratio. The two tag shapes have different heights, so the grid stays
     * aligned. Nothing is drawn until the texture has loaded. */
    static void DrawTextureInSquare(ImDrawList* drawList, const Texture* texture, const ImVec2& min, const float side,
                                    const ImVec2& uvMin = ImVec2(0.0f, 0.0f), const ImVec2& uvMax = ImVec2(1.0f, 1.0f))
    {
        if (!texture || !texture->Resource || (texture->Width == 0U) || (texture->Height == 0U))
        {
            return;
        }

        const float width = texture->Width * (uvMax.x - uvMin.x);
        const float height = texture->Height * (uvMax.y - uvMin.y);

        if ((width <= 0.0f) || (height <= 0.0f))
        {
            return;
        }

        const float scale = side / (std::max)(width, height);
        const ImVec2 size(IM_FLOOR(width * scale), IM_FLOOR(height * scale));
        const ImVec2 imageMin(IM_FLOOR(min.x + ((side - size.x) * 0.5f)), IM_FLOOR(min.y + ((side - size.y) * 0.5f)));

        drawList->AddImage(texture->Resource, imageMin, ImVec2(imageMin.x + size.x, imageMin.y + size.y), uvMin, uvMax);
    }

    /* Draws the game's icon for the tag, or a placeholder while it is loading or
     * if the game's textures could not be found */
    static void DrawCommanderTag(ImDrawList* drawList, const VitalSignsDataLink::ECommanderTag tag, const ImVec2& min, const float side)
    {
        const Texture texture = VitalsData->getCommanderTagTexture(tag);

        /* The visible part of the game's 128x128 tag textures, measured from their alpha */
        const bool isCatmander = (tag >= static_cast<VitalSignsDataLink::ECommanderTag>(9));
        const ImVec2 uvMin = isCatmander ? ImVec2(30.0f / 128.0f, 25.0f / 128.0f) : ImVec2(29.0f / 128.0f, 27.0f / 128.0f);
        const ImVec2 uvMax = isCatmander ? ImVec2(98.0f / 128.0f, 101.0f / 128.0f) : ImVec2(99.0f / 128.0f, 101.0f / 128.0f);

        if (nullptr != texture.Resource)
        {
            DrawTextureInSquare(drawList, &texture, min, side, uvMin, uvMax);
        }
        else
        {
            DrawTextureInSquare(drawList, UI::GetOrCreateTextureFromResource("TEX_VS_GROUP_ICON_READY_CHECK_NORMAL", IDB_GROUP_ICON_READY_CHECK_NORMAL), min, side);
        }
    }

    static float GetInputWidth(const int strLen)
    {
        const std::string sample(static_cast<size_t>(strLen), '0');
        return ImGui::CalcTextSize(sample.c_str()).x + (ImGui::GetStyle().FramePadding.x * 2.0f);
    }

    static float GetComboWidth(const int strLen)
    {
        float arrowButtonWidth = ImGui::GetFrameHeight(); /* button is square */
        return GetInputWidth(strLen) + arrowButtonWidth;
    }

    /* Group widget, laid out after the native party/squad bar. Quick actions
     * sit next to the group icon, and the icon opens the settings popup. 
     * Actions that are not valid for the group state or the client's 
     * permissions are hidden, never disabled. */
    void RenderGroupPanel(const VitalSignsDataLink::EGroupType groupType)
    {
        const char* POPUP_GROUP_SETTINGS = "GroupSettings##VitalSignsGroupWidget";
        const char* JOIN_BEHAVIOR_LABELS[] = { "Yes, anyone can join.", "Yes, with approval.", "No." };
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

        /* Invite text box (arrow button or `ENTER` key to send the invite).
         * ImGui's text box can't make room for icons in its own frame, so the
         * the frame is drawn behind a frameless text box and the two icons. */
        auto inviteBox = [](const EGroupIcon groupIcon, const char* hint, const char* tooltip)
        {
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            drawList->ChannelsSplit(2);
            drawList->ChannelsSetCurrent(1);

            ImGui::BeginGroup();

            /* Group icon, clicking it focuses the text box */
            Texture* groupTexture = GetGroupIconTexture(groupIcon, false);
            ImGui::Dummy(GetGroupIconSize(groupTexture));
            if (groupTexture)
            {
                drawList->AddImage(groupTexture->Resource, ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
            }
            if (ImGui::IsItemClicked())
            {
                ImGui::SetKeyboardFocusHere(0);
            }

            /* Text box without its own frame */
            ImGui::SameLine(0.0f, 0.0f);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
            ImGui::SetNextItemWidth(GetInputWidth(PANEL_TEXT_BOX_LENGTH));
            bool isSubmitted = ImGui::InputTextWithHint("##InviteName", hint, inviteName, sizeof(inviteName), ImGuiInputTextFlags_EnterReturnsTrue);
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();

            /* Arrow icon */
            ImGui::SameLine(0.0f, 0.0f);
            isSubmitted |= GroupIconButton("##Invite", EGroupIcon::RightArrow, tooltip);

            ImGui::EndGroup();

            /* Shared frame behind all three */
            drawList->ChannelsSetCurrent(0);
            ImGui::RenderFrame(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImGui::GetColorU32(ImGuiCol_FrameBg), true, ImGui::GetStyle().FrameRounding);
            drawList->ChannelsMerge();

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

        const StyleScope styleScope(StylePresets.groupPanel);

        if (ImGui::Begin("Group##VitalSignsGroupWidget", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse))
        {
            styleScope.ApplyFontScale();

            /* Group icon */
            const EGroupIcon groupIcon = isRaidSquad ? EGroupIcon::Raid : (isSquad ? EGroupIcon::Squad : (isParty ? EGroupIcon::Party : EGroupIcon::Solo));
            if (GroupIconButton("##GroupIcon", groupIcon, nullptr))
            {
                ImGui::OpenPopup(POPUP_GROUP_SETTINGS);
            }

            /* Quick actions, right of the icon. The icons are as tall as a button, so they line up. */
            if (isSquad)
            {
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
                    ImGui::SetNextItemWidth(GetInputWidth(BROADCAST_TEXT_BOX_LENGTH));

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
                styleScope.ApplyFontScale();

                beginPopupSection();

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
                                VitalsData->createSquad(false);
                                ImGui::CloseCurrentPopup();
                            }
                        }

                        beginGroupTypeButton();

                        if (GroupIconButton("##CreateRaidSquad", EGroupIcon::Raid, isParty ? "Convert to Raid Squad" : "Create Raid Squad"))
                        {
                            VitalsData->createSquad(true);
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

                    if (!isParty)
                    {
                        beginGroupTypeButton();
                        inviteBox(EGroupIcon::Party, "Player name", "Send Party Request");
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

                if (isParty || (isSquad && (isCommander || isLieutenant || squadSettings.IsAllowInvites)))
                {
                    beginPopupSection();
                    auto hint = (isRaidSquad ? "Invite to Raid Squad" : (isSquad ? "Invite to Squad" : "Invite to Party"));
                    auto tooltip = (isSquad ? "Send Squad Invite" : "Send Party Invite");
                    inviteBox(groupIcon, hint, tooltip);
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
                    /* Built from `BeginCombo`, so the dropdown's font can be scaled like the popup's */
                    const int joinBehavior = static_cast<int>(squadSettings.JoinBehavior);
                    const bool isJoinBehaviorValid = (joinBehavior >= 0) && (joinBehavior < IM_ARRAYSIZE(JOIN_BEHAVIOR_LABELS));
                    ImGui::SetNextItemWidth(GetComboWidth(PANEL_TEXT_BOX_LENGTH));

                    if (ImGui::BeginCombo("##JoinBehavior", isJoinBehaviorValid ? JOIN_BEHAVIOR_LABELS[joinBehavior] : ""))
                    {
                        styleScope.ApplyFontScale();

                        for (int option = 0; option < IM_ARRAYSIZE(JOIN_BEHAVIOR_LABELS); option++)
                        {
                            if (ImGui::Selectable(JOIN_BEHAVIOR_LABELS[option], option == joinBehavior))
                            {
                                VitalsData->setSquadJoinBehavior(static_cast<VitalSignsDataLink::ESquadJoinBehavior>(option));
                            }
                        }

                        ImGui::EndCombo();
                    }

                    /* Commander tag, picked from the unlocked tags */
                    const std::vector<VitalSignsDataLink::ECommanderTag> unlockedTags = VitalsData->getUnlockedCommanderTags();

                    if (isCommanderUnlocked && !unlockedTags.empty())
                    {
                        const VitalSignsDataLink::ECommanderTag currentTag = VitalsData->getCommanderTag();

                        ImGui::Text("Commander tag");

                        const float previewSide = ImGui::GetTextLineHeight();
                        const float arrowWidth = ImGui::GetFrameHeight(); /* button is square */
                        ImGuiWindow* panelWindow = ImGui::GetCurrentWindow();

                        ImGui::SetNextItemWidth(previewSide + (ImGui::GetStyle().FramePadding.x * 2.0f) + arrowWidth);
                        const bool isComboOpen = ImGui::BeginCombo("##CommanderTag", "");

                        if (VitalSignsDataLink::ECommanderTag::None != currentTag)
                        {
                            const ImRect comboRect = panelWindow->DC.LastItemRect;
                            const ImVec2 previewMin(
                                IM_FLOOR(comboRect.Min.x + ((comboRect.GetWidth() - arrowWidth - previewSide) * 0.5f)),
                                IM_FLOOR(comboRect.Min.y + ((comboRect.GetHeight() - previewSide) * 0.5f)));

                            DrawCommanderTag(panelWindow->DrawList, currentTag, previewMin, previewSide);
                        }

                        if (isComboOpen)
                        {
                            styleScope.ApplyFontScale();

                            /* Sized after the font scale, so the grid scales with the popup */
                            const float cellSide = ImGui::GetFrameHeight();

                            /* A row per shape: tags are unlocked a whole shape at a time, and
                             * listed by shape then colour, so each row starts with red */
                            for (const VitalSignsDataLink::ECommanderTag tag : unlockedTags)
                            {
                                const int COMMANDER_TAG_COLOUR_COUNT = 9;
                                const int index = static_cast<int>(tag);

                                if ((index % COMMANDER_TAG_COLOUR_COUNT) != 0)
                                {
                                    ImGui::SameLine();
                                }

                                ImGui::PushID(index);

                                if (ImGui::Selectable("##Tag", tag == currentTag, ImGuiSelectableFlags_None, ImVec2(cellSide, cellSide)))
                                {
                                    VitalsData->setCommanderTag(tag);
                                }

                                /* The selectable's rect is padded by half the item spacing on each
                                 * side, so the icon is centred in it rather than at its corner. */
                                const ImVec2 cellMin(
                                    IM_FLOOR(ImGui::GetItemRectMin().x + ((ImGui::GetItemRectSize().x - cellSide) * 0.5f)),
                                    IM_FLOOR(ImGui::GetItemRectMin().y + ((ImGui::GetItemRectSize().y - cellSide) * 0.5f)));

                                DrawCommanderTag(ImGui::GetWindowDrawList(), tag, cellMin, cellSide);

                                ImGui::PopID();
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

    /* Group requests widget. Kick votes are open votes to remove a group member.
     * The channel's requests are invites the group has sent and players asking
     * to join it. The client's requests are invites the client has received and
     * groups the client has asked to join. */
    void RenderGroupRequests()
    {
        const std::vector<VitalSignsDataLink::KickVote_t> kickVotes = VitalsData->getKickVotes();
        const std::vector<VitalSignsDataLink::UserId_t> channelInvites = VitalsData->getChannelInvites();
        const std::vector<VitalSignsDataLink::UserId_t> channelJoinRequests = VitalsData->getChannelJoinRequests();
        const std::vector<VitalSignsDataLink::ClientPendingChannel_t> clientInvites = VitalsData->getClientInvites();
        const std::vector<VitalSignsDataLink::ClientPendingChannel_t> clientJoinRequests = VitalsData->getClientJoinRequests();

        if (kickVotes.empty() && channelInvites.empty() && channelJoinRequests.empty() && clientInvites.empty() && clientJoinRequests.empty())
        {
            return;
        }

        /* Anyone in a party or a commanderless squad, otherwise the commander and lieutenants */
        const bool canManagePending = VitalsData->canManagePendingMembers();

        /* Key each line on its section and user or channel, as the vectors are rebuilt every frame */
        bool hasLine = false;
        auto beginLine = [&hasLine](const char* section, const void* key, const size_t keySize, const std::string& name, const char* description)
        {
            const char* keyBytes = static_cast<const char*>(key);
            ImGui::PushID(section);
            ImGui::PushID(keyBytes, keyBytes + keySize);

            /* The table has no vertical cell padding, so lines are spaced by an empty row instead */
            if (hasLine)
            {
                ImGui::TableNextRow(ImGuiTableRowFlags_None, ImGui::GetStyle().ItemSpacing.y);
            }

            hasLine = true;

            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("%s", name.c_str());

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::TextDisabled("%s", description);

            ImGui::TableNextColumn();
        };

        auto endLine = []()
        {
            ImGui::PopID();
            ImGui::PopID();
        };

        /* Names the party or squad a client invite or join request is for */
        auto getGroupName = [](const VitalSignsDataLink::ClientPendingChannel_t& group) -> std::string
        {
            if (group.LeaderName.empty())
            {
                return group.IsSquad ? "Squad" : "Party";
            }

            return group.LeaderName;
        };

        const StyleScope styleScope(StylePresets.groupRequests);

        if (ImGui::Begin("Group Requests##VitalSignsGroupRequestsWidget", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse))
        {
            styleScope.ApplyFontScale();

            /* Columns: name, description, actions. Without vertical cell padding,
             * a line is as tall as a button, the same height as the group widget.
             * The table reads the padding once, in `BeginTable`. */
            ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(ImGui::GetStyle().CellPadding.x, 0.0f));
            const bool isTableVisible = ImGui::BeginTable("##GroupRequests", 3, ImGuiTableFlags_SizingFixedFit);
            ImGui::PopStyleVar();

            if (isTableVisible)
            {
                /* Kick votes, one line per votee. The client seconds the vote or withdraws their own. */
                for (const VitalSignsDataLink::KickVote_t& kickVote : kickVotes)
                {
                    beginLine("KickVote", &kickVote.VoteeId, sizeof(kickVote.VoteeId), VitalsData->getUserData(kickVote.VoteeId).GetDisplayName(), "Pending Vote Kick");

                    if (kickVote.HasClientVoted)
                    {
                        if (GroupIconButton("##CancelKickVote", EGroupIcon::NotReady, "Cancel Vote to Kick"))
                        {
                            VitalsData->cancelKickVote(kickVote.VoteeId);
                        }
                    }
                    else
                    {
                        if (GroupIconButton("##AgreeKickVote", EGroupIcon::Ready, "Vote to Kick"))
                        {
                            VitalsData->agreeKickVote(kickVote.VoteeId);
                        }
                    }

                    endLine();
                }

                /* Channel invites, cancelled by the group */
                for (const VitalSignsDataLink::UserId_t& inviteeId : channelInvites)
                {
                    beginLine("ChannelInvite", &inviteeId, sizeof(inviteeId), VitalsData->getUserData(inviteeId).GetDisplayName(), "Pending Invitation");

                    if (canManagePending)
                    {
                        if (GroupIconButton("##CancelInvite", EGroupIcon::NotReady, "Cancel Invite"))
                        {
                            VitalsData->cancelInvite(inviteeId);
                        }
                    }

                    endLine();
                }

                /* Channel join requests, accepted or declined by the group */
                for (const VitalSignsDataLink::UserId_t& requesterId : channelJoinRequests)
                {
                    beginLine("ChannelJoinRequest", &requesterId, sizeof(requesterId), VitalsData->getUserData(requesterId).GetDisplayName(), "Pending Request");

                    if (canManagePending)
                    {
                        if (GroupIconButton("##AcceptJoinRequest", EGroupIcon::Ready, "Accept Join Request"))
                        {
                            VitalsData->acceptJoinRequest(requesterId);
                        }

                        ImGui::SameLine();

                        if (GroupIconButton("##DeclineJoinRequest", EGroupIcon::NotReady, "Decline Join Request"))
                        {
                            VitalsData->declineJoinRequest(requesterId);
                        }
                    }

                    endLine();
                }

                /* Client invites, accepted or declined by the client */
                for (const VitalSignsDataLink::ClientPendingChannel_t& invite : clientInvites)
                {
                    beginLine("ClientInvite", &invite.Channel, sizeof(invite.Channel), getGroupName(invite), invite.IsSquad ? "Squad Join Request" : "Party Join Request");

                    if (GroupIconButton("##AcceptInvite", EGroupIcon::Ready, "Accept Invite"))
                    {
                        VitalsData->acceptInvite(invite.Channel);
                    }

                    ImGui::SameLine();

                    if (GroupIconButton("##DeclineInvite", EGroupIcon::NotReady, "Decline Invite"))
                    {
                        VitalsData->declineInvite(invite.Channel);
                    }

                    endLine();
                }

                /* Client join requests, cancelled by the client */
                for (const VitalSignsDataLink::ClientPendingChannel_t& request : clientJoinRequests)
                {
                    beginLine("ClientJoinRequest", &request.Channel, sizeof(request.Channel), getGroupName(request), request.IsSquad ? "Squad Join Request" : "Party Join Request");

                    if (GroupIconButton("##CancelJoinRequest", EGroupIcon::NotReady, "Cancel Join Request"))
                    {
                        VitalsData->cancelJoinRequest(request.Channel);
                    }

                    endLine();
                }

                ImGui::EndTable();
            }
        }
        ImGui::End();
    }

} // namespace UI::Widget

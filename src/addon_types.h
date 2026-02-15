#ifndef SHARED_TYPES_H
#define SHARED_TYPES_H

#include <string>

#include "imgui/imgui.h"

struct GeneralConfig_t
{
    /* Party (5 Players) */
    std::string partyLayout;
    bool isHiddenNativeParty = false;
    
    /* Raid (10 Players) */
    std::string raidLayout;
    bool isHiddenNativeRaid = false;

    /* Squad (50 Players) */
    std::string squadLayout;
    bool isHiddenNativeSquad = false;

    /* Solo (No Group) */
    std::string soloLayout;
};

struct ColourPresets_t
{
    /* Simple Colours */
    ImColor COLOUR_BACKGROUND = ImColor(0, 0, 0);
    ImColor COLOUR_HEALTH = ImColor(74, 133, 62);
    ImColor COLOUR_HEALTH_DOWNED = ImColor(157, 31, 31);
    ImColor COLOUR_HEALTH_DEFEATED = ImColor(78, 15, 15);
    ImColor COLOUR_SHROUD_NECROMANCER = ImColor(38, 70, 45);
    ImColor COLOUR_SHROUD_SPECTER = ImColor(108, 21, 95);
    ImColor COLOUR_BARRIER = ImColor(170, 144, 78, 128);
    ImColor COLOUR_HOVERED = ImColor(255, 255, 255, 32);

    /** TODO: Role Colours */
    // ImColor COLOUR_ROLE_BACKGROUND = ImColor(0, 0, 0);
    // ImColor COLOUR_ROLE_HEALTH_DAMAGE = ImColor(74, 133, 62);
    // ImColor COLOUR_ROLE_HEALTH_BOON_SUPPORT = ImColor(198, 128, 28);
    // ImColor COLOUR_ROLE_HEALTH_HEALER = ImColor(28, 143, 198);
    // ImColor COLOUR_ROLE_HEALTH_DOWNED = ImColor(157, 31, 31);
    // ImColor COLOUR_ROLE_HEALTH_DEFEATED = ImColor(78, 15, 15);
    // ImColor COLOUR_ROLE_SHROUD_NECROMANCER = ImColor(38, 70, 45);
    // ImColor COLOUR_ROLE_SHROUD_SPECTER = ImColor(108, 21, 95);
    // ImColor COLOUR_ROLE_BARRIER = ImColor(170, 144, 78, 128);
    // ImColor COLOUR_ROLE_HOVERED = ImColor(255, 255, 255, 32);

    /** TODO: Profession Colours */
    // ImColor COLOUR_PROF_BACKGROUND = ImColor(0, 0, 0);
    // ImColor COLOUR_PROF_HEALTH_ELEMENTALIST = ImColor(197, 110, 108);
    // ImColor COLOUR_PROF_HEALTH_ENGINEER = ImColor(166, 125, 71);
    // ImColor COLOUR_PROF_HEALTH_GUARDIAN = ImColor(91, 154, 174);
    // ImColor COLOUR_PROF_HEALTH_MESMER = ImColor(146, 97, 170);
    // ImColor COLOUR_PROF_HEALTH_NECROMANCER = ImColor(66, 134, 89);
    // ImColor COLOUR_PROF_HEALTH_RANGER = ImColor(114, 178, 54);
    // ImColor COLOUR_PROF_HEALTH_REVENANT = ImColor(166, 87, 71);
    // ImColor COLOUR_PROF_HEALTH_THIEF = ImColor(154, 114, 119);
    // ImColor COLOUR_PROF_HEALTH_WARRIOR = ImColor(204, 167, 82);
    // ImColor COLOUR_PROF_BARRIER = ImColor(255, 255, 255, 128);
    // ImColor COLOUR_PROF_HOVERED = ImColor(255, 255, 255, 32);

    /** TODO: Party Colours */
    // ImColor COLOUR_PARTY_BACKGROUND = ImColor(0, 0, 0);
    // ImColor COLOUR_PARTY_HEALTH_1 = ImColor(219, 113, 113); /* Red */
    // ImColor COLOUR_PARTY_HEALTH_2 = ImColor(219, 172, 113); /* Yellow */
    // ImColor COLOUR_PARTY_HEALTH_3 = ImColor(138, 219, 113); /* Green */
    // ImColor COLOUR_PARTY_HEALTH_4 = ImColor(113, 219, 191); /* Cyan/Teal */
    // ImColor COLOUR_PARTY_HEALTH_5 = ImColor(113, 149, 219); /* Blue */
    // ImColor COLOUR_PARTY_HEALTH_6 = ImColor(201, 130, 130); /* Dusty Red */
    // ImColor COLOUR_PARTY_HEALTH_7 = ImColor(201, 166, 130); /* Dusty Yellow */
    // ImColor COLOUR_PARTY_HEALTH_8 = ImColor(145, 201, 130); /* Dusty Green */
    // ImColor COLOUR_PARTY_HEALTH_9 = ImColor(130, 201, 183); /* Dusty Cyan */
    // ImColor COLOUR_PARTY_HEALTH_10 = ImColor(130, 151, 201); /* Dusty Blue */
    // ImColor COLOUR_PARTY_HEALTH_11 = ImColor(187, 161, 161); /* Pale Red */
    // ImColor COLOUR_PARTY_HEALTH_12 = ImColor(187, 178, 161); /* Pale Yellow */
    // ImColor COLOUR_PARTY_HEALTH_13 = ImColor(166, 187, 161); /* Pale Green */
    // ImColor COLOUR_PARTY_HEALTH_14 = ImColor(161, 187, 180); /* Pale Cyan */
    // ImColor COLOUR_PARTY_HEALTH_15 = ImColor(161, 168, 187); /* Pale Blue */
    // ImColor COLOUR_PARTY_BARRIER = ImColor(255, 255, 255, 128);
    // ImColor COLOUR_PARTY_HOVERED = ImColor(255, 255, 255, 32);

    /** TODO: Move into separate `Indicator` config. */
    ImColor COLOUR_BOONS_1 = ImColor(198, 128, 28);
    ImColor COLOUR_BOONS_2 = ImColor(188, 118, 18);
    ImColor COLOUR_CONDITIONS_1 = ImColor(146, 32, 32);
    ImColor COLOUR_CONDITIONS_2 = ImColor(132, 32, 32);
};

struct BorderPresets_t
{
    ImColor COLOUR_BORDER = ImColor(0, 0, 0);
    ImColor COLOUR_BORDER_HOVERED = ImColor(255, 255, 255);
    ImColor COLOUR_BORDER_SELECTED = ImColor(34, 153, 238);
    ImColor COLOUR_BORDER_SELF = ImColor(255, 255, 255);
};

#endif /* SHARED_TYPES_H */

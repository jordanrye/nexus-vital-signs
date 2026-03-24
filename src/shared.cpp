#include "shared.h"

#include <string>

HMODULE hSelf = nullptr;
HWND hClient = nullptr;

AddonAPI* APIDefs = nullptr;
Mumble::Data* MumbleLink = nullptr;
Mumble::Identity* MumbleIdentity = nullptr;
NexusLinkData* NexusLink = nullptr;
VitalSignsData* VitalsData = nullptr;

std::filesystem::path GameDir{};
std::filesystem::path AddonDir{};
std::filesystem::path PacksDir{};
std::filesystem::path IconsDir{};
std::filesystem::path FontsDir{};

GeneralConfig_t ConfigGeneral{};
LayoutManager g_LayoutManager{};
ColourPresets_t ColourPresets{};
BorderPresets_t BorderPresets{};

TextStyle_t ConfigText = []{ TextStyle_t c; c.fontSource = "Nexus font"; c.fontSizeSource = "Nexus font size"; c.fontSize = 0.0f; return c; }();
IconText_t ConfigIconDuration = []{ IconText_t c; c.position.anchor = "Bottom-right"; c.textStyle.fontSource = "Nexus font"; c.textStyle.fontSizeSource = "Nexus font size"; c.textStyle.fontSize = 0.0f; return c; }();
IconText_t ConfigIconStacks = []{ IconText_t c; c.position.anchor = "Top-right"; c.textStyle.fontSource = "Nexus font"; c.textStyle.fontSizeSource = "Nexus font size"; c.textStyle.fontSize = 0.0f; return c; }();

TreeView g_LayoutEditor{};
TreeView g_PresetConfig{};

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

TextConfig_t ConfigText{};
IconTextConfig_t ConfigIconDuration("Bottom-right");
IconTextConfig_t ConfigIconStacks("Top-right");

TreeView g_LayoutEditor{};
TreeView g_PresetConfig{};

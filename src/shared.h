#ifndef SHARED_H
#define SHARED_H

#include <filesystem>
#include <map>

#include "nexus/Nexus.h"
#include "mumble/Mumble.h"
#include "interface/vital_signs_interface.h"

#include "addon_types.h"
#include "imgui_tree_view.h"
#include "layout_manager.h"

extern HMODULE hSelf;
extern HWND hClient;

extern AddonAPI* APIDefs;
extern Mumble::Data* MumbleLink;
extern Mumble::Identity* MumbleIdentity;
extern NexusLinkData* NexusLink;
extern VitalSignsData* VitalsData;

extern std::filesystem::path GameDir;
extern std::filesystem::path AddonDir;
extern std::filesystem::path PacksDir;
extern std::filesystem::path IconsDir;
extern std::filesystem::path FontsDir;

extern GeneralConfig_t ConfigGeneral;
extern LayoutManager g_LayoutManager;
extern ColourPresets_t ColourPresets;
extern BorderPresets_t BorderPresets;
extern TextConfig_t ConfigText;
extern IconTextConfig_t ConfigIconDuration;
extern IconTextConfig_t ConfigIconStacks;

extern TreeView g_LayoutEditor;
extern TreeView g_PresetConfig;

#endif /* SHARED_H */

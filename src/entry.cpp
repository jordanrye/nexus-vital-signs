#include <Windows.h>
#include <d3d11.h>

#include "nexus/Nexus.h"
#include "mumble/Mumble.h"
#include "interface/vital_signs_interface.h"
#include "imgui/imgui.h"
#include "imgui_extensions.h"

#include "addon.h"
#include "settings.h"
#include "shared.h"
#include "imgui_tree_view.h"
#include "utilities.h"
#include "version.h"

void AddonLoad(AddonAPI* aApi);
void AddonUnload();
void OnMumbleIdentityUpdated(void* aEventArgs);

AddonDefinition AddonDef = {};

BOOL APIENTRY DllMain(HMODULE hInstDll, DWORD dwReason, LPVOID lpvReserved)
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            hSelf = hInstDll;
            DisableThreadLibraryCalls(hInstDll);
            break;
        case DLL_PROCESS_DETACH:
            break;
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
    }
    return TRUE;
}

extern "C" __declspec(dllexport) AddonDefinition * GetAddonDef()
{
    AddonDef.Signature = 39;
    AddonDef.APIVersion = NEXUS_API_VERSION;
    AddonDef.Name = "Vital Signs";
    AddonDef.Version.Major = V_MAJOR;
    AddonDef.Version.Minor = V_MINOR;
    AddonDef.Version.Build = V_BUILD;
    AddonDef.Version.Revision = V_REVISION;
    AddonDef.Author = "Jordan";
    AddonDef.Description = "Monitor the health, effects and status of your party/squad with customisable layouts.";
    AddonDef.Load = AddonLoad;
    AddonDef.Unload = AddonUnload;
    AddonDef.Flags = EAddonFlags_IsVolatile;
    AddonDef.Provider = EUpdateProvider_GitHub;
    AddonDef.UpdateLink = "https://github.com/jordanrye/nexus-vital-signs";

    return &AddonDef;
}

void AddonLoad(AddonAPI* aApi)
{
    APIDefs = aApi;

    DXGI_SWAP_CHAIN_DESC desc{};
    ((IDXGISwapChain*)APIDefs->SwapChain)->GetDesc(&desc);
    hClient = desc.OutputWindow;

    ImGui::SetCurrentContext((ImGuiContext*)APIDefs->ImguiContext);
    ImGui::SetAllocatorFunctions((void* (*)(size_t, void*))APIDefs->ImguiMalloc, (void(*)(void*, void*))APIDefs->ImguiFree); // on imgui 1.80+

    VitalsData = VitalSignsData::create(APIDefs);
	if (VitalsData->isInitialised())
	{
		APIDefs->Renderer.Register(ERenderType_Render, Addon::Render);
    	APIDefs->Renderer.Register(ERenderType_OptionsRender, Addon::Options);

		APIDefs->InputBinds.RegisterWithString("KB_HEALING_RADIAL", Addon::KeybindHandler, "(null)");
		APIDefs->Localization.Set("KB_HEALING_RADIAL", "en", "Activate Radial Menu");
	}

    NexusLink = (NexusLinkData*)APIDefs->DataLink.Get("DL_NEXUS_LINK");
    MumbleLink = (Mumble::Data*)APIDefs->DataLink.Get("DL_MUMBLE_LINK");
    APIDefs->Events.Subscribe("EV_MUMBLE_IDENTITY_UPDATED", OnMumbleIdentityUpdated);

    GameDir = APIDefs->Paths.GetGameDirectory();
    AddonDir = APIDefs->Paths.GetAddonDirectory("VitalSigns/");
    PacksDir = APIDefs->Paths.GetAddonDirectory("VitalSigns/layouts/");
    IconsDir = APIDefs->Paths.GetAddonDirectory("VitalSigns/icons/");
    FontsDir = APIDefs->Paths.GetAddonDirectory("VitalSigns/fonts/");

    std::filesystem::create_directory(AddonDir);
    std::filesystem::create_directory(PacksDir);
    std::filesystem::create_directory(IconsDir);
    std::filesystem::create_directory(FontsDir);

    g_LayoutManager.SetOnLayoutCreatedCallback([&](const std::filesystem::path& filePath, TreeNodeUID uid, const std::string& label, const std::string& type) {
        g_LayoutEditor.AppendNode(TreeNodeUID::NONE, uid, label, type, TreeNodeType::ROOT);
        g_LayoutEditor.RegisterContentView(uid, [filePath]() { //
            auto& layout = g_LayoutManager.GetLayoutFromFilePath(filePath);
            Addon::ContentViewGeneral(layout.name, layout.colors, layout.position, layout.layout);
        });
    });
    g_LayoutManager.SetOnLayoutDeletedCallback([&](TreeNodeUID uid) {
        g_LayoutEditor.RemoveNode(uid);
    });
    g_LayoutManager.SetOnIndicatorCreatedCallback([&](TreeNodeUID parentId, TreeNodeUID indicatorId, const std::string& label, const std::string& type) {
        TreeNodeType nodeType = (type == "Group") ? TreeNodeType::BRANCH : TreeNodeType::LEAF;
        g_LayoutEditor.AppendNode(parentId, indicatorId, label, type, nodeType);
        g_LayoutEditor.RegisterContentView(indicatorId, [indicatorId]() {
            Indicator_t* indicator = g_LayoutManager.GetIndicator(indicatorId);
            if (indicator != nullptr)
            {
                Addon::ContentViewIndicator(*indicator);
            }
        });
    });
    g_LayoutManager.SetOnIndicatorDeletedCallback([&](TreeNodeUID uid) {
        g_LayoutEditor.RemoveNode(uid);
    });
    g_LayoutManager.SetOnIndicatorSwapCallback([&](TreeNodeUID parentId, size_t idx_1, size_t idx_2) {
        g_LayoutEditor.SwapNode(parentId, idx_1, idx_2);
    });

    Settings::Load(APIDefs->Paths.GetAddonDirectory("VitalSigns/settings.json"));
    for (const auto& file : std::filesystem::directory_iterator(PacksDir))
    {
        if (file.is_regular_file())
        {
            const std::filesystem::path filePath = file.path();
            
            if (filePath.extension() == ".json")
            {
                Settings::LoadLayout(filePath);
            } 
            else 
            {
                std::string warningMessage = filePath.filename().string() + " is not a valid layout file.";
                APIDefs->Log(ELogLevel_WARNING, "VitalSigns", warningMessage.c_str());
            }
        }
    }
}

void AddonUnload()
{
    for (const auto& file : g_LayoutManager.GetAllLayouts())
    {
        Settings::SaveLayout(file.first);
    }

    // for (const auto& file : std::filesystem::directory_iterator(PacksDir))
    // {
    //     if (file.is_regular_file())
    //     {
    //         const std::filesystem::path filePath = file.path();
            
    //         if (filePath.extension() == ".json")
    //         {
    //             Settings::SaveLayout(filePath);
    //         } 
    //         else 
    //         {
    //             std::string warningMessage = filePath.filename().string() + " is not a valid layout file.";
    //             APIDefs->Log(ELogLevel_WARNING, "VitalSigns", warningMessage.c_str());
    //         }
    //     }
    // }

    Settings::Save(APIDefs->Paths.GetAddonDirectory("VitalSigns/settings.json"));

    APIDefs->Events.Unsubscribe("EV_MUMBLE_IDENTITY_UPDATED", OnMumbleIdentityUpdated);
    MumbleLink = nullptr;
    NexusLink = nullptr;

    if (VitalsData->isInitialised())
    {
        APIDefs->InputBinds.Deregister("KB_HEALING_RADIAL");

        APIDefs->Renderer.Deregister(Addon::Options);
        APIDefs->Renderer.Deregister(Addon::Render);
    }
    VitalsData->destroy();

    utils::font::ReleaseFonts();

    hClient = nullptr;

    APIDefs = nullptr;
}

void OnMumbleIdentityUpdated(void* aEventArgs)
{
    MumbleIdentity = (Mumble::Identity*)aEventArgs;
}

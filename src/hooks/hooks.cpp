#include <icecap/agent/hooks/hook_manager.hpp>

#include <windows.h>
#include <d3d9.h>
#include "MinHook.h"

#include <icecap/agent/hooks/d3d9_hooks.hpp>
#include <icecap/agent/hooks/framescript_hooks.hpp>
#include <icecap/agent/raii_wrappers.hpp>

namespace icecap::agent::hooks {

void InstallHooks(const bool enableEvents)
{
    // Use RAII wrappers for automatic resource management
    raii::D3D9Device d3d(Direct3DCreate9(D3D_SDK_VERSION));
    if (!d3d) {
        return; // Failed to create D3D9 device
    }

    D3DPRESENT_PARAMETERS d3dpp = {};
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.hDeviceWindow = GetForegroundWindow();

    IDirect3DDevice9* pDummyDev = nullptr;

    if (SUCCEEDED(d3d->CreateDevice(
        D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dpp.hDeviceWindow,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pDummyDev)))
    {
        // Use RAII wrapper for the device
        raii::D3D9DeviceWrapper device(pDummyDev);

        void** vtable = *reinterpret_cast<void***>(device.get());
        void* pEndScene = vtable[42]; // 42 = EndScene index

        MH_Initialize();

        // Hook EndScene
        MH_CreateHook(pEndScene,
                     reinterpret_cast<LPVOID>(&HookedEndScene),
                     reinterpret_cast<LPVOID*>(&g_OriginalEndScene));
        MH_EnableHook(pEndScene);

        // Hook FrameScript__SignalEvent
        if (enableEvents == true) {
            void* pFrameScriptSignalEvent = reinterpret_cast<void*>(0x0081ac90);
            MH_CreateHook(pFrameScriptSignalEvent,
                         reinterpret_cast<LPVOID>(&HookedFrameScriptSignalEvent),
                         reinterpret_cast<LPVOID*>(&g_OriginalFrameScriptSignalEvent));
            MH_EnableHook(pFrameScriptSignalEvent);
        }

        // Device and D3D objects are automatically released via RAII destructors
    }
}

}

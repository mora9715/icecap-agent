#include <icecap/agent/hooks/hook_manager.hpp>

#include <windows.h>
#include <d3d9.h>
#include "MinHook.h"

#include <icecap/agent/hooks/d3d9_hooks.hpp>
#include <icecap/agent/hooks/framescript_hooks.hpp>
#include <icecap/agent/raii_wrappers.hpp>
#include <icecap/agent/logging.hpp>

namespace icecap::agent::hooks {

void InstallHooks(const bool enableEvents)
{
    LOG_DEBUG(enableEvents ? "Installing hooks (enableEvents=true)" : "Installing hooks (enableEvents=false)");

    // Use RAII wrappers for automatic resource management
    raii::D3D9Device d3d(Direct3DCreate9(D3D_SDK_VERSION));
    if (!d3d) {
        LOG_ERROR("Failed to create D3D9 device");
        throw std::runtime_error("Failed to create D3D9 device");
    }

    LOG_DEBUG("D3D9 device created successfully");

    D3DPRESENT_PARAMETERS d3dpp = {};
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.hDeviceWindow = GetForegroundWindow();

    IDirect3DDevice9* pDummyDev = nullptr;

    if (SUCCEEDED(d3d->CreateDevice(
        D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dpp.hDeviceWindow,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pDummyDev)))
    {
        LOG_DEBUG("D3D9 device creation succeeded");
        // Use RAII wrapper for the device
        raii::D3D9DeviceWrapper device(pDummyDev);

        void** vtable = *reinterpret_cast<void***>(device.get());
        void* pEndScene = vtable[42]; // 42 = EndScene index

        LOG_DEBUG("EndScene address found in vtable");

        MH_STATUS mhStatus = MH_Initialize();
        if (mhStatus != MH_OK) {
            LOG_ERROR("MinHook initialization failed");
            throw std::runtime_error("MinHook initialization failed");
        }

        LOG_DEBUG("MinHook initialized successfully");

        // Hook EndScene
        mhStatus = MH_CreateHook(pEndScene,
                     reinterpret_cast<LPVOID>(&HookedEndScene),
                     reinterpret_cast<LPVOID*>(&g_OriginalEndScene));
        if (mhStatus != MH_OK) {
            LOG_ERROR("Failed to create EndScene hook");
            throw std::runtime_error("Failed to create EndScene hook");
        }

        mhStatus = MH_EnableHook(pEndScene);
        if (mhStatus != MH_OK) {
            LOG_ERROR("Failed to enable EndScene hook");
            throw std::runtime_error("Failed to enable EndScene hook");
        }

        LOG_INFO("EndScene hook installed successfully");

        // Hook FrameScript__SignalEvent
        if (enableEvents == true) {
            void* pFrameScriptSignalEvent = reinterpret_cast<void*>(0x0081ac90);
            LOG_DEBUG("FrameScript address located");

            mhStatus = MH_CreateHook(pFrameScriptSignalEvent,
                         reinterpret_cast<LPVOID>(&HookedFrameScriptSignalEvent),
                         reinterpret_cast<LPVOID*>(&g_OriginalFrameScriptSignalEvent));
            if (mhStatus != MH_OK) {
                LOG_ERROR("Failed to create FrameScript hook");
                throw std::runtime_error("Failed to create FrameScript hook");
            }

            mhStatus = MH_EnableHook(pFrameScriptSignalEvent);
            if (mhStatus != MH_OK) {
                LOG_ERROR("Failed to enable FrameScript hook");
                throw std::runtime_error("Failed to enable FrameScript hook");
            }

            LOG_INFO("FrameScript hook installed successfully");
        }

        LOG_INFO("All hooks installed successfully");
        // Device and D3D objects are automatically released via RAII destructors
    }
    else {
        LOG_ERROR("Failed to create D3D9 device for hook installation");
        throw std::runtime_error("Failed to create D3D9 device for hook installation");
    }
}

}

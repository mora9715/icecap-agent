#include <windows.h>

#include "MinHook.h"

#include <icecap/agent/application_context.hpp>
#include <icecap/agent/core/MessageProcessor.hpp>
#include <icecap/agent/hooks/D3D9Hook.hpp>
#include <icecap/agent/logging.hpp>
#include <icecap/agent/shared_state.hpp>

namespace icecap::agent::hooks {

// Static member initialization
D3D9Hook::EndSceneFunc D3D9Hook::s_originalEndScene = nullptr;

D3D9Hook::D3D9Hook() : BaseHook("D3D9EndScene") {}

bool D3D9Hook::doInstall() {
    if (!findEndSceneAddress()) {
        return false;
    }

    return installMinHook();
}

bool D3D9Hook::doUninstall() {
    return uninstallMinHook();
}

long(__stdcall* D3D9Hook::GetOriginalEndScene())(IDirect3DDevice9*) {
    return s_originalEndScene;
}

bool D3D9Hook::findEndSceneAddress() {
    try {
        // Get D3D9 module handle
        HMODULE d3d9Module = GetModuleHandleA("d3d9.dll");
        if (!d3d9Module) {
            LOG_ERROR("D3D9Hook: Failed to get d3d9.dll module handle");
            return false;
        }

        // Create a temporary D3D9 object to get the vtable
        IDirect3D9* d3d9 = Direct3DCreate9(D3D_SDK_VERSION);
        if (!d3d9) {
            LOG_ERROR("D3D9Hook: Failed to create D3D9 object");
            return false;
        }

        // Create a dummy device to get the device vtable
        D3DPRESENT_PARAMETERS pp = {};
        pp.Windowed = TRUE;
        pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
        pp.hDeviceWindow = GetDesktopWindow();

        IDirect3DDevice9* device = nullptr;
        HRESULT hr = d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, GetDesktopWindow(),
                                        D3DCREATE_SOFTWARE_VERTEXPROCESSING, &pp, &device);

        d3d9->Release();

        if (FAILED(hr) || !device) {
            LOG_ERROR("D3D9Hook: Failed to create D3D9 device");
            return false;
        }

        // Get vtable and extract EndScene address
        void** vtable = *reinterpret_cast<void***>(device);
        m_targetAddress = vtable[42]; // EndScene is at index 42

        device->Release();

        LOG_DEBUG("D3D9Hook: Found EndScene address: " + std::to_string(reinterpret_cast<uintptr_t>(m_targetAddress)));
        return true;

    } catch (...) {
        LOG_ERROR("D3D9Hook: Exception while finding EndScene address");
        return false;
    }
}

bool D3D9Hook::installMinHook() {
    if (!m_targetAddress) {
        LOG_ERROR("D3D9Hook: No target address available");
        return false;
    }

    // Create hook using MinHook
    MH_STATUS status = MH_CreateHook(m_targetAddress, reinterpret_cast<LPVOID>(&HookedEndScene),
                                     reinterpret_cast<LPVOID*>(&s_originalEndScene));

    if (status != MH_OK) {
        LOG_ERROR("D3D9Hook: MH_CreateHook failed with status " + std::to_string(status));
        return false;
    }

    // Enable the hook
    status = MH_EnableHook(m_targetAddress);
    if (status != MH_OK) {
        LOG_ERROR("D3D9Hook: MH_EnableHook failed with status " + std::to_string(status));
        MH_RemoveHook(m_targetAddress);
        return false;
    }

    return true;
}

bool D3D9Hook::uninstallMinHook() {
    if (!m_targetAddress) {
        LOG_WARN("D3D9Hook: No target address to unhook");
        return true;
    }

    // Disable the hook
    MH_STATUS status = MH_DisableHook(m_targetAddress);
    if (status != MH_OK) {
        LOG_ERROR("D3D9Hook: MH_DisableHook failed with status " + std::to_string(status));
    }

    // Remove the hook
    status = MH_RemoveHook(m_targetAddress);
    if (status != MH_OK) {
        LOG_ERROR("D3D9Hook: MH_RemoveHook failed with status " + std::to_string(status));
        return false;
    }

    s_originalEndScene = nullptr;
    m_targetAddress = nullptr;
    return true;
}

long __stdcall D3D9Hook::HookedEndScene(IDirect3DDevice9* pDevice) {
    auto* appContext = GetApplicationContext();
    if (!appContext) {
        LOG_WARN("D3D9Hook: No application context available");
        return s_originalEndScene(pDevice);
    }

    // Process any pending commands using the message processor
    std::lock_guard lk(appContext->getInboxMutex());
    if (!appContext->getInboxQueue().empty()) {
        auto cmd = appContext->getInboxQueue().front();
        appContext->getInboxQueue().pop();

        // Use MessageProcessor to handle the command
        try {
            core::MessageProcessor processor(appContext);
            processor.processCommand(cmd);
        } catch (const std::exception& e) {
            LOG_ERROR("D3D9Hook: Exception in MessageProcessor: " + std::string(e.what()));
        } catch (...) {
            LOG_ERROR("D3D9Hook: Unknown exception in MessageProcessor");
        }
    }

    return s_originalEndScene(pDevice);
}

} // namespace icecap::agent::hooks
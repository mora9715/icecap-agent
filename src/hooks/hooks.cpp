#include <icecap/agent/hooks/hook_manager.hpp>
#include <icecap/agent/hooks/HookRegistry.hpp>
#include <icecap/agent/hooks/D3D9Hook.hpp>
#include <icecap/agent/logging.hpp>
#include "MinHook.h"

namespace icecap::agent::hooks {

// Global hook registry instance
static std::unique_ptr<HookRegistry> g_hookRegistry;

void InstallHooks(const bool enableEvents)
{
    LOG_DEBUG(enableEvents ? "Installing hooks (enableEvents=true)" : "Installing hooks (enableEvents=false)");

    try {
        // Initialize MinHook
        MH_STATUS mhStatus = MH_Initialize();
        if (mhStatus != MH_OK) {
            LOG_ERROR("MinHook initialization failed with status " + std::to_string(mhStatus));
            throw std::runtime_error("MinHook initialization failed");
        }
        LOG_DEBUG("MinHook initialized successfully");

        // Create hook registry
        g_hookRegistry = std::make_unique<HookRegistry>();

        // Register D3D9 EndScene hook
        auto d3d9Hook = std::make_shared<D3D9Hook>();
        g_hookRegistry->registerHook(d3d9Hook);

        // TODO: Add FrameScript hook when refactored
        // if (enableEvents) {
        //     auto frameScriptHook = std::make_shared<FrameScriptHook>();
        //     g_hookRegistry->registerHook(frameScriptHook);
        // }

        // Install all registered hooks
        if (!g_hookRegistry->installAllHooks()) {
            LOG_ERROR("Failed to install all hooks");
            throw std::runtime_error("Failed to install all hooks");
        }

        LOG_INFO("Hook installation completed successfully. Installed " +
                std::to_string(g_hookRegistry->getInstalledHookCount()) + " hooks.");

    } catch (const std::exception& e) {
        LOG_ERROR("Exception during hook installation: " + std::string(e.what()));
        throw;
    } catch (...) {
        LOG_ERROR("Unknown exception during hook installation");
        throw;
    }
}

}

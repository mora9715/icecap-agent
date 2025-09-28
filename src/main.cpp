#include <winsock2.h>
#include <windows.h>
#include "MinHook.h"
#include <memory>
#include <icecap/agent/application_context.hpp>
#include <icecap/agent/raii_wrappers.hpp>
#include "shared_state.hpp"

// Global application context (managed via RAII)
static std::unique_ptr<icecap::agent::ApplicationContext> g_appContext;


void Cleanup()
{
    if (g_appContext) {
        icecap::agent::SetApplicationContext(nullptr);
        g_appContext->shutdown();
        g_appContext.reset();
    }
}

DWORD WINAPI MainThread(LPVOID hMod)
{
    HMODULE hModule = static_cast<HMODULE>(hMod);

    try {
        // Create and initialize application context
        g_appContext = std::make_unique<icecap::agent::ApplicationContext>();

        if (!g_appContext->initialize(hModule)) {
            return 1; // Initialization failed
        }

        // Set the application context for hooks to access
        icecap::agent::SetApplicationContext(g_appContext.get());

        // Keep the main thread alive while the application runs
        while (g_appContext->isRunning()) {
            Sleep(100);
        }

    } catch (...) {
        // Handle any unexpected exceptions
        if (g_appContext) {
            g_appContext->shutdown();
        }
        return 1;
    }

    return 0;
}

DWORD WINAPI CleanupThread(LPVOID hMod)
{
    HMODULE hModule = static_cast<HMODULE>(hMod);

    while (g_appContext && g_appContext->isRunning())
    {
        if (GetAsyncKeyState(VK_DELETE) & 1)
        {
            if (g_appContext) {
                g_appContext->stop();
            }

            // Cleanup first
            Cleanup();

            MessageBoxA(nullptr,
                "Hook unloaded!",
                "Cleanup", MB_OK | MB_TOPMOST);

            FreeLibraryAndExitThread(hModule, 0);
        }
        Sleep(50);
    }

    return 0;
}

bool APIENTRY DllMain(const HMODULE hMod, const DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hMod);

        // Create threads with proper RAII management
        icecap::agent::raii::ThreadHandle mainThread(
            CreateThread(nullptr, 0, MainThread, hMod, 0, nullptr)
        );

        icecap::agent::raii::ThreadHandle cleanupThread(
            CreateThread(nullptr, 0, CleanupThread, hMod, 0, nullptr)
        );

        // Note: ThreadHandle destructors will handle cleanup automatically
        // when the DLL is unloaded. For this simple case, we let them
        // detach since the threads manage their own lifecycle.
        if (mainThread && cleanupThread) {
            // Threads created successfully, detach them
            mainThread.get(); // Keep reference alive
            cleanupThread.get(); // Keep reference alive
        }
    }
    return TRUE;
}

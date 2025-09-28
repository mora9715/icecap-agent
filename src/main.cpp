#include <winsock2.h>
#include <windows.h>
#include <fstream>
#include "MinHook.h"
#include <queue>
#include <mutex>
#include <icecap/agent/networking.hpp>
#include <icecap/agent/hooks/hook_manager.hpp>

static HMODULE g_hModule = nullptr;
static volatile bool g_running = true;
static HANDLE g_mainThreadHandle = nullptr;

static constexpr char kDELIM = '\x1E';
static constexpr unsigned short kPORT = 5050;

// Define the shared state variables in the namespace
namespace icecap::agent {
    std::mutex inbox_mtx;
    std::mutex outbox_mtx;
    std::queue<IncomingMessage> inboxQueue;
    std::queue<OutgoingMessage> outboxQueue;
}

static icecap::agent::NetworkManager g_networkManager;


void Cleanup()
{
    g_networkManager.stopServer();

    MH_Uninitialize();
}

DWORD WINAPI MainThread(LPVOID hMod)
{
    g_hModule = static_cast<HMODULE>(hMod);

    icecap::agent::hooks::InstallHooks(false);

    // Start the network server
    g_networkManager.startServer(icecap::agent::inboxQueue, icecap::agent::outboxQueue, kPORT, kDELIM, icecap::agent::inbox_mtx, icecap::agent::outbox_mtx);

    // Keep the main thread alive while the network server runs
    while (g_running && g_networkManager.isRunning()) {
        Sleep(100);
    }

    return 0;
}

DWORD WINAPI CleanupThread(LPVOID hMod)
{
    g_hModule = static_cast<HMODULE>(hMod);

    while (g_running)
    {
        if (GetAsyncKeyState(VK_DELETE) & 1)
        {
            g_running = false;

            // Cleanup first
            Cleanup();

            // Wait for the main thread to finish
            if (g_mainThreadHandle) {
                WaitForSingleObject(g_mainThreadHandle, 2000);
                CloseHandle(g_mainThreadHandle);
            }

            MessageBoxA(nullptr,
                "Hook unloaded!",
                "Cleanup", MB_OK | MB_TOPMOST);

            FreeLibraryAndExitThread(g_hModule, 0);
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
        g_mainThreadHandle = CreateThread(nullptr, 0, MainThread, hMod, 0, nullptr);
        CreateThread(nullptr, 0, CleanupThread, hMod, 0, nullptr);
    }
    return TRUE;
}

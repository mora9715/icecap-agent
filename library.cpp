// DemoDll.cpp  (Release | Win32) – minimal packet-logger with self-unload

#include <winsock2.h>   // must be first!
#include <windows.h>
#include <fstream>
#include "MinHook.h"
#include <d3d9.h>
#include <queue>
#include <mutex>
#include "networking.h"

#pragma comment(lib, "d3d9.lib")

#include "hooks/hooks.h"

static HMODULE g_hModule = nullptr;
static volatile bool g_running = true;
static HANDLE g_mainThreadHandle = nullptr;

std::mutex inbox_mtx;
std::mutex outbox_mtx;

static constexpr char kDELIM = '\x1E';
static constexpr unsigned short kPORT = 5050;

std::queue<IncomingMessage> inboxQueue;
std::queue<OutgoingMessage> outboxQueue;

static NetworkManager g_networkManager;


void Cleanup()
{
    g_networkManager.stopServer();

    MH_Uninitialize();
}

DWORD WINAPI MainThread(LPVOID hMod)
{
    g_hModule = static_cast<HMODULE>(hMod);

    InstallHooks(false);

    // Start the network server
    g_networkManager.startServer(inboxQueue, outboxQueue, kPORT, kDELIM);

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
                WaitForSingleObject(g_mainThreadHandle, 2000); // Wait up to 2 seconds
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

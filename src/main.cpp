#include <winsock2.h>
#include <windows.h>
#include "MinHook.h"
#include <memory>
#include <atomic>
#include <icecap/agent/application_context.hpp>
#include <icecap/agent/raii_wrappers.hpp>
#include <icecap/agent/logging.hpp>
#include "shared_state.hpp"

// Global application context (managed via RAII)
static std::unique_ptr<icecap::agent::ApplicationContext> g_appContext;


void Cleanup()
{
    static std::atomic<bool> cleanup_called{false};

    // Prevent multiple cleanup calls
    if (cleanup_called.exchange(true)) {
        return;
    }

    try {
        if (g_appContext) {
            LOG_INFO("Shutting down application context");

            // Clear global reference first to prevent other threads from accessing
            icecap::agent::SetApplicationContext(nullptr);

            // Shutdown the context
            g_appContext->shutdown();
            g_appContext.reset();
        }

        // Small delay to allow any pending log messages to flush
        Sleep(1000);

        // Shutdown logging last
        icecap::agent::Logger::getInstance().shutdown();
    }
    catch (...) {
        // Swallow any exceptions during cleanup to prevent crashes
    }
}

DWORD WINAPI MainThread(LPVOID hMod)
{
    HMODULE hModule = static_cast<HMODULE>(hMod);

    try {
        // Initialize logging system first
        icecap::agent::Logger::getInstance().initialize();
        LOG_INFO("Icecap Agent starting up");

        // Create and initialize application context
        g_appContext = std::make_unique<icecap::agent::ApplicationContext>();

        if (!g_appContext->initialize(hModule)) {
            LOG_ERROR("Failed to initialize application context");
            return 1;
        }

        LOG_INFO("Application context initialized successfully");

        // Set the application context for hooks to access
        icecap::agent::SetApplicationContext(g_appContext.get());

        // Keep the main thread alive while the application runs
        while (g_appContext->isRunning()) {
            Sleep(100);
        }

        LOG_INFO("Main thread shutting down");

    } catch (const std::exception& ex) {
        LOG_CRITICAL("Unhandled exception in main thread");
        if (g_appContext) {
            g_appContext->shutdown();
        }
        return 1;
    } catch (...) {
        LOG_CRITICAL("Unknown exception in main thread");
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

    // Wait for main thread to initialize logging and application context
    for (int i = 0; i < 100; ++i) { // Wait up to 5 seconds
        Sleep(50);
        if (g_appContext) {
            break;
        }
    }

    LOG_INFO("Cleanup thread started - press DELETE to unload DLL");

    // If g_appContext is still null after waiting, something went wrong
    if (!g_appContext) {
        LOG_ERROR("Cleanup thread: g_appContext is null after initialization wait");
        return 1;
    }

    LOG_DEBUG("Cleanup thread: g_appContext found, entering main loop");

    // Wait for NetworkManager to fully start before entering main loop
    bool appReady = false;
    for (int waitCount = 0; waitCount < 200 && g_appContext; waitCount++) { // Wait up to 10 seconds
        if (g_appContext->isRunning()) {
            appReady = true;
            break;
        }
        Sleep(50);
    }

    if (!appReady) {
        LOG_ERROR("App failed to start - cleanup thread exiting");
        return 1;
    }

    // Track DELETE key state for edge detection
    static bool deleteKeyWasPressed = false;

    while (g_appContext && g_appContext->isRunning())
    {
        // Check DELETE key state
        bool deleteKeyIsPressed = (GetAsyncKeyState(VK_DELETE) & 0x8000) != 0;

        // Edge detection: key was not pressed before, but is pressed now
        if (deleteKeyIsPressed && !deleteKeyWasPressed) {
            LOG_INFO("DELETE key pressed - initiating DLL unload sequence");

            if (g_appContext) {
                LOG_DEBUG("Stopping application context");
                g_appContext->stop();
            }

            Cleanup();

            LOG_INFO("Cleanup complete - calling FreeLibraryAndExitThread");
            FreeLibraryAndExitThread(hModule, 0);
        }

        // Update previous state
        deleteKeyWasPressed = deleteKeyIsPressed;

        Sleep(50);
    }

    LOG_DEBUG("Cleanup thread exiting");
    return 0;
}

bool APIENTRY DllMain(const HMODULE hMod, const DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        // Create threads
        HANDLE mainThread = CreateThread(nullptr, 0, MainThread, hMod, 0, nullptr);
        HANDLE cleanupThread = CreateThread(nullptr, 0, CleanupThread, hMod, 0, nullptr);

        // Close handles since we don't need them in DllMain
        // The threads will continue running independently
        if (mainThread && mainThread != INVALID_HANDLE_VALUE) {
            CloseHandle(mainThread);
        }
        if (cleanupThread && cleanupThread != INVALID_HANDLE_VALUE) {
            CloseHandle(cleanupThread);
        }
    }
    else if (reason == DLL_PROCESS_DETACH)
    {
        // Process is terminating - perform emergency cleanup
        // This happens when the game closes without our cleanup thread being triggered
        try {
            Cleanup();
        }
        catch (...) {
            // Ignore any exceptions during emergency cleanup
        }
    }
    return TRUE;
}

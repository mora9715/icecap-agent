#include <icecap/agent/application_context.hpp>
#include <icecap/agent/hooks/hook_manager.hpp>
#include "MinHook.h"

namespace icecap::agent {

ApplicationContext::ApplicationContext()
    : m_networkManager(std::make_unique<NetworkManager>())
{
}

ApplicationContext::~ApplicationContext()
{
    shutdown();
}

bool ApplicationContext::initialize(HMODULE hModule)
{
    if (m_initialized.exchange(true)) {
        return false; // Already initialized
    }

    m_hModule = hModule;
    m_running.store(true);

    try {
        // Initialize hooks with error checking
        try {
            hooks::InstallHooks(false);
        }
        catch (const std::exception& e) {
            // Hook installation failed
            m_running.store(false);
            m_initialized.store(false);
            return false;
        }

        // Start network manager with error checking
        constexpr unsigned short kPORT = 5050;
        constexpr char kDELIM = '\x1E';

        if (!m_networkManager || !m_networkManager->startServer(
            m_inboxQueue,
            m_outboxQueue,
            kPORT,
            kDELIM,
            m_inboxMutex,
            m_outboxMutex)) {

            // Network startup failed
            m_running.store(false);
            m_initialized.store(false);
            return false;
        }

        return true;
    }
    catch (const std::exception& e) {
        // Unexpected exception during initialization
        m_running.store(false);
        m_initialized.store(false);
        return false;
    }
    catch (...) {
        // Unknown exception
        m_running.store(false);
        m_initialized.store(false);
        return false;
    }
}

void ApplicationContext::shutdown()
{
    if (!m_initialized.load()) {
        return;
    }

    m_running.store(false);

    try {
        // Stop network manager first
        if (m_networkManager) {
            m_networkManager->stopServer();
        }
    }
    catch (...) {
        // Ignore network shutdown errors - we're shutting down anyway
    }

    try {
        // Cleanup MinHook
        MH_Uninitialize();
    }
    catch (...) {
        // Ignore MinHook cleanup errors - we're shutting down anyway
    }

    m_initialized.store(false);
}

bool ApplicationContext::isRunning() const
{
    return m_running.load() && m_networkManager && m_networkManager->isRunning();
}

void ApplicationContext::stop()
{
    m_running.store(false);
}

NetworkManager& ApplicationContext::getNetworkManager()
{
    return *m_networkManager;
}

std::queue<IncomingMessage>& ApplicationContext::getInboxQueue()
{
    return m_inboxQueue;
}

std::queue<OutgoingMessage>& ApplicationContext::getOutboxQueue()
{
    return m_outboxQueue;
}

std::mutex& ApplicationContext::getInboxMutex()
{
    return m_inboxMutex;
}

std::mutex& ApplicationContext::getOutboxMutex()
{
    return m_outboxMutex;
}

HMODULE ApplicationContext::getModuleHandle() const
{
    return m_hModule;
}

} // namespace icecap::agent
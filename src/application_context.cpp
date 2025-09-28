#include "MinHook.h"

#include <icecap/agent/application_context.hpp>
#include <icecap/agent/hooks/hook_manager.hpp>
#include <icecap/agent/logging.hpp>

namespace icecap::agent {

ApplicationContext::ApplicationContext() : m_networkManager(std::make_unique<transport::NetworkManager>()) {}

ApplicationContext::~ApplicationContext() {
    shutdown();
}

bool ApplicationContext::initialize(HMODULE hModule) {
    if (m_initialized.exchange(true)) {
        LOG_WARN("ApplicationContext is already initialized");
        return false;
    }

    LOG_INFO("Initializing ApplicationContext");
    m_hModule = hModule;
    m_running.store(true);

    try {
        // Initialize hooks with error checking
        try {
            LOG_DEBUG("Installing hooks");
            hooks::InstallHooks(false);
            LOG_INFO("Hooks installed successfully");
        } catch (const std::exception& e) {
            LOG_ERROR("Hook installation failed");
            m_running.store(false);
            m_initialized.store(false);
            return false;
        }

        // Start network manager with error checking
        constexpr unsigned short kPORT = 5050;

        LOG_DEBUG("Starting network server on port 5050");
        if (!m_networkManager ||
            !m_networkManager->startServer(m_inboxQueue, m_outboxQueue, kPORT, m_inboxMutex, m_outboxMutex)) {
            LOG_ERROR("Network server startup failed");
            m_running.store(false);
            m_initialized.store(false);
            return false;
        }

        LOG_INFO("ApplicationContext initialization completed successfully");
        return true;
    } catch (const std::exception& e) {
        LOG_CRITICAL("Unexpected exception during ApplicationContext initialization");
        m_running.store(false);
        m_initialized.store(false);
        return false;
    } catch (...) {
        // Unknown exception
        m_running.store(false);
        m_initialized.store(false);
        return false;
    }
}

void ApplicationContext::shutdown() {
    if (!m_initialized.load()) {
        LOG_DEBUG("ApplicationContext is not initialized, nothing to shutdown");
        return;
    }

    LOG_INFO("Shutting down ApplicationContext");
    m_running.store(false);

    try {
        // Stop network manager first
        if (m_networkManager) {
            LOG_DEBUG("Stopping network manager");
            m_networkManager->stopServer();
        }
    } catch (const std::exception& e) {
        LOG_WARN("Error during network manager shutdown");
    } catch (...) {
        LOG_WARN("Unknown error during network manager shutdown");
    }

    try {
        // Cleanup MinHook
        LOG_DEBUG("Uninitializing MinHook");
        MH_Uninitialize();
    } catch (const std::exception& e) {
        LOG_WARN("Error during MinHook cleanup");
    } catch (...) {
        LOG_WARN("Unknown error during MinHook cleanup");
    }

    m_initialized.store(false);
}

bool ApplicationContext::isRunning() const {
    return m_running.load() && m_networkManager && m_networkManager->isRunning();
}

void ApplicationContext::stop() {
    m_running.store(false);
}

transport::NetworkManager& ApplicationContext::getNetworkManager() {
    return *m_networkManager;
}

std::queue<IncomingMessage>& ApplicationContext::getInboxQueue() {
    return m_inboxQueue;
}

std::queue<OutgoingMessage>& ApplicationContext::getOutboxQueue() {
    return m_outboxQueue;
}

std::mutex& ApplicationContext::getInboxMutex() {
    return m_inboxMutex;
}

std::mutex& ApplicationContext::getOutboxMutex() {
    return m_outboxMutex;
}

HMODULE ApplicationContext::getModuleHandle() const {
    return m_hModule;
}

} // namespace icecap::agent
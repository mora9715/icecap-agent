#ifndef ICECAP_AGENT_APPLICATION_CONTEXT_HPP
#define ICECAP_AGENT_APPLICATION_CONTEXT_HPP

#include <memory>
#include <atomic>
#include <queue>
#include <mutex>
#include "icecap/agent/networking.hpp"

namespace icecap::agent {

/**
 * Application context that manages the lifecycle and dependencies
 * of the Icecap Agent. Replaces global state with proper RAII.
 */
class ApplicationContext {
public:
    ApplicationContext();
    ~ApplicationContext();

    // Non-copyable, non-movable
    ApplicationContext(const ApplicationContext&) = delete;
    ApplicationContext& operator=(const ApplicationContext&) = delete;
    ApplicationContext(ApplicationContext&&) = delete;
    ApplicationContext& operator=(ApplicationContext&&) = delete;

    // Initialize the application context
    bool initialize(HMODULE hModule);

    // Shutdown the application context
    void shutdown();

    // Check if the application should continue running
    bool isRunning() const;

    // Signal the application to stop
    void stop();

    // Get the network manager
    NetworkManager& getNetworkManager();

    // Get message queues
    std::queue<IncomingMessage>& getInboxQueue();
    std::queue<OutgoingMessage>& getOutboxQueue();

    // Get mutexes
    std::mutex& getInboxMutex();
    std::mutex& getOutboxMutex();

    // Get module handle
    HMODULE getModuleHandle() const;

private:
    std::atomic<bool> m_running{false};
    HMODULE m_hModule{nullptr};

    // Network management
    std::unique_ptr<NetworkManager> m_networkManager;

    // Message queues and synchronization
    std::queue<IncomingMessage> m_inboxQueue;
    std::queue<OutgoingMessage> m_outboxQueue;
    std::mutex m_inboxMutex;
    std::mutex m_outboxMutex;

    // Thread management
    std::atomic<bool> m_initialized{false};
};

} // namespace icecap::agent

#endif // ICECAP_AGENT_APPLICATION_CONTEXT_HPP
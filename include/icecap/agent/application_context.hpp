#ifndef ICECAP_AGENT_APPLICATION_CONTEXT_HPP
#define ICECAP_AGENT_APPLICATION_CONTEXT_HPP

#include <winsock2.h>

#include <atomic>
#include <memory>
#include <mutex>
#include <queue>

#include "icecap/agent/v1/commands.pb.h"
#include "icecap/agent/v1/events.pb.h"

#include "interfaces/IApplicationContext.hpp"
#include "transport/NetworkManager.hpp"

namespace icecap::agent {

// Message type aliases
using IncomingMessage = icecap::agent::v1::Command;
using OutgoingMessage = icecap::agent::v1::Event;

/**
 * Application context that manages the lifecycle and dependencies
 * of the Icecap Agent. Replaces global state with proper RAII.
 */
class ApplicationContext : public interfaces::IApplicationContext {
public:
    ApplicationContext();
    ~ApplicationContext();

    // Non-copyable, non-movable
    ApplicationContext(const ApplicationContext&) = delete;
    ApplicationContext& operator=(const ApplicationContext&) = delete;
    ApplicationContext(ApplicationContext&&) = delete;
    ApplicationContext& operator=(ApplicationContext&&) = delete;

    // Initialize the application context
    bool initialize(HMODULE hModule) override;

    // Shutdown the application context
    void shutdown() override;

    // Check if the application should continue running
    bool isRunning() const override;

    // Signal the application to stop
    void stop() override;

    // Get the network manager
    transport::NetworkManager& getNetworkManager();

    // Get message queues
    std::queue<IncomingMessage>& getInboxQueue() override;
    std::queue<OutgoingMessage>& getOutboxQueue() override;

    // Get mutexes
    std::mutex& getInboxMutex() override;
    std::mutex& getOutboxMutex() override;

    // Get module handle
    HMODULE getModuleHandle() const override;

private:
    std::atomic<bool> m_running{false};
    HMODULE m_hModule{nullptr};

    // Network management
    std::unique_ptr<transport::NetworkManager> m_networkManager;

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
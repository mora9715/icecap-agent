#ifndef ICECAP_AGENT_TRANSPORT_NETWORK_MANAGER_HPP
#define ICECAP_AGENT_TRANSPORT_NETWORK_MANAGER_HPP

#include <memory>
#include <string>
#include <queue>
#include <mutex>
#include <atomic>
#include "TcpServer.hpp"
#include "ProtocolHandler.hpp"
#include "../interfaces/IMessageHandler.hpp"
#include "icecap/agent/v1/commands.pb.h"
#include "icecap/agent/v1/events.pb.h"

namespace icecap::agent::transport {

using IncomingMessage = icecap::agent::v1::Command;
using OutgoingMessage = icecap::agent::v1::Event;

/**
 * High-level network coordinator that orchestrates TCP server and protocol handling.
 * Bridges between raw network transport and application message processing.
 */
class NetworkManager {
public:
    NetworkManager();
    ~NetworkManager();

    // Non-copyable, non-movable
    NetworkManager(const NetworkManager&) = delete;
    NetworkManager& operator=(const NetworkManager&) = delete;
    NetworkManager(NetworkManager&&) = delete;
    NetworkManager& operator=(NetworkManager&&) = delete;

    // Initialize and start the network services
    bool startServer(std::queue<IncomingMessage>& inbox,
                    std::queue<OutgoingMessage>& outbox,
                    unsigned short port,
                    std::mutex& inboxMutex,
                    std::mutex& outboxMutex);

    // Stop the network services
    void stopServer();

    // Check if server is running
    bool isRunning() const;

private:
    // Handle incoming raw data from TCP server
    void onDataReceived(const char* data, size_t length);
    void onClientConnected(SOCKET clientSocket);
    void onClientDisconnected(SOCKET clientSocket);
    void onNetworkError(const std::string& error);

    // Handle protocol-level messages
    void onMessageReceived(const std::string& message);
    void onProtocolError(const std::string& error);

    // Process outgoing messages
    void processOutgoingMessages();

    std::unique_ptr<TcpServer> m_tcpServer;
    std::unique_ptr<ProtocolHandler> m_protocolHandler;

    // Current client socket (single client support)
    SOCKET m_currentClient{INVALID_SOCKET};

    // Message queues (references to external queues)
    std::queue<IncomingMessage>* m_inboxQueue{nullptr};
    std::queue<OutgoingMessage>* m_outboxQueue{nullptr};
    std::mutex* m_inboxMutex{nullptr};
    std::mutex* m_outboxMutex{nullptr};

    // Protocol state
    std::string m_receiveBuffer;
    std::atomic<bool> m_running{false};
};

} // namespace icecap::agent::transport

#endif // ICECAP_AGENT_TRANSPORT_NETWORK_MANAGER_HPP
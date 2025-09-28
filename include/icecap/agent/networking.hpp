#ifndef ICECAP_AGENT_NETWORKING_HPP
#define ICECAP_AGENT_NETWORKING_HPP

#include <winsock2.h>
#include <string>
#include <queue>
#include <mutex>
#include <atomic>

#include "icecap/agent/v1/commands.pb.h"
#include "icecap/agent/v1/events.pb.h"
#include "icecap/agent/v1/common.pb.h"

namespace icecap::agent {

// Use protobuf messages directly for networking
using IncomingMessage = icecap::agent::v1::Command;
using OutgoingMessage = icecap::agent::v1::Event;

class NetworkManager {
public:
    NetworkManager();
    ~NetworkManager();

    // Initialize and start the TCP server
    bool startServer(std::queue<IncomingMessage>& inbox,
                    std::queue<OutgoingMessage>& outbox,
                    unsigned short port,
                    char delimiter,
                    std::mutex& inboxMutex,
                    std::mutex& outboxMutex);

    // Stop the server and cleanup
    void stopServer();

    // Check if server is running
    bool isRunning() const { return m_running; }

    // Server implementation (public for thread access)
    static void tcpServerThread(NetworkManager* manager,
                               std::queue<IncomingMessage>& inbox,
                               std::queue<OutgoingMessage>& outbox,
                               unsigned short port,
                               char delimiter,
                               std::mutex& inboxMutex,
                               std::mutex& outboxMutex);

    static void serveClient(SOCKET client,
                           std::queue<IncomingMessage>& inbox,
                           std::queue<OutgoingMessage>& outbox,
                           char delimiter,
                           const std::atomic<bool>& running,
                           std::mutex& inboxMutex,
                           std::mutex& outboxMutex);

private:
    static constexpr char DEFAULT_DELIMITER = '\x1E';
    static constexpr unsigned short DEFAULT_PORT = 5050;

    std::atomic<bool> m_running;
    SOCKET m_listenerSocket;
    HANDLE m_serverThread;
    std::mutex* m_inboxMutex;
    std::mutex* m_outboxMutex;
};

} // namespace icecap::agent

#endif // ICECAP_AGENT_NETWORKING_HPP
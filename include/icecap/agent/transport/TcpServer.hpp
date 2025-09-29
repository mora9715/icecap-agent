#ifndef ICECAP_AGENT_TRANSPORT_TCP_SERVER_HPP
#define ICECAP_AGENT_TRANSPORT_TCP_SERVER_HPP

#include <windows.h>
#include <winsock2.h>

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>

namespace icecap::agent::transport {

/**
 * Generic TCP server that handles raw byte streams.
 * Protocol-agnostic - delegates message parsing to handlers.
 */
class TcpServer {
public:
    // Callback types
    using DataCallback = std::function<void(const char* data, size_t length)>;
    using ClientConnectedCallback = std::function<void(SOCKET clientSocket)>;
    using ClientDisconnectedCallback = std::function<void(SOCKET clientSocket)>;
    using ErrorCallback = std::function<void(const std::string& error)>;

    TcpServer();
    ~TcpServer();

    // Non-copyable, non-movable
    TcpServer(const TcpServer&) = delete;
    TcpServer& operator=(const TcpServer&) = delete;
    TcpServer(TcpServer&&) = delete;
    TcpServer& operator=(TcpServer&&) = delete;

    // Server lifecycle
    bool start(unsigned short port);
    void stop();
    bool isRunning() const {
        return m_running.load();
    }

    // Send data to a specific client
    bool sendData(SOCKET clientSocket, const char* data, size_t length);

    // Callbacks
    void setDataCallback(DataCallback callback) {
        m_dataCallback = std::move(callback);
    }
    void setClientConnectedCallback(ClientConnectedCallback callback) {
        m_clientConnectedCallback = std::move(callback);
    }
    void setClientDisconnectedCallback(ClientDisconnectedCallback callback) {
        m_clientDisconnectedCallback = std::move(callback);
    }
    void setErrorCallback(ErrorCallback callback) {
        m_errorCallback = std::move(callback);
    }

private:
    // Threading
    static DWORD WINAPI ServerThreadProc(LPVOID param);
    void serverThreadMain();
    void handleClient(SOCKET clientSocket);

    std::atomic<bool> m_running{false};
    SOCKET m_listenerSocket{INVALID_SOCKET};
    SOCKET m_clientSocket{INVALID_SOCKET};
    std::mutex m_clientSocketMutex;
    HANDLE m_serverThread{nullptr};
    unsigned short m_port{0};

    // Callbacks
    DataCallback m_dataCallback;
    ClientConnectedCallback m_clientConnectedCallback;
    ClientDisconnectedCallback m_clientDisconnectedCallback;
    ErrorCallback m_errorCallback;
};

} // namespace icecap::agent::transport

#endif // ICECAP_AGENT_TRANSPORT_TCP_SERVER_HPP
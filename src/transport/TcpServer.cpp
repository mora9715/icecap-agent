#include <vector>

#include <icecap/agent/logging.hpp>
#include <icecap/agent/transport/TcpServer.hpp>

#pragma comment(lib, "ws2_32.lib")

namespace icecap::agent::transport {

TcpServer::TcpServer() = default;

TcpServer::~TcpServer() {
    stop();
}

bool TcpServer::start(unsigned short port) {
    if (m_running.load()) {
        if (m_errorCallback) {
            m_errorCallback("Server is already running");
        }
        return false;
    }

    m_port = port;

    // Initialize Winsock
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        if (m_errorCallback) {
            m_errorCallback("WSAStartup failed: " + std::to_string(result));
        }
        return false;
    }

    // Create listener socket
    m_listenerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_listenerSocket == INVALID_SOCKET) {
        if (m_errorCallback) {
            m_errorCallback("socket() failed: " + std::to_string(WSAGetLastError()));
        }
        WSACleanup();
        return false;
    }

    // Set socket to reuse address
    int opt = 1;
    setsockopt(m_listenerSocket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt));

    // Bind socket
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(m_listenerSocket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
        if (m_errorCallback) {
            m_errorCallback("bind() failed: " + std::to_string(WSAGetLastError()));
        }
        closesocket(m_listenerSocket);
        WSACleanup();
        return false;
    }

    // Start listening
    if (listen(m_listenerSocket, SOMAXCONN) == SOCKET_ERROR) {
        if (m_errorCallback) {
            m_errorCallback("listen() failed: " + std::to_string(WSAGetLastError()));
        }
        closesocket(m_listenerSocket);
        WSACleanup();
        return false;
    }

    // Start server thread
    m_running.store(true);
    m_serverThread = CreateThread(nullptr, 0, ServerThreadProc, this, 0, nullptr);
    if (m_serverThread == nullptr) {
        if (m_errorCallback) {
            m_errorCallback("CreateThread failed: " + std::to_string(GetLastError()));
        }
        m_running.store(false);
        closesocket(m_listenerSocket);
        WSACleanup();
        return false;
    }

    LOG_INFO("TCP Server started on port " + std::to_string(port));
    return true;
}

void TcpServer::stop() {
    if (!m_running.load()) {
        return;
    }

    LOG_INFO("Stopping TCP Server on port " + std::to_string(m_port));
    m_running.store(false);

    // Shutdown client socket to unblock recv() in handleClient()
    // shutdown() forces recv() to return immediately
    {
        std::lock_guard<std::mutex> lock(m_clientSocketMutex);
        if (m_clientSocket != INVALID_SOCKET) {
            shutdown(m_clientSocket, SD_BOTH);
        }
    }

    // Close listener socket to unblock accept()
    if (m_listenerSocket != INVALID_SOCKET) {
        closesocket(m_listenerSocket);
        m_listenerSocket = INVALID_SOCKET;
    }

    // Wait for server thread to finish
    if (m_serverThread != nullptr) {
        WaitForSingleObject(m_serverThread, 5000); // 5 second timeout
        CloseHandle(m_serverThread);
        m_serverThread = nullptr;
    }

    // Now close the client socket after thread has finished
    {
        std::lock_guard<std::mutex> lock(m_clientSocketMutex);
        if (m_clientSocket != INVALID_SOCKET) {
            closesocket(m_clientSocket);
            m_clientSocket = INVALID_SOCKET;
        }
    }

    WSACleanup();
    LOG_INFO("TCP Server stopped");
}

bool TcpServer::sendData(SOCKET clientSocket, const char* data, size_t length) {
    if (clientSocket == INVALID_SOCKET || data == nullptr || length == 0) {
        return false;
    }

    int totalSent = 0;
    while (totalSent < static_cast<int>(length)) {
        int sent = send(clientSocket, data + totalSent, static_cast<int>(length - totalSent), 0);
        if (sent == SOCKET_ERROR) {
            if (m_errorCallback) {
                m_errorCallback("send() failed: " + std::to_string(WSAGetLastError()));
            }
            return false;
        }
        totalSent += sent;
    }
    return true;
}

DWORD WINAPI TcpServer::ServerThreadProc(LPVOID param) {
    auto* server = static_cast<TcpServer*>(param);
    server->serverThreadMain();
    return 0;
}

void TcpServer::serverThreadMain() {
    LOG_INFO("TCP Server thread started");

    while (m_running.load()) {
        SOCKET clientSocket = accept(m_listenerSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            if (m_running.load()) {
                if (m_errorCallback) {
                    m_errorCallback("accept() failed: " + std::to_string(WSAGetLastError()));
                }
            }
            break;
        }

        // Store client socket for potential shutdown
        {
            std::lock_guard<std::mutex> lock(m_clientSocketMutex);
            m_clientSocket = clientSocket;
        }

        LOG_INFO("Client connected");
        if (m_clientConnectedCallback) {
            m_clientConnectedCallback(clientSocket);
        }

        // Handle client in the same thread (single client at a time)
        handleClient(clientSocket);

        LOG_INFO("Client disconnected");
        if (m_clientDisconnectedCallback) {
            m_clientDisconnectedCallback(clientSocket);
        }
        closesocket(clientSocket);

        {
            std::lock_guard<std::mutex> lock(m_clientSocketMutex);
            m_clientSocket = INVALID_SOCKET;
        }
    }

    LOG_INFO("TCP Server thread finished");
}

void TcpServer::handleClient(SOCKET clientSocket) {
    std::vector<char> buffer(4096);

    while (m_running.load()) {
        int received = recv(clientSocket, buffer.data(), static_cast<int>(buffer.size()), 0);

        if (received <= 0) {
            if (received == 0) {
                LOG_DEBUG("Client closed connection");
            } else {
                LOG_ERROR("recv() failed: " + std::to_string(WSAGetLastError()));
            }
            break;
        }

        // Notify callback about received data
        if (m_dataCallback) {
            m_dataCallback(buffer.data(), static_cast<size_t>(received));
        }
    }
}

} // namespace icecap::agent::transport
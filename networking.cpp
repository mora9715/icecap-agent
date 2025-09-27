#include "networking.h"
#include <windows.h>
#include <vector>

#pragma comment(lib, "ws2_32.lib")

// Helpers for length-prefixed (big-endian) framing
static inline bool tryExtractFrame(std::string &buffer, std::string &frame) {
    if (buffer.size() < 4) return false;
    uint32_t len = (static_cast<unsigned char>(buffer[0]) << 24) |
                   (static_cast<unsigned char>(buffer[1]) << 16) |
                   (static_cast<unsigned char>(buffer[2]) << 8)  |
                   (static_cast<unsigned char>(buffer[3]));
    if (buffer.size() < 4u + len) return false;
    frame.assign(buffer.data() + 4, len);
    buffer.erase(0, 4u + len);
    return true;
}

static inline void writeFrame(const std::string &payload, std::string &out) {
    uint32_t len = static_cast<uint32_t>(payload.size());
    char hdr[4];
    hdr[0] = static_cast<char>((len >> 24) & 0xFF);
    hdr[1] = static_cast<char>((len >> 16) & 0xFF);
    hdr[2] = static_cast<char>((len >> 8) & 0xFF);
    hdr[3] = static_cast<char>(len & 0xFF);
    out.assign(hdr, hdr + 4);
    out.append(payload);
}

NetworkManager::NetworkManager()
    : m_running(false)
    , m_listenerSocket(INVALID_SOCKET)
    , m_serverThread(nullptr)
    , m_inboxMutex(nullptr)
    , m_outboxMutex(nullptr)
{
}

NetworkManager::~NetworkManager()
{
    stopServer();
}

// Static thread function for Windows CreateThread
static DWORD WINAPI ServerThreadProc(LPVOID param)
{
    struct ThreadParams {
        NetworkManager* manager;
        std::queue<IncomingMessage>* inbox;
        std::queue<OutgoingMessage>* outbox;
        unsigned short port;
        char delimiter;
    };

    ThreadParams* p = static_cast<ThreadParams*>(param);
    NetworkManager::tcpServerThread(p->manager, *p->inbox, *p->outbox, p->port, p->delimiter);
    delete p;
    return 0;
}

bool NetworkManager::startServer(std::queue<IncomingMessage>& inbox,
                                std::queue<OutgoingMessage>& outbox,
                                unsigned short port,
                                char delimiter)
{
    if (m_running) {
        return false; // Already running
    }

    m_running = true;

    // Create server thread
    struct ThreadParams {
        NetworkManager* manager;
        std::queue<IncomingMessage>* inbox;
        std::queue<OutgoingMessage>* outbox;
        unsigned short port;
        char delimiter;
    };

    ThreadParams* params = new ThreadParams{this, &inbox, &outbox, port, delimiter};

    m_serverThread = CreateThread(nullptr, 0, ServerThreadProc, params, 0, nullptr);

    return m_serverThread != nullptr;
}

void NetworkManager::stopServer()
{
    if (!m_running) {
        return;
    }

    m_running = false;

    // Close the listener socket to interrupt the accept call
    if (m_listenerSocket != INVALID_SOCKET) {
        ::closesocket(m_listenerSocket);
        m_listenerSocket = INVALID_SOCKET;
    }

    // Wait for server thread to finish
    if (m_serverThread) {
        WaitForSingleObject(m_serverThread, 2000); // Wait up to 2 seconds
        CloseHandle(m_serverThread);
        m_serverThread = nullptr;
    }
}

void NetworkManager::serveClient(SOCKET client,
                                std::queue<IncomingMessage>& inbox,
                                std::queue<OutgoingMessage>& outbox,
                                char delim,
                                volatile bool& running,
                                std::mutex& inboxMutex,
                                std::mutex& outboxMutex)
{
    std::string assemble;
    char buf[1024];

    while (running) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(client, &readfds);
        timeval tv{ 0, 50'000 }; // 50 ms

        int ready = ::select(0, &readfds, nullptr, nullptr, &tv);
        if (ready == SOCKET_ERROR) {
            break;
        }

        if (ready > 0 && FD_ISSET(client, &readfds)) {
            int n = ::recv(client, buf, static_cast<int>(sizeof(buf)), 0);
            if (n <= 0) {
                break;
            }
            assemble.append(buf, buf + n);

            std::string frame;
            while (tryExtractFrame(assemble, frame)) {
                IncomingMessage cmd;
                if (cmd.ParseFromArray(frame.data(), static_cast<int>(frame.size()))) {
                    std::lock_guard<std::mutex> lk(inboxMutex);
                    inbox.push(std::move(cmd));
                }
            }
        }

        {
            std::lock_guard<std::mutex> lk(outboxMutex);
            if (!outbox.empty()) {
                const OutgoingMessage &evt = outbox.front();
                std::string payload;
                if (evt.SerializeToString(&payload)) {
                    std::string toSend;
                    writeFrame(payload, toSend);
                    int sent = ::send(client, toSend.data(), static_cast<int>(toSend.size()), 0);
                    if (sent == SOCKET_ERROR) {
                        break;
                    }
                }
                outbox.pop();
            }
        }
    }

    ::closesocket(client);
}

void NetworkManager::tcpServerThread(NetworkManager* manager,
                                    std::queue<IncomingMessage>& inbox,
                                    std::queue<OutgoingMessage>& outbox,
                                    unsigned short port,
                                    char delim)
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        return;
    }

    SOCKET listener = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == INVALID_SOCKET) {
        WSACleanup();
        return;
    }

    // Store the listener socket so we can close it during cleanup
    manager->m_listenerSocket = listener;

    BOOL opt = TRUE;
    ::setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if (::bind(listener, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        ::closesocket(listener);
        manager->m_listenerSocket = INVALID_SOCKET;
        WSACleanup();
        return;
    }
    if (::listen(listener, SOMAXCONN) == SOCKET_ERROR) {
        ::closesocket(listener);
        manager->m_listenerSocket = INVALID_SOCKET;
        WSACleanup();
        return;
    }

    // Get mutex references from the manager
    extern std::mutex inbox_mtx;
    extern std::mutex outbox_mtx;

    while (manager->m_running) {
        // Set socket to non-blocking for accept to allow checking m_running
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(listener, &readfds);
        timeval tv{ 0, 100'000 }; // 100 ms

        int ready = ::select(0, &readfds, nullptr, nullptr, &tv);
        if (ready == SOCKET_ERROR || !manager->m_running) {
            break;
        }

        if (ready > 0 && FD_ISSET(listener, &readfds)) {
            SOCKET client = ::accept(listener, nullptr, nullptr);
            if (client == INVALID_SOCKET) {
                int err = WSAGetLastError();
                if (err == WSAEINTR) continue;
                break;
            }

            serveClient(client, inbox, outbox, delim, manager->m_running, inbox_mtx, outbox_mtx);
        }
    }

    ::closesocket(listener);
    manager->m_listenerSocket = INVALID_SOCKET;
    WSACleanup();
}
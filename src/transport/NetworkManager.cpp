#include <chrono>
#include <thread>

#include <google/protobuf/util/json_util.h>

#include <icecap/agent/logging.hpp>
#include <icecap/agent/transport/NetworkManager.hpp>

namespace icecap::agent::transport {

NetworkManager::NetworkManager()
    : m_tcpServer(std::make_unique<TcpServer>()), m_protocolHandler(std::make_unique<ProtocolHandler>()) {}

NetworkManager::~NetworkManager() {
    stopServer();
}

bool NetworkManager::startServer(std::queue<IncomingMessage>& inbox, std::queue<OutgoingMessage>& outbox,
                                 unsigned short port, std::mutex& inboxMutex, std::mutex& outboxMutex) {
    if (m_running.load()) {
        LOG_WARN("NetworkManager: Server is already running");
        return false;
    }

    // Store references to message queues
    m_inboxQueue = &inbox;
    m_outboxQueue = &outbox;
    m_inboxMutex = &inboxMutex;
    m_outboxMutex = &outboxMutex;

    // Set up TCP server callbacks
    m_tcpServer->setDataCallback([this](const char* data, size_t length) { onDataReceived(data, length); });
    m_tcpServer->setClientConnectedCallback([this](SOCKET clientSocket) { onClientConnected(clientSocket); });
    m_tcpServer->setClientDisconnectedCallback([this](SOCKET clientSocket) { onClientDisconnected(clientSocket); });
    m_tcpServer->setErrorCallback([this](const std::string& error) { onNetworkError(error); });

    // Set up protocol handler callbacks
    m_protocolHandler->setMessageCallback([this](const std::string& message) { onMessageReceived(message); });
    m_protocolHandler->setErrorCallback([this](const std::string& error) { onProtocolError(error); });

    // Start TCP server
    if (!m_tcpServer->start(port)) {
        LOG_ERROR("NetworkManager: Failed to start TCP server on port " + std::to_string(port));
        return false;
    }

    m_running.store(true);

    // Start background thread for processing outgoing messages
    try {
        m_outgoingMessageThread = std::thread(&NetworkManager::outgoingMessageThreadMain, this);
        LOG_DEBUG("NetworkManager: Started outgoing message processing thread");
    } catch (const std::exception& e) {
        LOG_ERROR("NetworkManager: Failed to start outgoing message thread: " + std::string(e.what()));
        m_running.store(false);
        m_tcpServer->stop();
        return false;
    }

    LOG_INFO("NetworkManager: Started on port " + std::to_string(port));
    return true;
}

void NetworkManager::stopServer() {
    if (!m_running.load()) {
        return;
    }

    LOG_INFO("NetworkManager: Stopping server");
    m_running.store(false);

    // Clear callbacks first to prevent use-after-free when TcpServer thread shuts down
    if (m_tcpServer) {
        m_tcpServer->setDataCallback(nullptr);
        m_tcpServer->setClientConnectedCallback(nullptr);
        m_tcpServer->setClientDisconnectedCallback(nullptr);
        m_tcpServer->setErrorCallback(nullptr);
    }

    // Stop and join the outgoing message thread
    if (m_outgoingMessageThread.joinable()) {
        LOG_DEBUG("NetworkManager: Waiting for outgoing message thread to finish");
        m_outgoingMessageThread.join();
        LOG_DEBUG("NetworkManager: Outgoing message thread finished");
    }

    if (m_tcpServer) {
        m_tcpServer->stop();
    }

    // Reset state
    m_currentClient = INVALID_SOCKET;
    m_receiveBuffer.clear();
    m_inboxQueue = nullptr;
    m_outboxQueue = nullptr;
    m_inboxMutex = nullptr;
    m_outboxMutex = nullptr;

    LOG_INFO("NetworkManager: Stopped");
}

bool NetworkManager::isRunning() const {
    return m_running.load() && m_tcpServer && m_tcpServer->isRunning();
}

void NetworkManager::onDataReceived(const char* data, size_t length) {
    if (!m_running.load() || !data || length == 0) {
        return;
    }

    // Append to receive buffer
    m_receiveBuffer.append(data, length);

    // Try to extract complete frames
    std::string frame;
    while (m_protocolHandler->extractFrame(m_receiveBuffer, frame)) {
        onMessageReceived(frame);
    }
}

void NetworkManager::onClientConnected(SOCKET clientSocket) {
    LOG_INFO("NetworkManager: Client connected");
    m_currentClient = clientSocket;
    m_receiveBuffer.clear();
}

void NetworkManager::onClientDisconnected(SOCKET clientSocket) {
    LOG_INFO("NetworkManager: Client disconnected");
    if (m_currentClient == clientSocket) {
        m_currentClient = INVALID_SOCKET;
        m_receiveBuffer.clear();
    }
}

void NetworkManager::onNetworkError(const std::string& error) {
    LOG_ERROR("NetworkManager: Network error: " + error);
}

void NetworkManager::onMessageReceived(const std::string& message) {
    if (!m_running.load() || !m_inboxQueue || !m_inboxMutex) {
        return;
    }

    // Parse protobuf message
    IncomingMessage command;
    if (!command.ParseFromString(message)) {
        LOG_ERROR("NetworkManager: Failed to parse incoming protobuf message");
        return;
    }

    LOG_DEBUG("NetworkManager: Received command with ID '" + command.id() + "'");

    // Add to inbox queue
    {
        std::lock_guard<std::mutex> lock(*m_inboxMutex);
        m_inboxQueue->push(std::move(command));
    }
}

void NetworkManager::onProtocolError(const std::string& error) {
    LOG_ERROR("NetworkManager: Protocol error: " + error);
}

void NetworkManager::processOutgoingMessages() {
    if (!m_running.load() || !m_outboxQueue || !m_outboxMutex || m_currentClient == INVALID_SOCKET) {
        return;
    }

    // Process all pending outgoing messages
    std::queue<OutgoingMessage> localQueue;
    {
        std::lock_guard<std::mutex> lock(*m_outboxMutex);
        localQueue.swap(*m_outboxQueue);
    }

    while (!localQueue.empty()) {
        const auto& event = localQueue.front();

        // Serialize to protobuf
        std::string serialized;
        if (!event.SerializeToString(&serialized)) {
            LOG_ERROR("NetworkManager: Failed to serialize outgoing event with ID '" + event.id() + "'");
            localQueue.pop();
            continue;
        }

        // Encode with protocol handler
        std::string encoded = m_protocolHandler->encodeMessage(serialized);

        // Send via TCP
        if (!m_tcpServer->sendData(m_currentClient, encoded.data(), encoded.size())) {
            LOG_ERROR("NetworkManager: Failed to send event with ID '" + event.id() + "'");
        } else {
            LOG_DEBUG("NetworkManager: Sent event with ID '" + event.id() + "'");
        }

        localQueue.pop();
    }
}

void NetworkManager::outgoingMessageThreadMain() {
    LOG_DEBUG("NetworkManager: Outgoing message thread started");

    while (m_running.load()) {
        try {
            // Process outgoing messages if we have a connected client
            if (m_currentClient != INVALID_SOCKET) {
                processOutgoingMessages();
            }

            // Sleep for a short time to avoid busy waiting
            std::this_thread::sleep_for(std::chrono::milliseconds(10));

        } catch (const std::exception& e) {
            LOG_ERROR("NetworkManager: Exception in outgoing message thread: " + std::string(e.what()));
            // Continue running even if there's an error
        } catch (...) {
            LOG_ERROR("NetworkManager: Unknown exception in outgoing message thread");
            // Continue running even if there's an error
        }
    }

    LOG_DEBUG("NetworkManager: Outgoing message thread finished");
}

} // namespace icecap::agent::transport
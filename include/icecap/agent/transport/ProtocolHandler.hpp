#ifndef ICECAP_AGENT_TRANSPORT_PROTOCOL_HANDLER_HPP
#define ICECAP_AGENT_TRANSPORT_PROTOCOL_HANDLER_HPP

#include "../interfaces/INetworkProtocol.hpp"
#include <string>
#include <functional>

namespace icecap::agent::transport {

/**
 * Length-prefixed protocol handler for TCP communication.
 * Uses big-endian 4-byte length prefix followed by payload.
 */
class ProtocolHandler : public interfaces::INetworkProtocol {
public:
    ProtocolHandler() = default;
    ~ProtocolHandler() override = default;

    // INetworkProtocol implementation
    bool extractFrame(std::string& buffer, std::string& frame) override;
    std::string encodeMessage(const std::string& payload) override;
    void setMessageCallback(MessageCallback callback) override;
    void setErrorCallback(ErrorCallback callback) override;

private:
    MessageCallback m_messageCallback;
    ErrorCallback m_errorCallback;
};

} // namespace icecap::agent::transport

#endif // ICECAP_AGENT_TRANSPORT_PROTOCOL_HANDLER_HPP
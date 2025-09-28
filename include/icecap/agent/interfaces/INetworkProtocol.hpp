#ifndef ICECAP_AGENT_INTERFACES_INETWORK_PROTOCOL_HPP
#define ICECAP_AGENT_INTERFACES_INETWORK_PROTOCOL_HPP

#include <functional>
#include <string>

namespace icecap::agent::interfaces {

class INetworkProtocol {
public:
    virtual ~INetworkProtocol() = default;

    // Callback types for message handling
    using MessageCallback = std::function<void(const std::string& message)>;
    using ErrorCallback = std::function<void(const std::string& error)>;

    // Try to extract a complete frame from the buffer
    // Returns true if a frame was extracted, false if more data is needed
    virtual bool extractFrame(std::string& buffer, std::string& frame) = 0;

    // Encode a message for transmission
    virtual std::string encodeMessage(const std::string& payload) = 0;

    // Set callbacks for message and error handling
    virtual void setMessageCallback(MessageCallback callback) = 0;
    virtual void setErrorCallback(ErrorCallback callback) = 0;
};

} // namespace icecap::agent::interfaces

#endif // ICECAP_AGENT_INTERFACES_INETWORK_PROTOCOL_HPP
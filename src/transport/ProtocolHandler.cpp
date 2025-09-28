#include <cstdint>

#include <icecap/agent/transport/ProtocolHandler.hpp>

namespace icecap::agent::transport {

bool ProtocolHandler::extractFrame(std::string& buffer, std::string& frame) {
    if (buffer.size() < 4) {
        return false;
    }

    // Extract big-endian 4-byte length
    uint32_t len = (static_cast<unsigned char>(buffer[0]) << 24) | (static_cast<unsigned char>(buffer[1]) << 16) |
                   (static_cast<unsigned char>(buffer[2]) << 8) | (static_cast<unsigned char>(buffer[3]));

    if (buffer.size() < 4u + len) {
        return false;
    }

    // Extract frame payload
    frame.assign(buffer.data() + 4, len);
    buffer.erase(0, 4u + len);
    return true;
}

std::string ProtocolHandler::encodeMessage(const std::string& payload) {
    uint32_t len = static_cast<uint32_t>(payload.size());

    // Create big-endian 4-byte header
    char header[4];
    header[0] = static_cast<char>((len >> 24) & 0xFF);
    header[1] = static_cast<char>((len >> 16) & 0xFF);
    header[2] = static_cast<char>((len >> 8) & 0xFF);
    header[3] = static_cast<char>(len & 0xFF);

    std::string result;
    result.assign(header, 4);
    result.append(payload);
    return result;
}

void ProtocolHandler::setMessageCallback(MessageCallback callback) {
    m_messageCallback = std::move(callback);
}

void ProtocolHandler::setErrorCallback(ErrorCallback callback) {
    m_errorCallback = std::move(callback);
}

} // namespace icecap::agent::transport
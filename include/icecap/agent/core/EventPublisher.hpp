#ifndef ICECAP_AGENT_CORE_EVENT_PUBLISHER_HPP
#define ICECAP_AGENT_CORE_EVENT_PUBLISHER_HPP

#include <string>

#include "icecap/agent/v1/commands.pb.h"
#include "icecap/agent/v1/events.pb.h"

namespace icecap::agent::core {

using IncomingMessage = icecap::agent::v1::Command;
using OutgoingMessage = icecap::agent::v1::Event;

/**
 * Factory for creating event messages.
 * Centralizes event creation logic and ensures consistent event structure.
 */
class EventPublisher {
public:
    EventPublisher() = default;
    ~EventPublisher() = default;

    // Create event for successful Lua variable read
    static OutgoingMessage createLuaVariableReadEvent(const IncomingMessage& originalCommand,
                                                      const std::string& result);

    // Create generic error event
    static OutgoingMessage createErrorEvent(const IncomingMessage& originalCommand, const std::string& errorMessage);

    // Generate unique event ID
    static std::string generateEventId();

private:
    // UUID generation helper
    static std::string generateUUID();
};

} // namespace icecap::agent::core

#endif // ICECAP_AGENT_CORE_EVENT_PUBLISHER_HPP
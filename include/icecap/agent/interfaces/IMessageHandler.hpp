#ifndef ICECAP_AGENT_INTERFACES_IMESSAGE_HANDLER_HPP
#define ICECAP_AGENT_INTERFACES_IMESSAGE_HANDLER_HPP

#include "icecap/agent/v1/commands.pb.h"
#include "icecap/agent/v1/events.pb.h"

namespace icecap::agent::interfaces {

// Message type aliases
using IncomingMessage = icecap::agent::v1::Command;
using OutgoingMessage = icecap::agent::v1::Event;

class IMessageHandler {
public:
    virtual ~IMessageHandler() = default;

    // Process an incoming command and optionally generate events
    virtual void processCommand(const IncomingMessage& command) = 0;

    // Check if there are pending outgoing events
    virtual bool hasOutgoingEvents() const = 0;

    // Get the next outgoing event (removes it from queue)
    virtual OutgoingMessage getNextOutgoingEvent() = 0;
};

} // namespace icecap::agent::interfaces

#endif // ICECAP_AGENT_INTERFACES_IMESSAGE_HANDLER_HPP
#ifndef ICECAP_AGENT_CORE_MESSAGE_PROCESSOR_HPP
#define ICECAP_AGENT_CORE_MESSAGE_PROCESSOR_HPP

#include "../interfaces/IMessageHandler.hpp"
#include "../interfaces/IApplicationContext.hpp"
#include "icecap/agent/v1/commands.pb.h"
#include "icecap/agent/v1/events.pb.h"
#include <memory>

namespace icecap::agent::core {

// Message type aliases
using IncomingMessage = icecap::agent::v1::Command;
using OutgoingMessage = icecap::agent::v1::Event;

/**
 * Central message processor that routes incoming commands to appropriate handlers.
 * Acts as the main orchestrator for command processing logic.
 */
class MessageProcessor : public interfaces::IMessageHandler {
public:
    explicit MessageProcessor(interfaces::IApplicationContext* context);
    ~MessageProcessor() override = default;

    // Non-copyable, non-movable
    MessageProcessor(const MessageProcessor&) = delete;
    MessageProcessor& operator=(const MessageProcessor&) = delete;
    MessageProcessor(MessageProcessor&&) = delete;
    MessageProcessor& operator=(MessageProcessor&&) = delete;

    // IMessageHandler implementation
    void processCommand(const IncomingMessage& command) override;
    bool hasOutgoingEvents() const override;
    OutgoingMessage getNextOutgoingEvent() override;

private:
    // Command handlers
    void handleLuaExecuteCommand(const IncomingMessage& command);
    void handleLuaReadVariableCommand(const IncomingMessage& command);
    void handleClickToMoveCommand(const IncomingMessage& command);

    // Helper to add event to outbox
    void enqueueEvent(const OutgoingMessage& event);

    interfaces::IApplicationContext* m_context;
};

} // namespace icecap::agent::core

#endif // ICECAP_AGENT_CORE_MESSAGE_PROCESSOR_HPP
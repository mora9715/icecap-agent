#include <icecap/agent/core/CommandExecutor.hpp>
#include <icecap/agent/core/EventPublisher.hpp>
#include <icecap/agent/core/MessageProcessor.hpp>
#include <icecap/agent/logging.hpp>

namespace icecap::agent::core {

MessageProcessor::MessageProcessor(interfaces::IApplicationContext* context) : m_context(context) {
    if (!m_context) {
        throw std::invalid_argument("MessageProcessor requires a valid IApplicationContext");
    }
}

void MessageProcessor::processCommand(const IncomingMessage& command) {
    if (!m_context) {
        LOG_ERROR("MessageProcessor: No application context available");
        return;
    }

    LOG_DEBUG("MessageProcessor: Processing command with ID '" + command.id() + "', operation_id '" +
              command.operation_id() + "', type: " + std::to_string(static_cast<int>(command.type())));

    // Route command to appropriate handler
    switch (command.type()) {
        case icecap::agent::v1::COMMAND_TYPE_LUA_EXECUTE:
            handleLuaExecuteCommand(command);
            break;

        case icecap::agent::v1::COMMAND_TYPE_LUA_READ_VARIABLE:
            handleLuaReadVariableCommand(command);
            break;

        case icecap::agent::v1::COMMAND_TYPE_CLICK_TO_MOVE:
            handleClickToMoveCommand(command);
            break;

        default:
            LOG_WARN("MessageProcessor: Unknown command type " + std::to_string(static_cast<int>(command.type())) +
                     " for message ID " + command.id());
            break;
    }
}

bool MessageProcessor::hasOutgoingEvents() const {
    if (!m_context) {
        return false;
    }

    std::lock_guard<std::mutex> lock(m_context->getOutboxMutex());
    return !m_context->getOutboxQueue().empty();
}

OutgoingMessage MessageProcessor::getNextOutgoingEvent() {
    if (!m_context) {
        throw std::runtime_error("MessageProcessor: No application context available");
    }

    std::lock_guard<std::mutex> lock(m_context->getOutboxMutex());
    if (m_context->getOutboxQueue().empty()) {
        throw std::runtime_error("MessageProcessor: No outgoing events available");
    }

    OutgoingMessage event = m_context->getOutboxQueue().front();
    m_context->getOutboxQueue().pop();
    return event;
}

void MessageProcessor::handleLuaExecuteCommand(const IncomingMessage& command) {
    if (!command.has_lua_execute_payload()) {
        LOG_WARN("MessageProcessor: LUA_EXECUTE command missing payload for message ID " + command.id());
        return;
    }

    const auto& payload = command.lua_execute_payload();
    LOG_INFO("MessageProcessor: Executing Lua code for message ID " + command.id());

    CommandExecutor executor;
    bool success = executor.executeLuaCode(payload.executable_code(), "source");

    if (success) {
        LOG_INFO("MessageProcessor: Lua execution successful for message ID " + command.id());
    } else {
        LOG_ERROR("MessageProcessor: Lua execution failed for message ID " + command.id());
    }

    // Note: LUA_EXECUTE typically doesn't generate response events unless there's an error
    // If error events are needed, they can be added here
}

void MessageProcessor::handleLuaReadVariableCommand(const IncomingMessage& command) {
    if (!command.has_lua_read_variable_payload()) {
        LOG_WARN("MessageProcessor: LUA_READ_VARIABLE command missing payload for message ID " + command.id());
        return;
    }

    const auto& payload = command.lua_read_variable_payload();
    LOG_INFO("MessageProcessor: Reading Lua variable '" + payload.variable_name() + "' for message ID " + command.id());

    CommandExecutor executor;
    std::string result = executor.readLuaVariable(payload.variable_name());

    // Create and enqueue response event
    OutgoingMessage event = EventPublisher::createLuaVariableReadEvent(command, result);

    LOG_DEBUG("MessageProcessor: Created event with ID '" + event.id() + "', operation_id '" + event.operation_id() +
              "' for command ID '" + command.id() + "'");

    enqueueEvent(event);

    LOG_INFO("MessageProcessor: Successfully read variable '" + payload.variable_name() + "' for message ID " +
             command.id());
}

void MessageProcessor::handleClickToMoveCommand(const IncomingMessage& command) {
    if (!command.has_click_to_move_payload()) {
        LOG_WARN("MessageProcessor: CLICK_TO_MOVE command missing payload for message ID " + command.id());
        return;
    }

    const auto& payload = command.click_to_move_payload();
    LOG_INFO("MessageProcessor: Processing ClickToMove command for message ID " + command.id());

    const uintptr_t playerBaseAddress = payload.player_base_address();
    if (playerBaseAddress == 0) {
        LOG_ERROR("MessageProcessor: ClickToMove command has invalid player_base_address (0) for message ID " +
                  command.id());
        return;
    }

    if (!payload.has_position()) {
        LOG_ERROR("MessageProcessor: ClickToMove command missing position for message ID " + command.id());
        return;
    }

    CommandExecutor executor;
    bool success =
        executor.executeClickToMove(playerBaseAddress, payload.position(), payload.action(), payload.precision());

    if (success) {
        LOG_INFO("MessageProcessor: ClickToMove successful for message ID " + command.id());
    } else {
        LOG_WARN("MessageProcessor: ClickToMove failed for message ID " + command.id());
    }

    // Note: ClickToMove typically doesn't generate response events unless there's an error
    // If status events are needed, they can be added here
}

void MessageProcessor::enqueueEvent(const OutgoingMessage& event) {
    if (!m_context) {
        LOG_ERROR("MessageProcessor: Cannot enqueue event - no application context");
        return;
    }

    std::lock_guard<std::mutex> lock(m_context->getOutboxMutex());
    m_context->getOutboxQueue().push(event);
}

} // namespace icecap::agent::core
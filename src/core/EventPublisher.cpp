#include <iomanip>
#include <random>
#include <sstream>

#include <icecap/agent/core/EventPublisher.hpp>

namespace icecap::agent::core {

OutgoingMessage EventPublisher::createLuaVariableReadEvent(const IncomingMessage& originalCommand,
                                                           const std::string& result) {
    OutgoingMessage event;
    event.set_id(generateEventId());
    event.set_operation_id(originalCommand.operation_id());
    event.set_type(icecap::agent::v1::EVENT_TYPE_LUA_VARIABLE_READ);

    auto* payload = event.mutable_lua_variable_read_event_payload();
    payload->set_result(result);

    return event;
}

OutgoingMessage EventPublisher::createErrorEvent(const IncomingMessage& originalCommand,
                                                 const std::string& errorMessage) {
    OutgoingMessage event;
    event.set_id(generateEventId());
    event.set_operation_id(originalCommand.operation_id());
    event.set_type(icecap::agent::v1::EVENT_TYPE_OPERATION_FAILED);

    return event;
}

OutgoingMessage EventPublisher::createSuccessEvent(const IncomingMessage& originalCommand) {
    OutgoingMessage event;
    event.set_id(generateEventId());
    event.set_operation_id(originalCommand.operation_id());
    event.set_type(icecap::agent::v1::EVENT_TYPE_OPERATION_SUCCEEDED);

    return event;
}

std::string EventPublisher::generateEventId() {
    return generateUUID();
}

std::string EventPublisher::generateUUID() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static std::uniform_int_distribution<> dis2(8, 11);

    std::stringstream ss;
    ss << std::hex;

    for (int i = 0; i < 8; i++) {
        ss << dis(gen);
    }
    ss << "-";
    for (int i = 0; i < 4; i++) {
        ss << dis(gen);
    }
    ss << "-4";
    for (int i = 0; i < 3; i++) {
        ss << dis(gen);
    }
    ss << "-";
    ss << dis2(gen);
    for (int i = 0; i < 3; i++) {
        ss << dis(gen);
    }
    ss << "-";
    for (int i = 0; i < 12; i++) {
        ss << dis(gen);
    }

    return ss.str();
}

} // namespace icecap::agent::core
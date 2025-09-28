#ifndef ICECAP_AGENT_INTERFACES_IAPPLICATION_CONTEXT_HPP
#define ICECAP_AGENT_INTERFACES_IAPPLICATION_CONTEXT_HPP

#include <windows.h>
#include <winsock2.h>

#include <mutex>
#include <queue>

#include "icecap/agent/v1/commands.pb.h"
#include "icecap/agent/v1/events.pb.h"

namespace icecap::agent::interfaces {

// Message type aliases
using IncomingMessage = icecap::agent::v1::Command;
using OutgoingMessage = icecap::agent::v1::Event;

class IApplicationContext {
public:
    virtual ~IApplicationContext() = default;

    // Lifecycle management
    virtual bool initialize(HMODULE hModule) = 0;
    virtual void shutdown() = 0;
    virtual bool isRunning() const = 0;
    virtual void stop() = 0;

    // Message queue access
    virtual std::queue<IncomingMessage>& getInboxQueue() = 0;
    virtual std::queue<OutgoingMessage>& getOutboxQueue() = 0;

    // Synchronization primitives
    virtual std::mutex& getInboxMutex() = 0;
    virtual std::mutex& getOutboxMutex() = 0;

    // Module information
    virtual HMODULE getModuleHandle() const = 0;
};

} // namespace icecap::agent::interfaces

#endif // ICECAP_AGENT_INTERFACES_IAPPLICATION_CONTEXT_HPP
#ifndef ICECAP_AGENT_SHARED_STATE_HPP
#define ICECAP_AGENT_SHARED_STATE_HPP

#include <mutex>
#include <queue>
#include <icecap/agent/networking.hpp>

namespace icecap::agent {

// Extern declarations for shared state used by hooks
extern std::mutex inbox_mtx;
extern std::mutex outbox_mtx;

extern std::queue<IncomingMessage> inboxQueue;
extern std::queue<OutgoingMessage> outboxQueue;

} // namespace icecap::agent

#endif // ICECAP_AGENT_SHARED_STATE_HPP
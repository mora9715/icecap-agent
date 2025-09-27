#pragma once

#include <mutex>
#include <queue>
#include "../networking.h"

// Extern declarations for shared state used by hooks
extern std::mutex inbox_mtx;
extern std::mutex outbox_mtx;

extern std::queue<IncomingMessage> inboxQueue;
extern std::queue<OutgoingMessage> outboxQueue;

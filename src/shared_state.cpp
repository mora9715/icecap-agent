#include "shared_state.hpp"
#include <atomic>

namespace icecap::agent {

// Thread-safe storage for the application context pointer
static std::atomic<ApplicationContext*> g_applicationContext{nullptr};

ApplicationContext* GetApplicationContext() {
    return g_applicationContext.load();
}

void SetApplicationContext(ApplicationContext* context) {
    g_applicationContext.store(context);
}

} // namespace icecap::agent
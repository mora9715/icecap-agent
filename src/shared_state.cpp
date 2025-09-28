#include <atomic>

#include <icecap/agent/application_context.hpp>
#include <icecap/agent/shared_state.hpp>

namespace icecap::agent {

// Thread-safe storage for the application context pointer
static std::atomic<ApplicationContext*> g_applicationContext{nullptr};

interfaces::IApplicationContext* GetApplicationContext() {
    return g_applicationContext.load();
}

void SetApplicationContext(ApplicationContext* context) {
    g_applicationContext.store(context);
}

} // namespace icecap::agent
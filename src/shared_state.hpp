#ifndef ICECAP_AGENT_SHARED_STATE_HPP
#define ICECAP_AGENT_SHARED_STATE_HPP

#include <icecap/agent/application_context.hpp>

namespace icecap::agent {

// Function to get the current application context
// This will be set by main.cpp and used by hooks
ApplicationContext* GetApplicationContext();

// Function to set the application context (called from main.cpp)
void SetApplicationContext(ApplicationContext* context);

} // namespace icecap::agent

#endif // ICECAP_AGENT_SHARED_STATE_HPP
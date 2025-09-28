#ifndef ICECAP_AGENT_SHARED_STATE_HPP
#define ICECAP_AGENT_SHARED_STATE_HPP

#include "interfaces/IApplicationContext.hpp"

namespace icecap::agent {

// Forward declaration
class ApplicationContext;

// Function to get the current application context interface
// This will be set by main.cpp and used by hooks
interfaces::IApplicationContext* GetApplicationContext();

// Function to set the application context (called from main.cpp)
void SetApplicationContext(ApplicationContext* context);

} // namespace icecap::agent

#endif // ICECAP_AGENT_SHARED_STATE_HPP
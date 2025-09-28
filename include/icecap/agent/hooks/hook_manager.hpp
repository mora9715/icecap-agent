#ifndef ICECAP_AGENT_HOOKS_HOOK_MANAGER_HPP
#define ICECAP_AGENT_HOOKS_HOOK_MANAGER_HPP

namespace icecap::agent::hooks {

// Installs all available hooks
void InstallHooks(bool enableEvents = true);

} // namespace icecap::agent::hooks

#endif // ICECAP_AGENT_HOOKS_HOOK_MANAGER_HPP

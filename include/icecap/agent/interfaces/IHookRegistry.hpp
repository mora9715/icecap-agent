#ifndef ICECAP_AGENT_INTERFACES_IHOOK_REGISTRY_HPP
#define ICECAP_AGENT_INTERFACES_IHOOK_REGISTRY_HPP

#include <functional>
#include <memory>
#include <string>

namespace icecap::agent::interfaces {

class IHook {
public:
    virtual ~IHook() = default;

    // Install the hook
    virtual bool install() = 0;

    // Uninstall the hook
    virtual bool uninstall() = 0;

    // Check if hook is currently installed
    virtual bool isInstalled() const = 0;

    // Get hook name for logging/debugging
    virtual std::string getName() const = 0;
};

class IHookRegistry {
public:
    virtual ~IHookRegistry() = default;

    // Register a hook for management
    virtual void registerHook(std::shared_ptr<IHook> hook) = 0;

    // Install all registered hooks
    virtual bool installAllHooks() = 0;

    // Uninstall all registered hooks
    virtual bool uninstallAllHooks() = 0;

    // Get the number of installed hooks
    virtual size_t getInstalledHookCount() const = 0;

    // Get hook by name
    virtual std::shared_ptr<IHook> getHook(const std::string& name) const = 0;
};

} // namespace icecap::agent::interfaces

#endif // ICECAP_AGENT_INTERFACES_IHOOK_REGISTRY_HPP
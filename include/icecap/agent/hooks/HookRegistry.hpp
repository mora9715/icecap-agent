#ifndef ICECAP_AGENT_HOOKS_HOOK_REGISTRY_HPP
#define ICECAP_AGENT_HOOKS_HOOK_REGISTRY_HPP

#include "../interfaces/IHookRegistry.hpp"
#include <vector>
#include <memory>
#include <unordered_map>

namespace icecap::agent::hooks {

/**
 * Concrete implementation of IHookRegistry.
 * Manages a collection of hooks and provides batch operations.
 */
class HookRegistry : public interfaces::IHookRegistry {
public:
    HookRegistry() = default;
    ~HookRegistry() override;

    // Non-copyable, non-movable
    HookRegistry(const HookRegistry&) = delete;
    HookRegistry& operator=(const HookRegistry&) = delete;
    HookRegistry(HookRegistry&&) = delete;
    HookRegistry& operator=(HookRegistry&&) = delete;

    // IHookRegistry implementation
    void registerHook(std::shared_ptr<interfaces::IHook> hook) override;
    bool installAllHooks() override;
    bool uninstallAllHooks() override;
    size_t getInstalledHookCount() const override;
    std::shared_ptr<interfaces::IHook> getHook(const std::string& name) const override;

private:
    std::vector<std::shared_ptr<interfaces::IHook>> m_hooks;
    std::unordered_map<std::string, std::shared_ptr<interfaces::IHook>> m_hooksByName;
};

} // namespace icecap::agent::hooks

#endif // ICECAP_AGENT_HOOKS_HOOK_REGISTRY_HPP
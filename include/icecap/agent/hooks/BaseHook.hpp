#ifndef ICECAP_AGENT_HOOKS_BASE_HOOK_HPP
#define ICECAP_AGENT_HOOKS_BASE_HOOK_HPP

#include <string>

#include "../interfaces/IHookRegistry.hpp"

namespace icecap::agent::hooks {

/**
 * Base class for all hook implementations.
 * Provides common functionality and standardized interface.
 */
class BaseHook : public interfaces::IHook {
public:
    explicit BaseHook(std::string name);
    ~BaseHook() override = default;

    // Non-copyable, non-movable
    BaseHook(const BaseHook&) = delete;
    BaseHook& operator=(const BaseHook&) = delete;
    BaseHook(BaseHook&&) = delete;
    BaseHook& operator=(BaseHook&&) = delete;

    // IHook implementation
    bool install() override;
    bool uninstall() override;
    bool isInstalled() const override {
        return m_installed;
    }
    std::string getName() const override {
        return m_name;
    }

protected:
    // Template method pattern - subclasses implement these
    virtual bool doInstall() = 0;
    virtual bool doUninstall() = 0;

    // Helper methods for subclasses
    void setInstalled(bool installed) {
        m_installed = installed;
    }

private:
    std::string m_name;
    bool m_installed{false};
};

} // namespace icecap::agent::hooks

#endif // ICECAP_AGENT_HOOKS_BASE_HOOK_HPP
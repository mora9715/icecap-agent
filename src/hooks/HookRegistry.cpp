#include <icecap/agent/hooks/HookRegistry.hpp>
#include <icecap/agent/logging.hpp>
#include <ranges>

namespace icecap::agent::hooks {

HookRegistry::~HookRegistry() {
    // Ensure all hooks are uninstalled during destruction
    HookRegistry::uninstallAllHooks();
}

void HookRegistry::registerHook(const std::shared_ptr<interfaces::IHook> hook) {
    if (!hook) {
        LOG_WARN("HookRegistry: Attempted to register null hook");
        return;
    }

    const std::string& name = hook->getName();

    // Check if a hook with this name already exists
    if (m_hooksByName.contains(name)) {
        LOG_WARN("HookRegistry: Hook with name '" + name + "' is already registered");
        return;
    }

    m_hooks.push_back(hook);
    m_hooksByName[name] = hook;

    LOG_DEBUG("HookRegistry: Registered hook '" + name + "'");
}

bool HookRegistry::installAllHooks() {
    bool allSuccessful = true;
    size_t successCount = 0;

    LOG_INFO("HookRegistry: Installing " + std::to_string(m_hooks.size()) + " hooks");

    for (const auto& hook : m_hooks) {
        if (hook->install()) {
            successCount++;
        } else {
            LOG_ERROR("HookRegistry: Failed to install hook '" + hook->getName() + "'");
            allSuccessful = false;
        }
    }

    LOG_INFO("HookRegistry: Successfully installed " + std::to_string(successCount) +
             " out of " + std::to_string(m_hooks.size()) + " hooks");

    return allSuccessful;
}

bool HookRegistry::uninstallAllHooks() {
    bool allSuccessful = true;
    size_t successCount = 0;

    LOG_INFO("HookRegistry: Uninstalling " + std::to_string(m_hooks.size()) + " hooks");

    // Uninstall in reverse order (LIFO)
    for (const auto & hook : std::ranges::reverse_view(m_hooks)) {
        if (hook->uninstall()) {
            successCount++;
        } else {
            LOG_ERROR("HookRegistry: Failed to uninstall hook '" + hook->getName() + "'");
            allSuccessful = false;
        }
    }

    LOG_INFO("HookRegistry: Successfully uninstalled " + std::to_string(successCount) +
             " out of " + std::to_string(m_hooks.size()) + " hooks");

    return allSuccessful;
}

size_t HookRegistry::getInstalledHookCount() const {
    size_t count = 0;
    for (const auto& hook : m_hooks) {
        if (hook->isInstalled()) {
            count++;
        }
    }
    return count;
}

std::shared_ptr<interfaces::IHook> HookRegistry::getHook(const std::string& name) const {
    auto it = m_hooksByName.find(name);
    if (it != m_hooksByName.end()) {
        return it->second;
    }
    return nullptr;
}

} // namespace icecap::agent::hooks
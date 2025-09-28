#include <icecap/agent/hooks/BaseHook.hpp>
#include <icecap/agent/logging.hpp>

namespace icecap::agent::hooks {

BaseHook::BaseHook(std::string name) : m_name(std::move(name)) {
}

bool BaseHook::install() {
    if (m_installed) {
        LOG_WARN("Hook '" + m_name + "' is already installed");
        return true;
    }

    LOG_DEBUG("Installing hook: " + m_name);

    if (doInstall()) {
        m_installed = true;
        LOG_INFO("Successfully installed hook: " + m_name);
        return true;
    } else {
        LOG_ERROR("Failed to install hook: " + m_name);
        return false;
    }
}

bool BaseHook::uninstall() {
    if (!m_installed) {
        LOG_WARN("Hook '" + m_name + "' is not installed");
        return true;
    }

    LOG_DEBUG("Uninstalling hook: " + m_name);

    if (doUninstall()) {
        m_installed = false;
        LOG_INFO("Successfully uninstalled hook: " + m_name);
        return true;
    } else {
        LOG_ERROR("Failed to uninstall hook: " + m_name);
        return false;
    }
}

} // namespace icecap::agent::hooks
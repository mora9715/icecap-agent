#ifndef ICECAP_AGENT_HOOKS_D3D9_HOOK_HPP
#define ICECAP_AGENT_HOOKS_D3D9_HOOK_HPP

#include "BaseHook.hpp"
#include <d3d9.h>

namespace icecap::agent::hooks {

/**
 * D3D9 EndScene hook implementation.
 * Intercepts the EndScene call to process pending commands.
 */
class D3D9Hook : public BaseHook {
public:
    D3D9Hook();
    ~D3D9Hook() override = default;

    // Get the original EndScene function pointer
    static long (__stdcall* GetOriginalEndScene())(IDirect3DDevice9*);

protected:
    // BaseHook implementation
    bool doInstall() override;
    bool doUninstall() override;

private:
    // Hook function signature
    using EndSceneFunc = long (__stdcall*)(IDirect3DDevice9*);

    // Original function pointer storage
    static EndSceneFunc s_originalEndScene;

    // Hook implementation
    static long __stdcall HookedEndScene(IDirect3DDevice9* pDevice);

    // Helper methods
    bool findEndSceneAddress();
    bool installMinHook();
    bool uninstallMinHook();

    // Target function address
    void* m_targetAddress{nullptr};
};

} // namespace icecap::agent::hooks

#endif // ICECAP_AGENT_HOOKS_D3D9_HOOK_HPP
#ifndef ICECAP_AGENT_HOOKS_D3D9_HOOKS_HPP
#define ICECAP_AGENT_HOOKS_D3D9_HOOKS_HPP

struct IDirect3DDevice9;

namespace icecap::agent::hooks {

extern long (__stdcall* g_OriginalEndScene)(IDirect3DDevice9*);

long __stdcall HookedEndScene(IDirect3DDevice9* pDevice);

} // namespace icecap::agent::hooks

#endif // ICECAP_AGENT_HOOKS_D3D9_HOOKS_HPP

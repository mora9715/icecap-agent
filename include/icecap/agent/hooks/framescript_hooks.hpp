#ifndef ICECAP_AGENT_HOOKS_FRAMESCRIPT_HOOKS_HPP
#define ICECAP_AGENT_HOOKS_FRAMESCRIPT_HOOKS_HPP

namespace icecap::agent::hooks {

// Function pointer for the original FrameScript__SignalEvent
using p_FrameScriptSignalEvent = void(__cdecl*)(int eventId, const char* fmt, int argsBase);
extern p_FrameScriptSignalEvent g_OriginalFrameScriptSignalEvent;

// Hooked function declaration
void __cdecl HookedFrameScriptSignalEvent(int eventid, const char* fmt, int argsBase);

} // namespace icecap::agent::hooks

#endif // ICECAP_AGENT_HOOKS_FRAMESCRIPT_HOOKS_HPP

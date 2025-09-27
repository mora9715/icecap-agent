#pragma once

// Function pointer for the original FrameScript__SignalEvent
using p_FrameScriptSignalEvent = void (__cdecl*)(int eventId, const char* fmt, int argsBase);
extern p_FrameScriptSignalEvent g_OriginalFrameScriptSignalEvent;

// Hooked function declaration
void __cdecl HookedFrameScriptSignalEvent(int eventid, const char* fmt, int argsBase);

#include <winsock2.h>
#include <windows.h>
#include <d3d9.h>
#include <mutex>
#include <string>
#include <cstdint>

#include <icecap/agent/hooks/d3d9_hooks.hpp>
#include "../shared_state.hpp"
#include "icecap/agent/v1/commands.pb.h"
#include "icecap/agent/v1/events.pb.h"
#include "icecap/agent/v1/common.pb.h"

namespace icecap::agent::hooks {

// Original function pointer definition
long (__stdcall* g_OriginalEndScene)(IDirect3DDevice9*) = nullptr;

long __stdcall HookedEndScene(IDirect3DDevice9* pDevice)
{
    using p_Dostring = int(__cdecl*)(const char* script, const char* scriptname, int null);
    static auto Dostring = reinterpret_cast<p_Dostring>(0x819210);

    using p_GetText = char* (__cdecl*)(const char* text, std::nullptr_t unk1, std::nullptr_t unk2);
    static auto GetText = reinterpret_cast<p_GetText>(0x819D40);

    std::lock_guard lk(inbox_mtx);
    if (!inboxQueue.empty()) {
        IncomingMessage cmd = inboxQueue.front();
        inboxQueue.pop();

        // Process the command according to its type
        switch (cmd.type()) {
            case v1::COMMAND_TYPE_LUA_EXECUTE: {
                if (cmd.has_lua_execute_payload()) {
                    const auto &payload = cmd.lua_execute_payload();
                    Dostring(payload.executable_code().c_str(), "source", 0);
                }
                break;
            }
            case v1::COMMAND_TYPE_LUA_READ_VARIABLE: {
                if (cmd.has_lua_read_variable_payload()) {
                    const auto &payload = cmd.lua_read_variable_payload();
                    // Read the variable
                    std::string result = GetText(payload.variable_name().c_str(), 0, 0);

                    // Build and enqueue Event
                    OutgoingMessage event;
                    event.set_id(cmd.id());
                    event.set_type(v1::EVENT_TYPE_LUA_VARIABLE_READ);
                    auto *ev_payload = event.mutable_lua_variable_read_event_payload();
                    ev_payload->set_result(result);

                    std::lock_guard outbox_lk(outbox_mtx);
                    outboxQueue.push(event);
                }
                break;
            }
            case v1::COMMAND_TYPE_CLICK_TO_MOVE: {
                if (cmd.has_click_to_move_payload()) {
                    const auto &payload = cmd.click_to_move_payload();

                    // Validate required fields
                    const uintptr_t thisPtr = payload.player_base_address();
                    if (thisPtr != 0) {
                        using p_ClickToMove = bool(__fastcall*)(void* /*this*/, int /*dummy_edx*/, int /*clickType*/, void* /*interactGuid*/, float* /*clickPos*/, float /*precision*/);
                        static auto CGPlayer_C__ClickToMove = reinterpret_cast<p_ClickToMove>(0x00727400);

                        float pos[3] = {0.0f, 0.0f, 0.0f};
                        if (payload.has_position()) {
                            const auto &p = payload.position();
                            pos[0] = p.x();
                            pos[1] = p.y();
                            pos[2] = p.z();
                        }

                        // Provide a non-null GUID buffer (32 bytes = 2x UInt128), zero-initialized.
                        // Some client builds expect to read two 128-bit values from this pointer.
                        unsigned char guidBuf[32] = {0};
                        void* interactGuid = guidBuf;

                        // Clamp action to known enum values (defensive)
                        const int action = static_cast<int>(payload.action());

                        // Perform the call (ECX=this via __fastcall)
                        CGPlayer_C__ClickToMove(reinterpret_cast<void*>(thisPtr), 0 /*dummy EDX*/, action, interactGuid, pos, payload.precision());
                    }
                }
                break;
            }
            default:
                break;
        }
    }

    return g_OriginalEndScene(pDevice);
}

}

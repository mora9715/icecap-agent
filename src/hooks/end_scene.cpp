#include <winsock2.h>
#include <windows.h>
#include <d3d9.h>
#include <mutex>
#include <string>
#include <cstdint>
#include <random>
#include <sstream>
#include <iomanip>

#include <icecap/agent/hooks/d3d9_hooks.hpp>
#include "../shared_state.hpp"
#include "icecap/agent/v1/commands.pb.h"
#include "icecap/agent/v1/events.pb.h"
#include "icecap/agent/v1/common.pb.h"
#include "icecap/agent/logging.hpp"

using icecap::agent::GetApplicationContext;

namespace icecap::agent::hooks {

// UUID generation function
std::string generateUUID() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static std::uniform_int_distribution<> dis2(8, 11);

    std::stringstream ss;
    ss << std::hex;

    for (int i = 0; i < 8; i++) {
        ss << dis(gen);
    }
    ss << "-";
    for (int i = 0; i < 4; i++) {
        ss << dis(gen);
    }
    ss << "-4";
    for (int i = 0; i < 3; i++) {
        ss << dis(gen);
    }
    ss << "-";
    ss << dis2(gen);
    for (int i = 0; i < 3; i++) {
        ss << dis(gen);
    }
    ss << "-";
    for (int i = 0; i < 12; i++) {
        ss << dis(gen);
    }

    return ss.str();
}

// Original function pointer definition
long (__stdcall* g_OriginalEndScene)(IDirect3DDevice9*) = nullptr;

long __stdcall HookedEndScene(IDirect3DDevice9* pDevice)
{
    using p_Dostring = int(__cdecl*)(const char* script, const char* scriptname, int null);
    static auto Dostring = reinterpret_cast<p_Dostring>(0x819210);

    using p_GetText = char* (__cdecl*)(const char* text, std::nullptr_t unk1, std::nullptr_t unk2);
    static auto GetText = reinterpret_cast<p_GetText>(0x819D40);

    auto* appContext = GetApplicationContext();
    if (!appContext) {
        LOG_WARN("EndScene: No application context available");
        return g_OriginalEndScene(pDevice);
    }

    std::lock_guard lk(appContext->getInboxMutex());
    if (!appContext->getInboxQueue().empty()) {
        IncomingMessage cmd = appContext->getInboxQueue().front();
        appContext->getInboxQueue().pop();

        LOG_DEBUG("EndScene: Received message with ID '" + cmd.id() + "', operation_id '" + cmd.operation_id() + "', type: " + std::to_string(static_cast<int>(cmd.type())));

        // Process the command according to its type
        switch (cmd.type()) {
            case v1::COMMAND_TYPE_LUA_EXECUTE: {
                if (cmd.has_lua_execute_payload()) {
                    const auto &payload = cmd.lua_execute_payload();
                    LOG_INFO("EndScene: Executing Lua code for message ID " + cmd.id());
                    try {
                        int result = Dostring(payload.executable_code().c_str(), "source", 0);
                        if (result == 0) {
                            LOG_INFO("EndScene: Lua execution successful for message ID " + cmd.id());
                        } else {
                            LOG_ERROR("EndScene: Lua execution failed with code " + std::to_string(result) + " for message ID " + cmd.id());
                        }
                    } catch (...) {
                        LOG_ERROR("EndScene: Exception during Lua execution for message ID " + cmd.id());
                    }
                } else {
                    LOG_WARN("EndScene: LUA_EXECUTE command missing payload for message ID " + cmd.id());
                }
                break;
            }
            case v1::COMMAND_TYPE_LUA_READ_VARIABLE: {
                if (cmd.has_lua_read_variable_payload()) {
                    const auto &payload = cmd.lua_read_variable_payload();
                    LOG_INFO("EndScene: Reading Lua variable '" + payload.variable_name() + "' for message ID " + cmd.id());
                    try {
                        // Read the variable
                        char* result_ptr = GetText(payload.variable_name().c_str(), 0, 0);
                        std::string result = result_ptr ? result_ptr : "";

                        // Build and enqueue Event
                        OutgoingMessage event;
                        event.set_id(generateUUID());
                        event.set_operation_id(cmd.operation_id());
                        event.set_type(v1::EVENT_TYPE_LUA_VARIABLE_READ);
                        auto *ev_payload = event.mutable_lua_variable_read_event_payload();
                        ev_payload->set_result(result);

                        LOG_DEBUG("EndScene: Created event with ID '" + event.id() + "', operation_id '" + event.operation_id() + "' for command ID '" + cmd.id() + "'");

                        std::lock_guard outbox_lk(appContext->getOutboxMutex());
                        appContext->getOutboxQueue().push(event);

                        LOG_INFO("EndScene: Successfully read variable '" + payload.variable_name() + "' for message ID " + cmd.id());
                    } catch (...) {
                        LOG_ERROR("EndScene: Exception while reading variable '" + payload.variable_name() + "' for message ID " + cmd.id());
                    }
                } else {
                    LOG_WARN("EndScene: LUA_READ_VARIABLE command missing payload for message ID " + cmd.id());
                }
                break;
            }
            case v1::COMMAND_TYPE_CLICK_TO_MOVE: {
                if (cmd.has_click_to_move_payload()) {
                    const auto &payload = cmd.click_to_move_payload();
                    LOG_INFO("EndScene: Processing ClickToMove command for message ID " + cmd.id());

                    // Validate required fields
                    const uintptr_t thisPtr = payload.player_base_address();
                    if (thisPtr != 0) {
                        try {
                            using p_ClickToMove = bool(__fastcall*)(void* /*this*/, int /*dummy_edx*/, int /*clickType*/, void* /*interactGuid*/, float* /*clickPos*/, float /*precision*/);
                            static auto CGPlayer_C__ClickToMove = reinterpret_cast<p_ClickToMove>(0x00727400);

                            float pos[3] = {0.0f, 0.0f, 0.0f};
                            if (payload.has_position()) {
                                const auto &p = payload.position();
                                pos[0] = p.x();
                                pos[1] = p.y();
                                pos[2] = p.z();
                                LOG_DEBUG("EndScene: ClickToMove position: (" + std::to_string(pos[0]) + ", " +
                                         std::to_string(pos[1]) + ", " + std::to_string(pos[2]) + ")");
                            }

                            // Provide a non-null GUID buffer (32 bytes = 2x UInt128), zero-initialized.
                            // Some client builds expect to read two 128-bit values from this pointer.
                            unsigned char guidBuf[32] = {0};
                            void* interactGuid = guidBuf;

                            // Clamp action to known enum values (defensive)
                            const int action = static_cast<int>(payload.action());
                            LOG_DEBUG("EndScene: ClickToMove action: " + std::to_string(action));

                            // Perform the call (ECX=this via __fastcall)
                            bool result = CGPlayer_C__ClickToMove(reinterpret_cast<void*>(thisPtr), 0 /*dummy EDX*/, action, interactGuid, pos, payload.precision());

                            if (result) {
                                LOG_INFO("EndScene: ClickToMove successful for message ID " + cmd.id());
                            } else {
                                LOG_WARN("EndScene: ClickToMove returned false for message ID " + cmd.id());
                            }
                        } catch (...) {
                            LOG_ERROR("EndScene: Exception during ClickToMove execution for message ID " + cmd.id());
                        }
                    } else {
                        LOG_ERROR("EndScene: ClickToMove command has invalid player_base_address (0) for message ID " + cmd.id());
                    }
                } else {
                    LOG_WARN("EndScene: CLICK_TO_MOVE command missing payload for message ID " + cmd.id());
                }
                break;
            }
            default:
                LOG_WARN("EndScene: Unknown command type " + std::to_string(static_cast<int>(cmd.type())) + " for message ID " + cmd.id());
                break;
        }
    }

    return g_OriginalEndScene(pDevice);
}

}

#ifndef ICECAP_AGENT_CORE_COMMAND_EXECUTOR_HPP
#define ICECAP_AGENT_CORE_COMMAND_EXECUTOR_HPP

#include <string>

#include "icecap/agent/v1/commands.pb.h"
#include "icecap/agent/v1/events.pb.h"

namespace icecap::agent::core {

using IncomingMessage = icecap::agent::v1::Command;
using OutgoingMessage = icecap::agent::v1::Event;

/**
 * Executor for specific command types.
 * Contains the actual implementation logic for each supported command.
 */
class CommandExecutor {
public:
    CommandExecutor() = default;
    ~CommandExecutor() = default;

    // Non-copyable, non-movable
    CommandExecutor(const CommandExecutor&) = delete;
    CommandExecutor& operator=(const CommandExecutor&) = delete;
    CommandExecutor(CommandExecutor&&) = delete;
    CommandExecutor& operator=(CommandExecutor&&) = delete;

    // Lua execution
    bool executeLuaCode(const std::string& code, const std::string& scriptName = "source");

    // Lua variable reading
    std::string readLuaVariable(const std::string& variableName);

    // ClickToMove execution
    bool executeClickToMove(uintptr_t playerBaseAddress, const icecap::agent::v1::Position& position,
                            icecap::agent::v1::ClickToMoveAction action, float precision);

private:
    // Game function pointers (cached for performance)
    struct GameFunctions {
        using p_Dostring = int(__cdecl*)(const char* script, const char* scriptname, int null);
        using p_GetText = char*(__cdecl*)(const char* text, std::nullptr_t unk1, std::nullptr_t unk2);
        using p_ClickToMove = bool(__fastcall*)(void* /*this*/, int /*dummy_edx*/, int /*clickType*/,
                                                void* /*interactGuid*/, float* /*clickPos*/, float /*precision*/);

        static p_Dostring Dostring;
        static p_GetText GetText;
        static p_ClickToMove CGPlayer_C__ClickToMove;
    };
};

} // namespace icecap::agent::core

#endif // ICECAP_AGENT_CORE_COMMAND_EXECUTOR_HPP
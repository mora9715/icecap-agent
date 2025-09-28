#include <icecap/agent/core/CommandExecutor.hpp>
#include <icecap/agent/logging.hpp>

namespace icecap::agent::core {

// Initialize static function pointers with hardcoded addresses
CommandExecutor::GameFunctions::p_Dostring CommandExecutor::GameFunctions::Dostring =
    reinterpret_cast<p_Dostring>(0x819210);

CommandExecutor::GameFunctions::p_GetText CommandExecutor::GameFunctions::GetText =
    reinterpret_cast<p_GetText>(0x819D40);

CommandExecutor::GameFunctions::p_ClickToMove CommandExecutor::GameFunctions::CGPlayer_C__ClickToMove =
    reinterpret_cast<p_ClickToMove>(0x00727400);

bool CommandExecutor::executeLuaCode(const std::string& code, const std::string& scriptName) {
    if (code.empty()) {
        LOG_WARN("CommandExecutor: Empty Lua code provided");
        return false;
    }

    try {
        LOG_DEBUG("CommandExecutor: Executing Lua code: " + code.substr(0, 100) + (code.length() > 100 ? "..." : ""));

        int result = GameFunctions::Dostring(code.c_str(), scriptName.c_str(), 0);

        if (result == 0) {
            LOG_DEBUG("CommandExecutor: Lua execution successful");
            return true;
        } else {
            LOG_ERROR("CommandExecutor: Lua execution failed with code " + std::to_string(result));
            return false;
        }
    } catch (...) {
        LOG_ERROR("CommandExecutor: Exception during Lua execution");
        return false;
    }
}

std::string CommandExecutor::readLuaVariable(const std::string& variableName) {
    if (variableName.empty()) {
        LOG_WARN("CommandExecutor: Empty variable name provided");
        return "";
    }

    try {
        LOG_DEBUG("CommandExecutor: Reading Lua variable '" + variableName + "'");

        char* result_ptr = GameFunctions::GetText(variableName.c_str(), nullptr, nullptr);
        std::string result = result_ptr ? result_ptr : "";

        LOG_DEBUG("CommandExecutor: Variable '" + variableName + "' = '" + result + "'");
        return result;
    } catch (...) {
        LOG_ERROR("CommandExecutor: Exception while reading variable '" + variableName + "'");
        return "";
    }
}

bool CommandExecutor::executeClickToMove(uintptr_t playerBaseAddress,
                                        const icecap::agent::v1::Position& position,
                                        icecap::agent::v1::ClickToMoveAction action,
                                        float precision) {
    if (playerBaseAddress == 0) {
        LOG_ERROR("CommandExecutor: Invalid player base address (0)");
        return false;
    }

    try {
        float pos[3] = {position.x(), position.y(), position.z()};

        LOG_DEBUG("CommandExecutor: ClickToMove position: (" +
                 std::to_string(pos[0]) + ", " +
                 std::to_string(pos[1]) + ", " +
                 std::to_string(pos[2]) + ")");

        // Provide a non-null GUID buffer (32 bytes = 2x UInt128), zero-initialized
        unsigned char guidBuf[32] = {0};
        void* interactGuid = guidBuf;

        const int actionValue = static_cast<int>(action);
        LOG_DEBUG("CommandExecutor: ClickToMove action: " + std::to_string(actionValue));

        // Perform the call (ECX=this via __fastcall)
        bool result = GameFunctions::CGPlayer_C__ClickToMove(
            reinterpret_cast<void*>(playerBaseAddress),
            0 /*dummy EDX*/,
            actionValue,
            interactGuid,
            pos,
            precision);

        if (result) {
            LOG_DEBUG("CommandExecutor: ClickToMove successful");
        } else {
            LOG_WARN("CommandExecutor: ClickToMove returned false");
        }

        return result;
    } catch (...) {
        LOG_ERROR("CommandExecutor: Exception during ClickToMove execution");
        return false;
    }
}

} // namespace icecap::agent::core
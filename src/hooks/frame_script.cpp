#include <windows.h>

#include <mutex>
#include <string>
#include <vector>

#include <icecap/agent/hooks/framescript_hooks.hpp>

namespace icecap::agent::hooks {

// Original function pointer definition
p_FrameScriptSignalEvent g_OriginalFrameScriptSignalEvent = nullptr;

// Safe memory read helpers (use ReadProcessMemory to avoid AVs on bad pointers)
static bool readMem(void* addr, void* out, SIZE_T size) {
    SIZE_T bytesRead = 0;
    return ReadProcessMemory(GetCurrentProcess(), addr, out, size, &bytesRead) && bytesRead == size;
}

template <typename T>
static bool readVal(uintptr_t addr, T& out) {
    return readMem(reinterpret_cast<void*>(addr), &out, sizeof(T));
}

static std::string readCStringBounded(uintptr_t addr, size_t maxLen = 1024) {
    if (addr == 0)
        return "(null)";
    std::string result;
    result.reserve(64);
    const size_t chunk = 128;
    std::vector<char> buf(chunk);

    size_t total = 0;
    for (;;) {
        SIZE_T bytesRead = 0;
        if (!ReadProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(addr + total), buf.data(), buf.size(),
                               &bytesRead) ||
            bytesRead == 0) {
            break;
        }
        for (SIZE_T i = 0; i < bytesRead && total < maxLen; ++i, ++total) {
            char c = buf[i];
            if (c == '\0') {
                return result;
            }
            result.push_back(c);
        }
        if (total >= maxLen) {
            result.append("…");
            break;
        }
        if (bytesRead < buf.size()) {
            break;
        }
    }
    return result;
}

// Reconstruct a string by walking fmt and reading args from the custom stack layout.
static std::string formatCustomArgs(const char* fmt, int argsBase) {
    if (!fmt)
        return "";

    uintptr_t i32 = static_cast<uintptr_t>(static_cast<uint32_t>(argsBase)) - 4;
    uintptr_t base = static_cast<uintptr_t>(static_cast<uint32_t>(argsBase)) - 8;

    std::string out;
    for (const char* p = fmt; *p; ++p) {
        if (*p != '%') {
            out.push_back(*p);
            continue;
        }
        ++p;
        if (!*p)
            break;

        switch (*p) {
            case 'd': {
                int v = 0;
                if (readVal(i32 + 4, v)) {
                    out += std::to_string(v);
                } else {
                    out += "<bad-int>";
                }
                i32 += 4;
                base += 4;
                break;
            }
            case 'u': {
                uint32_t v = 0;
                if (readVal(i32 + 4, v)) {
                    out += std::to_string(v);
                } else {
                    out += "<bad-uint>";
                }
                i32 += 4;
                base += 4;
                break;
            }
            case 'b': {
                int v = 0;
                if (readVal(i32 + 4, v)) {
                    out += (v ? "1" : "0");
                } else {
                    out += "<bad-bool>";
                }
                i32 += 4;
                base += 4;
                break;
            }
            case 'f': {
                double dv = 0.0;
                if (readVal(base + 8, dv)) {
                    char buf[64];
                    _snprintf_s(buf, _TRUNCATE, "%.6g", dv);
                    out += buf;
                } else {
                    out += "<bad-double>";
                }
                i32 += 8;
                base += 8;
                break;
            }
            case 's': {
                uint32_t pStr = 0;
                if (readVal(i32 + 4, pStr)) {
                    out += readCStringBounded(static_cast<uintptr_t>(pStr));
                } else {
                    out += "<bad-str*>";
                }
                i32 += 4;
                base += 4;
                break;
            }
            case '%': {
                out.push_back('%');
                break;
            }
            default: {
                out.push_back('%');
                out.push_back(*p);
                break;
            }
        }
    }
    return out;
}

// Hooked FrameScript__SignalEvent function
void __cdecl HookedFrameScriptSignalEvent(int eventid, const char* fmt, int argsBase) {
    // Safely reconstruct the formatted text (currently unused)
    (void)formatCustomArgs(fmt, argsBase);

    // Forward to original with the untouched parameters
    if (g_OriginalFrameScriptSignalEvent) {
        g_OriginalFrameScriptSignalEvent(eventid, fmt, argsBase);
    }
}

} // namespace icecap::agent::hooks

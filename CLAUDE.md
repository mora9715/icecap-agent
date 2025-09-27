# Icecap Agent - Claude Code Configuration

## Project Overview

This is the **Icecap Agent**, a 32-bit injectable DLL for World of Warcraft client automation on local servers. It's part of the larger Icecap ecosystem and provides in-process automation capabilities through TCP communication and Protocol Buffers.

**⚠️ Important**: This project is intended for local server environments and educational/research purposes only. It should not be used for cheating on live servers.

## Architecture

- **Language**: C++20
- **Platform**: Windows 32-bit
- **Build System**: CMake
- **Key Dependencies**: MinHook, Protocol Buffers, Winsock2, Direct3D9

## Build Commands

```bash
# Configure and build
# From command line:
cmake -B build -A Win32
cmake --build build --target injector
```

## Testing

No automated tests are currently configured. Testing requires:
1. Building `injector.dll`
2. Injecting into target WoW client process
3. Testing TCP communication on port 5050

## Project Structure

```
├── library.cpp           # DLL entry point and lifecycle
├── networking.{h,cpp}     # TCP server and protobuf messaging
├── hooks/                 # Function hooking implementation
│   ├── hooks.cpp         # Hook installation
│   ├── end_scene.cpp     # D3D9 EndScene hook
│   └── frame_script.cpp  # FrameScript event hook
├── cmake/                # Build configuration
└── frontend/             # Web interface components
```

## Key Features

- **Function Hooking**: MinHook-based hooks for D3D9 EndScene and FrameScript events
- **TCP Server**: Embedded server (port 5050) for command/event communication
- **Protocol Buffers**: Structured messaging with icecap-contracts schema
- **Lua Execution**: Execute arbitrary Lua code in WoW client
- **ClickToMove**: Automated movement commands
- **Self-Unload**: Clean unload via Delete key

## Dependencies

All dependencies are fetched automatically via CMake FetchContent:
- **MinHook v1.3.4**: Function hooking library
- **Protocol Buffers v3.21.12**: Message serialization
- **icecap-contracts**: Protobuf message definitions

## Important Notes

- **32-bit Only**: Must be compiled and used with 32-bit WoW clients
- **Version Specific**: Function addresses are hardcoded for specific client builds
- **Local Use Only**: No authentication/encryption - use in trusted environments only
- **Research Purpose**: Intended for automation research on private servers

## Common Tasks

- Build: Use `cmake --build build --target injector`
- Deploy: Inject resulting `injector.dll` into target process
- Update Contracts: Dependencies auto-fetch latest from icecap-contracts repo
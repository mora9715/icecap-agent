# Icecap Agent - Claude Code Configuration

## Project Overview

This is the **Icecap Agent**, a 32-bit injectable DLL for World of Warcraft client automation on local servers. It's part of the larger Icecap ecosystem and provides in-process automation capabilities through TCP communication and Protocol Buffers.

**⚠️ Important**: This project is intended for local server environments and educational/research purposes only. It should not be used for cheating on live servers.

## Architecture

- **Language**: C++20 with modern design patterns
- **Platform**: Windows 32-bit
- **Build System**: CMake with modular configuration
- **Key Dependencies**: MinHook, Protocol Buffers, spdlog (logging)
- **System Dependencies**: Winsock2, Direct3D9

### Design Principles

- **Layered Architecture**: Interface → Transport → Core → Hooks
- **Dependency Injection**: Constructor injection throughout, no global state
- **RAII**: Automatic resource management for all system resources
- **Thread Safety**: `std::atomic` and proper synchronization
- **Interface Segregation**: Clear abstractions for all major components

## Build Commands

```bash
# Configure and build
# From command line:
cmake -B build -A Win32
cmake --build build --target injector
```

## Testing

### Current Testing
Manual testing is performed by:
1. Building `injector.dll`
2. Injecting into target WoW client process
3. Testing TCP communication on port 5050
4. Monitoring logs at `%TEMP%\icecap-agent\icecap-agent.log`

### Planned Testing Infrastructure
- **Unit Tests**: Google Test framework integration
- **Mock Interfaces**: Mock implementations for external dependencies
- **Integration Tests**: End-to-end hook and network functionality testing
- **Static Analysis**: Enhanced clang-tidy and cppcheck integration

## Project Structure

```
├── src/                           # Implementation files
│   ├── main.cpp                   # DLL entry point and lifecycle
│   ├── application_context.cpp    # Core application context
│   ├── shared_state.cpp          # Shared state management
│   ├── logging.cpp               # Logging system implementation
│   ├── core/                     # Business logic layer
│   │   ├── CommandExecutor.cpp   # Command execution logic
│   │   ├── EventPublisher.cpp    # Event publishing system
│   │   └── MessageProcessor.cpp  # Message processing coordination
│   ├── hooks/                    # Hook system implementation
│   │   ├── BaseHook.cpp          # Base hook class
│   │   ├── D3D9Hook.cpp          # D3D9 EndScene hook
│   │   ├── framescript_hooks.cpp # FrameScript event hooks
│   │   ├── HookRegistry.cpp      # Hook registration and management
│   │   └── hook_manager.cpp      # Hook lifecycle management
│   └── transport/                # Network transport layer
│       ├── NetworkManager.cpp    # Network orchestration
│       ├── ProtocolHandler.cpp   # Protocol Buffer handling
│       └── TcpServer.cpp         # TCP server implementation
├── include/icecap/agent/         # Header files with clear layering
│   ├── interfaces/               # Abstract interfaces
│   ├── core/                     # Business logic headers
│   ├── hooks/                    # Hook system headers
│   ├── transport/                # Network transport headers
│   └── *.hpp                     # Core system headers
├── cmake/                        # Build configuration
│   ├── Dependencies.cmake       # Dependency management
│   ├── Options.cmake           # Build options
│   ├── ProtobufGen.cmake       # Protocol Buffer generation
│   ├── StaticAnalysis.cmake    # Code analysis tools
│   └── Targets.cmake           # Build targets
└── .clang-format/.clang-tidy    # Code quality tools
```

## Key Features

### Core Systems
- **Production-Grade Architecture**: Layered design with proper separation of concerns
- **Thread-Safe Design**: Atomic operations and synchronization throughout
- **Structured Logging**: spdlog-based logging with rotating files (`%TEMP%\icecap-agent\`)
- **Resource Management**: RAII wrappers for all D3D9 and system resources

### Functional Features
- **Function Hooking**: Class-based MinHook system for D3D9 EndScene and FrameScript
- **TCP Server**: Robust embedded server (port 5050) with connection management
- **Protocol Buffers**: Structured messaging with icecap-contracts schema
- **Command Execution**: Lua code execution and ClickToMove automation
- **Self-Unload**: Reliable unload via Delete key with edge detection

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

### Build & Development
- **Build**: `cmake --build build --target injector`
- **Static Analysis**: `cmake --build build --target clang-tidy` (if configured)
- **Format Code**: Uses `.clang-format` configuration

### Deployment & Debugging
- **Deploy**: Inject resulting `injector.dll` into target process
- **Monitor**: Check logs at `%TEMP%\icecap-agent\icecap-agent.log`
- **Debug**: Use structured logging levels (trace, debug, info, warn, error)

### Maintenance
- **Update Contracts**: Dependencies auto-fetch latest from icecap-contracts repo
- **Code Quality**: Configured with clang-tidy rules in `.clang-tidy`
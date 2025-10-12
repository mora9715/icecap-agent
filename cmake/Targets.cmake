# Project targets and linking

# Main shared library target
add_library(icecap-agent SHARED
    # Source files
    src/main.cpp
    src/application_context.cpp
    src/shared_state.cpp
    src/logging.cpp
    src/hooks/hooks.cpp
    src/hooks/frame_script.cpp

    # Transport layer
    src/transport/TcpServer.cpp
    src/transport/ProtocolHandler.cpp
    src/transport/NetworkManager.cpp

    # Core business logic
    src/core/MessageProcessor.cpp
    src/core/CommandExecutor.cpp
    src/core/EventPublisher.cpp

    # Hook implementations
    src/hooks/BaseHook.cpp
    src/hooks/D3D9Hook.cpp
    src/hooks/HookRegistry.cpp

    # Public headers - Interfaces
    include/icecap/agent/interfaces/IApplicationContext.hpp
    include/icecap/agent/interfaces/IMessageHandler.hpp
    include/icecap/agent/interfaces/INetworkProtocol.hpp
    include/icecap/agent/interfaces/IHookRegistry.hpp

    # Public headers - Transport
    include/icecap/agent/transport/TcpServer.hpp
    include/icecap/agent/transport/ProtocolHandler.hpp
    include/icecap/agent/transport/NetworkManager.hpp

    # Public headers - Core
    include/icecap/agent/core/MessageProcessor.hpp
    include/icecap/agent/core/CommandExecutor.hpp
    include/icecap/agent/core/EventPublisher.hpp

    # Public headers - Hooks
    include/icecap/agent/hooks/BaseHook.hpp
    include/icecap/agent/hooks/D3D9Hook.hpp
    include/icecap/agent/hooks/HookRegistry.hpp

    # Public headers - Legacy/General
    include/icecap/agent/application_context.hpp
    include/icecap/agent/raii_wrappers.hpp
    include/icecap/agent/logging.hpp
    include/icecap/agent/shared_state.hpp
    include/icecap/agent/hooks/hook_manager.hpp
    include/icecap/agent/hooks/framescript_hooks.hpp
)

# Add generated protobuf sources if any
if(GEN_SRCS)
    target_sources(icecap-agent PRIVATE ${GEN_SRCS})
endif()

# Modern CMake target properties
set_target_properties(icecap-agent PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    COMPILE_OPTIONS "-m32"
    LINK_FLAGS "-m32 -static-libgcc -static-libstdc++"
)


target_compile_definitions(icecap-agent PRIVATE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    _WINSOCK_DEPRECATED_NO_WARNINGS
)

# Include directories
target_include_directories(icecap-agent
    PUBLIC include
    PRIVATE src
    PRIVATE ${GENERATED_DIR}
)

# Link dependencies
target_link_libraries(icecap-agent PRIVATE
    minhook
    ws2_32
    d3d9
    protobuf::libprotobuf
    spdlog::spdlog
)

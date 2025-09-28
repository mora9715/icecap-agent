# Project targets and linking

# Main shared library target
add_library(icecap-agent SHARED
    # Source files
    src/main.cpp
    src/networking.cpp
    src/application_context.cpp
    src/shared_state.cpp
    src/logging.cpp
    src/hooks/hooks.cpp
    src/hooks/end_scene.cpp
    src/hooks/frame_script.cpp

    # Public headers
    include/icecap/agent/networking.hpp
    include/icecap/agent/application_context.hpp
    include/icecap/agent/raii_wrappers.hpp
    include/icecap/agent/logging.hpp
    include/icecap/agent/hooks/hook_manager.hpp
    include/icecap/agent/hooks/d3d9_hooks.hpp
    include/icecap/agent/hooks/framescript_hooks.hpp

    # Private headers
    src/shared_state.hpp
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

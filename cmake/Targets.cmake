# Project targets and linking

# Main shared library target
add_library(injector SHARED
    library.cpp
    networking.cpp
    networking.h
    hooks/hooks.cpp
    hooks/end_scene.cpp
    hooks/frame_script.cpp
    hooks/hooks.h
    hooks/end_scene.h
    hooks/frame_script.h
    hooks/state.h
)

# Add generated protobuf sources if any
if(GEN_SRCS)
    target_sources(injector PRIVATE ${GEN_SRCS})
endif()

# Preserve compile and link flags
set_target_properties(injector PROPERTIES
    COMPILE_OPTIONS "-m32"
    LINK_FLAGS "-m32 -static-libgcc -static-libstdc++"
)

# Include generated headers
target_include_directories(injector PRIVATE ${GENERATED_DIR})

# Link dependencies
target_link_libraries(injector PRIVATE
    minhook
    ws2_32
    d3d9
    protobuf::libprotobuf
)

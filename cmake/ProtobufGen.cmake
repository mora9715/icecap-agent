# Protobuf source generation from icecap-contracts

# Output directory for generated files
set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GENERATED_DIR})

# Collect all .proto files recursively from the fetched repository
file(GLOB_RECURSE ICECAP_PROTO_FILES "${icecap_contracts_SOURCE_DIR}/*.proto")
if(NOT ICECAP_PROTO_FILES)
    message(FATAL_ERROR "No .proto files found in ${icecap_contracts_SOURCE_DIR}")
endif()

# Derive expected output files to make CMake aware of them
set(GEN_SRCS)
set(GEN_HDRS)
foreach(PF IN LISTS ICECAP_PROTO_FILES)
    file(RELATIVE_PATH REL "${icecap_contracts_SOURCE_DIR}" "${PF}")
    get_filename_component(DIR "${REL}" DIRECTORY)
    get_filename_component(NAME_WE "${REL}" NAME_WE)
    set(OUT_CC "${GENERATED_DIR}/${DIR}/${NAME_WE}.pb.cc")
    set(OUT_H  "${GENERATED_DIR}/${DIR}/${NAME_WE}.pb.h")
    list(APPEND GEN_SRCS "${OUT_CC}")
    list(APPEND GEN_HDRS "${OUT_H}")
endforeach()

# Run protoc to generate all sources, preserving directory structure relative to repo root
add_custom_command(
    OUTPUT ${GEN_SRCS} ${GEN_HDRS}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${GENERATED_DIR}
    COMMAND $<TARGET_FILE:protobuf::protoc>
            --cpp_out=${GENERATED_DIR}
            -I ${icecap_contracts_SOURCE_DIR}
            ${ICECAP_PROTO_FILES}
    DEPENDS ${ICECAP_PROTO_FILES} protobuf::protoc
    COMMENT "Generating C++ protocol buffers from icecap-contracts"
    VERBATIM
)

add_custom_target(proto_gen DEPENDS ${GEN_SRCS} ${GEN_HDRS})

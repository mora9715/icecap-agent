# Static analysis tools configuration

# Find clang-tidy
find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
if(CLANG_TIDY_EXE)
    message(STATUS "Found clang-tidy: ${CLANG_TIDY_EXE}")

    # Get all source files for the target
    get_target_property(ICECAP_SOURCES icecap-agent SOURCES)

    # Filter for C++ source files only
    set(ICECAP_CPP_SOURCES "")
    foreach(SOURCE ${ICECAP_SOURCES})
        if(SOURCE MATCHES "\\.(cpp|cxx|cc|c\\+\\+)$")
            list(APPEND ICECAP_CPP_SOURCES ${SOURCE})
        endif()
    endforeach()

    # Create clang-tidy target
    add_custom_target(clang-tidy
        COMMAND ${CLANG_TIDY_EXE}
            -p ${CMAKE_BINARY_DIR}
            --config-file=${CMAKE_SOURCE_DIR}/.clang-tidy
            ${ICECAP_CPP_SOURCES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running clang-tidy static analysis"
        VERBATIM
    )

    # Create clang-tidy-fix target for automatic fixes
    add_custom_target(clang-tidy-fix
        COMMAND ${CLANG_TIDY_EXE}
            -p ${CMAKE_BINARY_DIR}
            --config-file=${CMAKE_SOURCE_DIR}/.clang-tidy
            --fix
            --fix-errors
            ${ICECAP_CPP_SOURCES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running clang-tidy with automatic fixes"
        VERBATIM
    )

    # Make clang-tidy depend on compile_commands.json
    add_dependencies(clang-tidy icecap-agent)
    add_dependencies(clang-tidy-fix icecap-agent)
else()
    message(WARNING "clang-tidy not found. Install LLVM/Clang to enable static analysis.")
endif()

# Find clang-format
find_program(CLANG_FORMAT_EXE NAMES "clang-format")
if(CLANG_FORMAT_EXE)
    message(STATUS "Found clang-format: ${CLANG_FORMAT_EXE}")

    # Get all source and header files
    file(GLOB_RECURSE ALL_SOURCE_FILES
        "${CMAKE_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/*.hpp"
        "${CMAKE_SOURCE_DIR}/include/*.hpp"
        "${CMAKE_SOURCE_DIR}/include/*.h"
    )

    # Create clang-format target for checking
    add_custom_target(clang-format-check
        COMMAND ${CLANG_FORMAT_EXE}
            --dry-run
            --Werror
            --style=file
            ${ALL_SOURCE_FILES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Checking code formatting with clang-format"
        VERBATIM
    )

    # Create clang-format target for fixing
    add_custom_target(clang-format
        COMMAND ${CLANG_FORMAT_EXE}
            -i
            --style=file
            ${ALL_SOURCE_FILES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Formatting code with clang-format"
        VERBATIM
    )
else()
    message(WARNING "clang-format not found. Install LLVM/Clang to enable code formatting.")
endif()

# Create combined static analysis target
add_custom_target(static-analysis
    COMMENT "Running all static analysis tools"
)

if(CLANG_TIDY_EXE)
    add_dependencies(static-analysis clang-tidy)
endif()

if(CLANG_FORMAT_EXE)
    add_dependencies(static-analysis clang-format-check)
endif()
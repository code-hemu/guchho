# Source files

set(GUCHHO_SOURCES
    # helpers
    helpers/utf8.cpp
    core/logger/message.cpp
)

# Library

add_library(guchho_lib STATIC)

target_sources(guchho_lib
    PRIVATE
        ${GUCHHO_SOURCES}
)

target_include_directories(guchho_lib
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_compile_features(guchho_lib
    PUBLIC
        cxx_std_20
)


# Compiler options

guchho_set_warnings(guchho_lib)
guchho_enable_sanitizers(guchho_lib)

if(WIN32)
    target_link_libraries(guchho_lib PRIVATE dbghelp)
endif()


# Executable

add_executable(guchho)

target_sources(guchho
    PRIVATE
        main.cpp
)

guchho_enable_resources(guchho)

target_link_libraries(guchho
    PRIVATE
        guchho_lib
)

guchho_set_warnings(guchho)
guchho_enable_sanitizers(guchho)


# Install

install(
    TARGETS
        guchho
        guchho_lib

    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

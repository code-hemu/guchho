# Windows resources

function(guchho_enable_resources TARGET)

    if(NOT WIN32)
        return()
    endif()

    set(GUCHHO_RESOURCE
        "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}.rc"
    )

    configure_file(
        "${PROJECT_SOURCE_DIR}/resources/windows.rc.in"
        "${GUCHHO_RESOURCE}"
        @ONLY
    )

    target_sources(
        ${TARGET}
        PRIVATE
            "${GUCHHO_RESOURCE}"
    )

endfunction()
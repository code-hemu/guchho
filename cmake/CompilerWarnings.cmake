function(guchho_set_warnings TARGET)
    if(GUCHHO_ENABLE_STRICT_WARNINGS)
        if(MSVC)
            target_compile_options(${TARGET} PRIVATE
                /W4
                /permissive-
                /w14242
                /w14254
                /w14263
                /w14287
                /w14296
                /w14311
            )
        elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
            target_compile_options(${TARGET} PRIVATE
                -Wall
                -Wextra
                -Wpedantic
                -Wshadow
                -Wnon-virtual-dtor
                -Wold-style-cast
                -Wcast-align
                -Wunused
                -Woverloaded-virtual
                -Wconversion
                -Wsign-conversion
                -Wnull-dereference
                -Wdouble-promotion
                -Wformat=2
            )
        endif()
    endif()
endfunction()
# CLI test sources.

add_library(guchho_cli_tests OBJECT
    cli_test.cpp
    ${PROJECT_SOURCE_DIR}/src/cli/cli.cpp
)

target_include_directories(
    guchho_cli_tests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/src
)

target_compile_definitions(
    guchho_cli_tests
    PRIVATE
        GUCHHO_VERSION_STRING="${PROJECT_VERSION}"
)

target_link_libraries(
    guchho_cli_tests
    PRIVATE
        GTest::gtest
)

guchho_set_warnings(guchho_cli_tests)
guchho_enable_sanitizers(guchho_cli_tests)

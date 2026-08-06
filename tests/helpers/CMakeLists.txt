# Helpers test sources.

add_library(guchho_helpers_tests OBJECT
    utf8_test.cpp
)

target_include_directories(
    guchho_helpers_tests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(
    guchho_helpers_tests
    PRIVATE
        GTest::gtest
)

guchho_set_warnings(guchho_helpers_tests)
guchho_enable_sanitizers(guchho_helpers_tests)

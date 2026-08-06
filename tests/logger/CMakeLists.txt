# Logger test sources.

add_library(guchho_logger_tests OBJECT
    message_test.cpp
    logger_test.cpp
)

target_include_directories(
    guchho_logger_tests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(
    guchho_logger_tests
    PRIVATE
        GTest::gtest
)

guchho_set_warnings(guchho_logger_tests)
guchho_enable_sanitizers(guchho_logger_tests)

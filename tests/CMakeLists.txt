include(FetchContent)
set(CMAKE_POLICY_DEFAULT_CMP0135 NEW)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)

# CLI tests
add_executable(cli_test
    cpp/cli/cli_test.cpp
    cpp/cli/command_test.cpp
)
target_link_libraries(cli_test PRIVATE gtest_main guchho_lib)
add_test(NAME cli_test COMMAND cli_test)

# Parser tests
add_executable(html_parser_test cpp/parser/html/parser_test.cpp)
target_link_libraries(html_parser_test PRIVATE gtest_main guchho_lib)
add_test(NAME html_parser_test COMMAND html_parser_test)

add_executable(js_parser_test cpp/parser/js/parser_test.cpp)
target_link_libraries(js_parser_test PRIVATE gtest_main guchho_lib)
add_test(NAME js_parser_test COMMAND js_parser_test)

add_executable(css_parser_test cpp/parser/css/parser_test.cpp)
target_link_libraries(css_parser_test PRIVATE gtest_main guchho_lib)
add_test(NAME css_parser_test COMMAND css_parser_test)

# Config tests
add_executable(config_test cpp/config/config_test.cpp)
target_link_libraries(config_test PRIVATE gtest_main guchho_lib)
add_test(NAME config_test COMMAND config_test)

# Resolver tests
add_executable(resolver_test cpp/resolver/resolver_test.cpp)
target_link_libraries(resolver_test PRIVATE gtest_main guchho_lib)
add_test(NAME resolver_test COMMAND resolver_test)

# Graph tests
add_executable(graph_test cpp/graph/graph_test.cpp)
target_link_libraries(graph_test PRIVATE gtest_main guchho_lib)
add_test(NAME graph_test COMMAND graph_test)

# Transform tests
add_executable(transform_test cpp/transform/transform_test.cpp)
target_link_libraries(transform_test PRIVATE gtest_main guchho_lib)
add_test(NAME transform_test COMMAND transform_test)

# Optimizer tests
add_executable(optimizer_test cpp/optimizer/optimizer_test.cpp)
target_link_libraries(optimizer_test PRIVATE gtest_main guchho_lib)
add_test(NAME optimizer_test COMMAND optimizer_test)

# Bundle tests
add_executable(bundle_test cpp/bundle/bundle_test.cpp)
target_link_libraries(bundle_test PRIVATE gtest_main guchho_lib)
add_test(NAME bundle_test COMMAND bundle_test)

# Utils tests
add_executable(utils_test cpp/utils/utils_test.cpp)
target_link_libraries(utils_test PRIVATE gtest_main guchho_lib)
add_test(NAME utils_test COMMAND utils_test)

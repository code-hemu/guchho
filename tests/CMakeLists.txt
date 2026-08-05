# GoogleTest

include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

include(GoogleTest)


# Test executable

add_executable(guchho_tests
    main.cpp
    helpers/utf8_test.cpp
)

target_link_libraries(
    guchho_tests
    PRIVATE
        guchho_lib
        GTest::gtest
)

if(WIN32)
    target_link_libraries(guchho_tests PRIVATE dbghelp)
endif()

target_include_directories(
    guchho_tests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

guchho_set_warnings(guchho_tests)
guchho_enable_sanitizers(guchho_tests)

gtest_discover_tests(guchho_tests)


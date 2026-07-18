set(LIB_SOURCES
    api_helpers/api_helpers.cpp
    bundle/bundle_writer.cpp
    bundle/emitter.cpp
    bundler/bundler.cpp
    cache/cache.cpp
    cli/cli.cpp
    cli/command.cpp
    cli_helpers/cli_helpers.cpp
    compat/compat.cpp
    config/loader.cpp
    config/validator.cpp
    context.cpp
    css_lexer/css_lexer.cpp
    css_parser/css_parser.cpp
    css_printer/css_printer.cpp
    dev/dev_server.cpp
    dev/watcher.cpp
    entry/entry_manager.cpp
    entry/html_entry.cpp
    entry/js_entry.cpp
    entry/ts_entry.cpp
    fs/fs.cpp
    graph/chunk.cpp
    graph/dependency_graph.cpp
    graph/module.cpp
    helpers/helpers.cpp
    html/html_ast.cpp
    html/html_lexer.cpp
    html/html_parser.cpp
    js_lexer/js_lexer.cpp
    js_parser/js_parser.cpp
    js_printer/js_printer.cpp
    linker/linker.cpp
    logger/logger.cpp
    optimizer/minifier.cpp
    optimizer/optimizer.cpp
    optimizer/tree_shaking.cpp
    plugin/plugin.cpp
    plugin/plugin_manager.cpp
    renamer/renamer.cpp
    resolver/filesystem.cpp
    resolver/module_resolver.cpp
    runtime/runtime.cpp
    sourcemap/sourcemap.cpp
    transform/javascript.cpp
    transform/transformer.cpp
    xxhash/xxhash.cpp
)

add_library(guchho_lib STATIC ${LIB_SOURCES})
target_include_directories(guchho_lib PUBLIC ../include)

add_executable(guchho main.cpp)
target_link_libraries(guchho PRIVATE guchho_lib)

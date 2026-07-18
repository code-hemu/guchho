set(LIB_SOURCES
    cli/cli.cpp
    cli/command.cpp
    config/loader.cpp
    config/validator.cpp
    context.cpp
    dev/dev_server.cpp
    dev/watcher.cpp
    entry/entry_manager.cpp
    entry/html_entry.cpp
    entry/js_entry.cpp
    entry/ts_entry.cpp
    graph/chunk.cpp
    graph/dependency_graph.cpp
    graph/module.cpp
    optimizer/minifier.cpp
    optimizer/optimizer.cpp
    optimizer/tree_shaking.cpp
    parser/html/ast.cpp
    parser/html/parser.cpp
    parser/html/tokenizer.cpp
    parser/css/parser.cpp
    parser/css/tokenizer.cpp
    parser/js/parser.cpp
    parser/js/tokenizer.cpp
    plugin/plugin.cpp
    plugin/plugin_manager.cpp
    resolver/filesystem.cpp
    resolver/module_resolver.cpp
    bundle/bundle_writer.cpp
    bundle/emitter.cpp
    bundle/source_map.cpp
    transform/javascript.cpp
    transform/transformer.cpp
    utils/filesystem.cpp
    utils/logger.cpp
)

add_library(guchho_lib STATIC ${LIB_SOURCES})
target_include_directories(guchho_lib PUBLIC ../include)

add_executable(guchho main.cpp)
target_link_libraries(guchho PRIVATE guchho_lib)

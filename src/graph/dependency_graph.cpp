#include "graph/dependency_graph.hpp"
#include "logger.hpp"
#include <algorithm>
#include <queue>

namespace guchho {

void DependencyGraph::add_node(const std::filesystem::path &path, std::string source) {
    auto key = path.lexically_normal().string();
    if (!nodes_.count(key)) {
        Node node;
        node.path = path.lexically_normal();
        node.source = std::move(source);
        nodes_[key] = std::move(node);
    }
}

Node* DependencyGraph::get_node(const std::filesystem::path &path) {
    auto it = nodes_.find(path.lexically_normal().string());
    return it != nodes_.end() ? &it->second : nullptr;
}

const Node* DependencyGraph::get_node(const std::filesystem::path &path) const {
    auto it = nodes_.find(path.lexically_normal().string());
    return it != nodes_.end() ? &it->second : nullptr;
}

bool DependencyGraph::has_node(const std::filesystem::path &path) const {
    return nodes_.count(path.lexically_normal().string()) > 0;
}

void DependencyGraph::set_entry(const std::filesystem::path &path) {
    auto key = path.lexically_normal().string();
    if (nodes_.count(key)) {
        nodes_[key].is_entry = true;
    }
}

std::vector<Node*> DependencyGraph::get_entry_nodes() {
    std::vector<Node*> entries;
    for (auto &[key, node] : nodes_) {
        if (node.is_entry) {
            entries.push_back(&node);
        }
    }
    return entries;
}

const std::vector<Node*> DependencyGraph::get_entry_nodes() const {
    std::vector<Node*> entries;
    for (auto &[key, node] : nodes_) {
        if (node.is_entry) {
            entries.push_back(const_cast<Node*>(&node));
        }
    }
    return entries;
}

std::vector<std::filesystem::path> DependencyGraph::resolve_order() const {
    std::vector<std::filesystem::path> result;
    std::unordered_set<std::string> visited;

    for (const auto &[key, node] : nodes_) {
        if (node.is_entry) {
            topo_sort_impl(key, visited, result);
        }
    }

    return result;
}

void DependencyGraph::topo_sort_impl(const std::string &key,
                                     std::unordered_set<std::string> &visited,
                                     std::vector<std::filesystem::path> &out) const {
    if (visited.count(key)) return;
    visited.insert(key);

    auto it = nodes_.find(key);
    if (it == nodes_.end()) return;

    for (const auto &dep : it->second.resolved_imports) {
        topo_sort_impl(dep.second, visited, out);
    }

    out.push_back(it->second.path);
}

}

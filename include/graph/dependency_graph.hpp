#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace guchho {

struct Node {
    std::filesystem::path path;
    std::string source;
    std::vector<std::string> imports;
    std::unordered_map<std::string, std::string> resolved_imports;
    bool is_entry = false;
};

class DependencyGraph {
public:
    void add_node(const std::filesystem::path &path, std::string source);
    Node* get_node(const std::filesystem::path &path);
    const Node* get_node(const std::filesystem::path &path) const;
    bool has_node(const std::filesystem::path &path) const;
    void set_entry(const std::filesystem::path &path);
    std::vector<Node*> get_entry_nodes();
    const std::vector<Node*> get_entry_nodes() const;
    std::vector<std::filesystem::path> resolve_order() const;

private:
    std::unordered_map<std::string, Node> nodes_;
    void topo_sort_impl(const std::string &key,
                        std::unordered_set<std::string> &visited,
                        std::vector<std::filesystem::path> &out) const;
};

}

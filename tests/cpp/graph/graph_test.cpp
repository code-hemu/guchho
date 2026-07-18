#include <gtest/gtest.h>
#include "graph/dependency_graph.hpp"

TEST(DependencyGraphTest, EmptyGraph) {
    guchho::DependencyGraph g;
    EXPECT_FALSE(g.has_node("nonexistent"));
    EXPECT_TRUE(g.get_entry_nodes().empty());
    EXPECT_TRUE(g.resolve_order().empty());
}

TEST(DependencyGraphTest, AddNode) {
    guchho::DependencyGraph g;
    g.add_node("test.js", "content");
    EXPECT_TRUE(g.has_node("test.js"));
}

TEST(DependencyGraphTest, GetNode) {
    guchho::DependencyGraph g;
    g.add_node("test.js", "content");
    auto *n = g.get_node("test.js");
    ASSERT_NE(n, nullptr);
    EXPECT_EQ(n->source, "content");
}

TEST(DependencyGraphTest, SetEntry) {
    guchho::DependencyGraph g;
    g.add_node("entry.js", "entry");
    g.set_entry("entry.js");
    auto entries = g.get_entry_nodes();
    ASSERT_EQ(entries.size(), 1u);
}

TEST(DependencyGraphTest, ResolveOrderSingleNode) {
    guchho::DependencyGraph g;
    g.add_node("a.js", "a");
    g.set_entry("a.js");
    auto order = g.resolve_order();
    ASSERT_EQ(order.size(), 1u);
    EXPECT_EQ(order[0].string(), "a.js");
}

TEST(DependencyGraphTest, NodeNotAddedReturnsNull) {
    guchho::DependencyGraph g;
    EXPECT_EQ(g.get_node("missing"), nullptr);
    EXPECT_EQ(std::as_const(g).get_node("missing"), nullptr);
}

TEST(DependencyGraphTest, ConstGetNode) {
    guchho::DependencyGraph g;
    g.add_node("test.js", "content");
    const auto &cg = g;
    EXPECT_NE(cg.get_node("test.js"), nullptr);
}

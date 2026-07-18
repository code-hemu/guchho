#include <gtest/gtest.h>
#include "transform/transformer.hpp"

TEST(TransformerTest, StripCommentsRemovesSingleLineComments) {
    std::string input = "a//comment\nb";
    std::string result = guchho::strip_comments(input);
    EXPECT_EQ(result, "a\nb");
}

TEST(TransformerTest, StripCommentsRemovesMultiLineComments) {
    std::string input = "a/*comment*/b";
    std::string result = guchho::strip_comments(input);
    EXPECT_EQ(result, "ab");
}

TEST(TransformerTest, StripCommentsHandlesNoComments) {
    std::string input = "hello";
    std::string result = guchho::strip_comments(input);
    EXPECT_EQ(result, "hello");
}

TEST(TransformerTest, AddPassAndRun) {
    guchho::Transformer t;
    t.add_pass([](const std::string &s) { return s + "!"; });
    EXPECT_EQ(t.run("hello"), "hello!");
}

TEST(TransformerTest, MultiplePasses) {
    guchho::Transformer t;
    t.add_pass([](const std::string &s) { return s + "!"; });
    t.add_pass([](const std::string &s) { return "(" + s + ")"; });
    EXPECT_EQ(t.run("hello"), "(hello!)");
}

#include <gtest/gtest.h>
#include "optimizer/minifier.hpp"

TEST(MinifierTest, MinifyCssRemovesComments) {
    std::string input = "a { color: red; /* comment */ }";
    std::string result = guchho::minify_css(input);
    EXPECT_EQ(result.find("/*"), std::string::npos);
}

TEST(MinifierTest, MinifyCssRemovesWhitespace) {
    std::string input = "a  {  color :  red;  }";
    std::string result = guchho::minify_css(input);
    EXPECT_LT(result.size(), input.size());
}

TEST(MinifierTest, MinifyCssPreservesContent) {
    std::string input = "a{color:red}";
    EXPECT_EQ(guchho::minify_css(input), input);
}

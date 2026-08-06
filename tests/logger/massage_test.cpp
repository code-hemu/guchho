#include <gtest/gtest.h>
#include "guchho/logger.hpp"

using guchho::logger::LogLevel;
using guchho::logger::MsgID;
using guchho::logger::StringToMsgIDs;
using guchho::logger::MsgIDToString;
using guchho::logger::StringToMaximumMsgID;

TEST(MessagesTest, MsgIDToStringKnown) {
    EXPECT_EQ(MsgIDToString(MsgID::kJS_EqualsNaN), "equals-nan");
    EXPECT_EQ(MsgIDToString(MsgID::kJS_DirectEval), "direct-eval");
    EXPECT_EQ(MsgIDToString(MsgID::kCSS_CSSSyntaxError), "css-syntax-error");
    EXPECT_EQ(MsgIDToString(MsgID::kBundler_EmptyGlob), "empty-glob");
    EXPECT_EQ(MsgIDToString(MsgID::kSourceMap_InvalidSourceMappings), "invalid-source-mappings");
}

TEST(MessagesTest, MsgIDToStringPackageJSON) {
    EXPECT_EQ(MsgIDToString(MsgID::kPackageJSON_DeadCondition), "package.json");
    EXPECT_EQ(MsgIDToString(MsgID::kPackageJSON_InvalidBrowser), "package.json");
}

TEST(MessagesTest, MsgIDToStringTSConfigJSON) {
    EXPECT_EQ(MsgIDToString(MsgID::kTSConfigJSON_Cycle), "tsconfig.json");
    EXPECT_EQ(MsgIDToString(MsgID::kTSConfigJSON_Missing), "tsconfig.json");
}

TEST(MessagesTest, MsgIDToStringNone) {
    EXPECT_EQ(MsgIDToString(MsgID::kNone), "");
}

TEST(MessagesTest, StringToMaximumMsgIDPackageJSON) {
    EXPECT_EQ(StringToMaximumMsgID("package.json"), MsgID::kPackageJSON_LAST);
}

TEST(MessagesTest, StringToMaximumMsgIDTSConfigJSON) {
    EXPECT_EQ(StringToMaximumMsgID("tsconfig.json"), MsgID::kTSConfigJSON_LAST);
}

TEST(MessagesTest, StringToMaximumMsgIDSingle) {
    EXPECT_EQ(StringToMaximumMsgID("equals-nan"), MsgID::kJS_EqualsNaN);
}

TEST(MessagesTest, StringToMsgIDsSingle) {
    std::unordered_map<MsgID, LogLevel> overrides;
    StringToMsgIDs("direct-eval", LogLevel::kWarning, overrides);
    EXPECT_EQ(overrides.size(), 1);
    EXPECT_EQ(overrides[MsgID::kJS_DirectEval], LogLevel::kWarning);
}

TEST(MessagesTest, StringToMsgIDsPackageJSONRange) {
    std::unordered_map<MsgID, LogLevel> overrides;
    StringToMsgIDs("package.json", LogLevel::kDebug, overrides);
    uint8_t first = static_cast<uint8_t>(MsgID::kPackageJSON_FIRST);
    uint8_t last = static_cast<uint8_t>(MsgID::kPackageJSON_LAST);
    EXPECT_EQ(overrides.size(), last - first + 1);
}

TEST(MessagesTest, StringToMsgIDsUnknown) {
    std::unordered_map<MsgID, LogLevel> overrides;
    StringToMsgIDs("nonexistent-message", LogLevel::kInfo, overrides);
    EXPECT_TRUE(overrides.empty());
}

TEST(MessagesTest, StringToMsgIDsOverwrite) {
    std::unordered_map<MsgID, LogLevel> overrides;
    StringToMsgIDs("equals-nan", LogLevel::kDebug, overrides);
    StringToMsgIDs("equals-nan", LogLevel::kError, overrides);
    EXPECT_EQ(overrides.size(), 1);
    EXPECT_EQ(overrides[MsgID::kJS_EqualsNaN], LogLevel::kError);
}

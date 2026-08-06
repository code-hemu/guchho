#include <gtest/gtest.h>
#include "guchho/logger.hpp"

#include <string>
#include <vector>
#include <unordered_map>

using guchho::logger::AllowOverride;
using guchho::logger::DeferLogKind;
using guchho::logger::EncodeImportAttributes;
using guchho::logger::EstimateWidthInTerminal;
using guchho::logger::ImportAttributes;
using guchho::logger::LineColumnTracker;
using guchho::logger::LinkifyText;
using guchho::logger::Loc;
using guchho::logger::Log;
using guchho::logger::LogLevel;
using guchho::logger::Msg;
using guchho::logger::MsgData;
using guchho::logger::MsgID;
using guchho::logger::MsgKind;
using guchho::logger::MsgLocation;
using guchho::logger::MsgString;
using guchho::logger::NewDeferLog;
using guchho::logger::NewStderrLog;
using guchho::logger::OutputOptions;
using guchho::logger::OutputOptionsForArgs;
using guchho::logger::Path;
using guchho::logger::PathFlags;
using guchho::logger::PathStyle;
using guchho::logger::PlatformIndependentPathDirBaseExt;
using guchho::logger::PrettyPaths;
using guchho::logger::Range;
using guchho::logger::RenderTabStops;
using guchho::logger::Source;
using guchho::logger::TerminalInfo;
using guchho::logger::UseColor;
using guchho::logger::WrapWordsInString;

// ---------------------------------------------------------------------------
// Range
// ---------------------------------------------------------------------------

TEST(RangeTest, End)
{
    Range r{Loc{5}, 3};
    EXPECT_EQ(r.End(), 8);
}

TEST(RangeTest, EndZeroLength)
{
    Range r{Loc{5}, 0};
    EXPECT_EQ(r.End(), 5);
}

TEST(RangeTest, ExpandByDisjoint)
{
    Range a{Loc{5}, 3};
    Range b{Loc{2}, 2};
    a.ExpandBy(b);
    EXPECT_EQ(a.loc.start, 2);
    EXPECT_EQ(a.len, 6);
    EXPECT_EQ(a.End(), 8);
}

TEST(RangeTest, ExpandByEmptyExtends)
{
    Range a{Loc{0}, 0};
    Range b{Loc{3}, 2};
    a.ExpandBy(b);
    EXPECT_EQ(a.loc.start, 3);
    EXPECT_EQ(a.len, 2);
}

TEST(RangeTest, ExpandByContained)
{
    Range a{Loc{5}, 3};
    Range b{Loc{6}, 1};
    a.ExpandBy(b);
    EXPECT_EQ(a.loc.start, 5);
    EXPECT_EQ(a.len, 3);
}

TEST(RangeTest, ExpandByExtendsEnd)
{
    Range a{Loc{5}, 3};
    Range b{Loc{2}, 5};
    a.ExpandBy(b);
    EXPECT_EQ(a.loc.start, 2);
    EXPECT_EQ(a.len, 6);
    EXPECT_EQ(a.End(), 8);
}

// ---------------------------------------------------------------------------
// Path
// ---------------------------------------------------------------------------

TEST(PathTest, IsDisabledDefault)
{
    Path p;
    EXPECT_FALSE(p.IsDisabled());
}

TEST(PathTest, IsDisabledFlag)
{
    Path p;
    p.flags = PathFlags::kPathDisabled;
    EXPECT_TRUE(p.IsDisabled());
}

// ---------------------------------------------------------------------------
// PrettyPaths::Select
// ---------------------------------------------------------------------------

TEST(PrettyPathsTest, SelectRelPath)
{
    PrettyPaths pp;
    pp.abs = "/root/src/a.ts";
    pp.rel = "src/a.ts";

    EXPECT_EQ(pp.Select(PathStyle::kRelPath), "src/a.ts");
    EXPECT_EQ(pp.Select(PathStyle::kAbsPath), "/root/src/a.ts");
}

TEST(PrettyPathsTest, SelectFallsBackToAbsWhenRelEmpty)
{
    PrettyPaths pp;
    pp.abs = "/root/src/a.ts";

    EXPECT_EQ(pp.Select(PathStyle::kRelPath), "/root/src/a.ts");
    EXPECT_EQ(pp.Select(PathStyle::kAbsPath), "/root/src/a.ts");
}

// ---------------------------------------------------------------------------
// ImportAttributes
// ---------------------------------------------------------------------------

TEST(ImportAttributesTest, EncodeEmpty)
{
    ImportAttributes attrs = EncodeImportAttributes({});
    EXPECT_TRUE(attrs.packed_data.empty());
}

TEST(ImportAttributesTest, EncodeWireFormat)
{
    ImportAttributes attrs = EncodeImportAttributes({{"a", "bc"}});
    std::string expected;
    expected.push_back(static_cast<char>(1));
    expected.push_back(static_cast<char>(0));
    expected.push_back(static_cast<char>(0));
    expected.push_back(static_cast<char>(0));
    expected.push_back('a');
    expected.push_back(static_cast<char>(2));
    expected.push_back(static_cast<char>(0));
    expected.push_back(static_cast<char>(0));
    expected.push_back(static_cast<char>(0));
    expected.push_back('b');
    expected.push_back('c');

    EXPECT_EQ(attrs.packed_data, expected);
}

TEST(ImportAttributesTest, DecodeEmpty)
{
    ImportAttributes attrs;
    EXPECT_TRUE(attrs.DecodeIntoArray().empty());
    EXPECT_TRUE(attrs.DecodeIntoMap().empty());
}

TEST(ImportAttributesTest, RoundTrip)
{
    std::unordered_map<std::string, std::string> input = {
        {"type", "module"},
        {"key", "value"},
    };

    ImportAttributes attrs = EncodeImportAttributes(input);
    EXPECT_EQ(attrs.DecodeIntoMap(), input);

    auto arr = attrs.DecodeIntoArray();
    ASSERT_EQ(arr.size(), 2);
    EXPECT_EQ(arr[0].key, "key");
    EXPECT_EQ(arr[0].value, "value");
    EXPECT_EQ(arr[1].key, "type");
    EXPECT_EQ(arr[1].value, "module");
}

TEST(ImportAttributesTest, EncodeSortsKeys)
{
    ImportAttributes attrs = EncodeImportAttributes({{"z", "0"}, {"a", "1"}});
    auto arr = attrs.DecodeIntoArray();
    ASSERT_EQ(arr.size(), 2);
    EXPECT_EQ(arr[0].key, "a");
    EXPECT_EQ(arr[1].key, "z");
}

// ---------------------------------------------------------------------------
// Source helpers
// ---------------------------------------------------------------------------

TEST(SourceTest, TextForRange)
{
    Source s;
    s.contents = "const x = foo + bar;";

    EXPECT_EQ(s.TextForRange(Range{Loc{0}, 5}), "const");
    EXPECT_EQ(s.TextForRange(Range{Loc{14}, 5}), "+ bar");
    EXPECT_EQ(s.TextForRange(Range{Loc{18}, 1}), "r");
}

TEST(SourceTest, RangeOfOperatorBefore)
{
    Source s;
    s.contents = "const x = foo + bar;";

    auto r = s.RangeOfOperatorBefore(Loc{15}, "+");
    EXPECT_EQ(r.loc.start, 14);
    EXPECT_EQ(r.len, 1);
}

TEST(SourceTest, RangeOfOperatorBeforeNotFound)
{
    Source s;
    s.contents = "const x = foo + bar;";

    auto r = s.RangeOfOperatorBefore(Loc{10}, "+");
    EXPECT_EQ(r.loc.start, 10);
    EXPECT_EQ(r.len, 0);
}

TEST(SourceTest, RangeOfOperatorAfter)
{
    Source s;
    s.contents = "const x = foo + bar;";

    auto r = s.RangeOfOperatorAfter(Loc{14}, "+");
    EXPECT_EQ(r.loc.start, 14);
    EXPECT_EQ(r.len, 1);
}

TEST(SourceTest, RangeOfOperatorAfterNotFound)
{
    Source s;
    s.contents = "const x = foo + bar;";

    auto r = s.RangeOfOperatorAfter(Loc{0}, "++");
    EXPECT_EQ(r.loc.start, 0);
    EXPECT_EQ(r.len, 0);
}

TEST(SourceTest, RangeOfString)
{
    Source s;
    s.contents = "let s = \"hi\";";

    auto r = s.RangeOfString(Loc{8});
    EXPECT_EQ(r.loc.start, 8);
    EXPECT_EQ(r.len, 4);
    EXPECT_EQ(s.TextForRange(r), "\"hi\"");
}

TEST(SourceTest, RangeOfStringSkipsEscapes)
{
    Source s;
    s.contents = "\"a\\\"b\"";

    auto r = s.RangeOfString(Loc{0});
    EXPECT_EQ(r.loc.start, 0);
    EXPECT_EQ(r.len, 6);
    EXPECT_EQ(s.TextForRange(r), "\"a\\\"b\"");
}

TEST(SourceTest, RangeOfStringNotAString)
{
    Source s;
    s.contents = "abc123";

    auto r = s.RangeOfString(Loc{0});
    EXPECT_EQ(r.len, 0);
}

TEST(SourceTest, RangeOfNumber)
{
    Source s;
    s.contents = "123 45";

    auto r = s.RangeOfNumber(Loc{0});
    EXPECT_EQ(r.loc.start, 0);
    EXPECT_EQ(r.len, 3);
    EXPECT_EQ(s.TextForRange(r), "123");
}

TEST(SourceTest, RangeOfNumberKeepsWordChars)
{
    Source s;
    s.contents = "123abc";

    auto r = s.RangeOfNumber(Loc{0});
    EXPECT_EQ(r.loc.start, 0);
    EXPECT_EQ(r.len, 6);
    EXPECT_EQ(s.TextForRange(r), "123abc");
}

TEST(SourceTest, RangeOfNumberStopsAtDotAllowed)
{
    Source s;
    s.contents = "1.5";

    auto r = s.RangeOfNumber(Loc{0});
    EXPECT_EQ(r.len, 3);
    EXPECT_EQ(s.TextForRange(r), "1.5");
}

TEST(SourceTest, RangeOfNumberAtNonNumber)
{
    Source s;
    s.contents = "abc";

    auto r = s.RangeOfNumber(Loc{0});
    EXPECT_EQ(r.len, 0);
}

TEST(SourceTest, RangeOfLegacyOctalEscape)
{
    Source s;
    s.contents = "\\123";

    auto r = s.RangeOfLegacyOctalEscape(Loc{0});
    EXPECT_EQ(r.len, 4);
}

TEST(SourceTest, RangeOfLegacyOctalEscapeShort)
{
    Source s;
    s.contents = "\\12";

    auto r = s.RangeOfLegacyOctalEscape(Loc{0});
    EXPECT_EQ(r.len, 3);
}

TEST(SourceTest, RangeOfLegacyOctalEscapeNotEscape)
{
    Source s;
    s.contents = "abc";

    auto r = s.RangeOfLegacyOctalEscape(Loc{0});
    EXPECT_EQ(r.len, 0);
}

TEST(SourceTest, LocBeforeWhitespace)
{
    Source s;
    s.contents = "hello  ";

    auto loc = s.LocBeforeWhitespace(Loc{7});
    EXPECT_EQ(loc.start, 5);
}

TEST(SourceTest, LocBeforeWhitespaceNewline)
{
    Source s;
    s.contents = "a\n\t";

    auto loc = s.LocBeforeWhitespace(Loc{3});
    EXPECT_EQ(loc.start, 1);
}

TEST(SourceTest, CommentTextWithoutIndentNormalizesCRLF)
{
    Source s;
    s.contents = "/* a\r\nb */";

    auto text = s.CommentTextWithoutIndent(Range{Loc{0}, static_cast<int32_t>(s.contents.size())});
    EXPECT_EQ(text, "/* a\nb */");
}

TEST(SourceTest, CommentTextWithoutIndentNotComment)
{
    Source s;
    s.contents = "// a";

    auto text = s.CommentTextWithoutIndent(Range{Loc{0}, 4});
    EXPECT_EQ(text, "// a");
}

// ---------------------------------------------------------------------------
// PlatformIndependentPathDirBaseExt
// ---------------------------------------------------------------------------

TEST(PathDirBaseExtTest, UnixPath)
{
    std::string dir, base, ext;
    PlatformIndependentPathDirBaseExt("src/js/parser.go", dir, base, ext);
    EXPECT_EQ(dir, "src/js");
    EXPECT_EQ(base, "parser");
    EXPECT_EQ(ext, ".go");
}

TEST(PathDirBaseExtTest, AbsoluteUnixPath)
{
    std::string dir, base, ext;
    PlatformIndependentPathDirBaseExt("/src/js/parser.go", dir, base, ext);
    EXPECT_EQ(dir, "/src/js");
    EXPECT_EQ(base, "parser");
    EXPECT_EQ(ext, ".go");
}

TEST(PathDirBaseExtTest, WindowsPath)
{
    std::string dir, base, ext;
    PlatformIndependentPathDirBaseExt("C:\\src\\js\\parser.go", dir, base, ext);
    EXPECT_EQ(dir, "C:\\src\\js");
    EXPECT_EQ(base, "parser");
    EXPECT_EQ(ext, ".go");
}

TEST(PathDirBaseExtTest, NoExtension)
{
    std::string dir, base, ext;
    PlatformIndependentPathDirBaseExt("src/js", dir, base, ext);
    EXPECT_EQ(dir, "src");
    EXPECT_EQ(base, "js");
    EXPECT_EQ(ext, "");
}

TEST(PathDirBaseExtTest, MultipleExtensions)
{
    std::string dir, base, ext;
    PlatformIndependentPathDirBaseExt("archive.tar.gz", dir, base, ext);
    EXPECT_EQ(dir, "");
    EXPECT_EQ(base, "archive.tar");
    EXPECT_EQ(ext, ".gz");
}

TEST(PathDirBaseExtTest, ModuleCSS)
{
    std::string dir, base, ext;
    PlatformIndependentPathDirBaseExt("styles.module.css", dir, base, ext);
    EXPECT_EQ(dir, "");
    EXPECT_EQ(base, "styles");
    EXPECT_EQ(ext, ".module.css");
}

TEST(PathDirBaseExtTest, PlainCSS)
{
    std::string dir, base, ext;
    PlatformIndependentPathDirBaseExt("foo.css", dir, base, ext);
    EXPECT_EQ(dir, "");
    EXPECT_EQ(base, "foo");
    EXPECT_EQ(ext, ".css");
}

TEST(PathDirBaseExtTest, TrailingSlash)
{
    std::string dir, base, ext;
    PlatformIndependentPathDirBaseExt("C:\\src\\js\\", dir, base, ext);
    EXPECT_EQ(dir, "C:\\src");
    EXPECT_EQ(base, "js");
    EXPECT_EQ(ext, "");
}

TEST(PathDirBaseExtTest, TrailingDot)
{
    std::string dir, base, ext;
    PlatformIndependentPathDirBaseExt("file.", dir, base, ext);
    EXPECT_EQ(dir, "");
    EXPECT_EQ(base, "file");
    EXPECT_EQ(ext, ".");
}

// ---------------------------------------------------------------------------
// AllowOverride
// ---------------------------------------------------------------------------

TEST(AllowOverrideTest, NoOverridePassesThrough)
{
    EXPECT_EQ(AllowOverride({}, MsgID::kJS_DirectEval, MsgKind::kInfo), MsgKind::kInfo);
    EXPECT_EQ(AllowOverride({}, MsgID::kJS_EqualsNaN, MsgKind::kWarning), MsgKind::kWarning);
}

TEST(AllowOverrideTest, OverrideToWarning)
{
    std::unordered_map<MsgID, LogLevel> overrides = {{MsgID::kJS_DirectEval, LogLevel::kWarning}};
    EXPECT_EQ(AllowOverride(overrides, MsgID::kJS_DirectEval, MsgKind::kInfo), MsgKind::kWarning);
}

TEST(AllowOverrideTest, OverrideToDebug)
{
    std::unordered_map<MsgID, LogLevel> overrides = {{MsgID::kJS_DirectEval, LogLevel::kDebug}};
    EXPECT_EQ(AllowOverride(overrides, MsgID::kJS_DirectEval, MsgKind::kWarning), MsgKind::kDebug);
}

TEST(AllowOverrideTest, OverrideToVerbose)
{
    std::unordered_map<MsgID, LogLevel> overrides = {{MsgID::kJS_DirectEval, LogLevel::kVerbose}};
    EXPECT_EQ(AllowOverride(overrides, MsgID::kJS_DirectEval, MsgKind::kInfo), MsgKind::kVerbose);
}

TEST(AllowOverrideTest, OverrideToError)
{
    std::unordered_map<MsgID, LogLevel> overrides = {{MsgID::kJS_DirectEval, LogLevel::kError}};
    EXPECT_EQ(AllowOverride(overrides, MsgID::kJS_DirectEval, MsgKind::kInfo), MsgKind::kError);
}

TEST(AllowOverrideTest, OverrideToSilent)
{
    std::unordered_map<MsgID, LogLevel> overrides = {{MsgID::kJS_DirectEval, LogLevel::kSilent}};
    auto result = AllowOverride(overrides, MsgID::kJS_DirectEval, MsgKind::kInfo);
    EXPECT_EQ(static_cast<uint8_t>(result), 0);
}

TEST(AllowOverrideTest, OverrideDoesNotAffectOtherIDs)
{
    std::unordered_map<MsgID, LogLevel> overrides = {{MsgID::kJS_DirectEval, LogLevel::kSilent}};
    EXPECT_EQ(AllowOverride(overrides, MsgID::kJS_EqualsNaN, MsgKind::kWarning), MsgKind::kWarning);
}

// ---------------------------------------------------------------------------
// OutputOptionsForArgs
// ---------------------------------------------------------------------------

TEST(OutputOptionsForArgsTest, Defaults)
{
    auto options = OutputOptionsForArgs({});
    EXPECT_TRUE(options.include_source);
    EXPECT_EQ(options.log_level, LogLevel::kNone);
    EXPECT_EQ(options.color, UseColor::kColorIfTerminal);
}

TEST(OutputOptionsForArgsTest, ColorNever)
{
    auto options = OutputOptionsForArgs({"--color=false"});
    EXPECT_EQ(options.color, UseColor::kColorNever);
}

TEST(OutputOptionsForArgsTest, ColorAlways)
{
    auto options = OutputOptionsForArgs({"--color=true"});
    EXPECT_EQ(options.color, UseColor::kColorAlways);
}

TEST(OutputOptionsForArgsTest, ColorBare)
{
    auto options = OutputOptionsForArgs({"--color"});
    EXPECT_EQ(options.color, UseColor::kColorAlways);
}

TEST(OutputOptionsForArgsTest, LogLevelInfo)
{
    auto options = OutputOptionsForArgs({"--log-level=info"});
    EXPECT_EQ(options.log_level, LogLevel::kInfo);
}

TEST(OutputOptionsForArgsTest, LogLevelSilent)
{
    auto options = OutputOptionsForArgs({"--log-level=silent"});
    EXPECT_EQ(options.log_level, LogLevel::kSilent);
}

TEST(OutputOptionsForArgsTest, Combined)
{
    auto options = OutputOptionsForArgs({"--color=false", "--log-level=warning"});
    EXPECT_EQ(options.color, UseColor::kColorNever);
    EXPECT_EQ(options.log_level, LogLevel::kWarning);
}

TEST(OutputOptionsForArgsTest, UnknownArgsIgnored)
{
    auto options = OutputOptionsForArgs({"--foo=bar"});
    EXPECT_TRUE(options.include_source);
    EXPECT_EQ(options.log_level, LogLevel::kNone);
    EXPECT_EQ(options.color, UseColor::kColorIfTerminal);
}

// ---------------------------------------------------------------------------
// LineColumnTracker
// ---------------------------------------------------------------------------

TEST(LineColumnTrackerTest, NoSource)
{
    LineColumnTracker tracker;
    auto data = tracker.MakeMsgData(Range{Loc{0}, 1}, "x");
    EXPECT_EQ(data.text, "x");
    EXPECT_EQ(data.location, nullptr);
}

TEST(LineColumnTrackerTest, FirstLine)
{
    Source s;
    s.contents = "one\ntwo\nthree";
    LineColumnTracker tracker(&s);

    auto loc = tracker.MsgLocationOrNil(Range{Loc{0}, 3});
    ASSERT_NE(loc, nullptr);
    EXPECT_EQ(loc->line, 1);
    EXPECT_EQ(loc->column, 0);
    EXPECT_EQ(loc->length, 3);
    EXPECT_EQ(loc->line_text, "one");
}

TEST(LineColumnTrackerTest, SecondLine)
{
    Source s;
    s.contents = "one\ntwo\nthree";
    LineColumnTracker tracker(&s);

    auto loc = tracker.MsgLocationOrNil(Range{Loc{4}, 3});
    ASSERT_NE(loc, nullptr);
    EXPECT_EQ(loc->line, 2);
    EXPECT_EQ(loc->column, 0);
    EXPECT_EQ(loc->line_text, "two");
}

TEST(LineColumnTrackerTest, MidLine)
{
    Source s;
    s.contents = "one\ntwo\nthree";
    LineColumnTracker tracker(&s);

    auto loc = tracker.MsgLocationOrNil(Range{Loc{9}, 1});
    ASSERT_NE(loc, nullptr);
    EXPECT_EQ(loc->line, 3);
    EXPECT_EQ(loc->column, 1);
    EXPECT_EQ(loc->line_text, "three");
}

TEST(LineColumnTrackerTest, CRLF)
{
    Source s;
    s.contents = "a\r\nb";
    LineColumnTracker tracker(&s);

    auto loc = tracker.MsgLocationOrNil(Range{Loc{3}, 1});
    ASSERT_NE(loc, nullptr);
    EXPECT_EQ(loc->line, 2);
    EXPECT_EQ(loc->column, 0);
    EXPECT_EQ(loc->line_text, "b");
}

TEST(LineColumnTrackerTest, MakeMsgDataCopiesText)
{
    Source s;
    s.contents = "one";
    LineColumnTracker tracker(&s);

    auto data = tracker.MakeMsgData(Range{Loc{0}, 3}, "problem");
    EXPECT_EQ(data.text, "problem");
    ASSERT_NE(data.location, nullptr);
    EXPECT_EQ(data.location->line, 1);
    EXPECT_EQ(data.location->column, 0);
}

// ---------------------------------------------------------------------------
// LinkifyText
// ---------------------------------------------------------------------------

TEST(LinkifyTextTest, NoUnderlineReturnsText)
{
    EXPECT_EQ(LinkifyText("See https://example.com", "", "\x1b[0m"), "See https://example.com");
}

TEST(LinkifyTextTest, NoURL)
{
    EXPECT_EQ(LinkifyText("no links here", "\x1b[4m", "\x1b[0m"), "no links here");
}

TEST(LinkifyTextTest, SingleLink)
{
    EXPECT_EQ(LinkifyText("See https://example.com/docs.", "\x1b[4m", "\x1b[0m"),
        "See \x1b[4mhttps://example.com/docs\x1b[0m.");
}

TEST(LinkifyTextTest, LinkAtStart)
{
    EXPECT_EQ(LinkifyText("https://example.com", "\x1b[4m", "\x1b[0m"),
        "\x1b[4mhttps://example.com\x1b[0m");
}

TEST(LinkifyTextTest, MultipleLinks)
{
    EXPECT_EQ(LinkifyText("Visit https://a.com and https://b.com!", "\x1b[4m", "\x1b[0m"),
        "Visit \x1b[4mhttps://a.com\x1b[0m and \x1b[4mhttps://b.com\x1b[0m!");
}

TEST(LinkifyTextTest, TrailingPunctuation)
{
    EXPECT_EQ(LinkifyText("see https://e.com/p).", "\x1b[4m", "\x1b[0m"),
        "see \x1b[4mhttps://e.com/p)\x1b[0m.");
}

// ---------------------------------------------------------------------------
// WrapWordsInString
// ---------------------------------------------------------------------------

TEST(WrapWordsInStringTest, Empty)
{
    auto runs = WrapWordsInString("", 10);
    ASSERT_EQ(runs.size(), 1);
    EXPECT_EQ(runs[0], "");
}

TEST(WrapWordsInStringTest, SingleWordFits)
{
    auto runs = WrapWordsInString("hello", 10);
    ASSERT_EQ(runs.size(), 1);
    EXPECT_EQ(runs[0], "hello");
}

TEST(WrapWordsInStringTest, SingleWordTooLong)
{
    auto runs = WrapWordsInString("abcdefgh", 5);
    ASSERT_EQ(runs.size(), 1);
    EXPECT_EQ(runs[0], "abcdefgh");
}

TEST(WrapWordsInStringTest, TwoWordsFit)
{
    auto runs = WrapWordsInString("hello world", 100);
    ASSERT_EQ(runs.size(), 1);
    EXPECT_EQ(runs[0], "hello world");
}

TEST(WrapWordsInStringTest, WrapsAtWidth)
{
    auto runs = WrapWordsInString("hello world", 10);
    ASSERT_EQ(runs.size(), 2);
    EXPECT_EQ(runs[0], "hello");
    EXPECT_EQ(runs[1], "world");
}

TEST(WrapWordsInStringTest, WrapsAtNarrowWidth)
{
    auto runs = WrapWordsInString("hello world", 5);
    ASSERT_EQ(runs.size(), 2);
    EXPECT_EQ(runs[0], "hello");
    EXPECT_EQ(runs[1], "world");
}

TEST(WrapWordsInStringTest, MultipleLines)
{
    auto runs = WrapWordsInString("the quick brown fox", 10);
    ASSERT_EQ(runs.size(), 2);
    EXPECT_EQ(runs[0], "the quick");
    EXPECT_EQ(runs[1], "brown fox");
}

// ---------------------------------------------------------------------------
// EstimateWidthInTerminal
// ---------------------------------------------------------------------------

TEST(EstimateWidthInTerminalTest, Empty)
{
    EXPECT_EQ(EstimateWidthInTerminal(""), 0);
}

TEST(EstimateWidthInTerminalTest, ASCII)
{
    EXPECT_EQ(EstimateWidthInTerminal("hello world"), 11);
}

TEST(EstimateWidthInTerminalTest, MultiByteRune)
{
    EXPECT_EQ(EstimateWidthInTerminal("\xE2\x9C\x93"), 1); // ✓
}

TEST(EstimateWidthInTerminalTest, SkipsBOM)
{
    EXPECT_EQ(EstimateWidthInTerminal("\xEF\xBB\xBF" "abc"), 3);
}

// ---------------------------------------------------------------------------
// RenderTabStops
// ---------------------------------------------------------------------------

TEST(RenderTabStopsTest, NoTabs)
{
    EXPECT_EQ(RenderTabStops("no tabs here", 4), "no tabs here");
}

TEST(RenderTabStopsTest, SingleTab)
{
    EXPECT_EQ(RenderTabStops("a\tb", 4), "a   b");
}

TEST(RenderTabStopsTest, TabAtStart)
{
    EXPECT_EQ(RenderTabStops("\t", 4), "    ");
}

TEST(RenderTabStopsTest, TabAligned)
{
    EXPECT_EQ(RenderTabStops("abcd\t", 4), "abcd    ");
}

TEST(RenderTabStopsTest, MultipleTabs)
{
    EXPECT_EQ(RenderTabStops("a\tb\tc", 4), "a   b   c");
}

// ---------------------------------------------------------------------------
// MsgString
// ---------------------------------------------------------------------------

TEST(MsgStringTest, NoSourceError)
{
    MsgData data;
    data.text = "boom";
    TerminalInfo info;

    auto out = MsgString(false, PathStyle::kRelPath, info, MsgID::kNone, MsgKind::kError, data, "");
    EXPECT_EQ(out, "ERROR: boom\n");
}

TEST(MsgStringTest, NoSourceInfo)
{
    MsgData data;
    data.text = "hi";
    TerminalInfo info;

    auto out = MsgString(false, PathStyle::kRelPath, info, MsgID::kNone, MsgKind::kInfo, data, "");
    EXPECT_EQ(out, "INFO: hi\n");
}

TEST(MsgStringTest, NoSourceWithLocation)
{
    MsgData data;
    data.text = "boom";
    data.location = std::make_shared<MsgLocation>();
    data.location->file.abs = "/root/src/a.ts";
    data.location->file.rel = "src/a.ts";
    TerminalInfo info;

    auto out = MsgString(false, PathStyle::kRelPath, info, MsgID::kNone, MsgKind::kError, data, "");
    EXPECT_EQ(out, "src/a.ts: ERROR: boom\n");
}

TEST(MsgStringTest, NoteWithoutLocation)
{
    MsgData data;
    data.text = "hello";
    TerminalInfo info;

    auto out = MsgString(true, PathStyle::kRelPath, info, MsgID::kNone, MsgKind::kNote, data, "");
    EXPECT_EQ(out, "  hello\n");
}

TEST(MsgStringTest, WithSourceContainsMarkers)
{
    MsgData data;
    data.text = "boom";
    data.location = std::make_shared<MsgLocation>();
    data.location->file.abs = "/root/test.ts";
    data.location->file.rel = "test.ts";
    data.location->line = 1;
    data.location->column = 3;
    data.location->length = 1;
    data.location->line_text = "const x = 1;";

    TerminalInfo info;
    info.width = 80;

    auto out = MsgString(true, PathStyle::kRelPath, info, MsgID::kNone, MsgKind::kError, data, "");
    EXPECT_NE(out.find("test.ts:1:3:"), std::string::npos);
    EXPECT_NE(out.find("const x = 1;"), std::string::npos);
    EXPECT_NE(out.find("^"), std::string::npos);
    EXPECT_EQ(out.back(), '\n');
}

TEST(MsgStringTest, WithPluginAndMessageID)
{
    MsgData data;
    data.text = "eval";
    TerminalInfo info;

    auto out = MsgString(true, PathStyle::kRelPath, info, MsgID::kJS_DirectEval, MsgKind::kWarning, data, "plugin-name");
    EXPECT_NE(out.find("[direct-eval]"), std::string::npos);
    EXPECT_NE(out.find("plugin plugin-name"), std::string::npos);
    EXPECT_EQ(out.back(), '\n');
}

// ---------------------------------------------------------------------------
// Log methods
// ---------------------------------------------------------------------------

TEST(LogTest, AddError)
{
    std::vector<Msg> collected;
    Log log;
    log.add_msg = [&collected](Msg m) { collected.push_back(std::move(m)); };

    log.AddError(nullptr, Range{Loc{0}, 0}, "boom");
    ASSERT_EQ(collected.size(), 1);
    EXPECT_EQ(collected[0].kind, MsgKind::kError);
    EXPECT_EQ(collected[0].data.text, "boom");
    EXPECT_EQ(collected[0].id, MsgID::kNone);
}

TEST(LogTest, AddErrorWithNotes)
{
    std::vector<Msg> collected;
    Log log;
    log.add_msg = [&collected](Msg m) { collected.push_back(std::move(m)); };

    std::vector<MsgData> notes;
    MsgData note1;
    note1.text = "note one";
    MsgData note2;
    note2.text = "note two";
    notes.push_back(std::move(note1));
    notes.push_back(std::move(note2));

    log.AddErrorWithNotes(nullptr, Range{Loc{0}, 0}, "boom", notes);
    ASSERT_EQ(collected.size(), 1);
    ASSERT_EQ(collected[0].notes.size(), 2);
    EXPECT_EQ(collected[0].notes[0].text, "note one");
    EXPECT_EQ(collected[0].notes[1].text, "note two");
}

TEST(LogTest, AddIDSuppressedBySilentOverride)
{
    std::vector<Msg> collected;
    Log log;
    log.add_msg = [&collected](Msg m) { collected.push_back(std::move(m)); };
    log.overrides = {{MsgID::kJS_DirectEval, LogLevel::kSilent}};

    log.AddID(MsgID::kJS_DirectEval, MsgKind::kInfo, nullptr, Range{Loc{0}, 0}, "x");
    EXPECT_TRUE(collected.empty());
}

TEST(LogTest, AddIDAppliesOverride)
{
    std::vector<Msg> collected;
    Log log;
    log.add_msg = [&collected](Msg m) { collected.push_back(std::move(m)); };
    log.overrides = {{MsgID::kJS_DirectEval, LogLevel::kDebug}};

    log.AddID(MsgID::kJS_DirectEval, MsgKind::kInfo, nullptr, Range{Loc{0}, 0}, "x");
    ASSERT_EQ(collected.size(), 1);
    EXPECT_EQ(collected[0].id, MsgID::kJS_DirectEval);
    EXPECT_EQ(collected[0].kind, MsgKind::kDebug);
}

TEST(LogTest, AddMsgID)
{
    std::vector<Msg> collected;
    Log log;
    log.add_msg = [&collected](Msg m) { collected.push_back(std::move(m)); };

    Msg msg;
    msg.kind = MsgKind::kError;
    msg.data.text = "y";

    log.AddMsgID(MsgID::kJS_EqualsNaN, msg);
    ASSERT_EQ(collected.size(), 1);
    EXPECT_EQ(collected[0].id, MsgID::kJS_EqualsNaN);
    EXPECT_EQ(collected[0].kind, MsgKind::kError);
    EXPECT_EQ(collected[0].data.text, "y");
}

TEST(LogTest, AddMsgIDSuppressedBySilentOverride)
{
    std::vector<Msg> collected;
    Log log;
    log.add_msg = [&collected](Msg m) { collected.push_back(std::move(m)); };
    log.overrides = {{MsgID::kJS_DirectEval, LogLevel::kSilent}};

    Msg msg;
    msg.kind = MsgKind::kError;
    msg.data.text = "y";

    log.AddMsgID(MsgID::kJS_DirectEval, msg);
    EXPECT_TRUE(collected.empty());
}

// ---------------------------------------------------------------------------
// NewDeferLog
// ---------------------------------------------------------------------------

static Msg MakeMsg(MsgKind kind, const std::string& text) {
    Msg msg;
    msg.kind = kind;
    msg.data.text = text;
    return msg;
}

TEST(NewDeferLogTest, NoErrorsInitially)
{
    auto log = NewDeferLog(DeferLogKind::kDeferLogAll, {});
    EXPECT_FALSE(log.has_errors());
}

TEST(NewDeferLogTest, DeferAllKeepsEverything)
{
    auto log = NewDeferLog(DeferLogKind::kDeferLogAll, {});
    log.add_msg(MakeMsg(MsgKind::kVerbose, "v"));
    log.add_msg(MakeMsg(MsgKind::kDebug, "d"));
    log.add_msg(MakeMsg(MsgKind::kInfo, "i"));
    log.add_msg(MakeMsg(MsgKind::kWarning, "w"));
    log.add_msg(MakeMsg(MsgKind::kError, "e"));

    EXPECT_TRUE(log.has_errors());
    EXPECT_EQ(log.peek().size(), 5);
    EXPECT_EQ(log.done().size(), 5);
}

TEST(NewDeferLogTest, NoVerboseOrDebugDrops)
{
    auto log = NewDeferLog(DeferLogKind::kDeferLogNoVerboseOrDebug, {});
    log.add_msg(MakeMsg(MsgKind::kVerbose, "v"));
    log.add_msg(MakeMsg(MsgKind::kDebug, "d"));
    log.add_msg(MakeMsg(MsgKind::kInfo, "i"));
    log.add_msg(MakeMsg(MsgKind::kError, "e"));

    EXPECT_TRUE(log.has_errors());
    auto msgs = log.peek();
    ASSERT_EQ(msgs.size(), 2);
    EXPECT_EQ(msgs[0].kind, MsgKind::kInfo);
    EXPECT_EQ(msgs[1].kind, MsgKind::kError);
}

TEST(NewDeferLogTest, NoVerboseOrDebugNoErrors)
{
    auto log = NewDeferLog(DeferLogKind::kDeferLogNoVerboseOrDebug, {});
    log.add_msg(MakeMsg(MsgKind::kWarning, "w"));

    EXPECT_FALSE(log.has_errors());
    EXPECT_EQ(log.peek().size(), 1);
}

// ---------------------------------------------------------------------------
// NewStderrLog
// ---------------------------------------------------------------------------

TEST(NewStderrLogTest, RecordsMessagesAtSilentLevel)
{
    OutputOptions options;
    options.log_level = LogLevel::kSilent;

    auto log = NewStderrLog(options);
    EXPECT_FALSE(log.has_errors());

    log.add_msg(MakeMsg(MsgKind::kError, "err"));
    EXPECT_TRUE(log.has_errors());
    EXPECT_EQ(log.peek().size(), 1);

    log.add_msg(MakeMsg(MsgKind::kWarning, "warn"));
    EXPECT_EQ(log.peek().size(), 2);

    auto done = log.done();
    ASSERT_EQ(done.size(), 2);
    EXPECT_EQ(done[0].data.text, "err");
    EXPECT_EQ(done[1].data.text, "warn");
}

TEST(NewStderrLogTest, DoneReturnsMessages)
{
    OutputOptions options;
    options.log_level = LogLevel::kSilent;

    auto log = NewStderrLog(options);
    log.add_msg(MakeMsg(MsgKind::kInfo, "info"));

    auto done = log.done();
    ASSERT_EQ(done.size(), 1);
    EXPECT_EQ(done[0].kind, MsgKind::kInfo);
    EXPECT_EQ(done[0].data.text, "info");
}

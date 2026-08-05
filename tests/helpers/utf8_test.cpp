#include <gtest/gtest.h>
#include "helpers/utf8.hpp"

#include <string>
#include <string_view>

using guchho::helpers::ContainsNonBMPCodePoint;
using guchho::helpers::ContainsNonBMPCodePointUTF16;
using guchho::helpers::DecodeWTF8Rune;
using guchho::helpers::EncodeWTF8Rune;
using guchho::helpers::StringToUTF16;
using guchho::helpers::UTF16EqualsString;
using guchho::helpers::UTF16EqualsUTF16;
using guchho::helpers::UTF16ToString;
using guchho::helpers::UTF16ToStringWithValidation;

// ---------------------------------------------------------------------------
// EncodeWTF8Rune
// ---------------------------------------------------------------------------

TEST(EncodeWTF8RuneTest, ASCII)
{
    char buf[4] = {};
    EXPECT_EQ(EncodeWTF8Rune(buf, U'a'), 1);
    EXPECT_EQ(buf[0], 'a');
}

TEST(EncodeWTF8RuneTest, TwoByte)
{
    char buf[4] = {};
    EXPECT_EQ(EncodeWTF8Rune(buf, 0x80), 2);
    EXPECT_EQ(static_cast<unsigned char>(buf[0]), 0xC2);
    EXPECT_EQ(static_cast<unsigned char>(buf[1]), 0x80);
}

TEST(EncodeWTF8RuneTest, ThreeByte)
{
    char buf[4] = {};
    EXPECT_EQ(EncodeWTF8Rune(buf, 0x800), 3);
    EXPECT_EQ(static_cast<unsigned char>(buf[0]), 0xE0);
    EXPECT_EQ(static_cast<unsigned char>(buf[1]), 0xA0);
    EXPECT_EQ(static_cast<unsigned char>(buf[2]), 0x80);
}

TEST(EncodeWTF8RuneTest, FourByte)
{
    char buf[4] = {};
    EXPECT_EQ(EncodeWTF8Rune(buf, 0x10000), 4);
    EXPECT_EQ(static_cast<unsigned char>(buf[0]), 0xF0);
    EXPECT_EQ(static_cast<unsigned char>(buf[1]), 0x90);
    EXPECT_EQ(static_cast<unsigned char>(buf[2]), 0x80);
    EXPECT_EQ(static_cast<unsigned char>(buf[3]), 0x80);
}

TEST(EncodeWTF8RuneTest, SurrogateAllowed)
{
    char buf[4] = {};
    EXPECT_EQ(EncodeWTF8Rune(buf, 0xD800), 3);
    EXPECT_EQ(static_cast<unsigned char>(buf[0]), 0xED);
    EXPECT_EQ(static_cast<unsigned char>(buf[1]), 0xA0);
    EXPECT_EQ(static_cast<unsigned char>(buf[2]), 0x80);
}

TEST(EncodeWTF8RuneTest, AboveMaxRuneMapsToError)
{
    char buf[4] = {};
    EXPECT_EQ(EncodeWTF8Rune(buf, 0x110000), 3);
    EXPECT_EQ(static_cast<unsigned char>(buf[0]), 0xEF);
    EXPECT_EQ(static_cast<unsigned char>(buf[1]), 0xBF);
    EXPECT_EQ(static_cast<unsigned char>(buf[2]), 0xBD);
}

// ---------------------------------------------------------------------------
// DecodeWTF8Rune
// ---------------------------------------------------------------------------

TEST(DecodeWTF8RuneTest, Empty)
{
    auto [cp, w] = DecodeWTF8Rune("");
    EXPECT_EQ(cp, 0xFFFD);
    EXPECT_EQ(w, 0);
}

TEST(DecodeWTF8RuneTest, ASCII)
{
    auto [cp, w] = DecodeWTF8Rune("a");
    EXPECT_EQ(cp, U'a');
    EXPECT_EQ(w, 1);
}

TEST(DecodeWTF8RuneTest, TwoByte)
{
    auto [cp, w] = DecodeWTF8Rune("\xC2\x80");
    EXPECT_EQ(cp, 0x80);
    EXPECT_EQ(w, 2);
}

TEST(DecodeWTF8RuneTest, ThreeByte)
{
    auto [cp, w] = DecodeWTF8Rune("\xE0\xA0\x80");
    EXPECT_EQ(cp, 0x800);
    EXPECT_EQ(w, 3);
}

TEST(DecodeWTF8RuneTest, FourByte)
{
    auto [cp, w] = DecodeWTF8Rune("\xF0\x90\x80\x80");
    EXPECT_EQ(cp, 0x10000);
    EXPECT_EQ(w, 4);
}

TEST(DecodeWTF8RuneTest, SurrogateDecoded)
{
    auto [cp, w] = DecodeWTF8Rune("\xED\xA0\x80");
    EXPECT_EQ(cp, 0xD800);
    EXPECT_EQ(w, 3);
}

TEST(DecodeWTF8RuneTest, OverlongTwoByte)
{
    auto [cp, w] = DecodeWTF8Rune("\xC0\x80");
    EXPECT_EQ(cp, 0xFFFD);
    EXPECT_EQ(w, 1);
}

TEST(DecodeWTF8RuneTest, OverlongThreeByte)
{
    auto [cp, w] = DecodeWTF8Rune("\xE0\x80\x80");
    EXPECT_EQ(cp, 0xFFFD);
    EXPECT_EQ(w, 1);
}

TEST(DecodeWTF8RuneTest, OverlongFourByte)
{
    auto [cp, w] = DecodeWTF8Rune("\xF0\x80\x80\x80");
    EXPECT_EQ(cp, 0xFFFD);
    EXPECT_EQ(w, 1);
}

TEST(DecodeWTF8RuneTest, InvalidLeadByte)
{
    auto [cp, w] = DecodeWTF8Rune("\xFF");
    EXPECT_EQ(cp, 0xFFFD);
    EXPECT_EQ(w, 1);
}

TEST(DecodeWTF8RuneTest, MissingContinuation)
{
    auto [cp, w] = DecodeWTF8Rune("\xC2");
    EXPECT_EQ(cp, 0xFFFD);
    EXPECT_EQ(w, 0);
}

TEST(DecodeWTF8RuneTest, BadContinuation)
{
    auto [cp, w] = DecodeWTF8Rune("\xC2\xC0");
    EXPECT_EQ(cp, 0xFFFD);
    EXPECT_EQ(w, 1);
}

// ---------------------------------------------------------------------------
// ContainsNonBMPCodePoint
// ---------------------------------------------------------------------------

TEST(ContainsNonBMPCodePointTest, ASCII)
{
    EXPECT_FALSE(ContainsNonBMPCodePoint("hello"));
}

TEST(ContainsNonBMPCodePointTest, BMP)
{
    EXPECT_FALSE(ContainsNonBMPCodePoint("\u00E9")); // é
}

TEST(ContainsNonBMPCodePointTest, NonBMP)
{
    EXPECT_TRUE(ContainsNonBMPCodePoint("\xF0\x9F\x98\x80")); // 😀
}

TEST(ContainsNonBMPCodePointTest, Mixed)
{
    EXPECT_TRUE(ContainsNonBMPCodePoint("a\xF0\x9F\x98\x80" "b"));
}

// ---------------------------------------------------------------------------
// ContainsNonBMPCodePointUTF16
// ---------------------------------------------------------------------------

TEST(ContainsNonBMPCodePointUTF16Test, NoSurrogates)
{
    std::u16string s = u"hello";
    EXPECT_FALSE(ContainsNonBMPCodePointUTF16(s));
}

TEST(ContainsNonBMPCodePointUTF16Test, HasSurrogatePair)
{
    std::u16string s = {0xD83D, 0xDE00}; // 😀
    EXPECT_TRUE(ContainsNonBMPCodePointUTF16(s));
}

TEST(ContainsNonBMPCodePointUTF16Test, UnpairedHighSurrogate)
{
    std::u16string s = {0xD800, u'a'};
    EXPECT_FALSE(ContainsNonBMPCodePointUTF16(s));
}

// ---------------------------------------------------------------------------
// StringToUTF16 / UTF16ToString  roundtrip
// ---------------------------------------------------------------------------

TEST(StringToUTF16Test, ASCII)
{
    auto u16 = StringToUTF16("hello");
    EXPECT_EQ(u16, u"hello");
}

TEST(StringToUTF16Test, BMP)
{
    auto u16 = StringToUTF16("\xC3\xA9"); // U+00E9
    ASSERT_EQ(u16.size(), 1);
    EXPECT_EQ(u16[0], 0x00E9);
}

TEST(StringToUTF16Test, NonBMP)
{
    auto u16 = StringToUTF16("\xF0\x9F\x98\x80"); // U+1F600
    ASSERT_EQ(u16.size(), 2);
    EXPECT_EQ(u16[0], 0xD83D);
    EXPECT_EQ(u16[1], 0xDE00);
}

TEST(UTF16ToStringTest, ASCII)
{
    std::u16string in = u"hello";
    auto s = UTF16ToString(in);
    EXPECT_EQ(s, "hello");
}

TEST(UTF16ToStringTest, BMP)
{
    std::u16string in = {0x00E9};
    EXPECT_EQ(UTF16ToString(in), "\xC3\xA9");
}

TEST(UTF16ToStringTest, NonBMP)
{
    std::u16string in = {0xD83D, 0xDE00};
    EXPECT_EQ(UTF16ToString(in), "\xF0\x9F\x98\x80");
}

TEST(UTF16RoundTrip, Various)
{
    std::string cases[] = {
        "",
        "hello",
        "\xC3\xA9",
        "\xF0\x9F\x98\x80",
        "a\xF0\x9F\x98\x80" "b\xC3\xA9" "c",
    };
    for (auto const& s : cases) {
        auto u16 = StringToUTF16(s);
        auto back = UTF16ToString(u16);
        EXPECT_EQ(s, back);
    }
}

// ---------------------------------------------------------------------------
// UTF16ToStringWithValidation
// ---------------------------------------------------------------------------

TEST(UTF16ToStringWithValidationTest, Valid)
{
    std::u16string in = {0xD83D, 0xDE00, u'a'};
    auto [s, bad, ok] = UTF16ToStringWithValidation(in);
    EXPECT_TRUE(ok);
    EXPECT_EQ(s, "\xF0\x9F\x98\x80" "a");
}

TEST(UTF16ToStringWithValidationTest, UnpairedHighSurrogate)
{
    std::u16string in = {0xD800, u'a'};
    auto [s, bad, ok] = UTF16ToStringWithValidation(in);
    EXPECT_FALSE(ok);
    EXPECT_EQ(bad, 0xD800);
}

TEST(UTF16ToStringWithValidationTest, UnpairedLowSurrogate)
{
    std::u16string in = {0xDC00};
    auto [s, bad, ok] = UTF16ToStringWithValidation(in);
    EXPECT_FALSE(ok);
    EXPECT_EQ(bad, 0xDC00);
}

TEST(UTF16ToStringWithValidationTest, HighSurrogateAtEnd)
{
    std::u16string in = {0xD800};
    auto [s, bad, ok] = UTF16ToStringWithValidation(in);
    EXPECT_FALSE(ok);
    EXPECT_EQ(bad, 0xD800);
}

TEST(UTF16ToStringWithValidationTest, LowSurrogatePair)
{
    std::u16string in = {0xDC00, 0xD800};
    auto [s, bad, ok] = UTF16ToStringWithValidation(in);
    EXPECT_FALSE(ok);
    EXPECT_EQ(bad, 0xDC00);
}

// ---------------------------------------------------------------------------
// UTF16EqualsString
// ---------------------------------------------------------------------------

TEST(UTF16EqualsStringTest, Equal)
{
    std::u16string a = u"hello";
    EXPECT_TRUE(UTF16EqualsString(a, "hello"));
}

TEST(UTF16EqualsStringTest, NotEqual)
{
    std::u16string a = u"hello";
    EXPECT_FALSE(UTF16EqualsString(a, "world"));
}

TEST(UTF16EqualsStringTest, NonBMPEqual)
{
    std::u16string u16 = {0xD83D, 0xDE00};
    EXPECT_TRUE(UTF16EqualsString(u16, "\xF0\x9F\x98\x80"));
}

TEST(UTF16EqualsStringTest, WrongLength)
{
    EXPECT_FALSE(UTF16EqualsString(u"abc", "abcdef"));
}

// ---------------------------------------------------------------------------
// UTF16EqualsUTF16
// ---------------------------------------------------------------------------

TEST(UTF16EqualsUTF16Test, Equal)
{
    EXPECT_TRUE(UTF16EqualsUTF16(u"abc", u"abc"));
}

TEST(UTF16EqualsUTF16Test, NotEqual)
{
    EXPECT_FALSE(UTF16EqualsUTF16(u"abc", u"abd"));
}

TEST(UTF16EqualsUTF16Test, DifferentLength)
{
    EXPECT_FALSE(UTF16EqualsUTF16(u"abc", u"abcd"));
}

TEST(UTF16EqualsUTF16Test, BothEmpty)
{
    EXPECT_TRUE(UTF16EqualsUTF16(u"", u""));
}



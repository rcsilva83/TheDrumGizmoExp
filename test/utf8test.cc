/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            utf8test.cc
 *
 *  Tue Mar 24 18:00:00 CET 2026
 *  Copyright 2026 DrumGizmo contributors
 ****************************************************************************/

/*
 *  This file is part of DrumGizmo.
 *
 *  DrumGizmo is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  DrumGizmo is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with DrumGizmo; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
 */
#include <doctest/doctest.h>

#include <dggui/utf8.h>

TEST_CASE("UTF8Test")
{
	dggui::UTF8 utf8;

	SUBCASE("fromLatin1_ascii_passthrough")
	{
		std::string ascii = "Hello, World!";
		CHECK_EQ(ascii, utf8.fromLatin1(ascii));
	}

	SUBCASE("fromLatin1_empty_string")
	{
		CHECK_EQ(std::string(""), utf8.fromLatin1(""));
	}

	SUBCASE("fromLatin1_extended_char_e_acute")
	{
		// Latin-1 0xe9 (é) should become UTF-8 0xc3 0xa9
		std::string latin1;
		latin1 += '\xe9';
		std::string expected;
		expected += '\xc3';
		expected += '\xa9';
		CHECK_EQ(expected, utf8.fromLatin1(latin1));
	}

	SUBCASE("fromLatin1_extended_char_ae")
	{
		// Latin-1 0xe6 (æ) -> UTF-8 0xc3 0xa6
		std::string latin1;
		latin1 += '\xe6';
		std::string expected;
		expected += '\xc3';
		expected += '\xa6';
		CHECK_EQ(expected, utf8.fromLatin1(latin1));
	}

	SUBCASE("fromLatin1_mixed_ascii_and_extended")
	{
		// "caf" + 0xe9 (é) -> "caf" + UTF-8 encoding of é
		std::string latin1 = "caf";
		latin1 += '\xe9';
		std::string result = utf8.fromLatin1(latin1);
		CHECK_EQ(result.substr(0, 3), std::string("caf"));
		CHECK_EQ(result.size(), std::size_t(5u));
	}

	SUBCASE("toLatin1_ascii_passthrough")
	{
		std::string ascii = "Hello, World!";
		CHECK_EQ(ascii, utf8.toLatin1(ascii));
	}

	SUBCASE("toLatin1_empty_string")
	{
		CHECK_EQ(std::string(""), utf8.toLatin1(""));
	}

	SUBCASE("toLatin1_two_byte_sequence")
	{
		// UTF-8 0xc3 0xa9 (é) -> Latin-1 0xe9
		std::string utf8_str;
		utf8_str += '\xc3';
		utf8_str += '\xa9';
		std::string expected;
		expected += '\xe9';
		CHECK_EQ(expected, utf8.toLatin1(utf8_str));
	}

	SUBCASE("toLatin1_two_byte_ae")
	{
		// UTF-8 0xc3 0xa6 (æ) -> Latin-1 0xe6
		std::string utf8_str;
		utf8_str += '\xc3';
		utf8_str += '\xa6';
		std::string expected;
		expected += '\xe6';
		CHECK_EQ(expected, utf8.toLatin1(utf8_str));
	}

	SUBCASE("roundtrip_latin1_to_utf8_and_back")
	{
		// Build a Latin-1 string with several extended characters
		std::string latin1;
		latin1 += '\xe9'; // é
		latin1 += '\xe6'; // æ
		latin1 += '\xf8'; // ø
		latin1 += '\xc5'; // Å
		std::string encoded = utf8.fromLatin1(latin1);
		std::string decoded = utf8.toLatin1(encoded);
		CHECK_EQ(latin1, decoded);
	}

	SUBCASE("roundtrip_ascii_unchanged")
	{
		std::string ascii = "DrumGizmo 0.9.20";
		CHECK_EQ(ascii, utf8.toLatin1(utf8.fromLatin1(ascii)));
	}

	// Tests for 3-byte UTF-8 sequences (unmapped - should return empty)
	SUBCASE("toLatin1_three_byte_sequence_unmapped")
	{
		// U+20AC Euro sign: UTF-8 0xE2 0x82 0xAC (3-byte sequence)
		// Not in decode map, should be stripped (return empty string)
		std::string utf8_str;
		utf8_str += '\xe2';
		utf8_str += '\x82';
		utf8_str += '\xac';
		std::string result = utf8.toLatin1(utf8_str);
		// Unmapped 3-byte sequences are not in the decode map
		// The function looks up in map_decode and if not found, returns ""
		CHECK_EQ(std::size_t(0u), result.size());
	}

	// Tests for 4-byte UTF-8 sequences (unmapped - should return empty)
	SUBCASE("toLatin1_four_byte_sequence_unmapped")
	{
		// U+1F600 Grinning face emoji: UTF-8 0xF0 0x9F 0x98 0x80 (4-byte)
		// Not in decode map, should be stripped
		std::string utf8_str;
		utf8_str += '\xf0';
		utf8_str += '\x9f';
		utf8_str += '\x98';
		utf8_str += '\x80';
		std::string result = utf8.toLatin1(utf8_str);
		CHECK_EQ(std::size_t(0u), result.size());
	}

	// Test for special ć -> c mapping (hack for Goran Mekic's name)
	SUBCASE("toLatin1_special_c_acute_mapping")
	{
		// U+0107 (ć): UTF-8 0xC4 0x87 -> should map to "c"
		// This is a special case in the decode map
		std::string utf8_str;
		utf8_str += '\xc4';
		utf8_str += '\x87';
		std::string result = utf8.toLatin1(utf8_str);
		CHECK_EQ(std::string("c"), result);
	}

	// Test mixed string with ASCII, 2-byte, 3-byte, and 4-byte sequences
	SUBCASE("toLatin1_mixed_ascii_and_multibyte")
	{
		// "Hi " + U+0107 (ć -> c) + " test"
		std::string utf8_str = "Hi ";
		utf8_str += '\xc4';
		utf8_str += '\x87';
		utf8_str += " test";
		std::string result = utf8.toLatin1(utf8_str);
		CHECK_EQ(std::string("Hi c test"), result);
	}

	// Test string starting with multibyte sequence
	SUBCASE("toLatin1_starts_with_multibyte")
	{
		// U+00E9 (é): UTF-8 0xC3 0xA9 -> Latin-1 0xE9
		std::string utf8_str;
		utf8_str += '\xc3';
		utf8_str += '\xa9';
		utf8_str += "nd";
		std::string result = utf8.toLatin1(utf8_str);
		std::string expected;
		expected += '\xe9';
		expected += "nd";
		CHECK_EQ(expected, result);
	}

	// Test multiple 3-byte sequences (all unmapped, should be stripped)
	SUBCASE("toLatin1_multiple_unmapped_three_byte")
	{
		// Multiple 3-byte sequences that are not in the decode map
		std::string utf8_str;
		utf8_str += '\xe2';
		utf8_str += '\x82';
		utf8_str += '\xac'; // Euro
		utf8_str += '\xe2';
		utf8_str += '\x80';
		utf8_str += '\x99'; // Smart quote
		std::string result = utf8.toLatin1(utf8_str);
		// Both characters are unmapped, so result should be empty
		CHECK_EQ(std::size_t(0u), result.size());
	}

	// Test boundary: C2 (start of 2-byte range)
	SUBCASE("toLatin1_boundary_c2_start")
	{
		// U+0080: UTF-8 0xC2 0x80 -> Latin-1 0x80
		std::string utf8_str;
		utf8_str += '\xc2';
		utf8_str += '\x80';
		std::string result = utf8.toLatin1(utf8_str);
		std::string expected;
		expected += '\x80';
		CHECK_EQ(expected, result);
	}

	// Test boundary: DF (end of 2-byte range)
	SUBCASE("toLatin1_boundary_df_end")
	{
		// U+00BF: UTF-8 0xC2 0xBF -> Latin-1 0xBF (inverted question mark)
		std::string utf8_str;
		utf8_str += '\xc2';
		utf8_str += '\xbf';
		std::string result = utf8.toLatin1(utf8_str);
		std::string expected;
		expected += '\xbf';
		CHECK_EQ(expected, result);
	}
}

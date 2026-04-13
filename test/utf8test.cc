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

	SUBCASE("toLatin1_unmapped_three_byte_sequence_is_dropped")
	{
		// U+20AC (Euro sign) uses three UTF-8 bytes and is not in map_decode.
		std::string utf8_str{"\xe2\x82\xac"};
		CHECK_EQ(std::string(""), utf8.toLatin1(utf8_str));
	}

	SUBCASE("toLatin1_unmapped_four_byte_sequence_is_dropped")
	{
		// U+1F600 (😀) uses four UTF-8 bytes and is not in map_decode.
		std::string utf8_str{"\xf0\x9f\x98\x80"};
		CHECK_EQ(std::string(""), utf8.toLatin1(utf8_str));
	}

	SUBCASE("toLatin1_supports_hardcoded_c_acute_fallback")
	{
		// UTF-8 sequence \xc4\x87 is explicitly mapped to latin-1 'c'.
		std::string utf8_str{"\xc4\x87"};
		CHECK_EQ(std::string("c"), utf8.toLatin1(utf8_str));
	}

	SUBCASE("toLatin1_mixed_ascii_two_three_and_four_byte_sequences")
	{
		std::string utf8_str{"A\xc3\xa9\xe2\x82\xac\xf0\x9f\x98\x80Z"};
		// 'A' + latin1 'é' + (drop euro) + (drop emoji) + 'Z'
		std::string expected{"A\xe9Z"};
		CHECK_EQ(expected, utf8.toLatin1(utf8_str));
	}

	SUBCASE("roundtrip_ascii_unchanged")
	{
		std::string ascii = "DrumGizmo 0.9.20";
		CHECK_EQ(ascii, utf8.toLatin1(utf8.fromLatin1(ascii)));
	}
}

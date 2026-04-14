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

	SUBCASE("fromLatin1_all_control_chars_0x80_to_0x9F")
	{
		for(int i = 0x80; i <= 0x9F; ++i)
		{
			std::string latin1;
			latin1 += static_cast<char>(i);
			std::string result = utf8.fromLatin1(latin1);
			CHECK_EQ(result.size(), std::size_t(2));
			CHECK_EQ(static_cast<unsigned char>(result[0]), 0xC2);
		}
	}

	SUBCASE("fromLatin1_all_extended_chars_0xA0_to_0xFF")
	{
		for(int i = 0xA0; i <= 0xFF; ++i)
		{
			std::string latin1;
			latin1 += static_cast<char>(i);
			std::string result = utf8.fromLatin1(latin1);
			CHECK_EQ(result.size(), std::size_t(2));
		}
	}

	SUBCASE("toLatin1_three_byte_sequence_unmapped")
	{
		std::string utf8_str;
		utf8_str += '\xE2'; // 0xE2 = 3-byte lead (0xE0-0xEF range)
		utf8_str += '\x82'; // continuation byte
		utf8_str += '\xAC'; // continuation byte
		std::string result = utf8.toLatin1(utf8_str);
		CHECK_EQ(result.size(), std::size_t(0));
	}

	SUBCASE("toLatin1_four_byte_sequence_unmapped")
	{
		std::string utf8_str;
		utf8_str += '\xF0'; // 0xF0 = 4-byte lead (0xF0-0xF4 range)
		utf8_str += '\x9F'; // continuation byte
		utf8_str += '\x98'; // continuation byte
		utf8_str += '\x80'; // continuation byte
		std::string result = utf8.toLatin1(utf8_str);
		CHECK_EQ(result.size(), std::size_t(0));
	}

	SUBCASE("toLatin1_hack_mapping_c4_87_to_c")
	{
		std::string utf8_str;
		utf8_str += '\xC4';
		utf8_str += '\x87';
		std::string result = utf8.toLatin1(utf8_str);
		CHECK_EQ(result, std::string("c"));
	}

	SUBCASE("toLatin1_mixed_ascii_and_multibyte")
	{
		std::string input;
		input += 'H';
		input += '\xC3';
		input += '\xA9';
		input += 'l';
		input += '\xC3';
		input += '\xB6';
		input += '!';
		std::string result = utf8.toLatin1(input);
		CHECK_EQ(result.size(), std::size_t(5));
		CHECK_EQ(result[0], 'H');
		CHECK_EQ(static_cast<unsigned char>(result[1]), 0xE9);
		CHECK_EQ(result[2], 'l');
		CHECK_EQ(static_cast<unsigned char>(result[3]), 0xF6);
		CHECK_EQ(result[4], '!');
	}

	SUBCASE("toLatin1_3byte_E0_range")
	{
		std::string utf8_str;
		utf8_str += '\xE0';
		utf8_str += '\xA0';
		utf8_str += '\x80';
		std::string result = utf8.toLatin1(utf8_str);
		CHECK_EQ(result.size(), std::size_t(0));
	}

	SUBCASE("toLatin1_4byte_F4_range")
	{
		std::string utf8_str;
		utf8_str += '\xF4';
		utf8_str += '\x8F';
		utf8_str += '\xBF';
		utf8_str += '\xBF';
		std::string result = utf8.toLatin1(utf8_str);
		CHECK_EQ(result.size(), std::size_t(0));
	}

	SUBCASE("toLatin1_multiple_3byte_sequences")
	{
		std::string utf8_str;
		utf8_str += '\xE2';
		utf8_str += '\x82';
		utf8_str += '\xAC';
		utf8_str += '\xE2';
		utf8_str += '\x82';
		utf8_str += '\xAD';
		std::string result = utf8.toLatin1(utf8_str);
		CHECK_EQ(result.size(), std::size_t(0));
	}

	SUBCASE("fromLatin1_single_byte_0x80")
	{
		std::string latin1;
		latin1 += '\x80';
		std::string result = utf8.fromLatin1(latin1);
		CHECK_EQ(result.size(), std::size_t(2));
		CHECK_EQ(static_cast<unsigned char>(result[0]), 0xC2);
		CHECK_EQ(static_cast<unsigned char>(result[1]), 0x80);
	}

	SUBCASE("fromLatin1_single_byte_0xFF")
	{
		std::string latin1;
		latin1 += '\xFF';
		std::string result = utf8.fromLatin1(latin1);
		CHECK_EQ(result.size(), std::size_t(2));
		CHECK_EQ(static_cast<unsigned char>(result[0]), 0xC3);
		CHECK_EQ(static_cast<unsigned char>(result[1]), 0xBF);
	}

	SUBCASE("toLatin1_consecutive_2byte_sequences")
	{
		std::string input;
		input += '\xC3';
		input += '\xA9';
		input += '\xC3';
		input += '\xB8';
		std::string result = utf8.toLatin1(input);
		CHECK_EQ(result.size(), std::size_t(2));
		CHECK_EQ(static_cast<unsigned char>(result[0]), 0xE9);
		CHECK_EQ(static_cast<unsigned char>(result[1]), 0xF8);
	}

	SUBCASE("fromLatin1_long_string_all_extended")
	{
		std::string latin1;
		for(int i = 0x80; i <= 0xFF; ++i)
		{
			latin1 += static_cast<char>(i);
		}
		std::string result = utf8.fromLatin1(latin1);
		CHECK_EQ(result.size(), std::size_t(128 * 2));

		std::string decoded = utf8.toLatin1(result);
		CHECK_EQ(decoded, latin1);
	}
}

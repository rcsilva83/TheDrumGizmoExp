/* -*- Mode: c++ -*- */
/***************************************************************************
 *            bytesizeparsertest.cc
 *
 *  Sun Mar 05 11:44:23 CET 2017
 *  Copyright 2017 Goran Mekić
 *  meka@tilda.center
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

#include "bytesizeparser.h"

TEST_CASE("ByteSizeParserTest")
{
	SUBCASE("suffixTest")
	{
		std::size_t computed_size, expected_size;
		std::size_t kilo = 1024, mega = kilo * 1024, giga = mega * 1024;
		computed_size = byteSizeParser("3");
		expected_size = 3;
		CHECK_EQ(expected_size, computed_size);

		computed_size = byteSizeParser("3k");
		expected_size = 3 * kilo;
		CHECK_EQ(expected_size, computed_size);

		computed_size = byteSizeParser("3M");
		expected_size = 3 * mega;
		CHECK_EQ(expected_size, computed_size);

		computed_size = byteSizeParser("3G");
		expected_size = 3 * giga;
		CHECK_EQ(expected_size, computed_size);
	}

	SUBCASE("falseSuffixTest")
	{
		std::size_t computed_size, expected_size = 0;
		computed_size = byteSizeParser("3K");
		CHECK_EQ(expected_size, computed_size);

		computed_size = byteSizeParser("3m");
		CHECK_EQ(expected_size, computed_size);

		computed_size = byteSizeParser("3g");
		CHECK_EQ(expected_size, computed_size);

		computed_size = byteSizeParser("3ddDD");
		CHECK_EQ(expected_size, computed_size);
	}

	SUBCASE("falseNumberTest")
	{
		std::size_t computed_size, expected_size = 0;
		computed_size = byteSizeParser("K3k");
		CHECK_EQ(expected_size, computed_size);

		computed_size = byteSizeParser("-3");
		CHECK_EQ(expected_size, computed_size);

		computed_size = byteSizeParser("-3k");
		CHECK_EQ(expected_size, computed_size);

		computed_size = byteSizeParser("-3M");
		CHECK_EQ(expected_size, computed_size);

		computed_size = byteSizeParser("-3G");
		CHECK_EQ(expected_size, computed_size);

		computed_size = byteSizeParser("3-");
		CHECK_EQ(expected_size, computed_size);

		computed_size = byteSizeParser("3-k");
		CHECK_EQ(expected_size, computed_size);

		computed_size = byteSizeParser("k-3");
		CHECK_EQ(expected_size, computed_size);

		computed_size = byteSizeParser("3-1");
		CHECK_EQ(expected_size, computed_size);

		computed_size = byteSizeParser("   -3");
		CHECK_EQ(expected_size, computed_size);
	}

	SUBCASE("tooBigNumberTest")
	{
		std::size_t computed_size, expected_size = 0;
		computed_size = byteSizeParser("999999999999999999999999999999999999G");
		CHECK_EQ(expected_size, computed_size);
	}
}

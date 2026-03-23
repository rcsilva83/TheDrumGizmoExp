/* -*- Mode: c++ -*- */
/***************************************************************************
 *            versionstrtest.cc
 *
 *  Mon Mar 23 20:00:00 CET 2026
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

#include "../src/versionstr.h"

TEST_CASE("VersionStrTest")
{
	SUBCASE("constructor_from_numeric_parts")
	{
		VersionStr v(1, 2, 3);
		CHECK_EQ(v.major(), 1u);
		CHECK_EQ(v.minor(), 2u);
		CHECK_EQ(v.patch(), 3u);
	}

	SUBCASE("constructor_default_zero")
	{
		VersionStr v(0, 0, 0);
		CHECK_EQ(v.major(), 0u);
		CHECK_EQ(v.minor(), 0u);
		CHECK_EQ(v.patch(), 0u);
	}

	SUBCASE("parse_major_minor")
	{
		VersionStr v("1.0");
		CHECK_EQ(v.major(), 1u);
		CHECK_EQ(v.minor(), 0u);
		CHECK_EQ(v.patch(), 0u);
	}

	SUBCASE("parse_major_minor_patch")
	{
		VersionStr v("2.4.6");
		CHECK_EQ(v.major(), 2u);
		CHECK_EQ(v.minor(), 4u);
		CHECK_EQ(v.patch(), 6u);
	}

	SUBCASE("parse_version_0_9_20")
	{
		VersionStr v("0.9.20");
		CHECK_EQ(v.major(), 0u);
		CHECK_EQ(v.minor(), 9u);
		CHECK_EQ(v.patch(), 20u);
	}

	SUBCASE("to_string_without_patch")
	{
		VersionStr v(3, 5, 0);
		std::string s = static_cast<std::string>(v);
		CHECK_EQ(s, "3.5");
	}

	SUBCASE("to_string_with_patch")
	{
		VersionStr v(1, 2, 3);
		std::string s = static_cast<std::string>(v);
		CHECK_EQ(s, "1.2.3");
	}

	SUBCASE("assignment_from_string")
	{
		VersionStr v(0, 0, 0);
		v = "4.7.2";
		CHECK_EQ(v.major(), 4u);
		CHECK_EQ(v.minor(), 7u);
		CHECK_EQ(v.patch(), 2u);
	}

	SUBCASE("comparison_equal")
	{
		VersionStr a(1, 2, 3);
		VersionStr b(1, 2, 3);
		CHECK_UNARY(a == b);
		CHECK_UNARY(!(a < b));
		CHECK_UNARY(!(a > b));
		CHECK_UNARY(a <= b);
		CHECK_UNARY(a >= b);
	}

	SUBCASE("comparison_less_by_major")
	{
		VersionStr a(1, 9, 9);
		VersionStr b(2, 0, 0);
		CHECK_UNARY(a < b);
		CHECK_UNARY(!(a > b));
		CHECK_UNARY(!(a == b));
	}

	SUBCASE("comparison_less_by_minor")
	{
		VersionStr a(1, 2, 9);
		VersionStr b(1, 3, 0);
		CHECK_UNARY(a < b);
		CHECK_UNARY(b > a);
	}

	SUBCASE("comparison_less_by_patch")
	{
		VersionStr a(1, 2, 3);
		VersionStr b(1, 2, 4);
		CHECK_UNARY(a < b);
		CHECK_UNARY(b > a);
	}

	SUBCASE("comparison_greater_or_equal")
	{
		VersionStr a(2, 0, 0);
		VersionStr b(1, 9, 9);
		CHECK_UNARY(a >= b);
		CHECK_UNARY(b <= a);
	}

	SUBCASE("parse_invalid_returns_zero")
	{
		// An illegal character in the string causes a reset to 0.0.0
		VersionStr v("abc");
		CHECK_EQ(v.major(), 0u);
		CHECK_EQ(v.minor(), 0u);
		CHECK_EQ(v.patch(), 0u);
	}

	SUBCASE("parse_too_many_segments_returns_zero")
	{
		// Four dots segments should reset to 0.0.0
		VersionStr v("1.2.3.4");
		CHECK_EQ(v.major(), 0u);
		CHECK_EQ(v.minor(), 0u);
		CHECK_EQ(v.patch(), 0u);
	}

	SUBCASE("round_trip_major_minor_patch")
	{
		VersionStr original(7, 3, 11);
		std::string s = static_cast<std::string>(original);
		VersionStr round_tripped(s);
		CHECK_UNARY(original == round_tripped);
	}
}

/* -*- Mode: c++ -*- */
/***************************************************************************
 *            rangemaptest.cc
 *
 *  Sat Apr 11 00:00:00 CET 2026
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

#include <rangemap.h>

TEST_CASE("RangeMapTest")
{
	SUBCASE("get_range_inside_stored_range_returns_value")
	{
		// Query [20,30] lies fully inside stored range [10,40].
		RangeMap<int, int> rm;
		rm.insert(10, 40, 99);
		auto res = rm.get(20, 30);
		REQUIRE_EQ(1u, res.size());
		CHECK_EQ(99, res[0]);
	}

	SUBCASE("get_range_containing_stored_range_returns_value")
	{
		// Query [5,50] contains stored range [10,40].
		RangeMap<int, int> rm;
		rm.insert(10, 40, 42);
		auto res = rm.get(5, 50);
		REQUIRE_EQ(1u, res.size());
		CHECK_EQ(42, res[0]);
	}

	SUBCASE("get_range_overlapping_lower_returns_value")
	{
		// Query [5,20] overlaps the lower end of stored range [10,40].
		RangeMap<int, int> rm;
		rm.insert(10, 40, 7);
		auto res = rm.get(5, 20);
		REQUIRE_EQ(1u, res.size());
		CHECK_EQ(7, res[0]);
	}

	SUBCASE("get_range_overlapping_upper_returns_value")
	{
		// Query [30,50] overlaps the upper end of stored range [10,40].
		RangeMap<int, int> rm;
		rm.insert(10, 40, 3);
		auto res = rm.get(30, 50);
		REQUIRE_EQ(1u, res.size());
		CHECK_EQ(3, res[0]);
	}

	SUBCASE("get_range_no_overlap_returns_empty")
	{
		// Query [50,60] does not overlap stored range [10,40].
		RangeMap<int, int> rm;
		rm.insert(10, 40, 5);
		auto res = rm.get(50, 60);
		CHECK_EQ(0u, res.size());
	}

	SUBCASE("insert_reversed_range_is_normalised")
	{
		// insert(40, 10, ...) must normalise to [10,40] so queries work.
		RangeMap<int, int> rm;
		rm.insert(40, 10, 11);
		auto res = rm.get(20, 30);
		REQUIRE_EQ(1u, res.size());
		CHECK_EQ(11, res[0]);
	}

	SUBCASE("get_point_inside_stored_range_returns_value")
	{
		// Single-point query at=25 lies inside stored range [10,40].
		RangeMap<int, int> rm;
		rm.insert(10, 40, 55);
		auto res = rm.get(25);
		REQUIRE_EQ(1u, res.size());
		CHECK_EQ(55, res[0]);
	}

	SUBCASE("get_point_outside_stored_range_returns_empty")
	{
		// Single-point query at=5 is outside stored range [10,40].
		RangeMap<int, int> rm;
		rm.insert(10, 40, 55);
		auto res = rm.get(5);
		CHECK_EQ(0u, res.size());
	}

	SUBCASE("get_point_on_boundary_returns_value")
	{
		// Boundary values (a==at or at==b) must be included.
		RangeMap<int, int> rm;
		rm.insert(10, 40, 77);
		CHECK_EQ(1u, rm.get(10).size());
		CHECK_EQ(1u, rm.get(40).size());
	}

	SUBCASE("multiple_ranges_multiple_results")
	{
		// Two overlapping ranges both matching the query are both returned.
		RangeMap<int, int> rm;
		rm.insert(0, 50, 1);
		rm.insert(20, 70, 2);
		auto res = rm.get(30);
		CHECK_EQ(2u, res.size());
	}

	SUBCASE("empty_map_returns_empty_for_range_query")
	{
		RangeMap<int, int> rm;
		CHECK_EQ(0u, rm.get(0, 100).size());
	}

	SUBCASE("empty_map_returns_empty_for_point_query")
	{
		RangeMap<int, int> rm;
		CHECK_EQ(0u, rm.get(50).size());
	}
}

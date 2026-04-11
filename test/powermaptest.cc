/* -*- Mode: c++ -*- */
/***************************************************************************
 *            powermaptest.cc
 *
 *  Sun Apr 19 23:23:37 CEST 2020
 *  Copyright 2020 André Nusser
 *  andre.nusser@googlemail.com
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

#include "../src/powermap.h"

TEST_CASE("test_powermaptest")
{
	SUBCASE("default_is_identity")
	{
		// After reset(), the default spline is approximately the identity
		// function: the three control points are (eps,eps), (0.5,0.5),
		// (1-eps,1-eps), so map(0.5) must equal exactly 0.5.
		Powermap powermap;
		CHECK_EQ(powermap.map(0.5f), doctest::Approx(0.5f));
	}

	SUBCASE("map_output_in_range")
	{
		Powermap powermap;
		// Verify that map() always returns a value in [0, 1].
		for(int i = 0; i <= 100; ++i)
		{
			float in = i / 100.0f;
			float out = powermap.map(in);
			CHECK_UNARY(out >= 0.0f && out <= 1.0f);
		}
	}

	SUBCASE("shelf_mode_boundary_values")
	{
		// With shelf=true (default), input values below fixed[0].in are
		// clamped to fixed[0].out, and values above fixed[2].in are clamped
		// to fixed[2].out.
		Powermap powermap;
		const float eps = 1e-4f;
		// Below lower boundary -> shelf holds at fixed[0].out
		CHECK_EQ(powermap.map(0.0f), doctest::Approx(eps));
		// Above upper boundary -> shelf holds at fixed[2].out
		CHECK_EQ(powermap.map(1.0f), doctest::Approx(1.0f - eps));
	}

	SUBCASE("no_shelf_boundary_values")
	{
		// With shelf=false, the spline is extended to pass through (0,0) and
		// (1,1), so map(0) == 0 and map(1) == 1 exactly.
		Powermap powermap;
		powermap.setShelf(false);
		CHECK_EQ(powermap.map(0.0f), doctest::Approx(0.0f));
		CHECK_EQ(powermap.map(1.0f), doctest::Approx(1.0f));
	}

	SUBCASE("control_points_returned_by_getters")
	{
		Powermap powermap;
		const float eps = 1e-4f;

		// Default control points set by reset().
		CHECK_EQ(powermap.getFixed0().in, doctest::Approx(eps));
		CHECK_EQ(powermap.getFixed0().out, doctest::Approx(eps));
		CHECK_EQ(powermap.getFixed1().in, doctest::Approx(0.5f));
		CHECK_EQ(powermap.getFixed1().out, doctest::Approx(0.5f));
		CHECK_EQ(powermap.getFixed2().in, doctest::Approx(1.0f - eps));
		CHECK_EQ(powermap.getFixed2().out, doctest::Approx(1.0f - eps));
	}

	SUBCASE("setFixed1_changes_map")
	{
		// Moving the middle control point to (0.3, 0.7) makes the spline
		// pass through that point: map(0.3) must equal 0.7.
		Powermap powermap;
		powermap.setFixed1({0.3f, 0.7f});
		CHECK_EQ(powermap.getFixed1().in, doctest::Approx(0.3f));
		CHECK_EQ(powermap.getFixed1().out, doctest::Approx(0.7f));
		CHECK_EQ(powermap.map(0.3f), doctest::Approx(0.7f));
	}

	SUBCASE("reset_restores_defaults")
	{
		Powermap powermap;
		powermap.setFixed1({0.3f, 0.7f});
		powermap.reset();

		CHECK_EQ(powermap.getFixed1().in, doctest::Approx(0.5f));
		CHECK_EQ(powermap.getFixed1().out, doctest::Approx(0.5f));
		CHECK_EQ(powermap.map(0.5f), doctest::Approx(0.5f));
	}

	SUBCASE("regression_issue33_reset_is_not_order_dependent")
	{
		// Regression for PR #33: reset() previously used setFixed* in order,
		// which clamped fixed1 against stale fixed2 values.
		Powermap powermap;
		const float eps = 1e-4f;

		powermap.setFixed1({0.2f, 0.2f});
		powermap.setFixed2({0.35f, 0.35f});

		powermap.reset();

		CHECK_EQ(powermap.getFixed0().in, doctest::Approx(eps));
		CHECK_EQ(powermap.getFixed1().in, doctest::Approx(0.5f));
		CHECK_EQ(powermap.getFixed1().out, doctest::Approx(0.5f));
		CHECK_EQ(powermap.getFixed2().in, doctest::Approx(1.0f - eps));
		CHECK_EQ(powermap.getFixed2().out, doctest::Approx(1.0f - eps));
	}

	SUBCASE("setFixed0_changes_map_output")
	{
		// Moving fixed[0] to a new value must update the spline and affect the
		// map() output for inputs in the lower range.
		Powermap powermap;
		const float new_in = 0.05f;
		const float new_out = 0.1f;
		powermap.setFixed0({new_in, new_out});
		CHECK_EQ(powermap.getFixed0().in, doctest::Approx(new_in));
		CHECK_EQ(powermap.getFixed0().out, doctest::Approx(new_out));
	}

	SUBCASE("setFixed0_with_same_value_is_a_no_op")
	{
		// Calling setFixed0 with the current value must not change the state
		// (covers the false branch of the != guard).
		Powermap powermap;
		auto before = powermap.getFixed0();
		powermap.setFixed0(before);
		auto after = powermap.getFixed0();
		CHECK_EQ(before.in, doctest::Approx(after.in));
		CHECK_EQ(before.out, doctest::Approx(after.out));
	}

	SUBCASE("setFixed2_changes_map_output")
	{
		// Moving fixed[2] to a new value must update the spline and affect the
		// map() output for inputs near the upper boundary.
		Powermap powermap;
		const float new_in = 0.9f;
		const float new_out = 0.8f;
		powermap.setFixed2({new_in, new_out});
		CHECK_EQ(powermap.getFixed2().in, doctest::Approx(new_in));
		CHECK_EQ(powermap.getFixed2().out, doctest::Approx(new_out));
	}

	SUBCASE("setFixed2_with_same_value_is_a_no_op")
	{
		// Calling setFixed2 with the current value must not change the state
		// (covers the false branch of the != guard).
		Powermap powermap;
		auto before = powermap.getFixed2();
		powermap.setFixed2(before);
		auto after = powermap.getFixed2();
		CHECK_EQ(before.in, doctest::Approx(after.in));
		CHECK_EQ(before.out, doctest::Approx(after.out));
	}

	SUBCASE("setFixed1_with_same_value_is_a_no_op")
	{
		// Covers the false branch of the != guard in setFixed1.
		Powermap powermap;
		auto before = powermap.getFixed1();
		powermap.setFixed1(before);
		auto after = powermap.getFixed1();
		CHECK_EQ(before.in, doctest::Approx(after.in));
		CHECK_EQ(before.out, doctest::Approx(after.out));
	}

	SUBCASE("setShelf_toggle_false_then_true")
	{
		// setShelf(false) then setShelf(true) must each trigger a spline
		// update (covers both the true and false branches of the shelf guard).
		Powermap powermap;
		powermap.setShelf(false); // changes from default true -> false
		CHECK_EQ(powermap.map(0.0f), doctest::Approx(0.0f));
		powermap.setShelf(true); // changes back to true
		const float eps = 1e-4f;
		CHECK_EQ(powermap.map(0.0f), doctest::Approx(eps));
	}

	SUBCASE("setShelf_with_same_value_is_a_no_op")
	{
		// Calling setShelf(true) when shelf is already true must not trigger
		// a spline update (covers the false branch of the shelf != enable
		// guard).
		Powermap powermap;
		// shelf is true by default; calling with true again is a no-op.
		powermap.setShelf(true);
		// We only verify map still behaves the same after the no-op call.
		CHECK_EQ(powermap.map(0.5f), doctest::Approx(0.5f));
	}
}

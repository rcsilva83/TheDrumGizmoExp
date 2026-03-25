/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            colourtest.cc
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

#include <dggui/colour.h>

TEST_CASE("ColourTest")
{
	SUBCASE("default_constructor_is_opaque_white")
	{
		dggui::Colour c;
		CHECK_EQ(255u, c.red());
		CHECK_EQ(255u, c.green());
		CHECK_EQ(255u, c.blue());
		CHECK_EQ(255u, c.alpha());
	}

	SUBCASE("float_grey_constructor")
	{
		dggui::Colour c(0.5f);
		CHECK_EQ(127u, c.red());
		CHECK_EQ(127u, c.green());
		CHECK_EQ(127u, c.blue());
		CHECK_EQ(255u, c.alpha());
	}

	SUBCASE("float_grey_with_alpha_constructor")
	{
		dggui::Colour c(1.0f, 0.0f);
		CHECK_EQ(255u, c.red());
		CHECK_EQ(255u, c.green());
		CHECK_EQ(255u, c.blue());
		CHECK_EQ(0u, c.alpha());
	}

	SUBCASE("float_rgb_constructor")
	{
		dggui::Colour c(1.0f, 0.0f, 0.5f);
		CHECK_EQ(255u, c.red());
		CHECK_EQ(0u, c.green());
		CHECK_EQ(127u, c.blue());
		CHECK_EQ(255u, c.alpha());
	}

	SUBCASE("float_rgba_constructor")
	{
		dggui::Colour c(0.0f, 1.0f, 0.5f, 0.25f);
		CHECK_EQ(0u, c.red());
		CHECK_EQ(255u, c.green());
		CHECK_EQ(127u, c.blue());
		CHECK_EQ(63u, c.alpha());
	}

	SUBCASE("uint8_constructor")
	{
		dggui::Colour c(std::uint8_t(10), std::uint8_t(20), std::uint8_t(30),
		    std::uint8_t(40));
		CHECK_EQ(std::uint8_t(10), c.red());
		CHECK_EQ(std::uint8_t(20), c.green());
		CHECK_EQ(std::uint8_t(30), c.blue());
		CHECK_EQ(std::uint8_t(40), c.alpha());
	}

	SUBCASE("copy_constructor")
	{
		dggui::Colour orig(std::uint8_t(10), std::uint8_t(20), std::uint8_t(30),
		    std::uint8_t(40));
		dggui::Colour copy(orig);
		CHECK_EQ(orig.red(), copy.red());
		CHECK_EQ(orig.green(), copy.green());
		CHECK_EQ(orig.blue(), copy.blue());
		CHECK_EQ(orig.alpha(), copy.alpha());
	}

	SUBCASE("assignment_operator")
	{
		dggui::Colour a(std::uint8_t(10), std::uint8_t(20), std::uint8_t(30),
		    std::uint8_t(40));
		dggui::Colour b;
		b = a;
		CHECK_EQ(a.red(), b.red());
		CHECK_EQ(a.green(), b.green());
		CHECK_EQ(a.blue(), b.blue());
		CHECK_EQ(a.alpha(), b.alpha());
	}

	SUBCASE("equality_operator_compares_rgb_only")
	{
		// operator== compares only R, G, B – not alpha
		dggui::Colour a(std::uint8_t(10), std::uint8_t(20), std::uint8_t(30),
		    std::uint8_t(100));
		dggui::Colour b(std::uint8_t(10), std::uint8_t(20), std::uint8_t(30),
		    std::uint8_t(200));
		CHECK(a == b);
	}

	SUBCASE("equality_operator_false_for_different_rgb")
	{
		dggui::Colour a(std::uint8_t(10), std::uint8_t(20), std::uint8_t(30),
		    std::uint8_t(255));
		dggui::Colour b(std::uint8_t(11), std::uint8_t(20), std::uint8_t(30),
		    std::uint8_t(255));
		CHECK_UNARY(!(a == b));
	}

	SUBCASE("inequality_operator")
	{
		dggui::Colour a(std::uint8_t(10), std::uint8_t(20), std::uint8_t(30),
		    std::uint8_t(255));
		dggui::Colour b(std::uint8_t(10), std::uint8_t(21), std::uint8_t(30),
		    std::uint8_t(255));
		CHECK(a != b);
	}

	SUBCASE("inequality_operator_false_when_rgb_equal")
	{
		dggui::Colour a(std::uint8_t(10), std::uint8_t(20), std::uint8_t(30),
		    std::uint8_t(0));
		dggui::Colour b(std::uint8_t(10), std::uint8_t(20), std::uint8_t(30),
		    std::uint8_t(128));
		CHECK_UNARY(!(a != b));
	}

	SUBCASE("black_colour")
	{
		dggui::Colour c(0.0f);
		CHECK_EQ(0u, c.red());
		CHECK_EQ(0u, c.green());
		CHECK_EQ(0u, c.blue());
	}

	SUBCASE("full_white_colour")
	{
		dggui::Colour c(1.0f);
		CHECK_EQ(255u, c.red());
		CHECK_EQ(255u, c.green());
		CHECK_EQ(255u, c.blue());
	}
}

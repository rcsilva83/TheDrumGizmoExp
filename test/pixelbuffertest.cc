/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            pixelbuffertest.cc
 *
 *  Tue Apr 14 00:00:00 CET 2026
 *  Copyright 2026 DrumGizmo contributors
 ****************************************************************************/

/*
 *  This file is part of DrumGizmo.
 *
 *  DrumGizmo is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  DrumGizmo is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with DrumGizmo; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
 */
#include <doctest/doctest.h>

#include <dggui/colour.h>
#include <dggui/pixelbuffer.h>

#include <cstdint>
#include <cstring>
#include <vector>

TEST_CASE("PixelBufferAlpha construction and basic operations")
{
	SUBCASE("default_constructor_creates_zero_size")
	{
		dggui::PixelBufferAlpha pb;
		CHECK_EQ(pb.width, std::size_t(0));
		CHECK_EQ(pb.height, std::size_t(0));
	}

	SUBCASE("sized_constructor_allocates_buffer")
	{
		dggui::PixelBufferAlpha pb(10, 20);
		CHECK_EQ(pb.width, std::size_t(10));
		CHECK_EQ(pb.height, std::size_t(20));
		CHECK_NE(pb.buf, nullptr);
	}

	SUBCASE("realloc_changes_dimensions")
	{
		dggui::PixelBufferAlpha pb(5, 5);
		CHECK_EQ(pb.width, std::size_t(5));
		CHECK_EQ(pb.height, std::size_t(5));

		pb.realloc(20, 30);
		CHECK_EQ(pb.width, std::size_t(20));
		CHECK_EQ(pb.height, std::size_t(30));
		CHECK_NE(pb.buf, nullptr);
	}

	SUBCASE("clear_zeros_all_pixels")
	{
		dggui::PixelBufferAlpha pb(4, 4);
		pb.setPixel(0, 0,
		    dggui::Colour(std::uint8_t(255), std::uint8_t(128),
		        std::uint8_t(64), std::uint8_t(255)));
		dggui::Colour c_before = pb.pixel(0, 0);
		CHECK_EQ(c_before.red(), 255);

		pb.clear();

		dggui::Colour c_after = pb.pixel(0, 0);
		CHECK_EQ(c_after.red(), 0);
		CHECK_EQ(c_after.green(), 0);
		CHECK_EQ(c_after.blue(), 0);
		CHECK_EQ(c_after.alpha(), 0);
	}

	SUBCASE("default_dirty_flag_is_true")
	{
		dggui::PixelBufferAlpha pb(4, 4);
		CHECK(pb.dirty);
	}

	SUBCASE("default_visible_flag_is_true")
	{
		dggui::PixelBufferAlpha pb(4, 4);
		CHECK(pb.visible);
	}

	SUBCASE("default_position_is_zero")
	{
		dggui::PixelBufferAlpha pb(4, 4);
		CHECK_EQ(pb.x, 0);
		CHECK_EQ(pb.y, 0);
	}

	SUBCASE("default_has_last_is_false")
	{
		dggui::PixelBufferAlpha pb(4, 4);
		CHECK_FALSE(pb.has_last);
	}
}

TEST_CASE("PixelBufferAlpha setPixel and pixel")
{
	dggui::PixelBufferAlpha pb(8, 8);

	SUBCASE("set_and_read_pixel")
	{
		dggui::Colour c(std::uint8_t(255), std::uint8_t(128), std::uint8_t(64),
		    std::uint8_t(200));
		pb.setPixel(3, 5, c);
		dggui::Colour result = pb.pixel(3, 5);
		CHECK_EQ(result.red(), 255);
		CHECK_EQ(result.green(), 128);
		CHECK_EQ(result.blue(), 64);
		CHECK_EQ(result.alpha(), 200);
	}

	SUBCASE("multiple_pixels_independent")
	{
		pb.setPixel(0, 0, dggui::Colour(std::uint8_t(10), std::uint8_t(20), std::uint8_t(30), std::uint8_t(40)));
		pb.setPixel(7, 7, dggui::Colour(std::uint8_t(100), std::uint8_t(150), std::uint8_t(200), std::uint8_t(250)));
		dggui::Colour c1 = pb.pixel(0, 0);
		dggui::Colour c2 = pb.pixel(7, 7);
		CHECK_EQ(c1.red(), 10);
		CHECK_EQ(c2.red(), 100);
	}

	SUBCASE("pixel_overwrite")
	{
		pb.setPixel(2, 2, dggui::Colour(std::uint8_t(255), std::uint8_t(0), std::uint8_t(0), std::uint8_t(255)));
		pb.setPixel(2, 2, dggui::Colour(std::uint8_t(0), std::uint8_t(255), std::uint8_t(0), std::uint8_t(128)));
		dggui::Colour c = pb.pixel(2, 2);
		CHECK_EQ(c.red(), 0);
		CHECK_EQ(c.green(), 255);
	}
}

TEST_CASE("PixelBufferAlpha writeLine")
{
	dggui::PixelBufferAlpha pb(8, 4);

	SUBCASE("write_line_within_bounds")
	{
		std::uint8_t line[] = {
		    10, 20, 30, 255, 40, 50, 60, 255, 70, 80, 90, 255};
		pb.writeLine(0, 0, line, 3);

		dggui::Colour c0 = pb.pixel(0, 0);
		CHECK_EQ(c0.red(), 10);
		CHECK_EQ(c0.green(), 20);

		dggui::Colour c1 = pb.pixel(1, 0);
		CHECK_EQ(c1.red(), 40);
		CHECK_EQ(c1.green(), 50);

		dggui::Colour c2 = pb.pixel(2, 0);
		CHECK_EQ(c2.red(), 70);
	}

	SUBCASE("write_line_with_offset")
	{
		std::uint8_t line[] = {100, 110, 120, 200, 130, 140, 150, 210};
		pb.writeLine(3, 1, line, 2);

		dggui::Colour c0 = pb.pixel(3, 1);
		CHECK_EQ(c0.red(), 100);
		CHECK_EQ(c0.alpha(), 200);
	}

	SUBCASE("write_line_clipped_at_buffer_edge")
	{
		std::uint8_t line[] = {1, 2, 3, 255, 4, 5, 6, 255, 7, 8, 9, 255, 10, 11,
		    12, 255, 13, 14, 15, 255};
		pb.writeLine(5, 0, line, 5);

		dggui::Colour c5 = pb.pixel(5, 0);
		CHECK_EQ(c5.red(), 1);

		dggui::Colour c6 = pb.pixel(6, 0);
		CHECK_EQ(c6.red(), 4);

		dggui::Colour c7 = pb.pixel(7, 0);
		CHECK_EQ(c7.red(), 7);

		dggui::Colour c0_before = pb.pixel(0, 1);
		CHECK_EQ(c0_before.red(), 0);
	}

	SUBCASE("write_line_out_of_bounds_x")
	{
		std::uint8_t line[] = {255, 0, 0, 255};
		pb.writeLine(10, 0, line, 1);
	}

	SUBCASE("write_line_out_of_bounds_y")
	{
		std::uint8_t line[] = {255, 0, 0, 255};
		pb.writeLine(0, 10, line, 1);
	}
}

TEST_CASE("PixelBufferAlpha blendLine")
{
	SUBCASE("blend_opaque_line_copies_pixels")
	{
		dggui::PixelBufferAlpha pb(8, 4);
		pb.clear();
		std::uint8_t line[] = {100, 150, 200, 255, 50, 60, 70, 255};
		pb.blendLine(0, 0, line, 2);

		dggui::Colour c0 = pb.pixel(0, 0);
		CHECK_EQ(c0.red(), 100);
		CHECK_EQ(c0.green(), 150);
		CHECK_EQ(c0.blue(), 200);
		CHECK_EQ(c0.alpha(), 255);
	}

	SUBCASE("blend_fully_transparent_line_does_nothing")
	{
		dggui::PixelBufferAlpha pb(8, 4);
		pb.setPixel(0, 0, dggui::Colour(std::uint8_t(200), std::uint8_t(200), std::uint8_t(200), std::uint8_t(255)));
		pb.setPixel(1, 0, dggui::Colour(std::uint8_t(100), std::uint8_t(100), std::uint8_t(100), std::uint8_t(128)));

		std::uint8_t line[] = {50, 60, 70, 0, 80, 90, 100, 0};
		pb.blendLine(0, 0, line, 2);

		dggui::Colour c0 = pb.pixel(0, 0);
		CHECK_EQ(c0.red(), 200);

		dggui::Colour c1 = pb.pixel(1, 0);
		CHECK_EQ(c1.red(), 100);
	}

	SUBCASE("blend_semi_transparent_line_blends")
	{
		dggui::PixelBufferAlpha pb(8, 4);
		pb.setPixel(0, 0, dggui::Colour(std::uint8_t(0), std::uint8_t(0), std::uint8_t(0), std::uint8_t(255)));

		std::uint8_t line[] = {255, 128, 0, 128};
		pb.blendLine(0, 0, line, 1);

		dggui::Colour c = pb.pixel(0, 0);
		CHECK(c.red() > 0);
		CHECK(c.green() > 0);
		CHECK(c.alpha() > 0);
	}

	SUBCASE("blend_mixed_opaque_and_transparent_pixels")
	{
		dggui::PixelBufferAlpha pb(8, 4);
		pb.clear();

		std::uint8_t line[] = {100, 100, 100, 255, 200, 200, 200, 0, 50, 50, 50,
		    255, 75, 75, 75, 128};
		pb.blendLine(0, 0, line, 4);

		dggui::Colour c0 = pb.pixel(0, 0);
		CHECK_EQ(c0.red(), 100);

		dggui::Colour c2 = pb.pixel(2, 0);
		CHECK_EQ(c2.red(), 50);

		dggui::Colour c3 = pb.pixel(3, 0);
		CHECK(c3.red() > 0);
	}

	SUBCASE("blend_line_clipped_at_buffer_edge")
	{
		dggui::PixelBufferAlpha pb(4, 4);
		pb.clear();

		std::uint8_t line[] = {255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255,
		    255, 255, 0, 255, 0, 255, 255, 255};
		pb.blendLine(2, 0, line, 5);

		dggui::Colour c0 = pb.pixel(2, 0);
		CHECK_EQ(c0.red(), 255);

		dggui::Colour c1 = pb.pixel(3, 0);
		CHECK_EQ(c1.green(), 255);
	}

	SUBCASE("blend_line_out_of_bounds")
	{
		dggui::PixelBufferAlpha pb(4, 4);
		pb.clear();

		std::uint8_t line[] = {255, 0, 0, 255};
		pb.blendLine(10, 0, line, 1);
		pb.blendLine(0, 10, line, 1);
	}
}

TEST_CASE("PixelBufferAlpha addPixel")
{
	dggui::PixelBufferAlpha pb(8, 8);

	SUBCASE("add_opaque_pixel_overwrites")
	{
		pb.setPixel(1, 1, dggui::Colour(std::uint8_t(50), std::uint8_t(60), std::uint8_t(70), std::uint8_t(80)));
		pb.addPixel(1, 1, dggui::Colour(std::uint8_t(200), std::uint8_t(210), std::uint8_t(220), std::uint8_t(255)));
		dggui::Colour c = pb.pixel(1, 1);
		CHECK_EQ(c.red(), 200);
		CHECK_EQ(c.green(), 210);
		CHECK_EQ(c.blue(), 220);
		CHECK_EQ(c.alpha(), 255);
	}

	SUBCASE("add_fully_transparent_pixel_does_nothing")
	{
		pb.setPixel(2, 2, dggui::Colour(std::uint8_t(100), std::uint8_t(150), std::uint8_t(200), std::uint8_t(128)));
		pb.addPixel(2, 2, dggui::Colour(std::uint8_t(0), std::uint8_t(0), std::uint8_t(0), std::uint8_t(0)));
		dggui::Colour c = pb.pixel(2, 2);
		CHECK_EQ(c.red(), 100);
		CHECK_EQ(c.green(), 150);
	}

	SUBCASE("add_semi_transparent_pixel_blends")
	{
		pb.clear();
		pb.setPixel(3, 3, dggui::Colour(std::uint8_t(100), std::uint8_t(100), std::uint8_t(100), std::uint8_t(100)));
		pb.addPixel(3, 3, dggui::Colour(std::uint8_t(200), std::uint8_t(200), std::uint8_t(200), std::uint8_t(50)));
		dggui::Colour c = pb.pixel(3, 3);
		CHECK(c.alpha() > 0);
	}

	SUBCASE("add_pixel_out_of_bounds_ignored")
	{
		pb.addPixel(100, 100, dggui::Colour(std::uint8_t(255), std::uint8_t(255), std::uint8_t(255), std::uint8_t(255)));
	}
}

TEST_CASE("PixelBufferAlpha getLine")
{
	dggui::PixelBufferAlpha pb(4, 4);

	SUBCASE("getLine_returns_pointer_to_pixel_data")
	{
		pb.setPixel(2, 1, dggui::Colour(std::uint8_t(42), std::uint8_t(84), std::uint8_t(126), std::uint8_t(255)));
		const std::uint8_t* line = pb.getLine(0, 1);
		CHECK_NE(line, nullptr);

		const std::uint8_t* pixel_2 = line + 2 * 4;
		CHECK_EQ(pixel_2[0], 42);
		CHECK_EQ(pixel_2[1], 84);
		CHECK_EQ(pixel_2[2], 126);
		CHECK_EQ(pixel_2[3], 255);
	}

	SUBCASE("getLine_with_x_offset")
	{
		pb.setPixel(3, 0, dggui::Colour(std::uint8_t(10), std::uint8_t(20), std::uint8_t(30), std::uint8_t(40)));
		const std::uint8_t* line = pb.getLine(3, 0);
		CHECK_EQ(line[0], 10);
		CHECK_EQ(line[1], 20);
		CHECK_EQ(line[2], 30);
		CHECK_EQ(line[3], 40);
	}
}

TEST_CASE("PixelBuffer (non-alpha) construction and basic operations")
{
	SUBCASE("constructor_allocates_buffer")
	{
		dggui::PixelBuffer pb(10, 20);
		CHECK_EQ(pb.width, std::size_t(10));
		CHECK_EQ(pb.height, std::size_t(20));
		CHECK_NE(pb.buf, nullptr);
	}

	SUBCASE("realloc_changes_dimensions")
	{
		dggui::PixelBuffer pb(5, 5);
		pb.realloc(30, 40);
		CHECK_EQ(pb.width, std::size_t(30));
		CHECK_EQ(pb.height, std::size_t(40));
	}
}

TEST_CASE("PixelBuffer::blendLine (non-alpha)")
{
	dggui::PixelBuffer pb(8, 4);

	SUBCASE("blend_opaque_pixels_copies_directly")
	{
		std::uint8_t line[] = {100, 150, 200, 255, 50, 60, 70, 255};
		pb.blendLine(0, 0, line, 2);

		std::uint8_t* pixel0 = pb.buf + 0 * 3;
		CHECK_EQ(pixel0[0], 100);
		CHECK_EQ(pixel0[1], 150);
		CHECK_EQ(pixel0[2], 200);

		std::uint8_t* pixel1 = pb.buf + 1 * 3;
		CHECK_EQ(pixel1[0], 50);
		CHECK_EQ(pixel1[1], 60);
		CHECK_EQ(pixel1[2], 70);
	}

	SUBCASE("blend_semi_transparent_pixels_blends")
	{
		std::memset(pb.buf, 200, pb.width * pb.height * 3);

		std::uint8_t line[] = {100, 100, 100, 128};
		pb.blendLine(0, 0, line, 1);

		std::uint8_t* pixel = pb.buf;
		unsigned int a = 128;
		unsigned int b = 255 - a;
		CHECK_EQ(pixel[0], (std::uint8_t)((100 * a + 200 * b) / 255));
		CHECK_EQ(pixel[1], (std::uint8_t)((100 * a + 200 * b) / 255));
		CHECK_EQ(pixel[2], (std::uint8_t)((100 * a + 200 * b) / 255));
	}
}

TEST_CASE("PixelBuffer::updateBuffer")
{
	SUBCASE("updateBuffer_empty_list_returns_empty_rect")
	{
		dggui::PixelBuffer pb(10, 10);
		std::vector<dggui::PixelBufferAlpha*> buffers;
		dggui::Rect rect = pb.updateBuffer(buffers);
		CHECK(rect.empty());
	}

	SUBCASE("updateBuffer_dirty_buffer_returns_dirty_rect")
	{
		dggui::PixelBuffer pb(20, 20);
		dggui::PixelBufferAlpha alpha(10, 10);
		alpha.x = 0;
		alpha.y = 0;
		alpha.dirty = true;
		alpha.visible = true;

		std::vector<dggui::PixelBufferAlpha*> buffers;
		buffers.push_back(&alpha);
		dggui::Rect rect = pb.updateBuffer(buffers);

		CHECK_FALSE(rect.empty());
		CHECK_EQ(rect.x1, std::size_t(0));
		CHECK_EQ(rect.y1, std::size_t(0));
		CHECK(rect.x2 >= std::size_t(10));
		CHECK(rect.y2 >= std::size_t(10));

		CHECK_FALSE(alpha.dirty);
	}

	SUBCASE("updateBuffer_clean_buffer_returns_empty")
	{
		dggui::PixelBuffer pb(10, 10);
		dggui::PixelBufferAlpha alpha(5, 5);
		alpha.x = 0;
		alpha.y = 0;
		alpha.dirty = false;
		alpha.visible = true;
		alpha.has_last = false;

		std::vector<dggui::PixelBufferAlpha*> buffers;
		buffers.push_back(&alpha);
		dggui::Rect rect = pb.updateBuffer(buffers);
		CHECK(rect.empty());
	}

	SUBCASE("updateBuffer_has_last_creates_dirty_rect")
	{
		dggui::PixelBuffer pb(20, 20);
		dggui::PixelBufferAlpha alpha(10, 10);
		alpha.x = 2;
		alpha.y = 3;
		alpha.dirty = false;
		alpha.visible = true;
		alpha.has_last = true;
		alpha.last_x = 0;
		alpha.last_y = 0;
		alpha.last_width = 10;
		alpha.last_height = 10;

		std::vector<dggui::PixelBufferAlpha*> buffers;
		buffers.push_back(&alpha);
		dggui::Rect rect = pb.updateBuffer(buffers);

		CHECK_FALSE(rect.empty());
		CHECK_FALSE(alpha.has_last);
	}

	SUBCASE("updateBuffer_multiple_dirty_buffers_expands_rect")
	{
		dggui::PixelBuffer pb(30, 30);
		dggui::PixelBufferAlpha alpha1(5, 5);
		alpha1.x = 0;
		alpha1.y = 0;
		alpha1.dirty = true;
		alpha1.visible = true;

		dggui::PixelBufferAlpha alpha2(5, 5);
		alpha2.x = 10;
		alpha2.y = 10;
		alpha2.dirty = true;
		alpha2.visible = true;

		std::vector<dggui::PixelBufferAlpha*> buffers;
		buffers.push_back(&alpha1);
		buffers.push_back(&alpha2);
		dggui::Rect rect = pb.updateBuffer(buffers);

		CHECK_FALSE(rect.empty());
		CHECK_EQ(rect.x1, std::size_t(0));
		CHECK_EQ(rect.y1, std::size_t(0));
		CHECK(rect.x2 >= std::size_t(15));
		CHECK(rect.y2 >= std::size_t(15));
	}

	SUBCASE("updateBuffer_invisible_buffer_skipped_drawing")
	{
		dggui::PixelBuffer pb(10, 10);
		dggui::PixelBufferAlpha alpha(5, 5);
		alpha.x = 0;
		alpha.y = 0;
		alpha.dirty = true;
		alpha.visible = false;

		std::vector<dggui::PixelBufferAlpha*> buffers;
		buffers.push_back(&alpha);
		dggui::Rect rect = pb.updateBuffer(buffers);

		CHECK_FALSE(rect.empty());
	}

	SUBCASE("updateBuffer_buffer_outside_window_skipped")
	{
		dggui::PixelBuffer pb(10, 10);
		dggui::PixelBufferAlpha alpha(5, 5);
		alpha.x = 20;
		alpha.y = 20;
		alpha.dirty = true;
		alpha.visible = true;

		std::vector<dggui::PixelBufferAlpha*> buffers;
		buffers.push_back(&alpha);
		dggui::Rect rect = pb.updateBuffer(buffers);

		CHECK_FALSE(rect.empty());
	}
}

TEST_CASE("PixelBufferAlpha realloc_preserves_dirty_flags")
{
	dggui::PixelBufferAlpha pb(4, 4);
	pb.x = 5;
	pb.y = 6;
	pb.dirty = true;
	pb.visible = true;

	pb.realloc(8, 8);

	CHECK_EQ(pb.width, std::size_t(8));
	CHECK_EQ(pb.height, std::size_t(8));
	CHECK_EQ(pb.x, 5);
	CHECK_EQ(pb.y, 6);
}

TEST_CASE("Rect empty method")
{
	SUBCASE("default_rect_is_empty")
	{
		dggui::Rect r{};
		CHECK(r.empty());
	}

	SUBCASE("equal_coords_empty")
	{
		dggui::Rect r{5, 5, 5, 5};
		CHECK(r.empty());
	}

	SUBCASE("non_empty_rect")
	{
		dggui::Rect r{0, 0, 10, 10};
		CHECK_FALSE(r.empty());
	}

	SUBCASE("asymmetric_empty")
	{
		dggui::Rect r{3, 7, 3, 7};
		CHECK(r.empty());
	}
}
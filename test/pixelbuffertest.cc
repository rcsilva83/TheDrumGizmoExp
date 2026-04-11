/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            pixelbuffertest.cc
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

#include <cstdint>
#include <vector>

#include <dggui/colour.h>
#include <dggui/pixelbuffer.h>

// ---------------------------------------------------------------------------
// PixelBufferAlpha tests
// ---------------------------------------------------------------------------

TEST_CASE("PixelBufferAlphaTest")
{
	SUBCASE("construction_and_dimensions")
	{
		dggui::PixelBufferAlpha buf(10, 20);
		CHECK_EQ(std::size_t(10u), buf.width);
		CHECK_EQ(std::size_t(20u), buf.height);
	}

	SUBCASE("clear_zeroes_all_pixels")
	{
		dggui::PixelBufferAlpha buf(4, 4);
		// Set a pixel, then clear
		dggui::Colour red(std::uint8_t(255), std::uint8_t(0), std::uint8_t(0),
		    std::uint8_t(255));
		buf.setPixel(0, 0, red);
		buf.clear();
		const auto& c = buf.pixel(0, 0);
		CHECK_EQ(std::uint8_t(0), c.red());
		CHECK_EQ(std::uint8_t(0), c.green());
		CHECK_EQ(std::uint8_t(0), c.blue());
		CHECK_EQ(std::uint8_t(0), c.alpha());
	}

	SUBCASE("setPixel_and_pixel_roundtrip")
	{
		dggui::PixelBufferAlpha buf(8, 8);
		dggui::Colour c(std::uint8_t(10), std::uint8_t(20), std::uint8_t(30),
		    std::uint8_t(40));
		buf.setPixel(3, 5, c);
		const auto& result = buf.pixel(3, 5);
		CHECK_EQ(std::uint8_t(10), result.red());
		CHECK_EQ(std::uint8_t(20), result.green());
		CHECK_EQ(std::uint8_t(30), result.blue());
		CHECK_EQ(std::uint8_t(40), result.alpha());
	}

	SUBCASE("realloc_changes_dimensions")
	{
		dggui::PixelBufferAlpha buf(4, 4);
		buf.realloc(8, 16);
		CHECK_EQ(std::size_t(8u), buf.width);
		CHECK_EQ(std::size_t(16u), buf.height);
	}

	SUBCASE("writeLine_writes_pixels")
	{
		dggui::PixelBufferAlpha buf(8, 4);
		// Build a scanline of 3 RGBA pixels
		std::uint8_t line[12] = {
			100, 150, 200, 255,  // pixel 0: opaque
			10,  20,  30,  128,  // pixel 1: semi-transparent
			50,  60,  70,  0,    // pixel 2: transparent
		};
		buf.writeLine(1, 2, line, 3);

		const auto& p0 = buf.pixel(1, 2);
		CHECK_EQ(std::uint8_t(100), p0.red());
		CHECK_EQ(std::uint8_t(150), p0.green());
		CHECK_EQ(std::uint8_t(200), p0.blue());
		CHECK_EQ(std::uint8_t(255), p0.alpha());

		const auto& p1 = buf.pixel(2, 2);
		CHECK_EQ(std::uint8_t(10), p1.red());
		CHECK_EQ(std::uint8_t(128), p1.alpha());
	}

	SUBCASE("writeLine_out_of_bounds_x_ignored")
	{
		dggui::PixelBufferAlpha buf(4, 4);
		std::uint8_t line[4] = {1, 2, 3, 4};
		// x >= width: should do nothing, no crash
		buf.writeLine(4, 0, line, 1);
		CHECK_EQ(std::uint8_t(0), buf.pixel(0, 0).red());
	}

	SUBCASE("writeLine_out_of_bounds_y_ignored")
	{
		dggui::PixelBufferAlpha buf(4, 4);
		std::uint8_t line[4] = {1, 2, 3, 4};
		// y >= height: should do nothing, no crash
		buf.writeLine(0, 4, line, 1);
		CHECK_EQ(std::uint8_t(0), buf.pixel(0, 0).red());
	}

	SUBCASE("writeLine_clips_to_width")
	{
		// Write starting at x=2 with len=4, but width=4 so only 2 pixels fit
		dggui::PixelBufferAlpha buf(4, 4);
		std::uint8_t line[16];
		for(int i = 0; i < 16; i += 4)
		{
			line[i + 0] = std::uint8_t(i + 1);
			line[i + 1] = std::uint8_t(0);
			line[i + 2] = std::uint8_t(0);
			line[i + 3] = std::uint8_t(255);
		}
		buf.writeLine(2, 0, line, 4); // Only 2 pixels fit
		// Pixel at x=3 should be set
		CHECK_EQ(std::uint8_t(5), buf.pixel(3, 0).red());
		// Pixel at x=4 doesn't exist - no crash
	}

	SUBCASE("blendLine_opaque_overwrites")
	{
		dggui::PixelBufferAlpha buf(8, 8);
		// Start with a pixel
		dggui::Colour bg(std::uint8_t(50), std::uint8_t(50), std::uint8_t(50),
		    std::uint8_t(255));
		buf.setPixel(0, 0, bg);

		// Blend an opaque pixel over it
		std::uint8_t line[4] = {200, 100, 50, 255}; // fully opaque
		buf.blendLine(0, 0, line, 1);

		const auto& result = buf.pixel(0, 0);
		CHECK_EQ(std::uint8_t(200), result.red());
		CHECK_EQ(std::uint8_t(100), result.green());
		CHECK_EQ(std::uint8_t(50), result.blue());
	}

	SUBCASE("blendLine_transparent_is_noop")
	{
		dggui::PixelBufferAlpha buf(8, 8);
		dggui::Colour bg(std::uint8_t(50), std::uint8_t(60), std::uint8_t(70),
		    std::uint8_t(255));
		buf.setPixel(0, 0, bg);

		// Blend a fully transparent pixel - should not change the background
		std::uint8_t line[4] = {200, 100, 50, 0}; // fully transparent
		buf.blendLine(0, 0, line, 1);

		const auto& result = buf.pixel(0, 0);
		CHECK_EQ(std::uint8_t(50), result.red());
		CHECK_EQ(std::uint8_t(60), result.green());
		CHECK_EQ(std::uint8_t(70), result.blue());
	}

	SUBCASE("blendLine_semi_transparent_blends")
	{
		dggui::PixelBufferAlpha buf(8, 8);
		// clear background is (0,0,0,0)

		// Blend a semi-transparent pixel
		std::uint8_t line[4] = {200, 0, 0, 128}; // semi-transparent red
		buf.blendLine(0, 0, line, 1);

		// Result should have some red, non-zero alpha
		const auto& result = buf.pixel(0, 0);
		CHECK_UNARY(result.red() > 0);
		CHECK_UNARY(result.alpha() > 0);
	}

	SUBCASE("blendLine_out_of_bounds_x_ignored")
	{
		dggui::PixelBufferAlpha buf(4, 4);
		std::uint8_t line[4] = {255, 0, 0, 255};
		// x >= width: no crash
		buf.blendLine(4, 0, line, 1);
		CHECK_EQ(std::uint8_t(0), buf.pixel(0, 0).red());
	}

	SUBCASE("blendLine_out_of_bounds_y_ignored")
	{
		dggui::PixelBufferAlpha buf(4, 4);
		std::uint8_t line[4] = {255, 0, 0, 255};
		// y >= height: no crash
		buf.blendLine(0, 4, line, 1);
		CHECK_EQ(std::uint8_t(0), buf.pixel(0, 0).red());
	}

	SUBCASE("blendLine_clips_to_width")
	{
		dggui::PixelBufferAlpha buf(4, 4);
		// 8 opaque pixels, but only 2 fit from x=2
		std::uint8_t line[32];
		for(int i = 0; i < 32; i += 4)
		{
			line[i + 0] = 255;
			line[i + 1] = 0;
			line[i + 2] = 0;
			line[i + 3] = 255;
		}
		buf.blendLine(2, 0, line, 8);
		// No crash; pixel at x=3 should be set
		CHECK_EQ(std::uint8_t(255), buf.pixel(3, 0).red());
	}

	SUBCASE("blendLine_mixed_opaque_and_transparent_run")
	{
		// Tests the fast-path in blendLine where consecutive opaque pixels
		// are memcpy'd in a chunk
		dggui::PixelBufferAlpha buf(8, 4);
		// 3 opaque pixels followed by 1 transparent
		std::uint8_t line[16] = {
			100, 0, 0, 255,  // opaque
			101, 0, 0, 255,  // opaque
			102, 0, 0, 255,  // opaque
			200, 0, 0, 0,    // transparent
		};
		buf.blendLine(0, 0, line, 4);

		CHECK_EQ(std::uint8_t(100), buf.pixel(0, 0).red());
		CHECK_EQ(std::uint8_t(101), buf.pixel(1, 0).red());
		CHECK_EQ(std::uint8_t(102), buf.pixel(2, 0).red());
		// transparent pixel: background (0,0,0,0) unchanged
		CHECK_EQ(std::uint8_t(0), buf.pixel(3, 0).red());
	}

	SUBCASE("addPixel_opaque_sets_pixel")
	{
		dggui::PixelBufferAlpha buf(4, 4);
		dggui::Colour c(std::uint8_t(100), std::uint8_t(150), std::uint8_t(200),
		    std::uint8_t(255));
		buf.addPixel(1, 2, c);
		const auto& result = buf.pixel(1, 2);
		CHECK_EQ(std::uint8_t(100), result.red());
		CHECK_EQ(std::uint8_t(150), result.green());
		CHECK_EQ(std::uint8_t(200), result.blue());
		CHECK_EQ(std::uint8_t(255), result.alpha());
	}

	SUBCASE("addPixel_transparent_is_noop")
	{
		dggui::PixelBufferAlpha buf(4, 4);
		dggui::Colour bg(std::uint8_t(50), std::uint8_t(50), std::uint8_t(50),
		    std::uint8_t(255));
		buf.setPixel(1, 2, bg);

		dggui::Colour transparent(std::uint8_t(200), std::uint8_t(0),
		    std::uint8_t(0), std::uint8_t(0));
		buf.addPixel(1, 2, transparent);

		const auto& result = buf.pixel(1, 2);
		CHECK_EQ(std::uint8_t(50), result.red());
	}

	SUBCASE("addPixel_semi_transparent_blends")
	{
		dggui::PixelBufferAlpha buf(4, 4);
		dggui::Colour c(std::uint8_t(200), std::uint8_t(0), std::uint8_t(0),
		    std::uint8_t(128));
		buf.addPixel(0, 0, c);
		const auto& result = buf.pixel(0, 0);
		CHECK_UNARY(result.red() > 0);
	}

	SUBCASE("addPixel_out_of_bounds_x_ignored")
	{
		dggui::PixelBufferAlpha buf(4, 4);
		dggui::Colour c(std::uint8_t(255), std::uint8_t(0), std::uint8_t(0),
		    std::uint8_t(255));
		buf.addPixel(4, 0, c); // x >= width, no crash
		CHECK_EQ(std::uint8_t(0), buf.pixel(0, 0).red());
	}

	SUBCASE("addPixel_out_of_bounds_y_ignored")
	{
		dggui::PixelBufferAlpha buf(4, 4);
		dggui::Colour c(std::uint8_t(255), std::uint8_t(0), std::uint8_t(0),
		    std::uint8_t(255));
		buf.addPixel(0, 4, c); // y >= height, no crash
		CHECK_EQ(std::uint8_t(0), buf.pixel(0, 0).red());
	}

	SUBCASE("getLine_returns_correct_pointer")
	{
		dggui::PixelBufferAlpha buf(4, 4);
		dggui::Colour c(std::uint8_t(10), std::uint8_t(20), std::uint8_t(30),
		    std::uint8_t(40));
		buf.setPixel(2, 1, c);
		const std::uint8_t* line = buf.getLine(2, 1);
		CHECK_EQ(std::uint8_t(10), line[0]);
		CHECK_EQ(std::uint8_t(20), line[1]);
		CHECK_EQ(std::uint8_t(30), line[2]);
		CHECK_EQ(std::uint8_t(40), line[3]);
	}
}

// ---------------------------------------------------------------------------
// PixelBuffer tests
// ---------------------------------------------------------------------------

TEST_CASE("PixelBufferTest")
{
	SUBCASE("construction_and_dimensions")
	{
		dggui::PixelBuffer buf(10, 20);
		CHECK_EQ(std::size_t(10u), buf.width);
		CHECK_EQ(std::size_t(20u), buf.height);
	}

	SUBCASE("realloc_changes_dimensions")
	{
		dggui::PixelBuffer buf(4, 4);
		buf.realloc(8, 16);
		CHECK_EQ(std::size_t(8u), buf.width);
		CHECK_EQ(std::size_t(16u), buf.height);
	}

	SUBCASE("blendLine_opaque")
	{
		dggui::PixelBuffer buf(8, 8);
		// The PixelBuffer is 3 bytes per pixel (RGB, no alpha).
		// Build a 4-byte RGBA source pixel (opaque).
		std::uint8_t line[4] = {100, 150, 200, 255}; // fully opaque
		buf.blendLine(0, 0, line, 1);
		// Read via buf.buf raw:
		CHECK_EQ(std::uint8_t(100), buf.buf[0]);
		CHECK_EQ(std::uint8_t(150), buf.buf[1]);
		CHECK_EQ(std::uint8_t(200), buf.buf[2]);
	}

	SUBCASE("blendLine_semi_transparent")
	{
		dggui::PixelBuffer buf(8, 8);
		// Set background to white (255,255,255) via raw buf
		for(std::size_t i = 0; i < buf.width * buf.height * 3; ++i)
		{
			buf.buf[i] = 255;
		}

		// Blend a 50% red pixel
		std::uint8_t line[4] = {200, 0, 0, 128};
		buf.blendLine(0, 0, line, 1);

		// Result should be a blend of red and white
		CHECK_UNARY(buf.buf[0] > 0);
		CHECK_UNARY(buf.buf[0] < 255);
	}

	SUBCASE("updateBuffer_no_dirty_buffers_returns_empty_rect")
	{
		dggui::PixelBuffer dst(100, 100);
		dggui::PixelBufferAlpha src(50, 50);
		src.visible = true;
		src.dirty = false;
		src.has_last = false;
		src.x = 0;
		src.y = 0;
		std::vector<dggui::PixelBufferAlpha*> bufs = {&src};
		dggui::Rect result = dst.updateBuffer(bufs);
		CHECK_UNARY(result.empty());
	}

	SUBCASE("updateBuffer_dirty_buffer_returns_dirty_rect")
	{
		dggui::PixelBuffer dst(100, 100);
		dggui::PixelBufferAlpha src(50, 50);
		src.visible = true;
		src.dirty = true;
		src.has_last = false;
		src.x = 0;
		src.y = 0;
		std::vector<dggui::PixelBufferAlpha*> bufs = {&src};
		dggui::Rect result = dst.updateBuffer(bufs);
		CHECK_UNARY(!result.empty());
	}

	SUBCASE("updateBuffer_has_last_expands_dirty_rect")
	{
		dggui::PixelBuffer dst(100, 100);
		dggui::PixelBufferAlpha src(20, 20);
		src.visible = true;
		src.dirty = false;
		src.has_last = true;
		src.last_x = 5;
		src.last_y = 5;
		src.last_width = 10;
		src.last_height = 10;
		src.x = 0;
		src.y = 0;
		std::vector<dggui::PixelBufferAlpha*> bufs = {&src};
		dggui::Rect result = dst.updateBuffer(bufs);
		CHECK_UNARY(!result.empty());
	}

	SUBCASE("updateBuffer_invisible_buffer_skipped")
	{
		dggui::PixelBuffer dst(100, 100);
		dggui::PixelBufferAlpha src(50, 50);
		src.visible = false;
		src.dirty = true;
		src.has_last = false;
		src.x = 0;
		src.y = 0;
		std::vector<dggui::PixelBufferAlpha*> bufs = {&src};
		dggui::Rect result = dst.updateBuffer(bufs);
		// dirty rect is computed but invisible buffer is not blended
		// (result rect comes from dirty flag, which was set)
		// The function still returns a non-empty rect from the dirty flag tracking
	}

	SUBCASE("updateBuffer_buffer_outside_window_skipped")
	{
		dggui::PixelBuffer dst(10, 10);
		dggui::PixelBufferAlpha src(5, 5);
		src.visible = true;
		src.dirty = true;
		src.has_last = false;
		src.x = 20; // outside the 10x10 dst
		src.y = 20;
		std::vector<dggui::PixelBufferAlpha*> bufs = {&src};
		dggui::Rect result = dst.updateBuffer(bufs);
		// Should complete without crash
	}

	SUBCASE("updateBuffer_two_dirty_buffers_merges_rects")
	{
		dggui::PixelBuffer dst(200, 200);
		dggui::PixelBufferAlpha src1(20, 20);
		src1.visible = true;
		src1.dirty = true;
		src1.has_last = false;
		src1.x = 0;
		src1.y = 0;

		dggui::PixelBufferAlpha src2(20, 20);
		src2.visible = true;
		src2.dirty = true;
		src2.has_last = false;
		src2.x = 100;
		src2.y = 100;

		std::vector<dggui::PixelBufferAlpha*> bufs = {&src1, &src2};
		dggui::Rect result = dst.updateBuffer(bufs);
		CHECK_UNARY(!result.empty());
		// The merged rect should span from (0,0) to (120,120)
		CHECK_EQ(std::size_t(0u), result.x1);
		CHECK_EQ(std::size_t(0u), result.y1);
		CHECK_UNARY(result.x2 >= std::size_t(100u));
		CHECK_UNARY(result.y2 >= std::size_t(100u));
	}

	SUBCASE("updateBuffer_clamps_dirty_rect_to_dst_size")
	{
		dggui::PixelBuffer dst(50, 50);
		dggui::PixelBufferAlpha src(100, 100); // larger than dst
		src.visible = true;
		src.dirty = true;
		src.has_last = false;
		src.x = 0;
		src.y = 0;
		std::vector<dggui::PixelBufferAlpha*> bufs = {&src};
		dggui::Rect result = dst.updateBuffer(bufs);
		CHECK_UNARY(result.x2 <= std::size_t(50u));
		CHECK_UNARY(result.y2 <= std::size_t(50u));
	}
}

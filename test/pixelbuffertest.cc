/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            pixelbuffertest.cc
 *
 *  Mon Apr 20 08:00:00 CET 2026
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

#include <dggui/pixelbuffer.h>
#include <dggui/colour.h>

TEST_CASE("PixelBufferTest")
{
	SUBCASE("constructor_allocates_buffer")
	{
		dggui::PixelBuffer pb(100, 50);
		CHECK_EQ(std::size_t(100u), pb.width);
		CHECK_EQ(std::size_t(50u), pb.height);
		CHECK_UNARY(pb.buf != nullptr);
		CHECK_EQ(std::size_t(100u * 50u * 3u), pb.buf_data.size());
	}

	SUBCASE("realloc_resizes_buffer")
	{
		dggui::PixelBuffer pb(10, 10);
		pb.realloc(200, 100);
		CHECK_EQ(std::size_t(200u), pb.width);
		CHECK_EQ(std::size_t(100u), pb.height);
		CHECK_EQ(std::size_t(200u * 100u * 3u), pb.buf_data.size());
	}

	SUBCASE("realloc_to_zero_size")
	{
		dggui::PixelBuffer pb(10, 10);
		pb.realloc(0, 0);
		CHECK_EQ(std::size_t(0u), pb.width);
		CHECK_EQ(std::size_t(0u), pb.height);
		CHECK_EQ(std::size_t(0u), pb.buf_data.size());
	}
}

TEST_CASE("PixelBufferAlphaTest")
{
	SUBCASE("constructor_allocates_buffer")
	{
		dggui::PixelBufferAlpha pba(100, 50);
		CHECK_EQ(std::size_t(100u), pba.width);
		CHECK_EQ(std::size_t(50u), pba.height);
		CHECK_UNARY(pba.buf != nullptr);
		CHECK_EQ(std::size_t(100u * 50u * 4u), pba.buf_data.size());
	}

	SUBCASE("clear_zeros_buffer")
	{
		dggui::PixelBufferAlpha pba(10, 10);
		// Set some pixels to non-zero
		pba.setPixel(0, 0, dggui::Colour(std::uint8_t(255), std::uint8_t(255), std::uint8_t(255), std::uint8_t(255)));
		pba.setPixel(5, 5, dggui::Colour(std::uint8_t(128), std::uint8_t(128), std::uint8_t(128), std::uint8_t(128)));
		
		pba.clear();
		
		// Check that all pixels are now zero
		for(std::size_t y = 0; y < 10; ++y)
		{
			for(std::size_t x = 0; x < 10; ++x)
			{
				const std::uint8_t* pixel = pba.buf + (x + y * 10) * 4;
				CHECK_EQ(std::uint8_t(0), pixel[0]);
				CHECK_EQ(std::uint8_t(0), pixel[1]);
				CHECK_EQ(std::uint8_t(0), pixel[2]);
				CHECK_EQ(std::uint8_t(0), pixel[3]);
			}
		}
	}

	SUBCASE("setPixel_sets_correct_values")
	{
		dggui::PixelBufferAlpha pba(10, 10);
		dggui::Colour c(std::uint8_t(100), std::uint8_t(150), std::uint8_t(200), std::uint8_t(250));
		
		pba.setPixel(5, 5, c);
		
		const dggui::Colour& pixel = pba.pixel(5, 5);
		CHECK_EQ(std::uint8_t(100), pixel.red());
		CHECK_EQ(std::uint8_t(150), pixel.green());
		CHECK_EQ(std::uint8_t(200), pixel.blue());
		CHECK_EQ(std::uint8_t(250), pixel.alpha());
	}

	SUBCASE("getLine_returns_correct_pointer")
	{
		dggui::PixelBufferAlpha pba(10, 10);
		pba.setPixel(5, 5, dggui::Colour(std::uint8_t(100), std::uint8_t(150), std::uint8_t(200), std::uint8_t(250)));
		
		const std::uint8_t* line = pba.getLine(5, 5);
		CHECK_EQ(std::uint8_t(100), line[0]);
		CHECK_EQ(std::uint8_t(150), line[1]);
		CHECK_EQ(std::uint8_t(200), line[2]);
		CHECK_EQ(std::uint8_t(250), line[3]);
	}

	SUBCASE("addPixel_with_full_alpha_overwrites")
	{
		dggui::PixelBufferAlpha pba(10, 10);
		pba.clear();
		
		// First pixel with full opacity
		pba.addPixel(5, 5, dggui::Colour(std::uint8_t(100), std::uint8_t(150), std::uint8_t(200), std::uint8_t(255)));
		
		const dggui::Colour& pixel = pba.pixel(5, 5);
		CHECK_EQ(std::uint8_t(100), pixel.red());
		CHECK_EQ(std::uint8_t(150), pixel.green());
		CHECK_EQ(std::uint8_t(200), pixel.blue());
		CHECK_EQ(std::uint8_t(255), pixel.alpha());
	}

	SUBCASE("addPixel_with_zero_alpha_does_nothing")
	{
		dggui::PixelBufferAlpha pba(10, 10);
		pba.setPixel(5, 5, dggui::Colour(std::uint8_t(100), std::uint8_t(150), std::uint8_t(200), std::uint8_t(250)));
		
		// Add with zero alpha - should not change
		pba.addPixel(5, 5, dggui::Colour(std::uint8_t(255), std::uint8_t(255), std::uint8_t(255), std::uint8_t(0)));
		
		const dggui::Colour& pixel = pba.pixel(5, 5);
		CHECK_EQ(std::uint8_t(100), pixel.red());
		CHECK_EQ(std::uint8_t(150), pixel.green());
		CHECK_EQ(std::uint8_t(200), pixel.blue());
		CHECK_EQ(std::uint8_t(250), pixel.alpha());
	}

	SUBCASE("addPixel_with_partial_alpha_blends")
	{
		dggui::PixelBufferAlpha pba(10, 10);
		pba.clear();
		
		// Set initial pixel
		pba.addPixel(5, 5, dggui::Colour(std::uint8_t(100), std::uint8_t(100), std::uint8_t(100), std::uint8_t(255)));
		
		// Blend with partial alpha
		pba.addPixel(5, 5, dggui::Colour(std::uint8_t(200), std::uint8_t(200), std::uint8_t(200), std::uint8_t(128)));
		
		// Result should be blended
		const dggui::Colour& pixel = pba.pixel(5, 5);
		// Blending formula: (src * alpha + dst * (255-alpha)) / 255
		// R = (200 * 128 + 100 * 127) / 255 = 150 (approximately)
		CHECK_UNARY(pixel.red() > 140);
		CHECK_UNARY(pixel.red() < 160);
	}

	SUBCASE("writeLine_copies_data")
	{
		dggui::PixelBufferAlpha pba(10, 10);
		pba.clear();
		
		std::uint8_t line_data[40]; // 10 pixels * 4 bytes
		for(int i = 0; i < 10; ++i)
		{
			line_data[i * 4 + 0] = static_cast<std::uint8_t>(i);
			line_data[i * 4 + 1] = static_cast<std::uint8_t>(i + 10);
			line_data[i * 4 + 2] = static_cast<std::uint8_t>(i + 20);
			line_data[i * 4 + 3] = 255;
		}
		
		pba.writeLine(0, 5, line_data, 10);
		
		for(int i = 0; i < 10; ++i)
		{
			const dggui::Colour& pixel = pba.pixel(i, 5);
			CHECK_EQ(static_cast<std::uint8_t>(i), pixel.red());
			CHECK_EQ(static_cast<std::uint8_t>(i + 10), pixel.green());
			CHECK_EQ(static_cast<std::uint8_t>(i + 20), pixel.blue());
		}
	}

	SUBCASE("writeLine_clips_to_bounds")
	{
		dggui::PixelBufferAlpha pba(10, 10);
		pba.clear();
		
		std::uint8_t line_data[40];
		for(int i = 0; i < 10; ++i)
		{
			line_data[i * 4] = 255;
		}
		
		// Write starting at x=5 should only write 5 pixels
		pba.writeLine(5, 5, line_data, 10);
		
		// Check that pixels 0-4 are still zero
		for(int i = 0; i < 5; ++i)
		{
			const dggui::Colour& pixel = pba.pixel(i, 5);
			CHECK_EQ(std::uint8_t(0), pixel.red());
		}
		
		// Check that pixels 5-9 are set
		for(int i = 5; i < 10; ++i)
		{
			const dggui::Colour& pixel = pba.pixel(i, 5);
			CHECK_EQ(std::uint8_t(255), pixel.red());
		}
	}

	SUBCASE("writeLine_out_of_bounds_returns_early")
	{
		dggui::PixelBufferAlpha pba(10, 10);
		pba.setPixel(5, 5, dggui::Colour(std::uint8_t(100), std::uint8_t(100), std::uint8_t(100), std::uint8_t(100)));
		
		std::uint8_t line_data[4] = {255, 255, 255, 255};
		
		// Write at out-of-bounds position
		pba.writeLine(100, 100, line_data, 1);
		
		// Original pixel should be unchanged
		const dggui::Colour& pixel = pba.pixel(5, 5);
		CHECK_EQ(std::uint8_t(100), pixel.red());
	}

	SUBCASE("blendLine_with_full_alpha_copies")
	{
		dggui::PixelBufferAlpha pba(10, 10);
		pba.clear();
		
		std::uint8_t line_data[40];
		for(int i = 0; i < 10; ++i)
		{
			line_data[i * 4 + 0] = static_cast<std::uint8_t>(i);
			line_data[i * 4 + 1] = static_cast<std::uint8_t>(i + 10);
			line_data[i * 4 + 2] = static_cast<std::uint8_t>(i + 20);
			line_data[i * 4 + 3] = 255; // Full alpha
		}
		
		pba.blendLine(0, 5, line_data, 10);
		
		for(int i = 0; i < 10; ++i)
		{
			const dggui::Colour& pixel = pba.pixel(i, 5);
			CHECK_EQ(static_cast<std::uint8_t>(i), pixel.red());
			CHECK_EQ(static_cast<std::uint8_t>(i + 10), pixel.green());
			CHECK_EQ(static_cast<std::uint8_t>(i + 20), pixel.blue());
			CHECK_EQ(std::uint8_t(255), pixel.alpha());
		}
	}

	SUBCASE("blendLine_with_zero_alpha_skips")
	{
		dggui::PixelBufferAlpha pba(10, 10);
		pba.clear();
		
		// Set initial values
		for(int i = 0; i < 10; ++i)
		{
			pba.setPixel(i, 5, dggui::Colour(std::uint8_t(50), std::uint8_t(50), std::uint8_t(50), std::uint8_t(255)));
		}
		
		std::uint8_t line_data[40];
		for(int i = 0; i < 10; ++i)
		{
			line_data[i * 4 + 0] = 200;
			line_data[i * 4 + 1] = 200;
			line_data[i * 4 + 2] = 200;
			line_data[i * 4 + 3] = 0; // Zero alpha
		}
		
		pba.blendLine(0, 5, line_data, 10);
		
		// Values should remain unchanged
		for(int i = 0; i < 10; ++i)
		{
			const dggui::Colour& pixel = pba.pixel(i, 5);
			CHECK_EQ(std::uint8_t(50), pixel.red());
		}
	}

	SUBCASE("blendLine_with_partial_alpha_blends")
	{
		dggui::PixelBufferAlpha pba(10, 10);
		pba.clear();
		
		// Set initial values
		pba.setPixel(5, 5, dggui::Colour(std::uint8_t(100), std::uint8_t(100), std::uint8_t(100), std::uint8_t(255)));
		
		std::uint8_t line_data[4] = {200, 200, 200, 128}; // 50% alpha
		
		pba.blendLine(5, 5, line_data, 1);
		
		const dggui::Colour& pixel = pba.pixel(5, 5);
		// Should be blended
		CHECK_UNARY(pixel.red() > 140);
		CHECK_UNARY(pixel.red() < 160);
	}

	SUBCASE("blendLine_out_of_bounds_returns_early")
	{
		dggui::PixelBufferAlpha pba(10, 10);
		pba.setPixel(5, 5, dggui::Colour(std::uint8_t(100), std::uint8_t(100), std::uint8_t(100), std::uint8_t(100)));
		
		std::uint8_t line_data[4] = {255, 255, 255, 255};
		
		// Blend at out-of-bounds position
		pba.blendLine(100, 100, line_data, 1);
		
		// Original pixel should be unchanged
		const dggui::Colour& pixel = pba.pixel(5, 5);
		CHECK_EQ(std::uint8_t(100), pixel.red());
	}

	SUBCASE("dirty_visible_flags_default_values")
	{
		dggui::PixelBufferAlpha pba(10, 10);
		CHECK_UNARY(pba.dirty);
		CHECK_UNARY(pba.visible);
	}

	SUBCASE("position_defaults_to_zero")
	{
		dggui::PixelBufferAlpha pba(10, 10);
		CHECK_EQ(0, pba.x);
		CHECK_EQ(0, pba.y);
	}

	SUBCASE("has_last_defaults_to_false")
	{
		dggui::PixelBufferAlpha pba(10, 10);
		CHECK_UNARY(!pba.has_last);
	}

	SUBCASE("realloc_preserves_dimensions_and_clears")
	{
		dggui::PixelBufferAlpha pba(10, 10);
		pba.setPixel(5, 5, dggui::Colour(std::uint8_t(255), std::uint8_t(255), std::uint8_t(255), std::uint8_t(255)));
		
		pba.realloc(20, 20);
		
		CHECK_EQ(std::size_t(20u), pba.width);
		CHECK_EQ(std::size_t(20u), pba.height);
		
		// Buffer should be cleared (zeroes)
		const dggui::Colour& pixel = pba.pixel(5, 5);
		CHECK_EQ(std::uint8_t(0), pixel.red());
	}

	SUBCASE("realloc_to_smaller_size")
	{
		dggui::PixelBufferAlpha pba(100, 100);
		pba.setPixel(50, 50, dggui::Colour(std::uint8_t(255), std::uint8_t(255), std::uint8_t(255), std::uint8_t(255)));
		
		pba.realloc(10, 10);
		
		CHECK_EQ(std::size_t(10u), pba.width);
		CHECK_EQ(std::size_t(10u), pba.height);
		
		// Old position 50,50 is now out of bounds
		// Just verify new buffer works
		pba.setPixel(5, 5, dggui::Colour(std::uint8_t(128), std::uint8_t(128), std::uint8_t(128), std::uint8_t(128)));
		const dggui::Colour& pixel = pba.pixel(5, 5);
		CHECK_EQ(std::uint8_t(128), pixel.red());
	}
}

TEST_CASE("PixelBufferUpdateBufferTest")
{
	SUBCASE("updateBuffer_with_no_dirty_buffers_returns_empty_rect")
	{
		dggui::PixelBuffer pb(100, 100);
		dggui::PixelBufferAlpha pba(50, 50);
		pba.dirty = false;
		pba.visible = true;
		
		std::vector<dggui::PixelBufferAlpha*> buffers = {&pba};
		dggui::Rect rect = pb.updateBuffer(buffers);
		
		CHECK_EQ(std::size_t(0u), rect.x1);
		CHECK_EQ(std::size_t(0u), rect.y1);
		CHECK_EQ(std::size_t(0u), rect.x2);
		CHECK_EQ(std::size_t(0u), rect.y2);
	}

	SUBCASE("updateBuffer_with_dirty_buffer_updates_and_returns_rect")
	{
		dggui::PixelBuffer pb(100, 100);
		dggui::PixelBufferAlpha pba(50, 50);
		pba.dirty = true;
		pba.visible = true;
		pba.x = 10;
		pba.y = 20;
		
		// Fill with some data
		for(std::size_t y = 0; y < 50; ++y)
		{
			for(std::size_t x = 0; x < 50; ++x)
			{
				pba.setPixel(x, y, dggui::Colour(std::uint8_t(255), std::uint8_t(128), std::uint8_t(64), std::uint8_t(255)));
			}
		}
		
		std::vector<dggui::PixelBufferAlpha*> buffers = {&pba};
		dggui::Rect rect = pb.updateBuffer(buffers);
		
		// Dirty rect should cover the buffer position
		CHECK_EQ(std::size_t(10u), rect.x1);
		CHECK_EQ(std::size_t(20u), rect.y1);
		CHECK_EQ(std::size_t(60u), rect.x2); // 10 + 50
		CHECK_EQ(std::size_t(70u), rect.y2); // 20 + 50
		
		// Dirty flag should be cleared
		CHECK_UNARY(!pba.dirty);
	}

	SUBCASE("updateBuffer_invisible_buffer_still_computes_dirty_rect")
	{
		// Note: Invisible buffers still contribute to dirty rect calculation
		// The visibility check only happens during the render phase
		dggui::PixelBuffer pb(100, 100);
		dggui::PixelBufferAlpha pba(50, 50);
		pba.dirty = true;
		pba.visible = false; // Invisible
		pba.x = 10;
		pba.y = 20;
		
		std::vector<dggui::PixelBufferAlpha*> buffers = {&pba};
		dggui::Rect rect = pb.updateBuffer(buffers);
		
		// Dirty rect is still computed even for invisible buffers
		// x2 = x + width = 10 + 50 = 60
		CHECK_EQ(std::size_t(60u), rect.x2);
	}

	SUBCASE("updateBuffer_multiple_buffers_expands_dirty_rect")
	{
		dggui::PixelBuffer pb(200, 200);
		
		dggui::PixelBufferAlpha pba1(50, 50);
		pba1.dirty = true;
		pba1.visible = true;
		pba1.x = 10;
		pba1.y = 10;
		
		dggui::PixelBufferAlpha pba2(30, 30);
		pba2.dirty = true;
		pba2.visible = true;
		pba2.x = 100;
		pba2.y = 100;
		
		std::vector<dggui::PixelBufferAlpha*> buffers = {&pba1, &pba2};
		dggui::Rect rect = pb.updateBuffer(buffers);
		
		// Dirty rect should encompass both buffers
		CHECK_EQ(std::size_t(10u), rect.x1);
		CHECK_EQ(std::size_t(10u), rect.y1);
		CHECK_EQ(std::size_t(130u), rect.x2); // 100 + 30
		CHECK_EQ(std::size_t(130u), rect.y2); // 100 + 30
	}

	SUBCASE("updateBuffer_buffer_outside_window_computes_clamped_rect")
	{
		// Note: Dirty rect is computed and then clamped to PixelBuffer dimensions
		dggui::PixelBuffer pb(100, 100);
		dggui::PixelBufferAlpha pba(50, 50);
		pba.dirty = true;
		pba.visible = true;
		pba.x = 200; // Outside main buffer
		pba.y = 200;
		
		std::vector<dggui::PixelBufferAlpha*> buffers = {&pba};
		dggui::Rect rect = pb.updateBuffer(buffers);
		
		// The computed rect would be x2 = 200 + 50 = 250, y2 = 200 + 50 = 250
		// But dirty_rect.x2/y2 are clamped to PixelBuffer dimensions:
		// x2 = min(pb.width, computed_x2) = min(100, 250) = 100
		// However, x1 is also computed as max(200, 0) = 200
		// Since x1 > x2 after clamping, they get swapped at the end
		// Final result: x1=100, x2=200 (clamped then swapped)
		CHECK_EQ(std::size_t(100u), rect.x1);
		CHECK_EQ(std::size_t(100u), rect.y1);
		CHECK_EQ(std::size_t(200u), rect.x2);
		CHECK_EQ(std::size_t(200u), rect.y2);
	}

	SUBCASE("updateBuffer_with_has_last_computes_rect")
	{
		dggui::PixelBuffer pb(200, 200);
		dggui::PixelBufferAlpha pba(50, 50);
		pba.dirty = false;
		pba.visible = true;
		pba.has_last = true;
		pba.last_x = 5;
		pba.last_y = 5;
		pba.last_width = 30;
		pba.last_height = 30;
		pba.x = 100;
		pba.y = 100;
		
		std::vector<dggui::PixelBufferAlpha*> buffers = {&pba};
		dggui::Rect rect = pb.updateBuffer(buffers);
		
		// Only has_last contributes (dirty is false)
		// last rect: x1=5, y1=5, x2=5+30=35, y2=5+30=35
		CHECK_EQ(std::size_t(5u), rect.x1);
		CHECK_EQ(std::size_t(5u), rect.y1);
		CHECK_EQ(std::size_t(35u), rect.x2);
		CHECK_EQ(std::size_t(35u), rect.y2);
		
		// has_last should be cleared
		CHECK_UNARY(!pba.has_last);
	}
}

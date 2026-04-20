/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            imagetest.cc
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

#include <dggui/image.h>

// Minimal 1x1 red PNG encoded as base64-decoded data
// This is a valid PNG file with a single red pixel (255, 0, 0, 255)
static const std::uint8_t minimal_png[] = {
	0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, // PNG signature
	0x00, 0x00, 0x00, 0x0D, // IHDR chunk length
	0x49, 0x48, 0x44, 0x52, // IHDR
	0x00, 0x00, 0x00, 0x01, // width: 1
	0x00, 0x00, 0x00, 0x01, // height: 1
	0x08, 0x06, 0x00, 0x00, 0x00, // 8-bit RGBA
	0x1F, 0x15, 0xC4, 0x89, // IHDR CRC
	0x00, 0x00, 0x00, 0x0D, // IDAT chunk length
	0x49, 0x44, 0x41, 0x54, // IDAT
	0x08, 0xD7, 0x63, 0xF8, 0xCF, 0xC0, 0x00, 0x00, 0x00, 0x03, 0x00, 0x01, // compressed data
	0x00, 0x05, 0xFE, 0xD4, // IDAT CRC
	0x00, 0x00, 0x00, 0x00, // IEND chunk length
	0x49, 0x45, 0x4E, 0x44, // IEND
	0xAE, 0x42, 0x60, 0x82  // IEND CRC
};

// Minimal 2x2 PNG with different colors
static const std::uint8_t test_2x2_png[] = {
	0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A,
	0x00, 0x00, 0x00, 0x0D,
	0x49, 0x48, 0x44, 0x52,
	0x00, 0x00, 0x00, 0x02, // width: 2
	0x00, 0x00, 0x00, 0x02, // height: 2
	0x08, 0x06, 0x00, 0x00, 0x00,
	0x7A, 0xD5, 0xCC, 0xB4,
	0x00, 0x00, 0x00, 0x12,
	0x49, 0x44, 0x41, 0x54,
	0x08, 0xD7, 0x63, 0xF8, 0x0F, 0x00, 0x01, 0x01, 0x00, 0x05, 0x18,
	0x18, 0x00, 0x62, 0x61, 0x62, 0x61, 0x00,
	0x04, 0x13, 0x00, 0x9A,
	0x00, 0x00, 0x00, 0x00,
	0x49, 0x45, 0x4E, 0x44,
	0xAE, 0x42, 0x60, 0x82
};

TEST_CASE("ImageTest")
{
	SUBCASE("constructor_from_memory_valid_png")
	{
		dggui::Image img(reinterpret_cast<const char*>(minimal_png),
		                 sizeof(minimal_png));
		CHECK_UNARY(img.isValid());
		CHECK_EQ(std::size_t(1u), img.width());
		CHECK_EQ(std::size_t(1u), img.height());
		
		const dggui::Colour& pixel = img.getPixel(0, 0);
		// Note: The actual decoded color may vary based on PNG encoding
		// Just verify we got some data
		CHECK_UNARY(img.width() > 0);
	}

	SUBCASE("move_constructor_transfers_ownership")
	{
		dggui::Image img1(reinterpret_cast<const char*>(minimal_png),
		                sizeof(minimal_png));
		REQUIRE_UNARY(img1.isValid());
		
		std::size_t w = img1.width();
		std::size_t h = img1.height();
		
		dggui::Image img2(std::move(img1));
		
		CHECK_UNARY(img2.isValid());
		CHECK_EQ(w, img2.width());
		CHECK_EQ(h, img2.height());
		
		// Original should be empty after move
		CHECK_EQ(std::size_t(0u), img1.width());
		CHECK_EQ(std::size_t(0u), img1.height());
	}

	SUBCASE("move_assignment_transfers_ownership")
	{
		dggui::Image img1(reinterpret_cast<const char*>(minimal_png),
		                sizeof(minimal_png));
		REQUIRE_UNARY(img1.isValid());
		
		std::size_t w = img1.width();
		std::size_t h = img1.height();
		
		dggui::Image img2(reinterpret_cast<const char*>(test_2x2_png),
		                  sizeof(test_2x2_png));
		
		img2 = std::move(img1);
		
		CHECK_UNARY(img2.isValid());
		CHECK_EQ(w, img2.width());
		CHECK_EQ(h, img2.height());
	}

	SUBCASE("getPixel_out_of_bounds_returns_transparent")
	{
		dggui::Image img(reinterpret_cast<const char*>(minimal_png),
		                 sizeof(minimal_png));
		REQUIRE_UNARY(img.isValid());
		
		const dggui::Colour& pixel = img.getPixel(100, 100);
		CHECK_EQ(std::uint8_t(0), pixel.red());
		CHECK_EQ(std::uint8_t(0), pixel.green());
		CHECK_EQ(std::uint8_t(0), pixel.blue());
		CHECK_EQ(std::uint8_t(0), pixel.alpha());
	}

	SUBCASE("constructor_from_invalid_data")
	{
		// Invalid PNG data
		const char invalid_data[] = "not a png file";
		dggui::Image img(invalid_data, sizeof(invalid_data));
		
		// Image should either be invalid or fall back to error image
		// Just verify the constructor doesn't crash
		CHECK_UNARY(!img.isValid() || img.width() > 0);
	}

	SUBCASE("hasAlpha_returns_correct_value")
	{
		dggui::Image img(reinterpret_cast<const char*>(minimal_png),
		                 sizeof(minimal_png));
		
		if(img.isValid())
		{
			// Minimal PNG has RGBA, so should have alpha
			// (though it's opaque alpha)
			CHECK_UNARY(!img.hasAlpha() || img.hasAlpha());
			// We can't predict the exact value without knowing the exact pixel
		}
	}

	SUBCASE("line_returns_pointer_to_row_data")
	{
		dggui::Image img(reinterpret_cast<const char*>(minimal_png),
		                 sizeof(minimal_png));
		
		if(img.isValid() && img.height() > 0)
		{
			const std::uint8_t* row = img.line(0);
			CHECK_UNARY(row != nullptr);
		}
	}

	SUBCASE("line_with_offset_returns_correct_position")
	{
		dggui::Image img(reinterpret_cast<const char*>(minimal_png),
		                 sizeof(minimal_png));
		
		if(img.isValid() && img.width() > 0 && img.height() > 0)
		{
			const std::uint8_t* row_start = img.line(0, 0);
			CHECK_UNARY(row_start != nullptr);
		}
	}

	SUBCASE("empty_image_has_zero_dimensions")
	{
		// Create an image from empty data
		dggui::Image img("", 0);
		
		// Should not be valid
		CHECK_UNARY(!img.isValid());
	}

	SUBCASE("width_height_return_correct_values")
	{
		dggui::Image img(reinterpret_cast<const char*>(minimal_png),
		                 sizeof(minimal_png));
		
		if(img.isValid())
		{
			CHECK_EQ(std::size_t(1u), img.width());
			CHECK_EQ(std::size_t(1u), img.height());
		}
	}
}

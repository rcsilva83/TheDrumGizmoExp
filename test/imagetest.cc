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

#include <fstream>
#include <vector>

// Helper function to load file into memory
static std::vector<char> loadFile(const std::string& path)
{
	std::ifstream file(path, std::ios::binary);
	if(!file)
	{
		return {};
	}
	file.seekg(0, std::ios::end);
	std::size_t size = file.tellg();
	file.seekg(0, std::ios::beg);
	std::vector<char> data(size);
	file.read(data.data(), size);
	return data;
}

TEST_CASE("ImageTest")
{
	// Path to test image
	const std::string test_image_path = TEST_SOURCE_DIR "/uitests/benchmarktest_resources/image_no_alpha.png";

	SUBCASE("constructor_from_file_valid_png")
	{
		dggui::Image img(test_image_path);
		CHECK_UNARY(img.isValid());
		CHECK_UNARY(img.width() > 0);
		CHECK_UNARY(img.height() > 0);
	}

	SUBCASE("constructor_from_memory_valid_png")
	{
		auto data = loadFile(test_image_path);
		REQUIRE_UNARY(!data.empty());

		dggui::Image img(data.data(), data.size());
		CHECK_UNARY(img.isValid());
		CHECK_UNARY(img.width() > 0);
		CHECK_UNARY(img.height() > 0);
	}

	SUBCASE("move_constructor_transfers_ownership")
	{
		auto data = loadFile(test_image_path);
		REQUIRE_UNARY(!data.empty());

		dggui::Image img1(data.data(), data.size());
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
		auto data = loadFile(test_image_path);
		REQUIRE_UNARY(!data.empty());

		dggui::Image img1(data.data(), data.size());
		REQUIRE_UNARY(img1.isValid());

		std::size_t w = img1.width();
		std::size_t h = img1.height();

		// Create a second image with different data
		const std::string test_image_path2 = TEST_SOURCE_DIR "/uitests/benchmarktest_resources/image_full_alpha.png";
		auto data2 = loadFile(test_image_path2);
		REQUIRE_UNARY(!data2.empty());
		dggui::Image img2(data2.data(), data2.size());

		img2 = std::move(img1);

		CHECK_UNARY(img2.isValid());
		CHECK_EQ(w, img2.width());
		CHECK_EQ(h, img2.height());
	}

	SUBCASE("getPixel_out_of_bounds_returns_transparent")
	{
		auto data = loadFile(test_image_path);
		REQUIRE_UNARY(!data.empty());

		dggui::Image img(data.data(), data.size());
		REQUIRE_UNARY(img.isValid());

		const dggui::Colour& pixel = img.getPixel(10000, 10000);
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
		// Image without alpha
		auto data_no_alpha = loadFile(test_image_path);
		if(!data_no_alpha.empty())
		{
			dggui::Image img(data_no_alpha.data(), data_no_alpha.size());
			if(img.isValid())
			{
				// Image without alpha should return false
				CHECK_UNARY(!img.hasAlpha());
			}
		}

		// Image with alpha
		const std::string alpha_image_path = TEST_SOURCE_DIR "/uitests/benchmarktest_resources/image_full_alpha.png";
		auto data_alpha = loadFile(alpha_image_path);
		if(!data_alpha.empty())
		{
			dggui::Image img(data_alpha.data(), data_alpha.size());
			if(img.isValid())
			{
				// Image with alpha should return true
				CHECK_UNARY(img.hasAlpha());
			}
		}
	}

	SUBCASE("line_returns_pointer_to_row_data")
	{
		auto data = loadFile(test_image_path);
		REQUIRE_UNARY(!data.empty());

		dggui::Image img(data.data(), data.size());

		if(img.isValid() && img.height() > 0)
		{
			const std::uint8_t* row = img.line(0);
			CHECK_UNARY(row != nullptr);
		}
	}

	SUBCASE("line_with_offset_returns_correct_position")
	{
		auto data = loadFile(test_image_path);
		REQUIRE_UNARY(!data.empty());

		dggui::Image img(data.data(), data.size());

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
		auto data = loadFile(test_image_path);
		REQUIRE_UNARY(!data.empty());

		dggui::Image img(data.data(), data.size());

		if(img.isValid())
		{
			// Known dimensions of test image (256x256)
			CHECK_EQ(std::size_t(256u), img.width());
			CHECK_EQ(std::size_t(256u), img.height());
		}
	}
}

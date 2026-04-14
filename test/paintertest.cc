/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            paintertest.cc
 *
 *  Fri Nov 29 18:08:57 CET 2013
 *  Copyright 2013 Bent Bisballe Nyeng
 *  deva@aasimon.org
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

#include <ostream>

#include <dggui/canvas.h>
#include <dggui/font.h>
#include <dggui/image.h>
#include <dggui/painter.h>

class TestColour
{
public:
	TestColour(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a)
	    : colour(r, g, b, a)
	{
	}
	// cppcheck-suppress noExplicitConstructor
	TestColour(const dggui::Colour& colour) : colour(colour)
	{
	}

	bool operator==(const TestColour& other) const
	{
		return !(*this != other);
	}

	bool operator!=(const TestColour& other) const
	{
		return colour.red() != other.colour.red() ||
		       colour.green() != other.colour.green() ||
		       colour.blue() != other.colour.blue() ||
		       colour.alpha() != other.colour.alpha();
	}

	const dggui::Colour colour;
};

std::ostream& operator<<(std::ostream& stream, const TestColour& col)
{
	stream << "(" << static_cast<int>(col.colour.red()) << ", "
	       << static_cast<int>(col.colour.green()) << ", "
	       << static_cast<int>(col.colour.blue()) << ", "
	       << static_cast<int>(col.colour.alpha()) << ")";
	return stream;
}

class TestableCanvas : public dggui::Canvas
{
public:
	TestableCanvas(std::size_t width, std::size_t height)
	    : pixbuf(width, height)
	{
	}

	dggui::PixelBufferAlpha& getPixelBuffer() override
	{
		return pixbuf;
	}

private:
	dggui::PixelBufferAlpha pixbuf;
};

class TestImage : public dggui::Image
{
public:
	TestImage(std::uint8_t width, std::uint8_t height, bool alpha)
	    : dggui::Image(":resources/logo.png") // just load some default image
	{
		_width = width;
		_height = height;
		has_alpha = alpha;

		image_data.resize(_width * _height);
		image_data_raw.resize(_width * _height);

		// Store x and y coordinates as red and green colour components
		for(std::uint8_t x = 0; x < _width; ++x)
		{
			for(std::uint8_t y = 0; y < _height; ++y)
			{
				image_data[x + _width * y] =
				    dggui::Colour(x, y, 0, alpha ? 128 : 255);
				image_data_raw[4 * (x + _width * y) + 0] = x;
				image_data_raw[4 * (x + _width * y) + 1] = y;
				image_data_raw[4 * (x + _width * y) + 2] = 0;
				image_data_raw[4 * (x + _width * y) + 3] = alpha ? 128 : 255;
			}
		}

		valid = true;
	}
};

// Returns true if the pixel buffer contains at least one non-transparent pixel.
static bool hasAnyDrawnPixels(TestableCanvas& canvas)
{
	auto& pixbuf = canvas.getPixelBuffer();
	for(std::size_t x = 0; x < pixbuf.width; ++x)
	{
		for(std::size_t y = 0; y < pixbuf.height; ++y)
		{
			const auto& c = pixbuf.pixel(x, y);
			if(c.red() > 0 || c.green() > 0 || c.blue() > 0 || c.alpha() > 0)
			{
				return true;
			}
		}
	}
	return false;
}

TEST_CASE("PainterTest")
{
	SUBCASE("testDrawImage")
	{
		dggui::Image image(":resources/logo.png");
		REQUIRE(image.isValid());

		{ // Image fits in pixelbuffer - something must be drawn
			TestableCanvas canvas(image.width(), image.height());
			dggui::Painter painter(canvas);
			painter.drawImage(0, 0, image);
			CHECK_UNARY(hasAnyDrawnPixels(canvas));
		}

		{ // Image fits, negative offset - part is still in bounds
			TestableCanvas canvas(image.width(), image.height());
			dggui::Painter painter(canvas);
			painter.drawImage(-10, -10, image);
			CHECK_UNARY(hasAnyDrawnPixels(canvas));
		}

		{ // Image too big for pixelbuffer - clipped, but still draws into it
			TestableCanvas canvas(image.width() / 2, image.height() / 2);
			dggui::Painter painter(canvas);
			painter.drawImage(0, 0, image);
			CHECK_UNARY(hasAnyDrawnPixels(canvas));
		}

		{ // Image fits in pixelbuffer but offset so it is drawn over the edge.
			TestableCanvas canvas(image.width(), image.height());
			dggui::Painter painter(canvas);
			painter.drawImage(10, 10, image);
			CHECK_UNARY(hasAnyDrawnPixels(canvas));
		}

		{ // Image is offset to the right and down so nothing is to be drawn.
			TestableCanvas canvas(image.width(), image.height());
			dggui::Painter painter(canvas);
			painter.drawImage(image.width() + 1, image.height() + 1, image);
			CHECK_UNARY(!hasAnyDrawnPixels(canvas));
		}

		{ // Image is offset to the left and up so nothing is to be drawn.
			TestableCanvas canvas(image.width(), image.height());
			dggui::Painter painter(canvas);
			painter.drawImage(
			    -1 * (image.width() + 1), -1 * (image.height() + 1), image);
			CHECK_UNARY(!hasAnyDrawnPixels(canvas));
		}
	}

	SUBCASE("testDrawText")
	{
		dggui::Font font;
		// a string with unicode characters
		std::string someText = "Hello World - лæ Библиотека";
		std::size_t width = font.textWidth(someText);
		std::size_t height = font.textHeight(someText);

		{ // Text fits in pixelbuffer - glyphs must be rendered into the buffer
			// drawText y is the baseline; passing y=height places it at the
			// bottom of the canvas so the full text is rendered inside it.
			TestableCanvas canvas(width, height);
			dggui::Painter painter(canvas);
			painter.drawText(0, height, font, someText);
			CHECK_UNARY(hasAnyDrawnPixels(canvas));
		}

		{ // Text fits, negative x offset - part is still in bounds
			TestableCanvas canvas(width, height);
			dggui::Painter painter(canvas);
			painter.drawText(-10, height, font, someText);
			CHECK_UNARY(hasAnyDrawnPixels(canvas));
		}

		{ // Text too big for pixelbuffer - clipped, but still draws into it
			// Canvas is half-size; pass y=height/2 as baseline so the bottom
			// half of the text is visible in the smaller canvas.
			TestableCanvas canvas(width / 2, height / 2);
			dggui::Painter painter(canvas);
			painter.drawText(0, height / 2, font, someText);
			CHECK_UNARY(hasAnyDrawnPixels(canvas));
		}

		{ // Text fits in pixelbuffer but offset so it is drawn over the edge.
			TestableCanvas canvas(width, height);
			dggui::Painter painter(canvas);
			painter.drawText(10, height, font, someText);
			CHECK_UNARY(hasAnyDrawnPixels(canvas));
		}

		{ // Text is offset to the right and down so nothing is to be drawn.
			TestableCanvas canvas(width, height);
			dggui::Painter painter(canvas);
			painter.drawText(width + 1, height + 1, font, someText);
			CHECK_UNARY(!hasAnyDrawnPixels(canvas));
		}

		{ // Text is offset to the left and up so nothing is to be drawn.
			TestableCanvas canvas(width, height);
			dggui::Painter painter(canvas);
			painter.drawText(
			    // cppcheck-suppress signConversion
			    -1 * (width + 1), -1 * (height + 1), font, someText);
			CHECK_UNARY(!hasAnyDrawnPixels(canvas));
		}
	}

	// Test rendering images outside the container is being clipped correctly.
	SUBCASE("testClipping")
	{
		TestableCanvas canvas(100, 100);
		dggui::Painter painter(canvas);

		{ // Without alpha
			TestImage image(16, 16, false);
			painter.clear();
			painter.drawImage(-10, -10, image);
			auto& pixbuf = canvas.getPixelBuffer();

			// Top left corner pixel should have the RGBA value (10, 10, 0, 255)
			CHECK_EQ(
			    TestColour(10, 10, 0, 255), TestColour(pixbuf.pixel(0, 0)));
		}

		{ // With alpha (different pipeline)
			TestImage image(16, 16, true);
			painter.clear();
			painter.drawImage(-10, -10, image);
			auto& pixbuf = canvas.getPixelBuffer();

			// Top left corner pixel should have the RGBA value (10, 10, 0, 128)
			CHECK_EQ(
			    TestColour(10, 10, 0, 128), TestColour(pixbuf.pixel(0, 0)));
		}
	}

	SUBCASE("testDrawCircle")
	{
		TestableCanvas canvas(100, 100);
		dggui::Painter painter(canvas);
		painter.clear();

		{ // Circle with radius 10 at center
			painter.drawCircle(50, 50, 10);
			CHECK_UNARY(hasAnyDrawnPixels(canvas));
		}

		{ // Circle with radius 0 - nothing drawn
			painter.clear();
			painter.drawCircle(50, 50, 0);
			auto& pixbuf = canvas.getPixelBuffer();
			bool allTransparent = true;
			for(std::size_t x = 0; x < pixbuf.width; ++x)
			{
				for(std::size_t y = 0; y < pixbuf.height; ++y)
				{
					const auto& c = pixbuf.pixel(x, y);
					if(c.red() > 0 || c.green() > 0 || c.blue() > 0)
					{
						allTransparent = false;
					}
				}
			}
			CHECK_UNARY(allTransparent);
		}

		{ // Circle where x == y path
			painter.clear();
			painter.drawCircle(50, 50, 5);
			CHECK_UNARY(hasAnyDrawnPixels(canvas));
		}
	}

	SUBCASE("testDrawFilledCircle")
	{
		TestableCanvas canvas(100, 100);
		dggui::Painter painter(canvas);
		painter.clear();

		{ // Filled circle with radius 10
			painter.drawFilledCircle(50, 50, 10);
			CHECK_UNARY(hasAnyDrawnPixels(canvas));
		}

		{ // Filled circle with radius 0
			painter.clear();
			painter.drawFilledCircle(50, 50, 0);
			CHECK_UNARY(!hasAnyDrawnPixels(canvas));
		}

		{ // Filled circle where x == y path
			painter.clear();
			painter.drawFilledCircle(50, 50, 5);
			CHECK_UNARY(hasAnyDrawnPixels(canvas));
		}
	}

	SUBCASE("testDrawRectangle")
	{
		TestableCanvas canvas(100, 100);
		dggui::Painter painter(canvas);
		painter.clear();

		{ // Rectangle drawn
			painter.drawRectangle(10, 10, 90, 90);
			CHECK_UNARY(hasAnyDrawnPixels(canvas));
		}
	}

	SUBCASE("testDrawFilledRectangle")
	{
		TestableCanvas canvas(100, 100);
		dggui::Painter painter(canvas);
		painter.clear();

		{ // Filled rectangle
			painter.drawFilledRectangle(10, 10, 50, 50);
			CHECK_UNARY(hasAnyDrawnPixels(canvas));
		}
	}

	SUBCASE("testDrawPoint")
	{
		TestableCanvas canvas(100, 100);
		dggui::Painter painter(canvas);
		painter.clear();

		{ // Point inside bounds
			painter.drawPoint(50, 50);
			CHECK_UNARY(hasAnyDrawnPixels(canvas));
		}

		{ // Point outside bounds (negative)
			painter.clear();
			painter.drawPoint(-1, -1);
			CHECK_UNARY(!hasAnyDrawnPixels(canvas));
		}

		{ // Point outside bounds (beyond width)
			painter.clear();
			painter.drawPoint(100, 50);
			CHECK_UNARY(!hasAnyDrawnPixels(canvas));
		}

		{ // Point outside bounds (beyond height)
			painter.clear();
			painter.drawPoint(50, 100);
			CHECK_UNARY(!hasAnyDrawnPixels(canvas));
		}
	}

	SUBCASE("testDrawLine")
	{
		TestableCanvas canvas(100, 100);
		dggui::Painter painter(canvas);
		painter.clear();

		{ // Line drawn
			painter.drawLine(10, 10, 90, 90);
			CHECK_UNARY(hasAnyDrawnPixels(canvas));
		}

		{ // Line with steep slope (steep path)
			painter.clear();
			painter.drawLine(50, 10, 50, 90);
			CHECK_UNARY(hasAnyDrawnPixels(canvas));
		}

		{ // Line with x0 > x1 (swap path)
			painter.clear();
			painter.drawLine(90, 50, 10, 50);
			CHECK_UNARY(hasAnyDrawnPixels(canvas));
		}
	}

	SUBCASE("testDrawRestrictedImage")
	{
		TestImage image(16, 16, false);
		REQUIRE(image.isValid());

		{ // Restricted image with matching colour - should draw
			TestableCanvas canvas(100, 100);
			dggui::Painter painter(canvas);
			painter.clear();
			dggui::Colour restriction(0, 0, 0, 255);
			painter.drawRestrictedImage(0, 0, restriction, image);
			CHECK_UNARY(hasAnyDrawnPixels(canvas));
		}

		{ // Restricted image with non-matching colour - should not draw
			TestableCanvas canvas(100, 100);
			dggui::Painter painter(canvas);
			painter.clear();
			dggui::Colour restriction(255, 255, 255, 255);
			painter.drawRestrictedImage(0, 0, restriction, image);
			auto& pixbuf = canvas.getPixelBuffer();
			bool allTransparent = true;
			for(std::size_t x = 0; x < pixbuf.width; ++x)
			{
				for(std::size_t y = 0; y < pixbuf.height; ++y)
				{
					const auto& c = pixbuf.pixel(x, y);
					if(c.alpha() > 0)
					{
						allTransparent = false;
					}
				}
			}
			CHECK_UNARY(allTransparent);
		}

		{ // Restricted image with clipping
			TestableCanvas canvas(8, 8);
			dggui::Painter painter(canvas);
			painter.clear();
			dggui::Colour restriction(0, 0, 0, 255);
			painter.drawRestrictedImage(-5, -5, restriction, image);
			CHECK_UNARY(hasAnyDrawnPixels(canvas));
		}
	}

	SUBCASE("testDrawImageStretched")
	{
		TestImage image(16, 16, false);
		REQUIRE(image.isValid());

		{ // Normal stretch
			TestableCanvas canvas(50, 50);
			dggui::Painter painter(canvas);
			painter.clear();
			painter.drawImageStretched(0, 0, image, 50, 50);
			CHECK_UNARY(hasAnyDrawnPixels(canvas));
		}

		{ // Stretched with clipping (exceeds buffer)
			TestableCanvas canvas(10, 10);
			dggui::Painter painter(canvas);
			painter.clear();
			painter.drawImageStretched(-5, -5, image, 50, 50);
			CHECK_UNARY(hasAnyDrawnPixels(canvas));
		}

		{ // Stretched with zero width - early return
			TestableCanvas canvas(100, 100);
			dggui::Painter painter(canvas);
			painter.clear();
			painter.drawImageStretched(0, 0, image, 0, 50);
			CHECK_UNARY(!hasAnyDrawnPixels(canvas));
		}

		{ // Stretched with zero height - early return
			TestableCanvas canvas(100, 100);
			dggui::Painter painter(canvas);
			painter.clear();
			painter.drawImageStretched(0, 0, image, 50, 0);
			CHECK_UNARY(!hasAnyDrawnPixels(canvas));
		}
	}

	SUBCASE("testDrawTextRotate")
	{
		dggui::Font font;
		std::string someText = "Hello";

		{ // Text with rotate=true
			TestableCanvas canvas(100, 100);
			dggui::Painter painter(canvas);
			painter.clear();
			painter.drawText(0, 50, font, someText, false, true);
			CHECK_UNARY(hasAnyDrawnPixels(canvas));
		}
	}
}
